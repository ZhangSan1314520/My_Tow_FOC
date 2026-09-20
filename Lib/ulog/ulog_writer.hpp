#pragma once

#include <stdint.h>
#include <string.h>

#include "ulog_messages.hpp"

class ULog_Writer
{
public:
    bool low_level_write_data(uint8_t *data, uint32_t len);

    void write_file_header(void);
    void write_message_flag_bits(void);
    void write_message_format(const char *format);
    void write_message_subscription(uint16_t msg_id, const char *msg_name);
    void write_message_data(uint16_t msg_id, uint8_t *data, uint16_t len);

    void write_gps_int(ulog_data_gps_int_s *gps_int);
};

void ULog_Writer::write_file_header(void)
{
    ulog_file_header_s file_header = {
        .magic = {'U', 'L', 'o', 'g', 0x01, 0x12, 0x35},
        .verison = 1,
        .timestamp = 1724229022'000'000};

    low_level_write_data((uint8_t *)&file_header, sizeof(file_header));
}

void ULog_Writer::write_message_flag_bits(void)
{
    ulog_message_flag_bits_s flag_bits = {
        .msg_size = sizeof(flag_bits) - ULOG_MSG_HEADER_LEN,
        .msg_type = (uint16_t)ULogMessageType::FLAG_BITS,
        .compat_flags = {},
        .incompat_flags = {},
        .appended_offsets = {}};

    low_level_write_data((uint8_t *)&flag_bits, sizeof(flag_bits));
}

void ULog_Writer::write_message_format(const char *format)
{
    ulog_message_format_s msg_format;
    msg_format.msg_size = strnlen(format, 500);
    msg_format.msg_type = (uint16_t)ULogMessageType::FORMAT;
    strncpy(msg_format.format, format, 500);

    low_level_write_data((uint8_t *)&msg_format, msg_format.msg_size + ULOG_MSG_HEADER_LEN);
}

void ULog_Writer::write_message_subscription(uint16_t msg_id, const char *msg_name)
{
    ulog_message_add_logged_s msg_sub;
    msg_sub.msg_size = strnlen(msg_name, 255) + 3;
    msg_sub.msg_type = (uint16_t)ULogMessageType::ADD_LOGGED_MSG;
    msg_sub.multi_id = 0;
    msg_sub.msg_id = msg_id;
    strncpy(msg_sub.msg_name, msg_name, 255);
    low_level_write_data((uint8_t *)&msg_sub, msg_sub.msg_size + ULOG_MSG_HEADER_LEN);
}

void ULog_Writer::write_message_data(uint16_t msg_id, uint8_t *data, uint16_t len)
{
    ulog_message_data_s msg_data;
    msg_data.msg_size = len + 2;
    msg_data.msg_type = (uint16_t)ULogMessageType::DATA;
    msg_data.msg_id = msg_id;
    memcpy(msg_data.data, data, len);
    low_level_write_data((uint8_t *)&msg_data, msg_data.msg_size + ULOG_MSG_HEADER_LEN);
}

void ULog_Writer::write_gps_int(ulog_data_gps_int_s *gps_int)
{
    write_message_data((uint16_t)ULog_MsgID::GPS_INT, (uint8_t *)gps_int, sizeof(ulog_data_gps_int_s));
}