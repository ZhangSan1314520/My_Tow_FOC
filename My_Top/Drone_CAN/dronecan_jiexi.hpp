#pragma once
#include "my_dronecan.hpp"


// ============================================================
// DroneCAN 消息解析后的数据存储结构体
// 收到消息 → DroneCan_Parse() → 解析 → 存入对应全局变量 → updated=true
// 业务代码读取数据后 → 手动设 updated=false
// ============================================================

// ============================================================
// 舵机 ArrayCommand (ID=1010): 最多 15 路舵机的位置/力/速度/PWM 命令
// ============================================================
typedef struct {
    uint8_t  actuator_id;     // 舵机编号 (0~255)
    uint8_t  command_type;    // 控制类型: 0=无量纲 1=位置(度) 2=力 3=速度 4=PWM
    float    command_value;   // 目标值 (位置=度, 速度=度/秒, PWM=占空比0~1)
} ServoCmd_t;                  // 单条舵机命令

typedef struct {
    uint8_t   cmd_cnt;        // 本次收到的命令条数 (0~15)
    ServoCmd_t cmd[15];       // 命令数组
    bool      updated;        // true=有新数据  false=已读取
} DroneCan_Servo_t;            // 飞控发来的舵机控制命令


// ============================================================
// 电调 RawCommand (ID=1030): 最多 20 路电调的转速命令
// ============================================================
typedef struct {
    uint8_t   cmd_cnt;        // 本次收到的电调路数 (0~20)
    int16_t   value[20];      // 每路电调值 [-8192=反向最大, 0=停止, 8191=正向最大]
    bool      updated;        // true=有新数据  false=已读取
} DroneCan_EscCmd_t;           // 飞控发来的电调控制命令


// ============================================================
// 心跳 NodeStatus (ID=341): 每个节点定期 1Hz 广播自身状态
// ============================================================
typedef struct {
    uint32_t  uptime_sec;     // 节点累计运行时间 (秒)
    uint8_t   health;         // 健康状态: 0=OK 1=Warning 2=Error 3=Critical
    uint8_t   mode;           // 运行模式: 0=正常运行 1=初始化 2=维护 3=固件升级
    uint8_t   sub_mode;       // 子模式 (预留)
    uint16_t  vendor_status;  // 厂商自定义状态码
    bool      updated;        // true=有新数据  false=已读取
} DroneCan_Heartbeat_t;       // 飞控发来的心跳包


// ============================================================
// 电调状态 Status (ID=1034): 电调反馈的电压/电流/温度/转速
// ============================================================
typedef struct {
    float     voltage;        // 输入电压 (V)
    float     current;        // 电流 (A)
    float     temperature;    // 温度 (°C)
    int32_t   rpm;            // 转速 (转/分钟)
    uint8_t   power_rating_pct;// 功率百分比 (0~127, 127=100%)
    uint8_t   esc_index;      // 电调编号 (0~255)
    bool      updated;        // true=有新数据  false=已读取
} DroneCan_EscStatus_t;       // 电调上报的运行状态


// ===== 全局数据 =====
extern DroneCan_Servo_t      drone_servo;       // 最新收到的舵机命令
extern DroneCan_EscCmd_t     drone_esc_cmd;     // 最新收到的电调命令
extern DroneCan_Heartbeat_t  drone_heartbeat;   // 最新收到的心跳
extern DroneCan_EscStatus_t  drone_esc_status;  // 最新收到的电调状态

// ===== 解析入口（在 on_transfer_received 回调中调用）=====
void DroneCan_Parse(CanardRxTransfer* transfer);
