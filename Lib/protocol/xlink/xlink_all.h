#pragma once

#include "xlink_common.h"
#include "xlink_config.h"

typedef enum
{
    XLINK_MSG_ID_HEARTBEAT,      // 心跳消息
    XLINK_MSG_ID_IMU,            // IMU数据
    XLINK_MSG_ID_ATTITUDE,       // 姿态数据
    XLINK_MSG_ID_QUATERNION,     // 四元数数据
    XLINK_MSG_ID_TEXT,           // 文本
    XLINK_MSG_ID_CLI,            // 命令行接口
    XLINK_MSG_ID_MOTOR_CMD,      // 电机命令
    XLINK_MSG_ID_MOTOR_CTRL,     // 电机控制
    XLINK_MSG_ID_MOTOR_DATA,     // 电机数据
    XLINK_MSG_ID_JOY_STICK,      // 摇杆数据
    XLINK_MSG_ID_PARAM_SET,      // 参数设置
    XLINK_MSG_ID_PARAM_GET,      // 参数请求
    XLINK_MSG_ID_PARAM_VALUE,    // 参数值
    XLINK_MSG_ID_PARAM_LIST_GET, // 参数列表请求
    XLINK_MSG_ID_PARAM_INFO,     // 参数信息
} XLINK_MSG_ID_TYPE;

typedef enum
{
    XLINK_MOTOR_MODE_NONE,     // 无模式
    XLINK_MOTOR_MODE_TORQUE,   // 力矩模式
    XLINK_MOTOR_MODE_VELOCITY, // 立即速度模式
    XLINK_MOTOR_MODE_POSITION, // 立即位置模式
} XLINK_MOTOR_MODE_TYPE;

typedef enum
{
    XLINK_MOTOR_CMD_ID_NONE,                     // 无命令
    XLINK_MOTOR_CMD_ID_REBOOT,                   // 重启命令
    XLINK_MOTOR_CMD_ID_START,                    // 启动命令
    XLINK_MOTOR_CMD_ID_PAUSE,                    // 暂停命令
    XLINK_MOTOR_CMD_ID_STOP,                     // 停止命令
    XLINK_MOTOR_CMD_ID_JOY_STICK_DISABLE = 0x10, // 关闭摇杆控制
    XLINK_MOTOR_CMD_ID_JOY_STICK_ENABLE = 0x11,  // 开启摇杆控制
    XLINK_MOTOR_CMD_ID_SET_ZERO = 0x20,          // 设置零位
    XLINK_MOTOR_CMD_ID_SAVE_ZERO = 0x21,         // 保存零位
    XLINK_MOTOR_CMD_ID_RECOVER_ZERO = 0x22,      // 恢复零位
    XLINK_MOTOR_CMD_ID_RECOVER_PARAM = 0x66,     // 恢复默认参数
} XLINK_MOTOR_CMD_ID_TYPE;

// 心跳消息
typedef __PACKED_STRUCT
{
    uint8_t board_id;   // 板子ID
    uint8_t sys_status; // 系统状态
    uint32_t timestamp; // 时间戳, 单位: ms
}
xlink_heartbeat_t;

// IMU数据
typedef __PACKED_STRUCT
{
    float acc_x;  // X轴加速度, 单位: m/s^2
    float acc_y;  // Y轴加速度, 单位: m/s^2
    float acc_z;  // Z轴加速度, 单位: m/s^2
    float gyro_x; // X轴角速度, 单位: °/s
    float gyro_y; // Y轴角速度, 单位: °/s
    float gyro_z; // Z轴角速度, 单位: °/s
}
xlink_imu_t;

// 姿态数据
typedef __PACKED_STRUCT
{
    float roll;  // 滚转角度, 单位: °
    float pitch; // 俯仰角度, 单位: °
    float yaw;   // 方位角度, 单位: °
}
xlink_attitude_t;

// 四元数数据
typedef __PACKED_STRUCT
{
    float q0; // q0
    float q1; // q1
    float q2; // q2
    float q3; // q3
}
xlink_quaternion_t;

// 文本
typedef __PACKED_STRUCT
{
    uint8_t level; // 等级
    char text[64]; // 文本内容
}
xlink_text_t;

// 命令行接口
typedef __PACKED_STRUCT
{
    uint8_t cli_id;    // 命令行接口ID
    char cli_text[32]; // 命令行文本
}
xlink_cli_t;

// 电机命令
typedef __PACKED_STRUCT
{
    uint8_t cmd_id; // 命令ID
    float param[2]; // 参数
}
xlink_motor_cmd_t;

// 电机控制
typedef __PACKED_STRUCT
{
    uint8_t mode; // 控制模式, 1: 力矩控制, 2: 位置控制, 3: 速度控制
    uint8_t flag; // 控制标志
    float value;  // 控制值
}
xlink_motor_ctrl_t;

// 电机数据
typedef __PACKED_STRUCT
{
    uint8_t status; // 状态
    float current;  // 电流
    float speed;    // 速度
    float theta;    // 角度
}
xlink_motor_data_t;

// 摇杆数据
typedef __PACKED_STRUCT
{
    float x;      // X轴, 范围: [-1.0, 1.0]
    float y;      // Y轴, 范围: [-1.0, 1.0]
    float z;      // Z轴, 范围: [-1.0, 1.0]
    float rz;     // RZ轴, 范围: [-1.0, 1.0]
    uint16_t key; // 按键
}
xlink_joy_stick_t;

// 参数设置
typedef __PACKED_STRUCT
{
    uint8_t param_id;  // 参数ID
    float param_value; // 参数值
}
xlink_param_set_t;

// 参数请求
typedef __PACKED_STRUCT
{
    uint8_t param_id; // 参数ID, 范围: 0-255, 255表示请求所有参数
}
xlink_param_get_t;

// 参数值
typedef __PACKED_STRUCT
{
    uint8_t param_id;  // 参数ID
    float param_value; // 参数值
}
xlink_param_value_t;

// 参数列表请求
typedef __PACKED_STRUCT
{
    uint8_t param_id_start; // 参数ID起始
    uint8_t param_id_end;   // 参数ID结束, 0表示到最后一个参数
}
xlink_param_list_get_t;

// 参数信息
typedef __PACKED_STRUCT
{
    uint8_t param_id;    // 参数ID
    char param_name[32]; // 参数名称
    uint8_t param_type;  // 参数类型
    float param_min;     // 参数最小值
    float param_max;     // 参数最大值
    float param_default; // 参数默认值
}
xlink_param_info_t;

