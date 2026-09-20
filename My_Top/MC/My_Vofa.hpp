#pragma once

#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>     // atof
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h" 
#include "bsp.hpp"

#define VOFA_PARAM_MAX    30   // 最大参数个数
#define VOFA_NAME_LEN     25   // 参数名最大长度
#define VOFA_RX_BUF_SIZE 128   // DMA 接收缓冲



/* 队列项：一帧 VOFA 接收数据 */
struct VofaRxFrame_t {
    uint16_t len;
    uint8_t  buf[VOFA_RX_BUF_SIZE];
};

extern QueueHandle_t vofaRxQueue;   // VOFA队列句柄

struct vofa_param_t {            // 单个参数
    char  name[VOFA_NAME_LEN];
    float value;
    bool  valid; //是否赋值
};

struct VoFA_Rx {
    uint8_t      rx_buf[VOFA_RX_BUF_SIZE]; // DMA 原始数据
    uint16_t     rx_len;                    // 本帧字节数
    volatile bool rx_ready;                 // 中断置1，Task清0

    vofa_param_t params[VOFA_PARAM_MAX];   // 参数表
    uint8_t      param_cnt;                // 已注册参数数

    /* 中断里调：拷贝数据到队列，替代 rx_ready 标志位 */
    void on_recv(uint8_t *buf, uint16_t len) {
        VofaRxFrame_t frame;
        if (len > VOFA_RX_BUF_SIZE) len = VOFA_RX_BUF_SIZE;
        frame.len = len;
        memcpy(frame.buf, buf, len);
        
        BaseType_t xHigherPriorityTaskWoken = pdFALSE; //中断里向队列发消息的标准三板斧
        xQueueSendFromISR(vofaRxQueue, &frame, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }


    /* Task 里调：解析 "name:value\n" */
    void parse() {
        char name[VOFA_NAME_LEN] = {}, val[16] = {};     // 临时缓冲区  
        uint8_t ni = 0, vi = 0;                          // name/val 索引
        bool in_val = false;                            // false=读名 true=读值

        for (uint16_t i = 0; i < rx_len; i++) {          // 逐字节扫描
            char c = (char)rx_buf[i];
            if (!in_val) {                                // 读参数名阶段
                if (c == ':') { name[ni] = '\0'; in_val = true; } // 遇到':'切换
                else if (ni < VOFA_NAME_LEN - 1) name[ni++] = c;
            } else {                                      // 读参数值阶段
                if (c == '\n') {                          // 遇到'\n'本条结束
                    val[vi] = '\0';
                    vofa_param_t *p = _find(name);       // 查表  p 指向 params
                    if (p) { p->value = atof(val); p->valid = true; } //将字符串转浮点数 塞进value里面
                    memset(name, 0, sizeof(name));       // 重置，准备下一条
                    ni = vi = 0; in_val = false; //清零重新读名
                } else if (vi < 15) val[vi++] = c;        // 存值
            }
        }
        
    }

    /* 获取参数值，找不到返回默认值 */
    float get(const char *name, float def = 0.0f) {
        for (uint8_t i = 0; i < param_cnt; i++)
            if (strncmp(params[i].name, name, VOFA_NAME_LEN) == 0 && params[i].valid)//输入的名字在表里 && 赋值过
                return params[i].value;
        return def;//返回0 
    }
    /* 批量读取参数：names 输入名字数组，vals 输出值数组，num 要读几个 */
    void vofa_get_batch(VoFA_Rx *v, const char **names, float *vals, uint8_t num)
    {
        for (uint8_t j = 0; j < num; j++) {      // 遍历每个要查的名字
            vals[j] = 0.0f;                       // 默认值
            for (uint8_t i = 0; i < v->param_cnt; i++) {  // 遍历参数表
                if (strncmp(v->params[i].name, names[j], VOFA_NAME_LEN) == 0
                    && v->params[i].valid) {
                    vals[j] = v->params[i].value;  // 找到了，写入
                    break;
                }
            }
        }
    }


private:
    /* 查找或新建参数槽 */
    // 第一次出现 "speed" → 新建槽位。第二次及以后出现 "speed" → 复用同一个槽位
    vofa_param_t *_find(const char *name) { 
        for (uint8_t i = 0; i < param_cnt; i++)  /* 第一步：遍历已有参数表，找名字相同的 */
            if (strncmp(params[i].name, name, VOFA_NAME_LEN) == 0) return &params[i]; //比较当前名字是否与库中相同，找到返回它的地址
        if (param_cnt >= VOFA_PARAM_MAX) return nullptr;                   /* 第二步：没找到，看看表有没有满 */
         /* 第三步：没满，新建一个 */
        strncpy(params[param_cnt].name, name, VOFA_NAME_LEN - 1);          // 复制名字字符串 //遇到\0会停下复制 剩下补零
        params[param_cnt].valid = false;                                    //  初始值未赋值
        return &params[param_cnt++];  // 返回新槽地址
    }
};

extern VoFA_Rx vofa1;

void Vofa_SendFireWater_VA(UART_HandleTypeDef *huart,uint16_t count, ...);
void Vofa_SendFireWater(float *data, uint16_t count, uint32_t timeout);
void Vofa_Init(UART_HandleTypeDef *huart);


