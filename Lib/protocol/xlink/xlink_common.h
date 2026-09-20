#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "xlink_define.h"
// #include "xlink_util.h"

#define XLINK_MSG_HEAD 0xFC
#define XLINK_MSG_LEN_MAX (sizeof(xlink_msg_t))

typedef uint8_t (*xlink_write_func_t)(uint8_t *data, uint16_t len);

extern uint8_t _xl_rx_buffer[XLINK_CHANNEL_MAX][XLINK_MSG_LEN_MAX];
extern xlink_status_t _xl_chan_status[XLINK_CHANNEL_MAX];
extern xlink_write_func_t _xl_chan_write_func[XLINK_CHANNEL_MAX];

static inline uint8_t _xlink_crc_calc(uint8_t *data, uint16_t len, uint8_t crc_init)
{
    uint8_t crc_val = crc_init;
    while (len--)
    {
        crc_val ^= data[len];
    }
    return crc_val;
}

static inline bool _xlink_crc_check(uint8_t *data, uint16_t len, uint8_t crc_val)
{
    return (_xlink_crc_calc(data, len, 0x00) == crc_val);
}

static inline uint8_t *_xlink_get_chan_buffer(uint8_t chan)
{
    if (chan >= XLINK_CHANNEL_MAX)
    {
        return NULL;
    }
    return _xl_rx_buffer[chan];
}

static inline xlink_status_t *_xlink_get_chan_status(uint8_t chan)
{
    if (chan >= XLINK_CHANNEL_MAX)
    {
        return NULL;
    }
    return &_xl_chan_status[chan];
}

static inline uint8_t xlink_frame_char(uint8_t chan, uint8_t c, xlink_msg_t *msg, xlink_status_t *_status)
{
    xlink_msg_t *buffer = (xlink_msg_t *)_xlink_get_chan_buffer(chan);
    xlink_status_t *status = _xlink_get_chan_status(chan);
    switch (status->state)
    {
    case XLINK_PARSE_STATE_UNINIT:
    case XLINK_PARSE_STATE_IDLE:
    case XLINK_PARSE_STATE_GOT_CRC:
        // 检查head
        if (c == XLINK_MSG_HEAD)
        {
            memset(buffer, 0, sizeof(xlink_msg_t));
            buffer->head = c;
            status->state = XLINK_PARSE_STATE_GOT_STX;
            status->framing = XLINK_FRAMING_INCOMPLETE;
        }
        else
        {
            status->state = XLINK_PARSE_STATE_IDLE;
            status->framing = XLINK_FRAMING_BAD_FRAME;
        }
        break;
    case XLINK_PARSE_STATE_GOT_STX:
        // 检查source_system_id
        buffer->source_system_id = c;
        status->state = XLINK_PARSE_STATE_GOT_SSID;
        break;
    case XLINK_PARSE_STATE_GOT_SSID:
        // 检查source_component_id
        buffer->source_component_id = c;
        status->state = XLINK_PARSE_STATE_GOT_SCID;
        break;
    case XLINK_PARSE_STATE_GOT_SCID:
        // 检查target_system_id
        buffer->target_system_id = c;
        status->state = XLINK_PARSE_STATE_GOT_TSID;
        break;
    case XLINK_PARSE_STATE_GOT_TSID:
        // 检查target_component_id
        buffer->target_component_id = c;
        status->state = XLINK_PARSE_STATE_GOT_TCID;
        break;
    case XLINK_PARSE_STATE_GOT_TCID:
        // 检查msg_id
        buffer->msg_id = c;
        status->state = XLINK_PARSE_STATE_GOT_MSG_ID;
        break;
    case XLINK_PARSE_STATE_GOT_MSG_ID:
        // 检查data_len
        if ((c > 0) &&
            (c <= XLINK_DATA_LEN_MAX))
        {
            buffer->data_len = c;
            status->data_pos = 0;
            status->state = XLINK_PARSE_STATE_GOT_DATA_LEN;
        }
        else
        {
            status->state = XLINK_PARSE_STATE_IDLE;
            status->framing = XLINK_FRAMING_BAD_FRAME;
        }
        break;
    case XLINK_PARSE_STATE_GOT_DATA_LEN:
        // 复制数据
        buffer->data[status->data_pos] = c;
        status->data_pos++;
        if (status->data_pos == buffer->data_len)
        {
            status->state = XLINK_PARSE_STATE_GOT_PAYLOAD;
        }
        break;
    case XLINK_PARSE_STATE_GOT_PAYLOAD:
        buffer->crc = c;
        if (_xlink_crc_check((uint8_t *)buffer + sizeof(buffer->head), buffer->data_len + offsetof(xlink_msg_t, data) - sizeof(buffer->head), buffer->crc))
        {
            status->state = XLINK_PARSE_STATE_GOT_CRC;
            status->framing = XLINK_FRAMING_OK;
        }
        else
        {
            status->state = XLINK_PARSE_STATE_IDLE;
            status->framing = XLINK_FRAMING_BAD_CRC;
        }
        break;
    case XLINK_PARSE_STATE_GOT_BAD_CRC:
        status->state = XLINK_PARSE_STATE_IDLE;
        status->framing = XLINK_FRAMING_BAD_CRC;
        break;
    default:
        break;
    }
    if (status->framing == XLINK_FRAMING_OK)
    {
        status->rx_cnt++;
        if (msg != NULL)
        {
            *msg = *buffer;
        }
    }
    if (_status != NULL)
    {
        *_status = *status;
    }
    return status->framing;
}