static inline bool xlink_heartbeat_decode(xlink_msg_t *msg, xlink_heartbeat_t *heartbeat)
{
    if (msg == NULL)
    {
        return false;
    }
    if ((msg->msg_id != XLINK_MSG_ID_HEARTBEAT) ||
        (msg->data_len != sizeof(xlink_heartbeat_t)))
    {
        return false;
    }
    if (heartbeat != NULL)
    {
        memcpy(heartbeat, msg->data, sizeof(xlink_heartbeat_t));
    }
    return true;
}

static inline bool xlink_imu_decode(xlink_msg_t *msg, xlink_imu_t *imu)
{
    if (msg == NULL)
    {
        return false;
    }
    if ((msg->msg_id != XLINK_MSG_ID_IMU) ||
        (msg->data_len != sizeof(xlink_imu_t)))
    {
        return false;
    }
    if (imu != NULL)
    {
        memcpy(imu, msg->data, sizeof(xlink_imu_t));
    }
    return true;
}

static inline bool xlink_attitude_decode(xlink_msg_t *msg, xlink_attitude_t *attitude)
{
    if (msg == NULL)
    {
        return false;
    }
    if ((msg->msg_id != XLINK_MSG_ID_ATTITUDE) ||
        (msg->data_len != sizeof(xlink_attitude_t)))
    {
        return false;
    }
    if (attitude != NULL)
    {
        memcpy(attitude, msg->data, sizeof(xlink_attitude_t));
    }
    return true;
}

static inline bool xlink_quaternion_decode(xlink_msg_t *msg, xlink_quaternion_t *quaternion)
{
    if (msg == NULL)
    {
        return false;
    }
    if ((msg->msg_id != XLINK_MSG_ID_QUATERNION) ||
        (msg->data_len != sizeof(xlink_quaternion_t)))
    {
        return false;
    }
    if (quaternion != NULL)
    {
        memcpy(quaternion, msg->data, sizeof(xlink_quaternion_t));
    }
    return true;
}

static inline bool xlink_text_decode(xlink_msg_t *msg, xlink_text_t *text)
{
    if (msg == NULL)
    {
        return false;
    }
    if ((msg->msg_id != XLINK_MSG_ID_TEXT) ||
        (msg->data_len != sizeof(xlink_text_t)))
    {
        return false;
    }
    if (text != NULL)
    {
        memcpy(text, msg->data, sizeof(xlink_text_t));
    }
    return true;
}

static inline bool xlink_cli_decode(xlink_msg_t *msg, xlink_cli_t *cli)
{
    if (msg == NULL)
    {
        return false;
    }
    if ((msg->msg_id != XLINK_MSG_ID_CLI) ||
        (msg->data_len != sizeof(xlink_cli_t)))
    {
        return false;
    }
    if (cli != NULL)
    {
        memcpy(cli, msg->data, sizeof(xlink_cli_t));
    }
    return true;
}

static inline bool xlink_motor_cmd_decode(xlink_msg_t *msg, xlink_motor_cmd_t *motor_cmd)
{
    if (msg == NULL)
    {
        return false;
    }
    if ((msg->msg_id != XLINK_MSG_ID_MOTOR_CMD) ||
        (msg->data_len != sizeof(xlink_motor_cmd_t)))
    {
        return false;
    }
    if (motor_cmd != NULL)
    {
        memcpy(motor_cmd, msg->data, sizeof(xlink_motor_cmd_t));
    }
    return true;
}

static inline bool xlink_motor_ctrl_decode(xlink_msg_t *msg, xlink_motor_ctrl_t *motor_ctrl)
{
    if (msg == NULL)
    {
        return false;
    }
    if ((msg->msg_id != XLINK_MSG_ID_MOTOR_CTRL) ||
        (msg->data_len != sizeof(xlink_motor_ctrl_t)))
    {
        return false;
    }
    if (motor_ctrl != NULL)
    {
        memcpy(motor_ctrl, msg->data, sizeof(xlink_motor_ctrl_t));
    }
    return true;
}

static inline bool xlink_motor_data_decode(xlink_msg_t *msg, xlink_motor_data_t *motor_data)
{
    if (msg == NULL)
    {
        return false;
    }
    if ((msg->msg_id != XLINK_MSG_ID_MOTOR_DATA) ||
        (msg->data_len != sizeof(xlink_motor_data_t)))
    {
        return false;
    }
    if (motor_data != NULL)
    {
        memcpy(motor_data, msg->data, sizeof(xlink_motor_data_t));
    }
    return true;
}

static inline bool xlink_joy_stick_decode(xlink_msg_t *msg, xlink_joy_stick_t *joy_stick)
{
    if (msg == NULL)
    {
        return false;
    }
    if ((msg->msg_id != XLINK_MSG_ID_JOY_STICK) ||
        (msg->data_len != sizeof(xlink_joy_stick_t)))
    {
        return false;
    }
    if (joy_stick != NULL)
    {
        memcpy(joy_stick, msg->data, sizeof(xlink_joy_stick_t));
    }
    return true;
}

static inline bool xlink_param_set_decode(xlink_msg_t *msg, xlink_param_set_t *param_set)
{
    if (msg == NULL)
    {
        return false;
    }
    if ((msg->msg_id != XLINK_MSG_ID_PARAM_SET) ||
        (msg->data_len != sizeof(xlink_param_set_t)))
    {
        return false;
    }
    if (param_set != NULL)
    {
        memcpy(param_set, msg->data, sizeof(xlink_param_set_t));
    }
    return true;
}

static inline bool xlink_param_get_decode(xlink_msg_t *msg, xlink_param_get_t *param_get)
{
    if (msg == NULL)
    {
        return false;
    }
    if ((msg->msg_id != XLINK_MSG_ID_PARAM_GET) ||
        (msg->data_len != sizeof(xlink_param_get_t)))
    {
        return false;
    }
    if (param_get != NULL)
    {
        memcpy(param_get, msg->data, sizeof(xlink_param_get_t));
    }
    return true;
}

static inline bool xlink_param_value_decode(xlink_msg_t *msg, xlink_param_value_t *param_value)
{
    if (msg == NULL)
    {
        return false;
    }
    if ((msg->msg_id != XLINK_MSG_ID_PARAM_VALUE) ||
        (msg->data_len != sizeof(xlink_param_value_t)))
    {
        return false;
    }
    if (param_value != NULL)
    {
        memcpy(param_value, msg->data, sizeof(xlink_param_value_t));
    }
    return true;
}

