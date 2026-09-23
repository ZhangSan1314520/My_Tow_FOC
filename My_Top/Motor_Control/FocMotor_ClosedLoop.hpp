#pragma once
#include "pid.hpp"
#include "pid_Increment.hpp"
#include "pll.hpp"
#include "foc_math.hpp"
#include "LQR.hpp"
#include "current_read.hpp"
#include "encoder.hpp"
#include "ex_math.hpp"
#include "bsp.hpp"
#include "my_dronecan.hpp"
#include "wave_generator.hpp"


#define My_PWM_DUTY_MAX 0.8 //电机PWM占空比最大值
#define Motor_ExpNum 21 //电机极对数
#define SingleTurn_Theta (M_2PI/(float)Motor_ExpNum) // 单圈对应的电角度(单位度)

enum Work_Mode
{
    null_mode = 0,        //无模式
    open_loop = 1,//开环模式
    Ia_loop = 2,//电流环模式
    speed_loop = 3,//速度环模式
    position_loop = 4,//位置环模式

    EncoderCalibration = 90,//编码器校准模式
    CurrentCalibration = 91,//电流零点校准模式
};

enum AngleEstimatorMode
{
    LPF = 0, //低通滤波
    PLL = 1, //PLL锁相环
    LPF_PLL = 2 //低通滤波+PLL锁相环
};




class FOC_Motor
{
public:
    FOC_Motor(const MotorPWM_Config *motor_config, Encoder *encoder,PhaseCurrent *phase_current,
            PID *pid_iq ,PID *pid_id,PID *pid_spd ,PID *pid_loc,LQR *lqr)
            : _motor_config(motor_config), _encoder(encoder), _phase_current(phase_current),
            _pid_iq(pid_iq), _pid_id(pid_id), _pid_spd(pid_spd),_pid_loc(pid_loc), _lqr(lqr)
            {}//构造函数
    float u_d; // d轴电压 为0最大扭矩  
    float u_q; // q轴电压 

    float _sine; // 存储sin(theta)的值
    float _cosine; // 存储cos(theta)的值
    float _IA; //电机三相电流大小
    float _IB; //电机三相电流大小
    float _IC; //电机三相电流大小
    float _VM; //电机母线电压


    Work_Mode work_mode = null_mode; //电机工作模式
    Work_Mode work_mode_last = null_mode; //电机上一次工作模式
    bool motor_encoder_dir = true; //编码器方向，true为正，false为反
    bool control_init_flag = false; //电机控制初始化标志
    float theta_zero = 0.0f; //电机零点角度
    float theta = 0.0f; //电角度  
    float theta_no_offic = 0.0f; //电机未偏移的原始弧度
    float theta_m; //电机原始弧度 偏移过的  
    float theta_m_speed; // 电机原始速度
    float filtered_speed;// 速度滤波后的值
    float reg_final;//最终的电机角度 (弧度) 
    float theta_deg_final ; //最终的电机角度 (角度)
    float Angular_velocity_final;  //最终的角速度
    float zero_offset;//电机零点偏移角度 (角度)  
    float Open_i = 0.0f; //开环模式下的角度
    float Now_Id; //电机实际电流id
    float Now_Iq; //电机实际电流iq
    float _target_Id; //电机目标电流id
    float _target_Iq; //电机目标电流iq
    float _target_speed = 0.0f;//电机速度目标值
    float _target_location2 = 0.0f;//电机目标位置 (-180°~+180°)

    float wave_mode = 0;   // 0=关闭, 1=方波, 2=正弦波
    uint8_t _last_wave_mode = 0; // 上一次波形模式
    uint16_t Calibration_Cnt = 0; //校准计数
    void set_wave_mode(uint8_t mode, uint16_t period_ms, float gain, float offset = 0.0f);


    Encoder   *_encoder; //编码器
    const MotorPWM_Config *_motor_config;//电机PWM和编码器配置
    PhaseCurrent *_phase_current; //电流读取
    PID * _pid_id; //电流环PID
    PID * _pid_iq; //电流环PID
    PID * _pid_spd; //速度环
    PID * _pid_loc; //位置环
    LQR * _lqr; //LQR控制式

    void setPwm();//设置三相电机PWM 
    void My_FOC_Motor_Reset();
    void My_FOC_Motor_Init(void);
    void Motor_EN(bool en) ;//电机使能
    void Update_FOC_Current(void); //更新电机电流
    void Update_Speed_Angle_LPFAndPLL(); //更新速度
    void Encoder_Calibration(void); //编码器校准 获取0点角度
    void FOC_Open_Loop1(); //开环模式
    void FOC_Open_Loop2(); //开环模式
    void FOC_Current_Loop(void); //电流环
    void FOC_Speed_Loop(void); //速度环
    void FOC_Location_Loop(void); //位置环
    void Current_Auto_Calibration(bool x); //电流自动校准使能
    int32_t laji; //用于记录中断间隔时间
    float motor_duty_a; // A相占空比(0-1) 
    float motor_duty_b; // B相占空比(0-1) 
    float motor_duty_c; // C相占空比(0-1) 
private:
    bool _motor_is_on_last = false; //上次电机使能标志
    float target_speed_last = 0.0f; //上次目标速度
    uint8_t wave_mode_last = 0xff; // 上一次波形模式

    uint32_t timer_clock_freq_; // 定时器的实际计数时钟频率
    LowpassFilter speed_lpf;     // 速度低通滤波器
    AvgFilter speed_avg;     // 速度平均值滤波器
    
    WaveGenerator _wave_gen; // 波形发生器实例

    AngleEstimatorMode Angle_Mode = PLL; //0 低通滤波 1 是PLL锁相环 2是低通滤波+PLL锁相环
    float theta_m_last; //电机上一次弧度
    float theta_m_offic;//角度差
    float theta_av_speed;// 电机平均速度
    

    float _pll_reg_out = 0.0f; //PLL锁相环的输出角度
    float _pll_Angular_velocity = 0.0f; //PLL锁相环的输出角速度
    PLL_Parameter _pll_conf{FOC_PLL_KP, FOC_PLL_KI}; //PLL锁相环参数
    FOC_Transform foc_math; //foc变换公式
    
    uint32_t Time_ARR; //定时器ARR值
    // float motor_duty_a; // A相占空比(0-1) 
    // float motor_duty_b; // B相占空比(0-1) 
    // float motor_duty_c; // C相占空比(0-1) 
    
};

extern FOC_Motor M1;
extern FOC_Motor M2;


