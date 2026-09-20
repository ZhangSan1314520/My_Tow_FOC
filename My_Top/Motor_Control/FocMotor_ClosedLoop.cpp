#include "FocMotor_ClosedLoop.hpp"

// 电机1相关对象
Encoder encoder1(&M1_PWM); //编码器对象
PhaseCurrent phaseCurrent1(&M1_PWM); //电流对象

PID pid_iq_M1(1.1f,120.0f,0.0f,1.0f/FOC_CURRRENT_LOOP_FREQ_HZ,0.8,1.01,-1.01); //Iq环pid
PID pid_id_M1(1.1f,120.0f,0.0f,1.0f/FOC_CURRRENT_LOOP_FREQ_HZ,0.8,1.01,-1.01); //Id环pid
PID pid_spd_M1(0.0f,0.0f,0.0f,1.0f/FOC_VELOCITY_LOOP_FREQ_HZ,0.8,1,-1); //速度环pid
PID pid_loc_M1(0.0f,0.0f,0.0f,1.0f/FOC_POSITION_LOOP_FREQ_HZ,0.8,10,-10); //位置环pid
LQR lqr_M1(0.0f,0.0f,10,-10); //位置环lqr

FOC_Motor M1(&M1_PWM, &encoder1,&phaseCurrent1,&pid_iq_M1,&pid_id_M1, &pid_spd_M1, &pid_loc_M1,&lqr_M1); //电机1



// 电机2相关对象
Encoder encoder2(&M2_PWM); //编码器对象
PhaseCurrent phaseCurrent2(&M2_PWM); //电流对象

PID pid_iq_M2(1.1f,120.0f,0.0f,1.0f/FOC_CURRRENT_LOOP_FREQ_HZ,0.8,1.01,-1.01); //Iq环pid
PID pid_id_M2(1.1f,120.0f,0.0f,1.0f/FOC_CURRRENT_LOOP_FREQ_HZ,0.8,1.01,-1.01); //Id环pid
PID pid_spd_M2(0.0f,0.0f,0.0f,1.0f/FOC_VELOCITY_LOOP_FREQ_HZ,0.8,1,-1); //速度环pid
PID pid_loc_M2(0.0f,0.0f,0.0f,1.0f/FOC_POSITION_LOOP_FREQ_HZ,0.8,10,-10); //位置环pid
LQR lqr_M2(0.0f,0.0f,10,-10); //位置环lqr 

FOC_Motor M2(&M2_PWM, &encoder2,&phaseCurrent2,&pid_iq_M2,&pid_id_M2, &pid_spd_M2, &pid_loc_M2,&lqr_M2); //电机2



void FOC_Motor::My_FOC_Motor_Init()
{
    uint32_t sysclk = HAL_RCC_GetSysClockFreq();
    timer_clock_freq_ = sysclk / (_motor_config->htim->Instance->PSC + 1);
    Time_ARR = __HAL_TIM_GetAutoreload(_motor_config->htim)+1; // 获取定时器自动重载值，即PWM周期
    
    __HAL_TIM_SET_COMPARE(_motor_config->htim, _motor_config->ch_hin1, (uint32_t)0); // 设置占空比为0
    __HAL_TIM_SET_COMPARE(_motor_config->htim, _motor_config->ch_hin2, (uint32_t)0);
    __HAL_TIM_SET_COMPARE(_motor_config->htim, _motor_config->ch_hin3, (uint32_t)0);
    
    HAL_GPIO_WritePin(_motor_config->Motor_EN_PORT, _motor_config->Motor_EN_PIN, GPIO_PIN_SET); // 使能驱动器
    HAL_TIM_PWM_Start(_motor_config->htim, _motor_config->ch_hin1);
    HAL_TIM_PWM_Start(_motor_config->htim, _motor_config->ch_hin2);
    HAL_TIM_PWM_Start(_motor_config->htim, _motor_config->ch_hin3);
    HAL_TIMEx_PWMN_Start(_motor_config->htim, _motor_config->ch_hin1); // 启动PWM互补通道
    HAL_TIMEx_PWMN_Start(_motor_config->htim, _motor_config->ch_hin2);
    HAL_TIMEx_PWMN_Start(_motor_config->htim, _motor_config->ch_hin3);
    __HAL_TIM_ENABLE(_motor_config->htim); // 使能定时器外设
    theta_zero = 5.36f; //电机零点角度
    // _phase_current->Zero_IA_avg = 7.816f;
    // _phase_current->Zero_IB_avg = 7.655f;
    // _phase_current->Zero_IC_avg = 7.850f;

    _phase_current->Zero_IA_avg = 7.98f;
    _phase_current->Zero_IB_avg = 7.925f;
    _phase_current->Zero_IC_avg = 7.925f;

    speed_lpf.init(0.7); //速度低通滤波
    error_lpf.init(0.7); //误差低通滤波
    speed_avg.init(5); //速度均值滤波

    _encoder->encoder_init(); //编码器初始化
    _phase_current->ADC_Injected_Init(); //电流初始化
    set_wave_mode(0, 0, 0.0f, 0.0f);// 关闭波形，恢复正常控制
    control_init_flag = true;
}