static inline bool xlink_param_list_get_decode(xlink_msg_t *msg, xlink_param_list_get_t *param_list_get)
{
    if (msg == NULL)
    {
        return false;
    }
    if ((msg->msg_id != XLINK_MSG_ID_PARAM_LIST_GET) ||
        (msg->data_len != sizeof(xlink_param_list_get_t)))
    {
        return false;
    }
    if (param_list_get != NULL)
    {
        memcpy(param_list_get, msg->data, sizeof(xlink_param_list_get_t));
    }
    return true;
}

static inline bool xlink_param_info_decode(xlink_msg_t *msg, xlink_param_info_t *param_info)
{
    if (msg == NULL)
    {
        return false;
    }
    if ((msg->msg_id != XLINK_MSG_ID_PARAM_INFO) ||
        (msg->data_len != sizeof(xlink_param_info_t)))
    {
        return false;
    }
    if (param_info != NULL)
    {
        memcpy(param_info, msg->data, sizeof(xlink_param_info_t));
    }
    return true;
}

static inline bool xlink_heartbeat_encode(xlink_heartbeat_t *heartbeat, xlink_msg_t *msg)
{
    if (msg == NULL || heartbeat == NULL)
    {
        return false;
    }
    msg->head = XLINK_MSG_HEAD;
    msg->msg_id = XLINK_MSG_ID_HEARTBEAT;
    msg->data_len = sizeof(xlink_heartbeat_t);
    memcpy(msg->data, heartbeat, msg->data_len);
    msg->crc = _xlink_crc_calc((uint8_t *)msg + sizeof(msg->head), msg->data_len + offsetof(xlink_msg_t, data) - sizeof(msg->head), 0x00);
    msg->data[msg->data_len] = msg->crc;
    return true;
}

static inline bool xlink_imu_encode(xlink_imu_t *imu, xlink_msg_t *msg)
{
    if (msg == NULL || imu == NULL)
    {
        return false;
    }
    msg->head = XLINK_MSG_HEAD;
    msg->msg_id = XLINK_MSG_ID_IMU;
    msg->data_len = sizeof(xlink_imu_t);
    memcpy(msg->data, imu, msg->data_len);
    msg->crc = _xlink_crc_calc((uint8_t *)msg + sizeof(msg->head), msg->data_len + offsetof(xlink_msg_t, data) - sizeof(msg->head), 0x00);
    msg->data[msg->data_len] = msg->crc;
    return true;
}

static inline bool xlink_attitude_encode(xlink_attitude_t *attitude, xlink_msg_t *msg)
{
    if (msg == NULL || attitude == NULL)
    {
        return false;
    }
    msg->head = XLINK_MSG_HEAD;
    msg->msg_id = XLINK_MSG_ID_ATTITUDE;
    msg->data_len = sizeof(xlink_attitude_t);
    memcpy(msg->data, attitude, msg->data_len);
    msg->crc = _xlink_crc_calc((uint8_t *)msg + sizeof(msg->head), msg->data_len + offsetof(xlink_msg_t, data) - sizeof(msg->head), 0x00);
    msg->data[msg->data_len] = msg->crc;
    return true;
}

static inline bool xlink_quaternion_encode(xlink_quaternion_t *quaternion, xlink_msg_t *msg)
{
    if (msg == NULL || quaternion == NULL)
    {
        return false;
    }
    msg->head = XLINK_MSG_HEAD;
    msg->msg_id = XLINK_MSG_ID_QUATERNION;
    msg->data_len = sizeof(xlink_quaternion_t);
    memcpy(msg->data, quaternion, msg->data_len);
    msg->crc = _xlink_crc_calc((uint8_t *)msg + sizeof(msg->head), msg->data_len + offsetof(xlink_msg_t, data) - sizeof(msg->head), 0x00);
    msg->data[msg->data_len] = msg->crc;
    return true;
}

static inline bool xlink_text_encode(xlink_text_t *text, xlink_msg_t *msg)
{
    if (msg == NULL || text == NULL)
    {
        return false;
    }
    msg->head = XLINK_MSG_HEAD;
    msg->msg_id = XLINK_MSG_ID_TEXT;
    msg->data_len = sizeof(xlink_text_t);
    memcpy(msg->data, text, msg->data_len);
    msg->crc = _xlink_crc_calc((uint8_t *)msg + sizeof(msg->head), msg->data_len + offsetof(xlink_msg_t, data) - sizeof(msg->head), 0x00);
    msg->data[msg->data_len] = msg->crc;
    return true;
}

static inline bool xlink_cli_encode(xlink_cli_t *cli, xlink_msg_t *msg)
{
    if (msg == NULL || cli == NULL)
    {
        return false;
    }
    msg->head = XLINK_MSG_HEAD;
    msg->msg_id = XLINK_MSG_ID_CLI;
    msg->data_len = sizeof(xlink_cli_t);
    memcpy(msg->data, cli, msg->data_len);
    msg->crc = _xlink_crc_calc((uint8_t *)msg + sizeof(msg->head), msg->data_len + offsetof(xlink_msg_t, data) - sizeof(msg->head), 0x00);
    msg->data[msg->data_len] = msg->crc;
    return true;
}

static inline bool xlink_motor_cmd_encode(xlink_motor_cmd_t *motor_cmd, xlink_msg_t *msg)
{
    if (msg == NULL || motor_cmd == NULL)
    {
        return false;
    }
    msg->head = XLINK_MSG_HEAD;
    msg->msg_id = XLINK_MSG_ID_MOTOR_CMD;
    msg->data_len = sizeof(xlink_motor_cmd_t);
    memcpy(msg->data, motor_cmd, msg->data_len);
    msg->crc = _xlink_crc_calc((uint8_t *)msg + sizeof(msg->head), msg->data_len + offsetof(xlink_msg_t, data) - sizeof(msg->head), 0x00);
    msg->data[msg->data_len] = msg->crc;
    return true;
}

static inline bool xlink_motor_ctrl_encode(xlink_motor_ctrl_t *motor_ctrl, xlink_msg_t *msg)
{
    if (msg == NULL || motor_ctrl == NULL)
    {
        return false;
    }
    msg->head = XLINK_MSG_HEAD;
    msg->msg_id = XLINK_MSG_ID_MOTOR_CTRL;
    msg->data_len = sizeof(xlink_motor_ctrl_t);
    memcpy(msg->data, motor_ctrl, msg->data_len);
    msg->crc = _xlink_crc_calc((uint8_t *)msg + sizeof(msg->head), msg->data_len + offsetof(xlink_msg_t, data) - sizeof(msg->head), 0x00);
    msg->data[msg->data_len] = msg->crc;
    return true;
}

