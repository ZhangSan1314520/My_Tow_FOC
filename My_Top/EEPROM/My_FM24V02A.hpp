#pragma once

#include "i2c.h"
#include "string.h"
#include "bsp.hpp"

#define FM24V02A_ADDRESS 0xA0
#define FM24V02A_ADDRESS_MAX 0x7FFF //最大地址为32767
#define FM24V02A_ID 0xF7 // 0xF7 ~ 0xF9 器件 ID 寄存器只读
#define FM24V02A_flag_data 0x65 //写入过标志位
#define FM24V02A_StartAdd 0x00 //写入开始地址

struct FOC_Motor_Params
{

    // 零位偏移	0°（磁编码器原始零位），掉电写入 FRAM 持久化
    // 软限位下限	-90°
    // 软限位上限	+90°
    // 最大速度	360 °/s
    // 最大电流	5 A
    // 速度环 PID	Kp=0.5, Ki=0.1, Kd=0.0（待整定）
    // 位置环 PID	Kp=10.0, Ki=0.0, Kd=0.5（待整定）
    // 预设方案 0	间隔 30s / 步距 0.125°（方案 0）
    // 上报频率	20 Hz
    uint8_t write_flag = 0x00;//标志位 表示是否写入过 FRAM
    float zero = 0.0f;//零位偏移值
    float limit_min = -90.0f;//软限位下限
    float limit_max = 90.0f;//软限位上限
    float max_speed = 360.0f;//最大速度
    float max_current = 5.0f;//最大电流
    float pid_v[3] = {0.8f, 200.0f, 0.00f};//速度环 PID
    float pid_p[3] = {8.0f, 0.0f, 0.00f};//位置环 PID
    float pid_iq[3] = {0.4f, 400.0f, 0.00f};//电流环 PID iq
    float pid_id[3] = {0.4f, 400.0f, 0.00f};//电流环 PID id
    float freq = 20.0f;//上报频率
    uint8_t scheme = 0;//预设方案
};


extern FOC_Motor_Params Motor_Params1;


class My_FM24V02A
{
    public:
        HAL_StatusTypeDef write(uint16_t address, uint8_t *data, uint16_t size);
        HAL_StatusTypeDef read(uint16_t address, uint8_t *data, uint16_t size);
        HAL_StatusTypeDef read_id(uint8_t *data, uint16_t size);
        FOC_Motor_Params read_motor_params(uint16_t address);
        HAL_StatusTypeDef write_motor_params(uint16_t address, FOC_Motor_Params Motor_Params_tx);
};


extern My_FM24V02A FM24V02A0;


