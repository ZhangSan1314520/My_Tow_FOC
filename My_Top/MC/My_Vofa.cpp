#include "My_Vofa.hpp"


// 1.发送部分
// 定义双缓冲区，防止连续发送时覆盖 
#define MAX_VOFA_FLOATS 30 // 临时缓冲区，最多发送 30 个浮点数 
#define VOFA_BUFFER_SIZE (MAX_VOFA_FLOATS * sizeof(float) + 4)  // 最多20个浮点数 + 4字节帧尾
static uint8_t vofa_buf1[VOFA_BUFFER_SIZE];
static uint8_t vofa_buf2[VOFA_BUFFER_SIZE];
static uint8_t vofa_buf_sel = 0;  // 简单轮换选择

static const uint8_t tail[4] = {0x00, 0x00, 0x80, 0x7F};

void Vofa_SendFireWater(UART_HandleTypeDef *huart,float *data, uint16_t count, uint32_t timeout)//发送指定参数
{
    if (count == 0) return;
    uint16_t data_len = count * sizeof(float);
    uint16_t total_len = data_len + 4;

    // 选择当前可用的缓冲区（简单轮换，确保不覆盖正在DMA发送中的数据）
    uint8_t *buf = (vofa_buf_sel == 0) ? vofa_buf1 : vofa_buf2;
    vofa_buf_sel = !vofa_buf_sel;  // 下次用另一个

    // 拼接数据
    memcpy(buf, data, data_len);
    memcpy(buf + data_len, tail, 4);
 
    if (timeout == 0) HAL_UART_Transmit_DMA(huart, buf, total_len);//DMA 发送
    else HAL_UART_Transmit(huart, (uint8_t*)buf, total_len, timeout); //轮询发送
}



void Vofa_SendFireWater_VA(UART_HandleTypeDef *huart,uint16_t count, ...)//发送不定长浮点型参数
{
    float buffer[MAX_VOFA_FLOATS];
    
    if (count > MAX_VOFA_FLOATS) count = MAX_VOFA_FLOATS;  // 防溢出
    
    va_list args;
    va_start(args, count); //让 args 指向第一个可变参数
    for (uint16_t i = 0; i < count; i++) {//取出下一个参数
        buffer[i] = (float)va_arg(args, double);  // 变参 float 会被提升为 double
    }
    va_end(args);
    
    Vofa_SendFireWater(huart, buffer, count,0);//dma发送
}



//2.接受部分
VoFA_Rx vofa1 = {};
uint8_t DMA_RxBuff[VOFA_RX_BUF_SIZE]={};

void My_Vofa_Callback(UART_HandleTypeDef *huart, uint16_t Size)
{
    // 解析收到的数据
    vofa1.on_recv(DMA_RxBuff, Size);
    HAL_UARTEx_ReceiveToIdle_DMA(huart, DMA_RxBuff, sizeof(DMA_RxBuff));
    __HAL_DMA_DISABLE_IT(huart->hdmarx, DMA_IT_HT);
}

void Vofa_Init(UART_HandleTypeDef *huart)
{
    huart->RxEventCallback = My_Vofa_Callback; //绑定Vofa串口3回调函数
    HAL_UART_AbortReceive(huart);  // 清除残留状态和错误指针 防止第一次接受到错误数据
    HAL_UARTEx_ReceiveToIdle_DMA(huart, DMA_RxBuff, sizeof(DMA_RxBuff));
    __HAL_DMA_DISABLE_IT(huart->hdmarx, DMA_IT_HT); // 禁止半传输中断
}