static inline bool xlink_motor_data_encode(xlink_motor_data_t *motor_data, xlink_msg_t *msg)
{
    if (msg == NULL || motor_data == NULL)
    {
        return false;
    }
    msg->head = XLINK_MSG_HEAD;
    msg->msg_id = XLINK_MSG_ID_MOTOR_DATA;
    msg->data_len = sizeof(xlink_motor_data_t);
    memcpy(msg->data, motor_data, msg->data_len);
    msg->crc = _xlink_crc_calc((uint8_t *)msg + sizeof(msg->head), msg->data_len + offsetof(xlink_msg_t, data) - sizeof(msg->head), 0x00);
    msg->data[msg->data_len] = msg->crc;
    return true;
}

static inline bool xlink_joy_stick_encode(xlink_joy_stick_t *joy_stick, xlink_msg_t *msg)
{
    if (msg == NULL || joy_stick == NULL)
    {
        return false;
    }
    msg->head = XLINK_MSG_HEAD;
    msg->msg_id = XLINK_MSG_ID_JOY_STICK;
    msg->data_len = sizeof(xlink_joy_stick_t);
    memcpy(msg->data, joy_stick, msg->data_len);
    msg->crc = _xlink_crc_calc((uint8_t *)msg + sizeof(msg->head), msg->data_len + offsetof(xlink_msg_t, data) - sizeof(msg->head), 0x00);
    msg->data[msg->data_len] = msg->crc;
    return true;
}

static inline bool xlink_param_set_encode(xlink_param_set_t *param_set, xlink_msg_t *msg)
{
    if (msg == NULL || param_set == NULL)
    {
        return false;
    }
    msg->head = XLINK_MSG_HEAD;
    msg->msg_id = XLINK_MSG_ID_PARAM_SET;
    msg->data_len = sizeof(xlink_param_set_t);
    memcpy(msg->data, param_set, msg->data_len);
    msg->crc = _xlink_crc_calc((uint8_t *)msg + sizeof(msg->head), msg->data_len + offsetof(xlink_msg_t, data) - sizeof(msg->head), 0x00);
    msg->data[msg->data_len] = msg->crc;
    return true;
}

static inline bool xlink_param_get_encode(xlink_param_get_t *param_get, xlink_msg_t *msg)
{
    if (msg == NULL || param_get == NULL)
    {
        return false;
    }
    msg->head = XLINK_MSG_HEAD;
    msg->msg_id = XLINK_MSG_ID_PARAM_GET;
    msg->data_len = sizeof(xlink_param_get_t);
    memcpy(msg->data, param_get, msg->data_len);
    msg->crc = _xlink_crc_calc((uint8_t *)msg + sizeof(msg->head), msg->data_len + offsetof(xlink_msg_t, data) - sizeof(msg->head), 0x00);
    msg->data[msg->data_len] = msg->crc;
    return true;
}

static inline bool xlink_param_value_encode(xlink_param_value_t *param_value, xlink_msg_t *msg)
{
    if (msg == NULL || param_value == NULL)
    {
        return false;
    }
    msg->head = XLINK_MSG_HEAD;
    msg->msg_id = XLINK_MSG_ID_PARAM_VALUE;
    msg->data_len = sizeof(xlink_param_value_t);
    memcpy(msg->data, param_value, msg->data_len);
    msg->crc = _xlink_crc_calc((uint8_t *)msg + sizeof(msg->head), msg->data_len + offsetof(xlink_msg_t, data) - sizeof(msg->head), 0x00);
    msg->data[msg->data_len] = msg->crc;
    return true;
}

static inline bool xlink_param_list_get_encode(xlink_param_list_get_t *param_list_get, xlink_msg_t *msg)
{
    if (msg == NULL || param_list_get == NULL)
    {
        return false;
    }
    msg->head = XLINK_MSG_HEAD;
    msg->msg_id = XLINK_MSG_ID_PARAM_LIST_GET;
    msg->data_len = sizeof(xlink_param_list_get_t);
    memcpy(msg->data, param_list_get, msg->data_len);
    msg->crc = _xlink_crc_calc((uint8_t *)msg + sizeof(msg->head), msg->data_len + offsetof(xlink_msg_t, data) - sizeof(msg->head), 0x00);
    msg->data[msg->data_len] = msg->crc;
    return true;
}

static inline bool xlink_param_info_encode(xlink_param_info_t *param_info, xlink_msg_t *msg)
{
    if (msg == NULL || param_info == NULL)
    {
        return false;
    }
    msg->head = XLINK_MSG_HEAD;
    msg->msg_id = XLINK_MSG_ID_PARAM_INFO;
    msg->data_len = sizeof(xlink_param_info_t);
    memcpy(msg->data, param_info, msg->data_len);
    msg->crc = _xlink_crc_calc((uint8_t *)msg + sizeof(msg->head), msg->data_len + offsetof(xlink_msg_t, data) - sizeof(msg->head), 0x00);
    msg->data[msg->data_len] = msg->crc;
    return true;
}

static inline uint8_t xlink_heartbeat_send(uint8_t channel, uint8_t target_system_id, uint8_t target_component_id, xlink_heartbeat_t *heartbeat)
{
    if ((channel >= XLINK_CHANNEL_MAX) || heartbeat == NULL)
    {
        return 0;
    }
    if (xlink_get_write_func(channel) == NULL)
    {
        return 0;
    }
    xlink_msg_t msg;
    msg.source_system_id = XLINK_SELF_SYSTEM_ID;
    msg.source_component_id = XLINK_SELF_COMPONENT_ID;
    msg.target_system_id = target_system_id;
    msg.target_component_id = target_component_id;
    uint8_t *buffer = (uint8_t *)&msg;
    xlink_heartbeat_encode(heartbeat, &msg);
    uint8_t size = xlink_get_msg_length(&msg);
    buffer[size - 1] = msg.crc;

    return xlink_get_write_func(channel)(buffer, size);
}

static inline uint8_t xlink_imu_send(uint8_t channel, uint8_t target_system_id, uint8_t target_component_id, xlink_imu_t *imu)
{
    if ((channel >= XLINK_CHANNEL_MAX) || imu == NULL)
    {
        return 0;
    }
    if (xlink_get_write_func(channel) == NULL)
    {
        return 0;
    }
    xlink_msg_t msg;
    msg.source_system_id = XLINK_SELF_SYSTEM_ID;
    msg.source_component_id = XLINK_SELF_COMPONENT_ID;
    msg.target_system_id = target_system_id;
    msg.target_component_id = target_component_id;
    uint8_t *buffer = (uint8_t *)&msg;
    xlink_imu_encode(imu, &msg);
    uint8_t size = xlink_get_msg_length(&msg);
    buffer[size - 1] = msg.crc;

    return xlink_get_write_func(channel)(buffer, size);
}