static inline bool xlink_parse_char(uint8_t chan, uint8_t c, xlink_msg_t *msg, xlink_status_t *_status)
{
    if ((chan >= XLINK_CHANNEL_MAX) ||
        (msg == NULL))
    {
        return false;
    }
    // xlink_status_t *_status = _xlink_get_chan_status(chan);
    // uint8_t received = xlink_frame_char(chan, c, msg, _status);
    // if (received == XLINK_FRAMING_BAD_CRC || received == XLINK_FRAMING_BAD_FRAME)
    // {
    //     status->state = XLINK_PARSE_STATE_IDLE;
    //     status->framing = XLINK_FRAMING_INCOMPLETE;
    //     if (c == XLINK_MSG_HEAD)
    //     {
    //         status->state = XLINK_PARSE_STATE_GOT_STX;
    //     }
    //     return 0;
    // }
    // return received;

    return (xlink_frame_char(chan, c, msg, _status) == XLINK_FRAMING_OK);
}

static inline uint8_t xlink_get_msg_length(xlink_msg_t *msg)
{
    if (msg == NULL)
    {
        return 0;
    }
    return (msg->data_len + offsetof(xlink_msg_t, data) + sizeof(msg->crc));
}

static inline uint8_t xlink_sendto_buffer(const xlink_msg_t *msg, uint8_t *buf)
{
    if (msg == NULL || buf == NULL)
    {
        return 0;
    }
    memcpy(buf, msg, offsetof(xlink_msg_t, data) + msg->data_len);
    memcpy(buf + offsetof(xlink_msg_t, data) + msg->data_len, &msg->crc, sizeof(msg->crc));
    uint8_t len = offsetof(xlink_msg_t, data) + msg->data_len + sizeof(msg->crc);
    return len;
}

static inline uint32_t xlink_get_rx_cnt(uint8_t chan)
{
    xlink_status_t *status = _xlink_get_chan_status(chan);
    if (status == NULL)
    {
        return 0;
    }
    return status->rx_cnt;
}

static inline uint32_t xlink_get_tx_cnt(uint8_t chan)
{
    xlink_status_t *status = _xlink_get_chan_status(chan);
    if (status == NULL)
    {
        return 0;
    }
    return status->tx_cnt;
}

static inline bool xlink_register_write_func(uint8_t chan, xlink_write_func_t write_func)
{
    if ((chan >= XLINK_CHANNEL_MAX) || write_func == NULL)
    {
        return false;
    }
    _xl_chan_write_func[chan] = write_func;
    return true;
}

static inline xlink_write_func_t xlink_get_write_func(uint8_t chan)
{
    if (chan >= XLINK_CHANNEL_MAX)
    {
        return NULL;
    }
    return _xl_chan_write_func[chan];
}