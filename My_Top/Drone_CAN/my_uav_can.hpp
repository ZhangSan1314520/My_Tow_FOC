#pragma once
#include <stdint.h>
#include "fdcan.h"
#include <string.h>
#include <canard.h>
#include "my_fdcan.hpp"
#include "HAL_System.hpp"
#include "FreeRTOS.h"
#include "task.h"
#include "dronecan_jiexi.hpp" //解析DroneCAN消息

#include <uavcan.equipment.actuator.Command.h>
#include <uavcan.equipment.actuator.ArrayCommand.h>
#include <uavcan.equipment.actuator.Status.h>
#include <uavcan.protocol.NodeStatus.h>

#include <uavcan.equipment.esc.RawCommand.h>
#include <uavcan.equipment.esc.Status.h>

#define This_node_id    91


// ===== 初始化与主循环 =====
void My_Canard_Init(void);            // 初始化Canard协议栈
void My_Canard_Process(void);         // 主循环处理（收+发+清理）
void My_DroneCanard_Process(void); // 主循环处理（收+发+清理）
void My_Canard_Process_Tx(void);      // 单独处理发送队列
void My_Canard_Process_Rx(void);      // 单独处理接收

// ===== 发送广播消息 =====
bool My_Canard_Broadcast(uint64_t data_type_signature,
                         uint16_t data_type_id,
                         const uint8_t* payload,
                         uint16_t payload_len);

// ===== 硬件适配层（供 Canard 协议栈调用）=====
int16_t dronecan_if_transmit(const CanardCANFrame* frame);
int16_t dronecan_if_receive(CanardCANFrame* out_frame);
void test_uavcan(); //测试uavcan的发送和接收 裸帧测试
void Canard_Test_TxRx(); //测试 Canard 收发

extern CanardInstance canard;  // 协议栈实例