static inline uint8_t xlink_attitude_send(uint8_t channel, uint8_t target_system_id, uint8_t target_component_id, xlink_attitude_t *attitude)
{
    if ((channel >= XLINK_CHANNEL_MAX) || attitude == NULL)
    {
        return 0;
    }
    if (xlink_get_write_func(channel) == NULL)
    {
        return 0;
    }
    xlink_msg_t msg;
    msg.source_system_id = XLINK_SELF_SYSTEM_ID;
    msg.source_component_id = XLINK_SELF_COMPONENT_ID;
    msg.target_system_id = target_system_id;
    msg.target_component_id = target_component_id;
    uint8_t *buffer = (uint8_t *)&msg;
    xlink_attitude_encode(attitude, &msg);
    uint8_t size = xlink_get_msg_length(&msg);
    buffer[size - 1] = msg.crc;

    return xlink_get_write_func(channel)(buffer, size);
}

static inline uint8_t xlink_quaternion_send(uint8_t channel, uint8_t target_system_id, uint8_t target_component_id, xlink_quaternion_t *quaternion)
{
    if ((channel >= XLINK_CHANNEL_MAX) || quaternion == NULL)
    {
        return 0;
    }
    if (xlink_get_write_func(channel) == NULL)
    {
        return 0;
    }
    xlink_msg_t msg;
    msg.source_system_id = XLINK_SELF_SYSTEM_ID;
    msg.source_component_id = XLINK_SELF_COMPONENT_ID;
    msg.target_system_id = target_system_id;
    msg.target_component_id = target_component_id;
    uint8_t *buffer = (uint8_t *)&msg;
    xlink_quaternion_encode(quaternion, &msg);
    uint8_t size = xlink_get_msg_length(&msg);
    buffer[size - 1] = msg.crc;

    return xlink_get_write_func(channel)(buffer, size);
}

static inline uint8_t xlink_text_send(uint8_t channel, uint8_t target_system_id, uint8_t target_component_id, xlink_text_t *text)
{
    if ((channel >= XLINK_CHANNEL_MAX) || text == NULL)
    {
        return 0;
    }
    if (xlink_get_write_func(channel) == NULL)
    {
        return 0;
    }
    xlink_msg_t msg;
    msg.source_system_id = XLINK_SELF_SYSTEM_ID;
    msg.source_component_id = XLINK_SELF_COMPONENT_ID;
    msg.target_system_id = target_system_id;
    msg.target_component_id = target_component_id;
    uint8_t *buffer = (uint8_t *)&msg;
    xlink_text_encode(text, &msg);
    uint8_t size = xlink_get_msg_length(&msg);
    buffer[size - 1] = msg.crc;

    return xlink_get_write_func(channel)(buffer, size);
}

static inline uint8_t xlink_cli_send(uint8_t channel, uint8_t target_system_id, uint8_t target_component_id, xlink_cli_t *cli)
{
    if ((channel >= XLINK_CHANNEL_MAX) || cli == NULL)
    {
        return 0;
    }
    if (xlink_get_write_func(channel) == NULL)
    {
        return 0;
    }
    xlink_msg_t msg;
    msg.source_system_id = XLINK_SELF_SYSTEM_ID;
    msg.source_component_id = XLINK_SELF_COMPONENT_ID;
    msg.target_system_id = target_system_id;
    msg.target_component_id = target_component_id;
    uint8_t *buffer = (uint8_t *)&msg;
    xlink_cli_encode(cli, &msg);
    uint8_t size = xlink_get_msg_length(&msg);
    buffer[size - 1] = msg.crc;

    return xlink_get_write_func(channel)(buffer, size);
}

static inline uint8_t xlink_motor_cmd_send(uint8_t channel, uint8_t target_system_id, uint8_t target_component_id, xlink_motor_cmd_t *motor_cmd)
{
    if ((channel >= XLINK_CHANNEL_MAX) || motor_cmd == NULL)
    {
        return 0;
    }
    if (xlink_get_write_func(channel) == NULL)
    {
        return 0;
    }
    xlink_msg_t msg;
    msg.source_system_id = XLINK_SELF_SYSTEM_ID;
    msg.source_component_id = XLINK_SELF_COMPONENT_ID;
    msg.target_system_id = target_system_id;
    msg.target_component_id = target_component_id;
    uint8_t *buffer = (uint8_t *)&msg;
    xlink_motor_cmd_encode(motor_cmd, &msg);
    uint8_t size = xlink_get_msg_length(&msg);
    buffer[size - 1] = msg.crc;

    return xlink_get_write_func(channel)(buffer, size);
}

static inline uint8_t xlink_motor_ctrl_send(uint8_t channel, uint8_t target_system_id, uint8_t target_component_id, xlink_motor_ctrl_t *motor_ctrl)
{
    if ((channel >= XLINK_CHANNEL_MAX) || motor_ctrl == NULL)
    {
        return 0;
    }
    if (xlink_get_write_func(channel) == NULL)
    {
        return 0;
    }
    xlink_msg_t msg;
    msg.source_system_id = XLINK_SELF_SYSTEM_ID;
    msg.source_component_id = XLINK_SELF_COMPONENT_ID;
    msg.target_system_id = target_system_id;
    msg.target_component_id = target_component_id;
    uint8_t *buffer = (uint8_t *)&msg;
    xlink_motor_ctrl_encode(motor_ctrl, &msg);
    uint8_t size = xlink_get_msg_length(&msg);
    buffer[size - 1] = msg.crc;

    return xlink_get_write_func(channel)(buffer, size);
}

static inline uint8_t xlink_motor_data_send(uint8_t channel, uint8_t target_system_id, uint8_t target_component_id, xlink_motor_data_t *motor_data)
{
    if ((channel >= XLINK_CHANNEL_MAX) || motor_data == NULL)
    {
        return 0;
    }
    if (xlink_get_write_func(channel) == NULL)
    {
        return 0;
    }
    xlink_msg_t msg;
    msg.source_system_id = XLINK_SELF_SYSTEM_ID;
    msg.source_component_id = XLINK_SELF_COMPONENT_ID;
    msg.target_system_id = target_system_id;
    msg.target_component_id = target_component_id;
    uint8_t *buffer = (uint8_t *)&msg;
    xlink_motor_data_encode(motor_data, &msg);
    uint8_t size = xlink_get_msg_length(&msg);
    buffer[size - 1] = msg.crc;

    return xlink_get_write_func(channel)(buffer, size);
}

