#include "xlink_common.h"

uint8_t _xl_rx_buffer[XLINK_CHANNEL_MAX][XLINK_MSG_LEN_MAX] = {};
xlink_status_t _xl_chan_status[XLINK_CHANNEL_MAX] = {
    {.channel = XLINK_CHANNEL_0, .state = XLINK_PARSE_STATE_IDLE, .framing = XLINK_FRAMING_INCOMPLETE, .rx_cnt = 0, .tx_cnt = 0},
    {.channel = XLINK_CHANNEL_1, .state = XLINK_PARSE_STATE_IDLE, .framing = XLINK_FRAMING_INCOMPLETE, .rx_cnt = 0, .tx_cnt = 0},
    {.channel = XLINK_CHANNEL_2, .state = XLINK_PARSE_STATE_IDLE, .framing = XLINK_FRAMING_INCOMPLETE, .rx_cnt = 0, .tx_cnt = 0},
    {.channel = XLINK_CHANNEL_3, .state = XLINK_PARSE_STATE_IDLE, .framing = XLINK_FRAMING_INCOMPLETE, .rx_cnt = 0, .tx_cnt = 0},
};
xlink_write_func_t _xl_chan_write_func[XLINK_CHANNEL_MAX] = {};