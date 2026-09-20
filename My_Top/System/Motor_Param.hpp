#pragma once


struct LQR_Param //LQR参数
{
    float k1;
    float k2;

    float max_speed;
    float min_speed;
};


struct PID_Param //PID参数
{
    float kp;
    float ki;
    float kd;
};


struct Motor_Limit_Param //电机限制参数
{
    float max_duty;
    float min_duty;

    float max_speed;
    float min_speed;

    float i_ratio;//积分系数
};



struct Motor_Param
{
    LQR_Param lqr; //LQR参数
    PID_Param speed_pid;//速度PID参数
    PID_Param location_pid;//位置PID参数
    Motor_Limit_Param limit;//电机限制参数
};



// 默认LQR参数
constexpr Motor_Param MOTOR_DEFAULT_PARAM = //常量表达式
{
    // LQR //串级控制参数
    // {
    //     5.5f,
    //     0.9f,
    //     1.4f, //电机速度极限
    //     -1.4f
    // },

    // LQR //直接控制参数
    {
        3.5f,
        0.1f,
        0.97f,
        -0.97f
    },

    // 速度环PID
    {
        0.62f,
        25.0f,
        0.1f
    },


    // 位置PID（备用）
    {
        0.6f,
        0.0f,
        0.0f
    },


    // 限制参数
    {
        0.97f, // 电机占空比极限
        -0.97f,

        1.8f, //电机速度环极限
        -1.8f,

        0.6f //积分系数
    }
};