static inline uint8_t xlink_joy_stick_send(uint8_t channel, uint8_t target_system_id, uint8_t target_component_id, xlink_joy_stick_t *joy_stick)
{
    if ((channel >= XLINK_CHANNEL_MAX) || joy_stick == NULL)
    {
        return 0;
    }
    if (xlink_get_write_func(channel) == NULL)
    {
        return 0;
    }
    xlink_msg_t msg;
    msg.source_system_id = XLINK_SELF_SYSTEM_ID;
    msg.source_component_id = XLINK_SELF_COMPONENT_ID;
    msg.target_system_id = target_system_id;
    msg.target_component_id = target_component_id;
    uint8_t *buffer = (uint8_t *)&msg;
    xlink_joy_stick_encode(joy_stick, &msg);
    uint8_t size = xlink_get_msg_length(&msg);
    buffer[size - 1] = msg.crc;

    return xlink_get_write_func(channel)(buffer, size);
}

static inline uint8_t xlink_param_set_send(uint8_t channel, uint8_t target_system_id, uint8_t target_component_id, xlink_param_set_t *param_set)
{
    if ((channel >= XLINK_CHANNEL_MAX) || param_set == NULL)
    {
        return 0;
    }
    if (xlink_get_write_func(channel) == NULL)
    {
        return 0;
    }
    xlink_msg_t msg;
    msg.source_system_id = XLINK_SELF_SYSTEM_ID;
    msg.source_component_id = XLINK_SELF_COMPONENT_ID;
    msg.target_system_id = target_system_id;
    msg.target_component_id = target_component_id;
    uint8_t *buffer = (uint8_t *)&msg;
    xlink_param_set_encode(param_set, &msg);
    uint8_t size = xlink_get_msg_length(&msg);
    buffer[size - 1] = msg.crc;

    return xlink_get_write_func(channel)(buffer, size);
}

static inline uint8_t xlink_param_get_send(uint8_t channel, uint8_t target_system_id, uint8_t target_component_id, xlink_param_get_t *param_get)
{
    if ((channel >= XLINK_CHANNEL_MAX) || param_get == NULL)
    {
        return 0;
    }
    if (xlink_get_write_func(channel) == NULL)
    {
        return 0;
    }
    xlink_msg_t msg;
    msg.source_system_id = XLINK_SELF_SYSTEM_ID;
    msg.source_component_id = XLINK_SELF_COMPONENT_ID;
    msg.target_system_id = target_system_id;
    msg.target_component_id = target_component_id;
    uint8_t *buffer = (uint8_t *)&msg;
    xlink_param_get_encode(param_get, &msg);
    uint8_t size = xlink_get_msg_length(&msg);
    buffer[size - 1] = msg.crc;

    return xlink_get_write_func(channel)(buffer, size);
}

static inline uint8_t xlink_param_value_send(uint8_t channel, uint8_t target_system_id, uint8_t target_component_id, xlink_param_value_t *param_value)
{
    if ((channel >= XLINK_CHANNEL_MAX) || param_value == NULL)
    {
        return 0;
    }
    if (xlink_get_write_func(channel) == NULL)
    {
        return 0;
    }
    xlink_msg_t msg;
    msg.source_system_id = XLINK_SELF_SYSTEM_ID;
    msg.source_component_id = XLINK_SELF_COMPONENT_ID;
    msg.target_system_id = target_system_id;
    msg.target_component_id = target_component_id;
    uint8_t *buffer = (uint8_t *)&msg;
    xlink_param_value_encode(param_value, &msg);
    uint8_t size = xlink_get_msg_length(&msg);
    buffer[size - 1] = msg.crc;

    return xlink_get_write_func(channel)(buffer, size);
}

static inline uint8_t xlink_param_list_get_send(uint8_t channel, uint8_t target_system_id, uint8_t target_component_id, xlink_param_list_get_t *param_list_get)
{
    if ((channel >= XLINK_CHANNEL_MAX) || param_list_get == NULL)
    {
        return 0;
    }
    if (xlink_get_write_func(channel) == NULL)
    {
        return 0;
    }
    xlink_msg_t msg;
    msg.source_system_id = XLINK_SELF_SYSTEM_ID;
    msg.source_component_id = XLINK_SELF_COMPONENT_ID;
    msg.target_system_id = target_system_id;
    msg.target_component_id = target_component_id;
    uint8_t *buffer = (uint8_t *)&msg;
    xlink_param_list_get_encode(param_list_get, &msg);
    uint8_t size = xlink_get_msg_length(&msg);
    buffer[size - 1] = msg.crc;

    return xlink_get_write_func(channel)(buffer, size);
}

static inline uint8_t xlink_param_info_send(uint8_t channel, uint8_t target_system_id, uint8_t target_component_id, xlink_param_info_t *param_info)
{
    if ((channel >= XLINK_CHANNEL_MAX) || param_info == NULL)
    {
        return 0;
    }
    if (xlink_get_write_func(channel) == NULL)
    {
        return 0;
    }
    xlink_msg_t msg;
    msg.source_system_id = XLINK_SELF_SYSTEM_ID;
    msg.source_component_id = XLINK_SELF_COMPONENT_ID;
    msg.target_system_id = target_system_id;
    msg.target_component_id = target_component_id;
    uint8_t *buffer = (uint8_t *)&msg;
    xlink_param_info_encode(param_info, &msg);
    uint8_t size = xlink_get_msg_length(&msg);
    buffer[size - 1] = msg.crc;

    return xlink_get_write_func(channel)(buffer, size);
}

static inline uint8_t xlink_heartbeat_send_broadcast(uint8_t channel, xlink_heartbeat_t *heartbeat)
{
    if ((channel >= XLINK_CHANNEL_MAX) || heartbeat == NULL)
    {
        return 0;
    }
    if (xlink_get_write_func(channel) == NULL)
    {
        return 0;
    }
    xlink_msg_t msg;
    msg.source_system_id = XLINK_SELF_SYSTEM_ID;
    msg.source_component_id = XLINK_SELF_COMPONENT_ID;
    msg.target_system_id = XLINK_SYSTEM_ID_BROADCAST;
    msg.target_component_id = XLINK_COMPONENT_ID_BROADCAST;
    uint8_t *buffer = (uint8_t *)&msg;
    xlink_heartbeat_encode(heartbeat, &msg);
    uint8_t size = xlink_get_msg_length(&msg);
    buffer[size - 1] = msg.crc;

    return xlink_get_write_func(channel)(buffer, size);
}

