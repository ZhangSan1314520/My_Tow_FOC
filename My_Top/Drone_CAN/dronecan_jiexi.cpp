#include "dronecan_jiexi.hpp"

// ===== 全局存储：最新接收到的 DroneCAN 消息数据 =====
// 收到消息后自动更新，updated=true
// 业务代码读取完后手动设 updated=false
DroneCan_Servo_t      drone_servo      = {0};
DroneCan_EscCmd_t     drone_esc_cmd    = {0};
DroneCan_Heartbeat_t  drone_heartbeat  = {0};
DroneCan_EscStatus_t  drone_esc_status = {0};


// ============================================================
// Parse_ArrayCommand：解析舵机 ArrayCommand (ID=1010)
// 飞控发出 → 本节点接收 → 存入 drone_servo
// ============================================================
static void Parse_ArrayCommand(CanardRxTransfer* transfer)
{
    struct uavcan_equipment_actuator_ArrayCommand msg = {0};
    if (uavcan_equipment_actuator_ArrayCommand_decode(transfer, &msg)) //解码
        return;  // 解码失败，丢弃

    // ② 转存到全局结构体
    drone_servo.cmd_cnt = msg.commands.len;
    for (uint8_t i = 0; i < msg.commands.len; i++)
    {
        drone_servo.cmd[i].actuator_id   = msg.commands.data[i].actuator_id;   // 舵机编号
        drone_servo.cmd[i].command_type  = msg.commands.data[i].command_type;  // 控制类型
        drone_servo.cmd[i].command_value = msg.commands.data[i].command_value; // 目标值
    }
    drone_servo.updated = true;//标记有数据更新
}


// ============================================================
// Parse_RawCommand：解析电调 RawCommand (ID=1030)
// 飞控发出 → 本节点接收 → 存入 drone_esc_cmd
// ============================================================
static void Parse_RawCommand(CanardRxTransfer* transfer)
{
    struct uavcan_equipment_esc_RawCommand msg = {0};
    if (uavcan_equipment_esc_RawCommand_decode(transfer, &msg)) //解码
        return;

    drone_esc_cmd.cmd_cnt = msg.cmd.len;
    for (uint8_t i = 0; i < msg.cmd.len; i++)
    {
        drone_esc_cmd.value[i] = msg.cmd.data[i];  // 每路电调 PWM 值
    }
    drone_esc_cmd.updated = true;
}


// ============================================================
// Parse_NodeStatus：解析心跳 NodeStatus (ID=341)
// 飞控每秒广播 → 本节点接收 → 存入 drone_heartbeat
// 如果超过 3 秒收不到 → 飞控离线
// ============================================================
static void Parse_NodeStatus(CanardRxTransfer* transfer)
{
    struct uavcan_protocol_NodeStatus msg = {0};
    if (uavcan_protocol_NodeStatus_decode(transfer, &msg))
        return;

    drone_heartbeat.uptime_sec    = msg.uptime_sec;                     // 飞控运行了多久
    drone_heartbeat.health        = msg.health;                         // 0=正常
    drone_heartbeat.mode          = msg.mode;                           // 运行模式
    drone_heartbeat.vendor_status = msg.vendor_specific_status_code;   // 厂商自定义
    drone_heartbeat.updated       = true;
}


// ============================================================
// Parse_EscStatus：解析电调状态 Status (ID=1034)
// 电调上报 → 本节点接收 → 存入 drone_esc_status
// ============================================================
static void Parse_EscStatus(CanardRxTransfer* transfer)
{
    struct uavcan_equipment_esc_Status msg = {0};
    if (uavcan_equipment_esc_Status_decode(transfer, &msg))
        return;

    drone_esc_status.voltage          = msg.voltage;           // V
    drone_esc_status.current          = msg.current;           // A
    drone_esc_status.temperature      = msg.temperature;       // °C
    drone_esc_status.rpm              = msg.rpm;               // 转/分钟
    drone_esc_status.power_rating_pct = msg.power_rating_pct; // 0~127
    drone_esc_status.esc_index        = msg.esc_index;         // 第几个电调
    drone_esc_status.updated          = true;
}


// ============================================================
// DroneCan_Parse：统一解析入口
// 在 on_transfer_received 回调中调用此函数
// 根据消息 ID 自动分发到对应的解析函数
// ============================================================
void DroneCan_Parse(CanardRxTransfer* transfer)
{
    switch (transfer->data_type_id)
    {
    case UAVCAN_EQUIPMENT_ACTUATOR_ARRAYCOMMAND_ID:  // 1010 舵机目标位置
        Parse_ArrayCommand(transfer);
        break;

    case UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_ID:         // 1030 电调控制值
        Parse_RawCommand(transfer);
        break;

    case UAVCAN_PROTOCOL_NODESTATUS_ID:              // 341  飞控心跳
        Parse_NodeStatus(transfer);
        break;

    case UAVCAN_EQUIPMENT_ESC_STATUS_ID:             // 1034 电调状态反馈
        Parse_EscStatus(transfer);
        break;

    default:
        break;  // 未知消息类型，忽略
    }
}