void FOC_Motor::setPwm()
{

    uint32_t ccr1, ccr2, ccr3;// pwm的实际占空比值
    motor_duty_a = constraint_value(motor_duty_a, 0.0f, 0.92f); // 限制占空比上限
    motor_duty_b = constraint_value(motor_duty_b, 0.0f, 0.92f);
    motor_duty_c = constraint_value(motor_duty_c, 0.0f, 0.92f);
    if(Time_ARR==0) return;

    float coeff = (Time_ARR) * My_PWM_DUTY_MAX; // 限制pwm周期为原来的0.8倍
    ccr1 = (uint32_t)(motor_duty_a * coeff);
    ccr2 = (uint32_t)(motor_duty_b * coeff);
    ccr3 = (uint32_t)(motor_duty_c * coeff);

    __HAL_TIM_SET_COMPARE(_motor_config->htim, _motor_config->ch_hin1, ccr2);//顺序BAC
    __HAL_TIM_SET_COMPARE(_motor_config->htim, _motor_config->ch_hin2, ccr1);
    __HAL_TIM_SET_COMPARE(_motor_config->htim, _motor_config->ch_hin3, ccr3); // 设置占空比
    

}

void FOC_Motor::Current_Auto_Calibration(bool x) //电流自动校准使能
{
    HAL_GPIO_WritePin(_motor_config->Motor_CAL_EN_PORT, _motor_config->Motor_CAL_EN_PIN, (x ? GPIO_PIN_SET : GPIO_PIN_RESET)); // 使能电流校准
}



void FOC_Motor::Encoder_Calibration() //编码器校准
{
    float theta_temp = 0.0f;
    u_q = 0.0f;
    u_d = 0.3f;
    _sine = 0.0f;
    _cosine = 1.0f;
    foc_math.IPark(u_d,u_q,_sine,_cosine);
    foc_math.SVPWM();
    motor_duty_a = foc_math.duty_a; // 占空比
    motor_duty_b = foc_math.duty_b;
    motor_duty_c = foc_math.duty_c;
    setPwm(); // 设置占空比

    theta_temp = fmodf(theta_m, SingleTurn_Theta);// 计算当前机械角度在单圈范围内的值 (度)
    theta_temp = (theta_temp < 0) ? (theta_temp + SingleTurn_Theta) : theta_temp; // 保持机械零点角度为正值
    theta_zero = theta_temp * Motor_ExpNum;  //取最后一次的值
}


void FOC_Motor::Motor_EN(bool en)
{
    if (en == _motor_is_on_last) return;  // 状态没变，跳过
    _motor_is_on_last = en;
    if (en)
    {
        // 重新使能 PWM 输出通道
        _motor_config->Tim_base->CCER |= (TIM_CCER_CC1E | TIM_CCER_CC1NE); // 开 CH1 + CH1N
        _motor_config->Tim_base->CCER |= (TIM_CCER_CC2E | TIM_CCER_CC2NE); // 开 CH2 + CH2N
        _motor_config->Tim_base->CCER |= (TIM_CCER_CC3E | TIM_CCER_CC3NE); // 开 CH3 + CH3N
    }else
    {
        // 只关输出（置0），不关定时器
        _motor_config->Tim_base->CCER &= ~(TIM_CCER_CC1E | TIM_CCER_CC1NE); // 关 CH1 + CH1N
        _motor_config->Tim_base->CCER &= ~(TIM_CCER_CC2E | TIM_CCER_CC2NE); // 关 CH2 + CH2N
        _motor_config->Tim_base->CCER &= ~(TIM_CCER_CC3E | TIM_CCER_CC3NE); // 关 CH3 + CH3N
    }
}