static inline uint8_t xlink_imu_send_broadcast(uint8_t channel, xlink_imu_t *imu)
{
    if ((channel >= XLINK_CHANNEL_MAX) || imu == NULL)
    {
        return 0;
    }
    if (xlink_get_write_func(channel) == NULL)
    {
        return 0;
    }
    xlink_msg_t msg;
    msg.source_system_id = XLINK_SELF_SYSTEM_ID;
    msg.source_component_id = XLINK_SELF_COMPONENT_ID;
    msg.target_system_id = XLINK_SYSTEM_ID_BROADCAST;
    msg.target_component_id = XLINK_COMPONENT_ID_BROADCAST;
    uint8_t *buffer = (uint8_t *)&msg;
    xlink_imu_encode(imu, &msg);
    uint8_t size = xlink_get_msg_length(&msg);
    buffer[size - 1] = msg.crc;

    return xlink_get_write_func(channel)(buffer, size);
}

static inline uint8_t xlink_attitude_send_broadcast(uint8_t channel, xlink_attitude_t *attitude)
{
    if ((channel >= XLINK_CHANNEL_MAX) || attitude == NULL)
    {
        return 0;
    }
    if (xlink_get_write_func(channel) == NULL)
    {
        return 0;
    }
    xlink_msg_t msg;
    msg.source_system_id = XLINK_SELF_SYSTEM_ID;
    msg.source_component_id = XLINK_SELF_COMPONENT_ID;
    msg.target_system_id = XLINK_SYSTEM_ID_BROADCAST;
    msg.target_component_id = XLINK_COMPONENT_ID_BROADCAST;
    uint8_t *buffer = (uint8_t *)&msg;
    xlink_attitude_encode(attitude, &msg);
    uint8_t size = xlink_get_msg_length(&msg);
    buffer[size - 1] = msg.crc;

    return xlink_get_write_func(channel)(buffer, size);
}

static inline uint8_t xlink_quaternion_send_broadcast(uint8_t channel, xlink_quaternion_t *quaternion)
{
    if ((channel >= XLINK_CHANNEL_MAX) || quaternion == NULL)
    {
        return 0;
    }
    if (xlink_get_write_func(channel) == NULL)
    {
        return 0;
    }
    xlink_msg_t msg;
    msg.source_system_id = XLINK_SELF_SYSTEM_ID;
    msg.source_component_id = XLINK_SELF_COMPONENT_ID;
    msg.target_system_id = XLINK_SYSTEM_ID_BROADCAST;
    msg.target_component_id = XLINK_COMPONENT_ID_BROADCAST;
    uint8_t *buffer = (uint8_t *)&msg;
    xlink_quaternion_encode(quaternion, &msg);
    uint8_t size = xlink_get_msg_length(&msg);
    buffer[size - 1] = msg.crc;

    return xlink_get_write_func(channel)(buffer, size);
}

static inline uint8_t xlink_text_send_broadcast(uint8_t channel, xlink_text_t *text)
{
    if ((channel >= XLINK_CHANNEL_MAX) || text == NULL)
    {
        return 0;
    }
    if (xlink_get_write_func(channel) == NULL)
    {
        return 0;
    }
    xlink_msg_t msg;
    msg.source_system_id = XLINK_SELF_SYSTEM_ID;
    msg.source_component_id = XLINK_SELF_COMPONENT_ID;
    msg.target_system_id = XLINK_SYSTEM_ID_BROADCAST;
    msg.target_component_id = XLINK_COMPONENT_ID_BROADCAST;
    uint8_t *buffer = (uint8_t *)&msg;
    xlink_text_encode(text, &msg);
    uint8_t size = xlink_get_msg_length(&msg);
    buffer[size - 1] = msg.crc;

    return xlink_get_write_func(channel)(buffer, size);
}

static inline uint8_t xlink_cli_send_broadcast(uint8_t channel, xlink_cli_t *cli)
{
    if ((channel >= XLINK_CHANNEL_MAX) || cli == NULL)
    {
        return 0;
    }
    if (xlink_get_write_func(channel) == NULL)
    {
        return 0;
    }
    xlink_msg_t msg;
    msg.source_system_id = XLINK_SELF_SYSTEM_ID;
    msg.source_component_id = XLINK_SELF_COMPONENT_ID;
    msg.target_system_id = XLINK_SYSTEM_ID_BROADCAST;
    msg.target_component_id = XLINK_COMPONENT_ID_BROADCAST;
    uint8_t *buffer = (uint8_t *)&msg;
    xlink_cli_encode(cli, &msg);
    uint8_t size = xlink_get_msg_length(&msg);
    buffer[size - 1] = msg.crc;

    return xlink_get_write_func(channel)(buffer, size);
}

static inline uint8_t xlink_motor_cmd_send_broadcast(uint8_t channel, xlink_motor_cmd_t *motor_cmd)
{
    if ((channel >= XLINK_CHANNEL_MAX) || motor_cmd == NULL)
    {
        return 0;
    }
    if (xlink_get_write_func(channel) == NULL)
    {
        return 0;
    }
    xlink_msg_t msg;
    msg.source_system_id = XLINK_SELF_SYSTEM_ID;
    msg.source_component_id = XLINK_SELF_COMPONENT_ID;
    msg.target_system_id = XLINK_SYSTEM_ID_BROADCAST;
    msg.target_component_id = XLINK_COMPONENT_ID_BROADCAST;
    uint8_t *buffer = (uint8_t *)&msg;
    xlink_motor_cmd_encode(motor_cmd, &msg);
    uint8_t size = xlink_get_msg_length(&msg);
    buffer[size - 1] = msg.crc;

    return xlink_get_write_func(channel)(buffer, size);
}

static inline uint8_t xlink_motor_ctrl_send_broadcast(uint8_t channel, xlink_motor_ctrl_t *motor_ctrl)
{
    if ((channel >= XLINK_CHANNEL_MAX) || motor_ctrl == NULL)
    {
        return 0;
    }
    if (xlink_get_write_func(channel) == NULL)
    {
        return 0;
    }
    xlink_msg_t msg;
    msg.source_system_id = XLINK_SELF_SYSTEM_ID;
    msg.source_component_id = XLINK_SELF_COMPONENT_ID;
    msg.target_system_id = XLINK_SYSTEM_ID_BROADCAST;
    msg.target_component_id = XLINK_COMPONENT_ID_BROADCAST;
    uint8_t *buffer = (uint8_t *)&msg;
    xlink_motor_ctrl_encode(motor_ctrl, &msg);
    uint8_t size = xlink_get_msg_length(&msg);
    buffer[size - 1] = msg.crc;

    return xlink_get_write_func(channel)(buffer, size);
}

