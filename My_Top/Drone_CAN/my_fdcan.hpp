#pragma once
#include <stdint.h>
#include <string.h>
#include <canard.h>
#include "bsp.hpp"
#define CAN_IT_Get_Data 0 //1:中断接收，0：轮询接收

typedef struct
{
    uint32_t can_id;// 兼容标准ID(11bit)或扩展ID(29bit)
    uint8_t data[8];//经典模式 每帧最大发8个字节，FD模式每帧最大发64个字节
    uint8_t len;
    bool ide; // 标准ID(0)或扩展ID(1)
} RxMsg; //接收到的报文信息

extern  RxMsg g_rx_msg; //中断接收的消息
extern  RxMsg g_rx_msg_poll; //轮询接收的消息

void My_Can_Init(void);
bool My_Can_Send(uint32_t can_id, uint8_t *data, uint8_t len, bool IDE) ;
bool My_CAN_Poll(RxMsg *msg) ;//轮询方式接收消息
// int16_t dronecan_if_transmit(const CanardCANFrame* frame);
// int16_t dronecan_if_receive(CanardCANFrame* out_frame);



