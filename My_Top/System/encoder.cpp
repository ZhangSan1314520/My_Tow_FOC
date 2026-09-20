#include "encoder.hpp"
Encoder* Encoder::encoder_list[4] = {nullptr}; // Encoder 对象地址列表

uint8_t Encoder::encoder_num = 0;

uint8_t Encoder::_crc_calc(uint8_t *buf, uint16_t len)
{

    uint8_t crc = 0;
    while (len--)
    {
        crc ^= buf[len];
    }
    return crc;
}


void Encoder::encoder_init()
{

    if(motor_config->huart->hdmarx == nullptr) 
    {
        //UART 没有DMA句柄
        Error_Handler();
    }
    motor_config->huart->RxEventCallback =My_Encode_Callback;
    HAL_UARTEx_ReceiveToIdle_DMA(motor_config->huart,encoder.rx_buf,sizeof(encoder.rx_buf)); //开启接收中断
    __HAL_DMA_DISABLE_IT(motor_config->huart->hdmarx,DMA_IT_HT);//禁止半传输中断
}


void Encoder::My_Encode_Callback(UART_HandleTypeDef *huart,uint16_t Size) //转发回调函数
{
    for(uint8_t i=0;i<encoder_num;i++) //判断是哪个回调函数然后去转发给对应的实例
    {
        if(encoder_list[i] != nullptr &&encoder_list[i]->motor_config->huart == huart)
        {
            encoder_list[i]->Encode_Callback(huart,Size);
            return;
        }
    }
}

void Encoder::Encode_Callback(UART_HandleTypeDef *huart,uint16_t Size)
{      
    if(huart != motor_config->huart) {return;}

    float temp = 0;
    if(Size != sizeof(encoder_msg_t)) //长度校验
    {
        encoder.error = 1;
        HAL_UARTEx_ReceiveToIdle_DMA(motor_config->huart,encoder.rx_buf,sizeof(encoder.rx_buf));//开启接收中断
        __HAL_DMA_DISABLE_IT(motor_config->huart->hdmarx,DMA_IT_HT);//禁止半传输中断
        return;
    }

    encoder_msg_t *msg =(encoder_msg_t *)encoder.rx_buf;
    

    if(_crc_calc(encoder.rx_buf,Size-1)!= msg->CRC_VAL)//CRC 校验
    {
        encoder.error = 2;
        HAL_UARTEx_ReceiveToIdle_DMA(motor_config->huart,encoder.rx_buf,sizeof(encoder.rx_buf)); //开启接收中断
        __HAL_DMA_DISABLE_IT(motor_config->huart->hdmarx,DMA_IT_HT); //禁止半传输中断
        return;
    }

    encoder.single_turn =       //提取单圈原始值（3字节拼合）
            (uint32_t)msg->AS0 |
            ((uint32_t)msg->AS1 << 8) |
            ((uint32_t)msg->AS2 << 16);


    encoder.theta =(float)encoder.single_turn *M_2PI /ENCODER_PRECISION; //转换为弧度

    encoder.error = 0;

    encoder.rcv_cnt++;


    HAL_UARTEx_ReceiveToIdle_DMA(motor_config->huart,encoder.rx_buf,sizeof(encoder.rx_buf)); //重新开启接收中断
    __HAL_DMA_DISABLE_IT(motor_config->huart->hdmarx,DMA_IT_HT); //禁止半传输中断
}



// ============================================================
// 发送位置请求命令
// ============================================================

void Encoder::encoder_request_pos()
{
    // if (motor_config != &M1_PWM) return; //只对M1_PWM发送请求
    encoder.tx_buf[0] = ENCODER_CMD_REQUEST_POS;
    HAL_UART_Transmit_DMA(motor_config->huart,encoder.tx_buf,sizeof(encoder.tx_buf));
}


float Encoder::Get_Encoder_Radian() //获取编码器弧度
{

    float rad =encoder.theta;

    return rad;
}



