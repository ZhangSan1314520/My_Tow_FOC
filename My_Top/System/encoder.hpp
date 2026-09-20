#pragma once

#include "bsp.hpp"

#define ENCODER_PRECISION (1<<20) // 2^20，编码器分辨率
#define ENCODER_CMD_REQUEST_POS 0x02 // 请求位置命令


typedef struct
{
    uint8_t rx_buf[16];
    uint8_t tx_buf[1];

    uint32_t single_turn;

    float theta;
    uint32_t rcv_cnt;
    uint8_t error;
} encoder_data_t;

// ============================================================
// 编码器通信协议帧 (6字节)
// ============================================================
typedef __PACKED_STRUCT //不要内存对齐
{
    uint8_t CM;      // 命令/状态字节
    uint8_t SA;      // 保留
    uint8_t AS0;     // 角度低字节
    uint8_t AS1;     // 角度中字节
    uint8_t AS2;     // 角度高字节
    uint8_t CRC_VAL; // CRC 异或校验
}
encoder_msg_t;


// ============================================================
// 编码器类
// ============================================================

class Encoder
{

public:

    // 构造函数
    Encoder(const MotorPWM_Config *config): motor_config(config)
    {
        encoder.rcv_cnt = 0;
        encoder.error = 0;
        if(encoder_num < 4)
        {
            encoder_list[encoder_num] = this;

            encoder_num++;
        }
    }

    void encoder_init(); // 初始化：开启 UART DMA 空闲中断接收
    void encoder_request_pos(); // 发送位置请求命令
    float Get_Encoder_Radian(); // 获取当前编码器角度（弧度）
    


private:
    static uint8_t encoder_num;
    static Encoder *encoder_list[4];
    static uint8_t _crc_calc(uint8_t *buf, uint16_t len); // CRC 计算（对前 len 字节异或）
    static void My_Encode_Callback(UART_HandleTypeDef *huart,uint16_t Size); //给HAL用的 静态桥接函数
    void Encode_Callback(UART_HandleTypeDef *huart,uint16_t Size); //真正处理函数
    encoder_data_t encoder;
    const MotorPWM_Config *motor_config;

};

