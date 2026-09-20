#pragma once

#include <stdint.h>

#if defined(__GNUC__)
#define __PACKED_STRUCT struct __attribute__((packed, aligned(1)))
#define __weak __attribute__((weak))
#endif

#define XLINK_DATA_LEN_MAX 80

typedef enum
{
    XLINK_CHANNEL_0,
    XLINK_CHANNEL_1,
    XLINK_CHANNEL_2,
    XLINK_CHANNEL_3,
    XLINK_CHANNEL_MAX
} xlink_channel_t;

typedef enum
{
    XLINK_PARSE_STATE_UNINIT = 0,
    XLINK_PARSE_STATE_IDLE,
    XLINK_PARSE_STATE_GOT_STX,
    XLINK_PARSE_STATE_GOT_SSID,
    XLINK_PARSE_STATE_GOT_SCID,
    XLINK_PARSE_STATE_GOT_TSID,
    XLINK_PARSE_STATE_GOT_TCID,
    XLINK_PARSE_STATE_GOT_MSG_ID,
    XLINK_PARSE_STATE_GOT_DATA_LEN,
    XLINK_PARSE_STATE_GOT_PAYLOAD,
    XLINK_PARSE_STATE_GOT_CRC,
    XLINK_PARSE_STATE_GOT_BAD_CRC
} xlink_parse_state_t;

typedef enum
{
    XLINK_FRAMING_INCOMPLETE = 0,
    XLINK_FRAMING_OK = 1,
    XLINK_FRAMING_BAD_CRC = 2,
    XLINK_FRAMING_BAD_FRAME = 3
} xlink_framing_t;

typedef struct
{
    uint8_t channel;
    xlink_parse_state_t state;
    xlink_framing_t framing;
    uint8_t data_pos;
    uint32_t rx_cnt;
    uint32_t tx_cnt;
} xlink_status_t;

typedef __PACKED_STRUCT
{
    uint8_t head;                     // 帧头
    uint8_t source_system_id;         // 源系统ID
    uint8_t source_component_id;      // 源组件ID
    uint8_t target_system_id;         // 目标系统ID
    uint8_t target_component_id;      // 目标组件ID
    uint8_t msg_id;                   // 消息ID
    uint8_t data_len;                 // 数据长度
    uint8_t data[XLINK_DATA_LEN_MAX]; // 数据载荷
    uint8_t crc;                      // CRC校验
}
xlink_msg_t;
