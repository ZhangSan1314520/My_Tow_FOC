#pragma once

typedef struct {
    // 实际位置/速度
    float theta;           //电角度
    float theta_zero;        //电角度零位
    float theta_no_offic;        //电机未偏移的原始弧度
    float theta_m;               //电机原始弧度
    float theta_m_speed;        //电机原始角速度(弧度)
    float theta_deg_final;       //最终的电机角度 (度)
    float reg_final;             //最终的电机角度 (弧度)
    float Angular_velocity_final; //最终的角速度
    
    float Now_Id;//电机实际电流id
    float Now_Iq;//电机实际电流iq

    float u_d; // 输出d轴电压 为0最大扭矩
    float u_q; // 输出q轴电压

    // 目标位置/速度
    float _target_location2;     //目标位置 (度)
    float _target_speed;         //目标速度
    float _target_Id;
    float _target_Iq;

    //ABC相占空比
    float motor_duty_a;       //电机占空比
    float motor_duty_b;
    float motor_duty_c;

    // ABC三相电流
    float _IA; 
    float _IB;
    float _IC;

    float Zero_IA; //电机三相电流零点偏移
    float Zero_IB;
    float Zero_IC;

    float Zero_IA_avg; //电机三相电流零点偏移平均值
    float Zero_IB_avg;
    float Zero_IC_avg;


    //laji
    float theta_m_offic; //

} motor_telem_t;                  // 每路电机一个结构体

#define UORB_TOPIC_MOTOR "motor_telem"