void FOC_Motor::My_FOC_Motor_Reset()
{

    Calibration_Cnt = 0;//复位校准计数
    _target_location2 = rad2deg(reg_final); //复位位置为当前位置
    _target_speed = 0.0;
    _pid_id->reset(); //复位pid
    _pid_iq->reset(); //复位pid
    _pid_spd->reset(); //复位pid
    _pid_loc->reset(); //复位pid
    _lqr->reset(); //复位lqr 
}


void FOC_Motor::Update_FOC_Current() //更新三相电流
{
    _phase_current->Update_Current(); //更新三相电流
    _IA = _phase_current->IA_filtered;
    _IB = _phase_current->IB_filtered;
    _IC = _phase_current->IC_filtered;
    _VM = _phase_current->VM;
}



void FOC_Motor::Update_Speed_Angle_LPFAndPLL() //更新速度和角度
{
    // 静态变量，用于临时存储角度值和上一次的角度值
    static float fudu_test = 0.0f;
    static float theta_m_offic_temp = 0.0;
    float theta_temp = 0.0;

    _encoder->encoder_request_pos();//发送角度指令
    fudu_test = _encoder->Get_Encoder_Radian();//获取编码器角度（弧度）
    theta_no_offic = fudu_test;//未偏移的真实角度
    theta_m = wrap_to_PI(fudu_test-zero_offset);//将角度规整到 [-PI, PI) 区间
    theta_temp = theta_m * Motor_ExpNum - theta_zero;
    theta = wrap_to_PI(theta_temp);//获取电角度 

    if (LPFAndPLL == false) //低通滤波角度
    {
      //方式1 低通滤波 更新机械角度速度
      theta_m_offic_temp = theta_m - theta_m_last; 
      theta_m_last = theta_m; // 更新上一次的机械角度
      theta_m_offic = wrap_to_PI(theta_m_offic_temp);// 将差值规整到 [-PI, PI) 区间

      theta_m_offic_filtered = error_lpf.filter(theta_m_offic);//对误差角度低通滤波
      theta_m_speed = theta_m_offic_filtered*FOC_VELOCITY_UP_FREQ_HZ;//speed=路程/t 
      theta_av_speed = speed_avg.filter(theta_m_speed);//对速度进行平均值滤波
      filtered_speed = speed_lpf.filter(theta_av_speed);//对速度进行低通滤波
      reg_final = theta_m; //将机械角度赋值给最终角度
      theta_deg_final = rad2deg(reg_final); //将弧度转换为360度
      Angular_velocity_final = filtered_speed;  //将最终速度赋值给最终角速度
      
    }else if (LPFAndPLL == true)
    {
      // 方式2PLL锁相环 更新PLL锁相环的输出角度和角速度
      foc_pll_run(theta_m,PLL_FREQ_Dt,&_pll_reg_out,&_pll_Angular_velocity,&_pll_conf); //更新PLL锁相环的输出角度和角速度
      reg_final = _pll_reg_out; //将PLL锁相环的输出角度赋值给最终角度
      theta_deg_final = rad2deg(reg_final); //将弧度转换为360度
      Angular_velocity_final = _pll_Angular_velocity; //将最终速度赋值给最终角速度
    }

}



void FOC_Motor::FOC_Open_Loop1()
{
    u_d = 0.0f;
    u_q = 0.3f;
    Open_i += 0.01f;
    if (Open_i >= 6.283185)
        Open_i = 0.0f;

    fast_sin_cos(Open_i, &_sine, &_cosine); // 获取正弦和余弦值
    foc_math.IPark(u_d,u_q,_sine,_cosine);
    foc_math.SVPWM();
    motor_duty_a = foc_math.duty_a; // 占空比
    motor_duty_b = foc_math.duty_b;
    motor_duty_c = foc_math.duty_c;
    setPwm(); // 设置占空比
}



void FOC_Motor::FOC_Open_Loop2()
{
    u_d = 0.0f;
    u_q = 0.30f;
    fast_sin_cos(theta, &_sine, &_cosine); // 获取正弦和余弦值
    foc_math.IPark(u_d,u_q,_sine,_cosine);
    foc_math.SVPWM();
    motor_duty_a = foc_math.duty_a; // 占空比
    motor_duty_b = foc_math.duty_b; 
    motor_duty_c = foc_math.duty_c; 
    setPwm(); // 设置占空比 
}



