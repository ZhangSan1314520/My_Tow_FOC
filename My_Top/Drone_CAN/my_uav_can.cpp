#include "my_uav_can.hpp"



// ===== Canard 协议栈实例 =====
#define CANARD_MEM_POOL_SIZE 4096                                 // 内存池大小
static uint8_t     canard_mem_pool[CANARD_MEM_POOL_SIZE];         // 内存池
CanardInstance canard;                                           // 协议栈实例

//  一. UAVCAN 协议裸帧发送与接收的硬件适配层（供 Canard 协议栈调用）
// ============================================================
// DroneCAN 硬件适配层：发送（Canard 协议栈通过 Process_Tx 间
// 接调用，也会被 Process_Tx 调用）
// 返回: 1=成功, 0=失败
// ============================================================
int16_t dronecan_if_transmit(const CanardCANFrame* frame)
{
    // Classic CAN 最多 8 字节
    if (frame->data_len > 8) return 0;

    // === 构建 FDCAN 发送头 ===
    FDCAN_TxHeaderTypeDef tx = {0};

    // 判断 最高位 bit31是否为1 表示扩展帧
    if (frame->id & CANARD_CAN_FRAME_EFF) 
    {
        // Identifier 是指定要发送的消息 ID
        tx.Identifier = frame->id & CANARD_CAN_EXT_ID_MASK;  // 只保留低29位其余置0
        tx.IdType     = FDCAN_EXTENDED_ID; // 通过扩展帧去发
    }
    else
    {
        tx.Identifier = frame->id & CANARD_CAN_STD_ID_MASK;  // 只保留低11位其余置0
        tx.IdType     = FDCAN_STANDARD_ID; // 通过标准帧去发
    }

    tx.TxFrameType          = FDCAN_DATA_FRAME; // 数据帧/远程帧
    tx.DataLength           = frame->data_len;       // Classic: 0~8 字节
    tx.ErrorStateIndicator  = FDCAN_ESI_ACTIVE; // 错误状态指示
    tx.BitRateSwitch        = FDCAN_BRS_OFF; // 关闭位速率切换（Classic CAN）
    tx.FDFormat             = FDCAN_CLASSIC_CAN; // 通过 Classic CAN 发送
    tx.TxEventFifoControl   = FDCAN_NO_TX_EVENTS; // // 发完要不要记录
    tx.MessageMarker        = 0; // 消息标记

    // TX FIFO 满时重试（最多 10 次）
    for (int i = 0; i < 10; i++)
    {
        if (HAL_FDCAN_AddMessageToTxFifoQ(FD_CAN1, &tx, frame->data) == HAL_OK)
            return 1;
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    return 0;
}


// ============================================================
// DroneCAN 硬件适配层：接收（从 FDCAN 硬件收一帧）
// 返回: 1=收到新帧, 0=没数据
// ============================================================
int16_t dronecan_if_receive(CanardCANFrame* out_frame)
{
    uint32_t fifo;

    // DroneCAN 所有帧都是扩展帧 → 优先查 FIFO1（过滤器配置：扩展帧→FIFO1）
    if (HAL_FDCAN_GetRxFifoFillLevel(FD_CAN1, FDCAN_RX_FIFO1) > 0) //如果 FIFO1 有数据也就是绑定的扩展帧
    {
        fifo = FDCAN_RX_FIFO1; //标记指定FIFO1里面去取数据
    }else if (HAL_FDCAN_GetRxFifoFillLevel(FD_CAN1, FDCAN_RX_FIFO0) > 0)
    {
        fifo = FDCAN_RX_FIFO0; //标记指定FIFO0里面去取数据
    }
    else
    {
        return 0;  // 两个 FIFO 都没有数据 强制退出
    }

    FDCAN_RxHeaderTypeDef rx;
    uint8_t data[8];

    // 从对应 FIFO 取出消息
    if (HAL_FDCAN_GetRxMessage(FD_CAN1, fifo, &rx, data) != HAL_OK)
        return 0;

    // === 转换为 Canard 帧格式 ===
    if (rx.IdType == FDCAN_EXTENDED_ID) //读数据类型是扩展帧还是标准帧
    {
        // 扩展帧: 取出来的ID保留低29位后手动将bit31置1
        out_frame->id = (rx.Identifier & CANARD_CAN_EXT_ID_MASK) | CANARD_CAN_FRAME_EFF;
    }else
    {
        // 标准帧: 取出来的ID保留低11位
        out_frame->id = rx.Identifier & CANARD_CAN_STD_ID_MASK;
    }

    out_frame->data_len = rx.DataLength;   // Classic CAN: DLC(0~8) 直接等于字节数
    memcpy(out_frame->data, data, out_frame->data_len);//data 不在FDCAN_RxHeaderTypeDef里面 需要单独传入
    return 1;   // 成功收到一帧
}

void test_uavcan() //测试uavcan的发送和接收 裸帧测试
{
    // ===== ① 构造并发送测试帧 =====
    CanardCANFrame tx = {0};
    tx.id       = 0x321 | CANARD_CAN_FRAME_EFF;  // 扩展帧，ID=0x321
    tx.data_len = 6;
    tx.data[0]  = 0x11;
    tx.data[1]  = 0x22;
    tx.data[2]  = 0x33;
    tx.data[3]  = 0x44;
    tx.data[4]  = 0x55;
    tx.data[5]  = 0x66;
    dronecan_if_transmit(&tx);// 发送测试帧
    vTaskDelay(pdMS_TO_TICKS(5));

    // ===== ② 接收 =====
    CanardCANFrame rx = {0};
    if (dronecan_if_receive(&rx) == 0)
    {
        // 没有收到数据
        return;
    }
}

//  一. UAVCAN 协议裸帧发送与接收的硬件适配层（供 Canard 协议栈调用）












//  二. Canard 协议栈的初始化与处理
static void on_transfer_received(CanardInstance* ins, CanardRxTransfer* transfer);
static bool should_accept(const CanardInstance* ins,
                          uint64_t* out_data_type_signature,
                          uint16_t data_type_id,
                          CanardTransferType transfer_type,
                          uint8_t source_node_id);


void My_Canard_Init(void) //初始化Canard协议栈
{
    My_Can_Init();   // 初始化 FDCAN 硬件（过滤器+启动）

    // 初始化 Canard 协议栈
    canardInit(&canard,
               canard_mem_pool,           // 内存池
               CANARD_MEM_POOL_SIZE,      // 内存池大小
               on_transfer_received,      // 接收完成回调
               should_accept,             // 消息过滤器
               NULL);                     // 用户引用，不需要

    // 设置本节点 ID（每个 DroneCAN 节点唯一，范围 1~127）
    canardSetLocalNodeID(&canard, This_node_id);
}


// 遍历协议栈 TX 队列，逐帧通过 dronecan_if_transmit 发出去
// ============================================================
void My_Canard_Process_Tx(void)
{
    const CanardCANFrame* tx_frame;

    // 从队列中取出一帧，发出去，成功就移出队列，失败就退出等待下次再发
    while ((tx_frame = canardPeekTxQueue(&canard)) != NULL)
    {
        if (dronecan_if_transmit(tx_frame) == 1)
            canardPopTxQueue(&canard);   // 发送成功，移出队列
        else
            break;                       // TX FIFO 满，等下次再发
    }
}


// ============================================================
// 处理接收（从硬件收帧 → 喂给协议栈）
// ============================================================
void My_Canard_Process_Rx(void)
{
    CanardCANFrame rx_frame;

    // 有数据就收，直到 FIFO 空
    while (dronecan_if_receive(&rx_frame) == 1)
    {
        // 喂给协议栈，收到完整传输时自动调 on_transfer_received
        canardHandleRxFrame(&canard, &rx_frame, HAL_System::get_tick_us());
    }
}


// ============================================================
// 主处理函数（FreeRTOS 任务中定期调用）
// ============================================================


void My_DroneCanard_Process(void) //内部环回模式只能用这个
{
    // 发一帧 → 立刻收一帧，防止多帧传输时 RX FIFO（仅3槽）溢出丢帧
    const CanardCANFrame* tx_frame;
    while ((tx_frame = canardPeekTxQueue(&canard)) != NULL)
    {
        if (dronecan_if_transmit(tx_frame) == 1)
        {
            canardPopTxQueue(&canard);          // 发送成功，出队
            My_Canard_Process_Rx();             // 立刻收回来
        }
        else
            break;                              // TX FIFO 满，等下次
    }

    My_Canard_Process_Rx();  // 收剩余帧（可能来自外部节点）

    // 清理超时的传输（协议栈要求定期调用，约每秒一次）
    static uint32_t last_cleanup = 0;
    uint32_t now = HAL_System::get_tick_ms();
    if (now - last_cleanup > 1000)
    {
        canardCleanupStaleTransfers(&canard, now * 1000); // 清理超时传输
        last_cleanup = now;
    }
}


void My_Canard_Process(void) //实际用这个 内部环回调用这个多帧会丢包
{
    My_Canard_Process_Tx();  // 从队列取帧 → 调硬件发出去
    My_Canard_Process_Rx();  // 从硬件收帧 → 喂协议栈解析 → 回调

    // 清理超时的传输（协议栈要求定期调用，约每秒一次）
    static uint32_t last_cleanup = 0;
    uint32_t now = HAL_System::get_tick_ms();
    if (now - last_cleanup > 1000)
    {
        canardCleanupStaleTransfers(&canard, now * 1000); // 清理超时传输
        last_cleanup = now;
    }
}




// ============================================================
// 接收完成回调（Canard 收到一条完整传输时自动调用）
// 在这里处理业务逻辑
// ============================================================
#include "MC_Serial.hpp"
static void on_transfer_received(CanardInstance* ins, CanardRxTransfer* transfer)
{

    float debug_buf[64] = {0};
    transfer->data_type_id;//消息ID
    transfer->source_node_id;//节点ID 设备的ID
    transfer->payload_len;//数据长度
    transfer->payload_head;//数据指针(void*类型 不能直接用需要转换类型)
    memcpy(debug_buf, transfer->payload_head, transfer->payload_len);
    transfer->transfer_id;//传输ID 协议栈自动递增
    transfer->transfer_type;//2 广播消息类型还是其他 
    transfer->priority;//传输优先级

    transfer->timestamp_usec;// 收到这条消息的时间戳（微秒）
    transfer->payload_middle;//多帧消息中间帧 超过8字节的数据
    transfer->payload_tail;//多帧消息最后一帧 超过8字节的数据

    DroneCan_Parse(transfer);//解析DroneCAN消息

    canardReleaseRxTransferPayload(ins, transfer); // 释放协议栈内存 防止内存泄漏
}


// ============================================================
// 消息过滤回调（决定接收哪些消息类型）
// 返回 true=接收, false=丢弃
// ============================================================
static bool should_accept(const CanardInstance* ins,
                          uint64_t* out_data_type_signature,
                          uint16_t data_type_id,
                          CanardTransferType transfer_type,
                          uint8_t source_node_id)
{
    //只收广播消息
    if (transfer_type != CanardTransferTypeBroadcast)
        return false;

    // *out_data_type_signature = 0; //不验证格式，啥都收
    // return true;  // 暂时全部接收

    if (source_node_id == This_node_id) // 不收本节点发来的消息
        return false; // 不收本节点发来的消息

    switch (data_type_id)
    {
    // 舵机目标位置 (1010)
    case UAVCAN_EQUIPMENT_ACTUATOR_ARRAYCOMMAND_ID:
        *out_data_type_signature = UAVCAN_EQUIPMENT_ACTUATOR_ARRAYCOMMAND_SIGNATURE;
        return true;//true表示接收这条消息 flase表示丢弃这条消息

    // 电调状态 (1034)
    case 5231:
        *out_data_type_signature = 0x12358925; //模拟的假签名
        return true;

    // 节点状态/心跳 (341)
    case UAVCAN_PROTOCOL_NODESTATUS_ID:
        *out_data_type_signature = UAVCAN_PROTOCOL_NODESTATUS_SIGNATURE;
        return true;

    default:
        return false;  // true表示接收这条消息 flase表示丢弃这条消息
    }

}


void Canard_Test_TxRx() //测试 Canard 收发
{
    uint8_t data[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88}; //低于8字节的数据不做签名校验
    uint16_t len = sizeof(data)/sizeof(data[0]);
    static uint8_t transfer_id = 0;  // ← 传输ID，协议栈自动递增

    CanardTxTransfer t = {};
    t.transfer_type       = CanardTransferTypeBroadcast; // 表示这条消息是广播消息 发给所有人
    t.data_type_signature = 0x12358925;  //模拟假签名
    t.data_type_id        = 5231; // 消息类型ID
    t.inout_transfer_id   = &transfer_id;  // 必须设传输ID 协议栈自动递增
    t.payload             = data;
    t.payload_len         = len;
    

    int16_t ret = canardBroadcastObj(&canard, &t); //把数据打包成 Canard 协议格式 → 进 TX 队列
    if (ret == 0) return; //  失败，没进队列
    My_Canard_Process(); // 处理实际的收发
}


//  二. Canard 协议栈的初始化与处理




