
#include "FreeRTOS.h"
#include "task.h"
#include "MC_Serial.hpp"
#include "dronecan_jiexi.hpp"

float duoji[] = {30.0, 45.0, 99.0, 360.0, 300.0, 250.0, 200.0, 150.0};
uint16_t duoji_len = sizeof(duoji) / sizeof(duoji[0]);

int16_t diantiao[] = {-60, 20, 100, 30, -600, 100, 200, 300, -200, -300, -400, 500, 600, 700, 800};
uint16_t diantiao_len = sizeof(diantiao) / sizeof(diantiao[0]);

void Task_DroneCAN(void *argument)
{
    // printf("1-DroneCAN任务启动中\r\n");
    vTaskDelay(pdMS_TO_TICKS(50));
    while (1)
    {
        // DroneCan_Send_Servo(duoji, duoji_len); //
        DroneCan_Send_Heartbeat(); // 发送心跳包
        if (drone_servo.updated)
        {
            printf("舵机指令");
            for (uint16_t i = 0; i < drone_servo.cmd_cnt; i++)
            {
                printf("id[%d] = %.3f  ", drone_servo.cmd[i].actuator_id, drone_servo.cmd[i].command_value);
            }
            printf("\r\n");
            drone_servo.updated = false;
        }

        if (drone_heartbeat.updated)
        {
            printf("%d %d  %d  %d  %d\r\n",
                drone_heartbeat.uptime_sec   ,   // 飞控运行了多久
                drone_heartbeat.health    ,   
                drone_heartbeat.mode    ,     
                drone_heartbeat.vendor_status,
                drone_heartbeat.updated);
            drone_heartbeat.updated = false;
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
