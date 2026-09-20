
#pragma once

#include "usart.h"
#include "tim.h"
#include "gpio.h"
#include "i2c.h"  
#include "adc.h"  
#include "ex_math.hpp"  
#include "fdcan.h"

#define Calibration_DIV 160  //校准控制频率
#define FOC_VELOCITY_UP_FREQ_DIV 4  //速度更新频率
#define FOC_Open_Loop_FREQ_DIV 16  //开环控制 
#define FOC_VELOCITY_LOOP_FREQ_DIV 4  //速度环 
#define FOC_POSITION_LOOP_FREQ_DIV 16 //位置环
#define FOC_CURRRENT_LOOP_FREQ_DIV 1 //电流环
#define FOC_MAIN_LOOP_FREQ_HZ 16000.0
#define FOC_VELOCITY_UP_FREQ_HZ (FOC_MAIN_LOOP_FREQ_HZ / FOC_VELOCITY_UP_FREQ_DIV)
#define FOC_VELOCITY_LOOP_FREQ_HZ (FOC_MAIN_LOOP_FREQ_HZ / FOC_VELOCITY_LOOP_FREQ_DIV)
#define FOC_POSITION_LOOP_FREQ_HZ (FOC_MAIN_LOOP_FREQ_HZ / FOC_POSITION_LOOP_FREQ_DIV)
#define FOC_CURRRENT_LOOP_FREQ_HZ (FOC_MAIN_LOOP_FREQ_HZ / FOC_CURRRENT_LOOP_FREQ_DIV)
#define FOC_Open_Loop_FREQ_HZ (FOC_MAIN_LOOP_FREQ_HZ / FOC_Open_Loop_FREQ_DIV)
#define CALIBRATION_FREQ_HZ (FOC_MAIN_LOOP_FREQ_HZ / Calibration_DIV)


#define closed_loop_htim (&htim1)   //闭环控制定时器
#define System_htim (&htim5)    //系统计时定时器
#define Vofa_huart (&huart3)    //Vofa串口
#define CLI_huart (&huart1)     //CLI打印输出

#define Encoder1_huart (&huart2) //编码器ENC1串口
#define Encoder2_huart (&huart4) //编码器ENC2串口

#define EEPROM_hi2c       (&hi2c3)      // FM24V02A0 EEPROM I2C句柄
#define FD_CAN1       (&hfdcan1)      // FDCAN 句柄

#define VM_adc (&hadc2)   //母线电压adc句柄

// ============================================================
// 电机 PWM 配置结构体
// ============================================================
struct MotorPWM_Config {
    TIM_HandleTypeDef*  htim;        // HAL 定时器句柄（用于 HAL 函数调用）
    TIM_TypeDef*        Tim_base;    // TIM 大写
    uint32_t            ch_hin1;     // PWM 输出通道
    uint32_t            ch_hin2;     // PWM 输出通道
    uint32_t            ch_hin3;     // PWM 输出通道
    UART_HandleTypeDef *huart;        //编码器串口句柄
    ADC_HandleTypeDef*  hadc;          // ADC句柄
    GPIO_TypeDef *    Motor_EN_PORT;     // 电机驱动使能端口
    uint16_t          Motor_EN_PIN;    // 电机驱动使能引脚
    GPIO_TypeDef *    Motor_CAL_EN_PORT;     // 电机校准使能端口
    uint16_t          Motor_CAL_EN_PIN;    // 电机校准使能引脚

};

extern const MotorPWM_Config M1_PWM;
extern const MotorPWM_Config M2_PWM;