static inline uint8_t xlink_motor_data_send_broadcast(uint8_t channel, xlink_motor_data_t *motor_data)
{
    if ((channel >= XLINK_CHANNEL_MAX) || motor_data == NULL)
    {
        return 0;
    }
    if (xlink_get_write_func(channel) == NULL)
    {
        return 0;
    }
    xlink_msg_t msg;
    msg.source_system_id = XLINK_SELF_SYSTEM_ID;
    msg.source_component_id = XLINK_SELF_COMPONENT_ID;
    msg.target_system_id = XLINK_SYSTEM_ID_BROADCAST;
    msg.target_component_id = XLINK_COMPONENT_ID_BROADCAST;
    uint8_t *buffer = (uint8_t *)&msg;
    xlink_motor_data_encode(motor_data, &msg);
    uint8_t size = xlink_get_msg_length(&msg);
    buffer[size - 1] = msg.crc;

    return xlink_get_write_func(channel)(buffer, size);
}

static inline uint8_t xlink_joy_stick_send_broadcast(uint8_t channel, xlink_joy_stick_t *joy_stick)
{
    if ((channel >= XLINK_CHANNEL_MAX) || joy_stick == NULL)
    {
        return 0;
    }
    if (xlink_get_write_func(channel) == NULL)
    {
        return 0;
    }
    xlink_msg_t msg;
    msg.source_system_id = XLINK_SELF_SYSTEM_ID;
    msg.source_component_id = XLINK_SELF_COMPONENT_ID;
    msg.target_system_id = XLINK_SYSTEM_ID_BROADCAST;
    msg.target_component_id = XLINK_COMPONENT_ID_BROADCAST;
    uint8_t *buffer = (uint8_t *)&msg;
    xlink_joy_stick_encode(joy_stick, &msg);
    uint8_t size = xlink_get_msg_length(&msg);
    buffer[size - 1] = msg.crc;

    return xlink_get_write_func(channel)(buffer, size);
}

static inline uint8_t xlink_param_set_send_broadcast(uint8_t channel, xlink_param_set_t *param_set)
{
    if ((channel >= XLINK_CHANNEL_MAX) || param_set == NULL)
    {
        return 0;
    }
    if (xlink_get_write_func(channel) == NULL)
    {
        return 0;
    }
    xlink_msg_t msg;
    msg.source_system_id = XLINK_SELF_SYSTEM_ID;
    msg.source_component_id = XLINK_SELF_COMPONENT_ID;
    msg.target_system_id = XLINK_SYSTEM_ID_BROADCAST;
    msg.target_component_id = XLINK_COMPONENT_ID_BROADCAST;
    uint8_t *buffer = (uint8_t *)&msg;
    xlink_param_set_encode(param_set, &msg);
    uint8_t size = xlink_get_msg_length(&msg);
    buffer[size - 1] = msg.crc;

    return xlink_get_write_func(channel)(buffer, size);
}

static inline uint8_t xlink_param_get_send_broadcast(uint8_t channel, xlink_param_get_t *param_get)
{
    if ((channel >= XLINK_CHANNEL_MAX) || param_get == NULL)
    {
        return 0;
    }
    if (xlink_get_write_func(channel) == NULL)
    {
        return 0;
    }
    xlink_msg_t msg;
    msg.source_system_id = XLINK_SELF_SYSTEM_ID;
    msg.source_component_id = XLINK_SELF_COMPONENT_ID;
    msg.target_system_id = XLINK_SYSTEM_ID_BROADCAST;
    msg.target_component_id = XLINK_COMPONENT_ID_BROADCAST;
    uint8_t *buffer = (uint8_t *)&msg;
    xlink_param_get_encode(param_get, &msg);
    uint8_t size = xlink_get_msg_length(&msg);
    buffer[size - 1] = msg.crc;

    return xlink_get_write_func(channel)(buffer, size);
}

static inline uint8_t xlink_param_value_send_broadcast(uint8_t channel, xlink_param_value_t *param_value)
{
    if ((channel >= XLINK_CHANNEL_MAX) || param_value == NULL)
    {
        return 0;
    }
    if (xlink_get_write_func(channel) == NULL)
    {
        return 0;
    }
    xlink_msg_t msg;
    msg.source_system_id = XLINK_SELF_SYSTEM_ID;
    msg.source_component_id = XLINK_SELF_COMPONENT_ID;
    msg.target_system_id = XLINK_SYSTEM_ID_BROADCAST;
    msg.target_component_id = XLINK_COMPONENT_ID_BROADCAST;
    uint8_t *buffer = (uint8_t *)&msg;
    xlink_param_value_encode(param_value, &msg);
    uint8_t size = xlink_get_msg_length(&msg);
    buffer[size - 1] = msg.crc;

    return xlink_get_write_func(channel)(buffer, size);
}

static inline uint8_t xlink_param_list_get_send_broadcast(uint8_t channel, xlink_param_list_get_t *param_list_get)
{
    if ((channel >= XLINK_CHANNEL_MAX) || param_list_get == NULL)
    {
        return 0;
    }
    if (xlink_get_write_func(channel) == NULL)
    {
        return 0;
    }
    xlink_msg_t msg;
    msg.source_system_id = XLINK_SELF_SYSTEM_ID;
    msg.source_component_id = XLINK_SELF_COMPONENT_ID;
    msg.target_system_id = XLINK_SYSTEM_ID_BROADCAST;
    msg.target_component_id = XLINK_COMPONENT_ID_BROADCAST;
    uint8_t *buffer = (uint8_t *)&msg;
    xlink_param_list_get_encode(param_list_get, &msg);
    uint8_t size = xlink_get_msg_length(&msg);
    buffer[size - 1] = msg.crc;

    return xlink_get_write_func(channel)(buffer, size);
}

static inline uint8_t xlink_param_info_send_broadcast(uint8_t channel, xlink_param_info_t *param_info)
{
    if ((channel >= XLINK_CHANNEL_MAX) || param_info == NULL)
    {
        return 0;
    }
    if (xlink_get_write_func(channel) == NULL)
    {
        return 0;
    }
    xlink_msg_t msg;
    msg.source_system_id = XLINK_SELF_SYSTEM_ID;
    msg.source_component_id = XLINK_SELF_COMPONENT_ID;
    msg.target_system_id = XLINK_SYSTEM_ID_BROADCAST;
    msg.target_component_id = XLINK_COMPONENT_ID_BROADCAST;
    uint8_t *buffer = (uint8_t *)&msg;
    xlink_param_info_encode(param_info, &msg);
    uint8_t size = xlink_get_msg_length(&msg);
    buffer[size - 1] = msg.crc;

    return xlink_get_write_func(channel)(buffer, size);
}
