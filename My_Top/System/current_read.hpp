#pragma once // 保证头文件只被编译一次，防止头文件被重复引用
#include "adc.h"
#include "bsp.hpp"

#define MOTOR_SHUNT_RES 0.005f    // 采样电阻
#define MOTOR_CURR_AMP_GAIN 40.0f // 电流增益
#define MOTOR_CURR_OFFSET_DEFAULT (1.58f / MOTOR_SHUNT_RES / MOTOR_CURR_AMP_GAIN)
#define MOTOR_ADC_RAW_TO_CURR_GAIN (1.0f / 4096.0f * 3.3f / MOTOR_SHUNT_RES / MOTOR_CURR_AMP_GAIN)

#define MOTOR_VM_GAIN (1.0f/4095.0f*3.3f*(1.0f/(7.5f/(82+7.5f)))) // 母线电压增益

class PhaseCurrent
{
public:
    float IA; //无低通滤波的电流
    float IB;
    float IC;
    float VM; //母线电压
    float IA_filtered; //低通滤波的电流
    float IB_filtered;
    float IC_filtered;
  
    float Zero_IA;//0点电流
    float Zero_IB;
    float Zero_IC;

    float Zero_IA_avg;//0点电流平均值
    float Zero_IB_avg;
    float Zero_IC_avg;    

    PhaseCurrent(const MotorPWM_Config *config): motor_config(config) {}
    void ADC_Injected_Init(); //ADC注入初始化
    void Update_Current(); //更新电流
    void Current_Calibration();//电流校准

private:
    const MotorPWM_Config *motor_config;
    LowpassFilter filter_ia; // 电流低通滤波器
    LowpassFilter filter_ib; // 电流低通滤波器
    LowpassFilter filter_ic; // 电流低通滤波器
    AvgFilter avg_filter_ia; // 平均值滤波器
    AvgFilter avg_filter_ib; // 平均值滤波器
    AvgFilter avg_filter_ic; // 平均值滤波器


    uint32_t Primitive_UA;
    uint32_t Primitive_UB;
    uint32_t Primitive_UC;
    
    float Primitive_IA;   
    float Primitive_IB;
    float Primitive_IC;

    float Zero_Offset_IA;
    float Zero_Offset_IB;
    float Zero_Offset_IC;
};