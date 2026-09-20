#include "my_fdcan.hpp"

#include "FreeRTOS.h"
#include "task.h"


RxMsg g_rx_msg; //中断接收的消息
RxMsg g_rx_msg_poll; //轮询接收的消息

void My_FilterConfig(void)// 配置CAN过滤器
{
    FDCAN_FilterTypeDef sFilterConfig;
    // --- 标准 ID 过滤器：全接收 → FIFO0
    sFilterConfig.IdType = FDCAN_STANDARD_ID; //标准类型ID/扩展类型ID
    sFilterConfig.FilterIndex = 0; //过滤器索引号（0-15）
    sFilterConfig.FilterType = FDCAN_FILTER_MASK; //过滤器类型选择屏蔽位模式 
    sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0; //过滤器配置，将匹配到的报文存储到FIFO0
    sFilterConfig.FilterID1 = 0x0000; // 要匹配的目标 ID    
    sFilterConfig.FilterID2 = 0x0000; // 与哪些位参与比较
    
    if (HAL_FDCAN_ConfigFilter(FD_CAN1, &sFilterConfig) != HAL_OK) // 配置标准帧的FDCAN过滤器
    {
        Error_Handler();
    }
    // --- 扩展 ID 过滤器：全接收 → FIFO1 ---
    sFilterConfig.IdType = FDCAN_EXTENDED_ID; //标准类型ID/扩展类型ID
    sFilterConfig.FilterIndex = 0; //过滤器索引号（0-15）
    sFilterConfig.FilterType = FDCAN_FILTER_RANGE; //过滤器类型选择筛选模式 从ID1到ID2的范围
    sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO1; //过滤器配置，将匹配到的报文存储到FIFO0
    sFilterConfig.FilterID1 = 0x00000000; // 要匹配的目标 ID    
    sFilterConfig.FilterID2 = 0x1fffffff; // 与哪些位参与比较
    
    if (HAL_FDCAN_ConfigFilter(FD_CAN1, &sFilterConfig) != HAL_OK) // 配置扩展帧的FDCAN过滤器
    {
        Error_Handler();
    }


    HAL_FDCAN_Start(FD_CAN1);// 启动FDCAN

    #if CAN_IT_Get_Data == 1 // 中断接收模式
    if (HAL_FDCAN_ActivateNotification(FD_CAN1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK)
    {
        // 启用FDCAN接收中断fifo0新消息中断
        Error_Handler();
    }
    if (HAL_FDCAN_ActivateNotification(FD_CAN1, FDCAN_IT_RX_FIFO1_NEW_MESSAGE, 0) != HAL_OK)
    {
        // 启用FDCAN接收中断fifo1新消息中断
        Error_Handler();
    }
    #endif
}


void My_Can_Init(void) // 初始化CAN
{
    My_FilterConfig(); // 配置CAN过滤器
}

bool My_Can_Send(uint32_t can_id, uint8_t *data, uint8_t len, bool IDE) // 原始发送CAN报文
{
    HAL_StatusTypeDef status = HAL_ERROR;
    FDCAN_TxHeaderTypeDef tx = {0};
    tx.Identifier           = can_id;
    tx.IdType               = IDE ? FDCAN_EXTENDED_ID : FDCAN_STANDARD_ID; //true是扩展ID false是标准ID
    tx.TxFrameType          = FDCAN_DATA_FRAME;// 发数据帧
    tx.DataLength           = FDCAN_DLC_BYTES_8;      // 设置每一帧发送8个字节 经典模式只能0-8字节，FD模式可以0-64字节
    tx.ErrorStateIndicator  = FDCAN_ESI_ACTIVE; // 错误状态
    tx.BitRateSwitch        = FDCAN_BRS_OFF; // 是否启用切换比特率速率
    tx.FDFormat             = FDCAN_CLASSIC_CAN;  // 用经典CAN / CAN-FD 前提配置好
    tx.TxEventFifoControl   = FDCAN_NO_TX_EVENTS; // 发完要不要记录
    tx.MessageMarker        = 0; // 给消息打个标签
    // 最多重试 10 次
    for (int i = 0; i < 10; i++)
    {
        status = HAL_FDCAN_AddMessageToTxFifoQ(FD_CAN1, &tx, data);
        if (status == HAL_OK) return true;  // 发送成功
        vTaskDelay(pdMS_TO_TICKS(1));  // 延时 1 毫秒，等待邮箱释放
    }
    return false;  // 超时失败
}


#if CAN_IT_Get_Data == 1 // 中断接收模式

// ===== 接收回调（共用逻辑）=====
static void CAN_StoreMsg(FDCAN_HandleTypeDef *hfdcan, uint32_t fifo, uint8_t ide)
{
    FDCAN_RxHeaderTypeDef rx;
    if (HAL_FDCAN_GetRxMessage(hfdcan, fifo, &rx, g_rx_msg.data) != HAL_OK)
        return;

    g_rx_msg.can_id = rx.Identifier;
    g_rx_msg.len    = rx.DataLength;
    g_rx_msg.ide    = ide;
}

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t it) //中断收FOFO0
{
    if (hfdcan->Instance == FDCAN2 && (it & FDCAN_IT_RX_FIFO0_NEW_MESSAGE))
        CAN_StoreMsg(hfdcan, FDCAN_RX_FIFO0, 0);   // 标准 ID
}

void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t it) //中断收FOFO1
{
    if (hfdcan->Instance == FDCAN2 && (it & FDCAN_IT_RX_FIFO1_NEW_MESSAGE))
        CAN_StoreMsg(hfdcan, FDCAN_RX_FIFO1, 1);   // 扩展 ID
}


#elif CAN_IT_Get_Data == 0 // 轮询接收模式

bool My_CAN_Poll(RxMsg *msg) // 轮询接收CAN报文
{
    // 先查 FIFO0
    if (HAL_FDCAN_GetRxFifoFillLevel(FD_CAN1, FDCAN_RX_FIFO0) > 0)
    {
        FDCAN_RxHeaderTypeDef rx;
        if (HAL_FDCAN_GetRxMessage(FD_CAN1, FDCAN_RX_FIFO0, &rx, msg->data) == HAL_OK)
        {
            msg->can_id = rx.Identifier;
            msg->len    = rx.DataLength;
            msg->ide    = (rx.IdType == FDCAN_EXTENDED_ID);
            return true;
        }
    }
    // 再查 FIFO1
    if (HAL_FDCAN_GetRxFifoFillLevel(FD_CAN1, FDCAN_RX_FIFO1) > 0)
    {
        FDCAN_RxHeaderTypeDef rx;
        if (HAL_FDCAN_GetRxMessage(FD_CAN1, FDCAN_RX_FIFO1, &rx, msg->data) == HAL_OK)
        {
            msg->can_id = rx.Identifier;
            msg->len    = rx.DataLength;
            msg->ide    = (rx.IdType == FDCAN_EXTENDED_ID);
            return true;
        }
    }
    return false;
}

#endif










