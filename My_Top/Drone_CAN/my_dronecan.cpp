#include "my_dronecan.hpp"


void DroneCan_Send_Servo(float* positions, uint8_t cmd_cnt) { //主动发送一个舵机位置命令，参数是目标位置（单位：度）
    static uint8_t transfer_id = 0;  //0~31 循环递增 的计数器，每发一帧新消息，这个值就 +1
    
    // 1. 填充要发送的消息结构体
    struct uavcan_equipment_actuator_ArrayCommand msg;
    msg.commands.len = cmd_cnt; //发送的指令数量
    for (uint8_t i = 0; i < cmd_cnt; i++)
    {
        msg.commands.data[i].actuator_id = i; //目标舵机的编号（0~255）
        msg.commands.data[i].command_type = UAVCAN_EQUIPMENT_ACTUATOR_COMMAND_COMMAND_TYPE_POSITION; // 位置控制指令
        msg.commands.data[i].command_value = positions[i]; //目标位置（单位：度）
    }
    
    // 2. 把结构体序列化为 CAN 总线可传输的字节流
    uint8_t buffer[UAVCAN_EQUIPMENT_ACTUATOR_ARRAYCOMMAND_MAX_SIZE]; // 61字节的缓冲区，足够编码这个消息
    uint32_t len = uavcan_equipment_actuator_ArrayCommand_encode(&msg, buffer);//返回实际编码后的字节数
    
    // 3. 通过 libcanard 广播
    CanardTxTransfer transfer = {
        .transfer_type = CanardTransferTypeBroadcast,// 广播消息 表示这条消息是广播消息 发给所有人
        .data_type_signature = UAVCAN_EQUIPMENT_ACTUATOR_ARRAYCOMMAND_SIGNATURE,   // 数据类型签名（64位）
        .data_type_id = UAVCAN_EQUIPMENT_ACTUATOR_ARRAYCOMMAND_ID,  // 1010
        .inout_transfer_id = &transfer_id, //传输递增的id 0-31 循环
        .priority = CANARD_TRANSFER_PRIORITY_MEDIUM, //优先级 0-31，0 最高
        .payload = buffer, //编码后的消息数据
        .payload_len = (uint16_t)len, //编码后的数据长度
    };

    if (canardBroadcastObj(&canard, &transfer) < 0) //把数据打包成 Canard 协议格式 → 进 TX 队列
    {
        return; // 失败，没进队列
    }
    My_Canard_Process();  //处理实际的收发

}



// void DroneCan_Send_EscCmd(const int16_t* values, uint8_t cmd_cnt) // 发送电调命令（ID=1030）
// {
//     static uint8_t transfer_id = 0;

//     // ① 填 DSDL 结构体
//     struct uavcan_equipment_esc_RawCommand msg;
//     msg.cmd.len = cmd_cnt;
//     for (uint8_t i = 0; i < cmd_cnt; i++)
//     {
//         msg.cmd.data[i] = values[i];  // [-8192, 8191]
//     }

//     // ② 序列化
//     uint8_t buffer[UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_MAX_SIZE];  // 36 字节
//     uint32_t len = uavcan_equipment_esc_RawCommand_encode(&msg, buffer);//编码

//     // ③ 广播
//     CanardTxTransfer transfer = {
//         .transfer_type       = CanardTransferTypeBroadcast, // 广播消息
//         .data_type_signature = UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_SIGNATURE,  // 数据类型签名（64位）
//         .data_type_id        = UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_ID,         // 1030
//         .inout_transfer_id   = &transfer_id, //传输递增的id 0-31 循环
//         .priority            = CANARD_TRANSFER_PRIORITY_MEDIUM, //优先级 0-31，0 最高
//         .payload             = buffer, //编码后的消息数据
//         .payload_len         = (uint16_t)len, //编码后的数据长度
//     };

//     if (canardBroadcastObj(&canard, &transfer) < 0) return;
//     My_Canard_Process();
// }








void DroneCan_Send_Heartbeat(void) // 发送心跳包（ID=341）
{
    static uint8_t transfer_id = 0;

    struct uavcan_protocol_NodeStatus msg;
    msg.uptime_sec                 = HAL_System::get_tick_ms() / 1000;  // 运行秒数
    msg.uptime_sec = 78;
    msg.health                     = 0;   // OK
    msg.mode                       = 0;   // 运行中 
    msg.sub_mode                   = 0;
    msg.vendor_specific_status_code = 0;

    uint8_t buffer[UAVCAN_PROTOCOL_NODESTATUS_MAX_SIZE];  // 7 字节
    uint32_t len = uavcan_protocol_NodeStatus_encode(&msg, buffer); //编码

    CanardTxTransfer transfer = {
        .transfer_type       = CanardTransferTypeBroadcast,
        .data_type_signature = UAVCAN_PROTOCOL_NODESTATUS_SIGNATURE,
        .data_type_id        = UAVCAN_PROTOCOL_NODESTATUS_ID,  // 341
        .inout_transfer_id   = &transfer_id,
        .priority            = CANARD_TRANSFER_PRIORITY_MEDIUM,
        .payload             = buffer,
        .payload_len         = (uint16_t)len,
    };

    if (canardBroadcastObj(&canard, &transfer) < 0) return;
    My_Canard_Process();
}