void FOC_Motor::FOC_Current_Loop()// 电流闭环
{
    float _error_Id = 0.0f;
    float _error_Iq = 0.0f;

    uint8_t wave_mode_temp = 0;
    wave_mode_temp = (uint8_t)wave_mode;
    if (work_mode == Ia_loop)
    {
        if (wave_mode_temp == 1 || wave_mode_temp == 2)
        {
            set_wave_mode(wave_mode_temp, FOC_CURRRENT_LOOP_FREQ_HZ, 1.0f, 0.0f);
            // _target_Iq = _wave_gen.next(1);
            _target_Id = _wave_gen.next(1);
        }
        else if (wave_mode_temp == 0 && (_last_wave_mode == 1 || _last_wave_mode == 2))
        {
            _target_Iq = 0.0f; 
            _target_Id = 0.0f; 
        }

        _last_wave_mode = wave_mode_temp; // 更新上一次的波形模式
    }

    fast_sin_cos(theta, &_sine, &_cosine);
    foc_math.Clarke(_IA,_IB); // ② Clarke 变换
    foc_math.Park(_sine,_cosine);   // ③ Park 变换
    Now_Iq = foc_math.i_q; 
    Now_Id = foc_math.i_d;
    _error_Iq = _target_Iq - Now_Iq; // 1.用给定的_target_Iq 输入给电流环 执行电流闭环
    _error_Id = _target_Id - Now_Id;

    u_q = _pid_iq->update(_error_Iq);
    u_d = _pid_id->update(_error_Id);
    
    

    foc_math.IPark(u_d,u_q,_sine,_cosine);
    foc_math.SVPWM();
    motor_duty_a = foc_math.duty_a; // 占空比
    motor_duty_b = foc_math.duty_b;
    motor_duty_c = foc_math.duty_c;
    setPwm(); // 设置占空比
}



void FOC_Motor::FOC_Speed_Loop(void) //速度环
{
    float _error_speed = 0.0f;

    uint8_t wave_mode_temp = 0;
    wave_mode_temp = (uint8_t)wave_mode;
    
    if( (work_mode == speed_loop) && (wave_mode_temp==1 ||wave_mode_temp ==2) )
    {
        set_wave_mode(wave_mode_temp, FOC_VELOCITY_LOOP_FREQ_HZ, 5.0f, 0.0f);
        _target_speed = _wave_gen.next(1);//cnt计数每次加1
    }

    _error_speed = _target_speed - Angular_velocity_final;
    _target_Id = 0; //用速度PID求出 _target_Iq 输入给电流环 执行速度闭环
    _target_Iq = _pid_spd->update(_error_speed);
}


void FOC_Motor::FOC_Location_Loop(void) //位置环
{
    float error_loc = 0.0f;

    uint8_t wave_mode_temp = 0;
    wave_mode_temp = (uint8_t)wave_mode;
    
    if( (work_mode == position_loop) && (wave_mode_temp==1 ||wave_mode_temp ==2)  )
    {
        set_wave_mode(wave_mode_temp, FOC_POSITION_LOOP_FREQ_HZ, 90.0f, 20.0f);
        _target_location2 = _wave_gen.next(1);//cnt计数每次加1
    }

    error_loc = wrap_to_PI(deg2rad(_target_location2) - reg_final); //计算位置误差
    _target_speed = _pid_loc->update(error_loc); //传入实际误差值
}



void FOC_Motor::set_wave_mode(uint8_t mode, uint16_t period_ms, float gain, float offset)
{
    if (mode == wave_mode_last && (work_mode == work_mode_last)) 
    {
        return; // 如果波形模式和工作模式没有变化，则直接返回
    }
    wave_mode_last = mode;
    work_mode_last = work_mode;

    
    switch (mode) {
    case 0:
        _wave_gen.change_wave_type(WaveType::NONE);
        break;
        
    case 1: // 方波公式y=(+-)_gain + _offset  前半周期输出负幅值，后半周期输出正幅值
        _wave_gen.change_wave_type(WaveType::SQUARE);
        _wave_gen.set_period_tick(period_ms);//设置周期计数最大值
        _wave_gen.set_gain(gain);//设置方波 _gain值
        _wave_gen.set_offset(offset);//设置偏移量offset
        _wave_gen.reset_tick();//清零计数值
        break;
        
    case 2: // 正弦波公式：y = A*sin(wt)+offset
        _wave_gen.change_wave_type(WaveType::SINE);
        _wave_gen.set_period_tick(period_ms);//设置周期计数最大值
        _wave_gen.set_gain(gain);//设置增益A
        _wave_gen.set_offset(offset);//设置偏移量offset
        _wave_gen.reset_tick();//清零计数值
        break;
        
    default:
        _wave_gen.change_wave_type(WaveType::NONE);
        break;
    }
}



