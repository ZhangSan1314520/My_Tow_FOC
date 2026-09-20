#pragma once

#include "my_uav_can.hpp"

void DroneCan_Send_Servo(float* positions, uint8_t cmd_cnt);
void DroneCan_Send_Heartbeat(void); // 发送心跳包（ID=341）