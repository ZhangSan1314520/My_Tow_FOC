#include "my_tim.hpp"

My_Tim* My_Tim::_list[4] = {nullptr};
uint8_t My_Tim::_num = 0;

// 全局实例：M1 绑 TIM20（&M1 地址编译期确定，构造只存指针不访问内容，跨文件顺序安全）
My_Tim M1_TIM(&M1);
My_Tim M2_TIM(&M2);

void My_Tim::init()
{
    uint16_t ARR = __HAL_TIM_GET_AUTORELOAD(_motor->_motor_config->htim);// 获取定时器ARR寄存器的值
    __HAL_TIM_SET_COMPARE(_motor->_motor_config->htim, TIM_CHANNEL_4, ARR*0.84f); // 设置占空比为84% 控制My_Tim_OC_Callback回调函数的触发时机
    __HAL_TIM_SET_COMPARE(_motor->_motor_config->htim, TIM_CHANNEL_5, ARR*0.80f); // 设置占空比为80% 控制电流采样触发时机 和回调函数无关
    _motor->_motor_config->htim->PeriodElapsedCallback = My_Tim_Callback;  // 设置定时器周期结束回调函数
    _motor->_motor_config->htim->OC_DelayElapsedCallback = My_Tim_OC_Callback;// PWM的模式的比较匹配回调函数
    HAL_TIM_OC_Start_IT(_motor->_motor_config->htim, TIM_CHANNEL_5); //启动定时器比较中断
    HAL_TIM_Base_Start_IT(_motor->_motor_config->htim); //启动定时器周期中断和启动定时器计数
    __HAL_TIM_ENABLE_IT(_motor->_motor_config->htim, TIM_IT_CC4); //只打开中断，不启动定时器
    __HAL_TIM_ENABLE(_motor->_motor_config->htim); // 启动定时器
    HAL_TIM_PWM_Start(_motor->_motor_config->htim, TIM_CHANNEL_5);// 启动 TIM1_CH5 用于产生ADC触发事件 ????
}

// ---- 静态桥接：按 htim 找到实例，转发 ----
void My_Tim::My_Tim_Callback(TIM_HandleTypeDef *htim)
{
    for (uint8_t i = 0; i < _num; i++)//判断是哪个回调函数然后去转发给对应的实例
        if (_list[i] != nullptr && _list[i]->_motor->_motor_config->htim == htim)
        { _list[i]->Tim_Callback(); return; }
}

void My_Tim::My_Tim_OC_Callback(TIM_HandleTypeDef *htim)
{
    for (uint8_t i = 0; i < _num; i++)//判断是哪个回调函数然后去转发给对应的实例
        if (_list[i] != nullptr && _list[i]->_motor->_motor_config->htim == htim)
        { _list[i]->OC_Callback(); return; }
}



// ---- 真正的周期处理：原 My_Tim_Callback 内容，数组循环换成 _motor ----
void My_Tim::Tim_Callback()
{
    TIM_HandleTypeDef* htim = _motor->_motor_config->htim; // 定时器句柄
    FOC_Motor* m = _motor; // 指向当前电机句柄
    if ((htim->Instance->CR1 & 0x10) != 0x00U) return; // 非向上计数不执行
    _count = (_count % 16000) + 1;


    if (!m->control_init_flag) return;

    if (_count % FOC_VELOCITY_UP_FREQ_DIV == 0)
    {
        m->Update_Speed_Angle_LPFAndPLL();
    }

    switch (m->work_mode) 
    { 
        case open_loop: // 开环控制
            if (_count % FOC_Open_Loop_FREQ_DIV == 0) 
            {
                // start_time = HAL_System::get_tick_us(); //记录开始时间
                m->FOC_Open_Loop2(); //耗时8us
                // end_time = HAL_System::get_tick_us(); // 记录结束时间
                // m->laji = (float)(end_time - start_time);  // 计算时间差
            }
            break;

        case speed_loop: // 速度控制
            if (_count % FOC_VELOCITY_LOOP_FREQ_DIV == 0)
            {
                // start_time = HAL_System::get_tick_us(); //记录开始时间
                m->FOC_Speed_Loop();//耗时
                // end_time = HAL_System::get_tick_us(); // 记录结束时间
                // m->laji = (float)(end_time - start_time);  // 计算时间差
            }
            break;

        case position_loop: // 位置控制 
            if (_count % FOC_POSITION_LOOP_FREQ_DIV == 0)
            {
                // start_time = HAL_System::get_tick_us(); //记录开始时间
                m->FOC_Location_Loop();//耗时
                // end_time = HAL_System::get_tick_us(); // 记录结束时间
                // m->laji = (float)(end_time - start_time);  // 计算时间差
            }
            if (_count % FOC_VELOCITY_LOOP_FREQ_DIV == 0)
            {
                m->FOC_Speed_Loop();//耗时
            }
            break;

        case EncoderCalibration: //编码器校准模式（求电角度零点）
            if (_count % Calibration_DIV == 0)
            {
                m->Encoder_Calibration();
            }
            break;

    }
}

// ---- 真正的 OC 处理
uint16_t laji1 = 0;
uint16_t laji2 = 0;
uint16_t laji3 = 0;
void My_Tim::OC_Callback()
{
    _oc_count = (_oc_count % 16000) + 1;

    FOC_Motor* m = _motor;

    if (!m->control_init_flag) return;

    if (_oc_count % FOC_CURRRENT_LOOP_FREQ_DIV == 0)
    {
        m->Update_FOC_Current();
        if (m->work_mode == Ia_loop || m->work_mode == speed_loop || m->work_mode == position_loop)
        {
            // uint32_t start_time = HAL_System::get_tick_us();
            m->FOC_Current_Loop();
            // m->laji = (float)(HAL_System::get_tick_us() - start_time);
        }
    }
    if ( m->work_mode == CurrentCalibration && _oc_count % Calibration_DIV == 0)
    {
        
        if(m->Calibration_Cnt < ((uint16_t)(CALIBRATION_FREQ_HZ/10.0f+0.5f)+1))//100ms内
        {
            m->Current_Auto_Calibration(true);//保证100us以上
        }else
        {
            m->Current_Auto_Calibration(false);
            m->_phase_current->Current_Calibration();
        }
        
        if(m->Calibration_Cnt<1000) m->Calibration_Cnt++;
    }
}
