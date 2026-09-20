#pragma once

#include <stdint.h>

#define ULOG_MSG_HEADER_LEN 3

#define __PACKED_STRUCT struct __attribute__((packed, aligned(1)))

enum class ULogMessageType : uint8_t
{
    FORMAT = 'F',
    DATA = 'D',
    INFO = 'I',
    INFO_MULTIPLE = 'M',
    PARAMETER = 'P',
    PARAMETER_DEFAULT = 'Q',
    ADD_LOGGED_MSG = 'A',
    REMOVE_LOGGED_MSG = 'R',
    SYNC = 'S',
    DROPOUT = 'O',
    LOGGING = 'L',
    LOGGING_TAGGED = 'C',
    FLAG_BITS = 'B',
};

typedef __PACKED_STRUCT
{
    const uint8_t magic[7] = {'U', 'L', 'o', 'g', 0x01, 0x12, 0x35};
    uint8_t verison = 1;
    uint64_t timestamp;
}
ulog_file_header_s;

static constexpr uint8_t ulog_file_magic_bytes[] = {'U', 'L', 'o', 'g', 0x01, 0x12, 0x35};

typedef __PACKED_STRUCT
{
    uint16_t msg_size;
    uint8_t msg_type;
}
ulog_message_header_s;

typedef __PACKED_STRUCT
{
    uint16_t msg_size; ///< size of message - ULOG_MSG_HEADER_LEN
    uint8_t msg_type = static_cast<uint8_t>(ULogMessageType::FORMAT);

    // char format[1500];
    char format[500];
}
ulog_message_format_s;

typedef __PACKED_STRUCT
{
    uint16_t msg_size; ///< size of message - ULOG_MSG_HEADER_LEN
    uint8_t msg_type = static_cast<uint8_t>(ULogMessageType::ADD_LOGGED_MSG);

    uint8_t multi_id;       ///< Multi instance id, if the topic is one of a multi instance uORB topic
    uint16_t msg_id;        ///< Message ID, an internally tracked id in the logger, which matches with the msg_id in ulog_message_data_s message
    char msg_name[255]; ///< Name of the uORB topic
}
ulog_message_add_logged_s;

typedef __PACKED_STRUCT
{
    uint16_t msg_size; ///< size of message - ULOG_MSG_HEADER_LEN
    uint8_t msg_type = static_cast<uint8_t>(ULogMessageType::DATA);

    uint16_t msg_id;

    uint8_t data[200];
}
ulog_message_data_s;

typedef __PACKED_STRUCT
{
    uint16_t msg_size; ///< size of message - ULOG_MSG_HEADER_LEN
    uint8_t msg_type = static_cast<uint8_t>(ULogMessageType::INFO);

    uint8_t key_len;         ///<  Length of the 'key'
    char key_value_str[255]; ///<  String with the key and value information
}
ulog_message_info_s;

typedef __PACKED_STRUCT
{
    uint16_t msg_size; ///< size of message - ULOG_MSG_HEADER_LEN
    uint8_t msg_type = static_cast<uint8_t>(ULogMessageType::LOGGING);

    uint8_t log_level; ///< same levels as in the linux kernel
    uint64_t timestamp;
    char message[128]; ///< defines the maximum length of a logged message string
}
ulog_message_logging_s;

typedef __PACKED_STRUCT
{
    uint16_t msg_size; ///< size of message - ULOG_MSG_HEADER_LEN
    uint8_t msg_type = static_cast<uint8_t>(ULogMessageType::LOGGING_TAGGED);

    uint8_t log_level; ///< same levels as in the linux kernel
    uint16_t tag;
    uint64_t timestamp;
    char message[128]; ///< defines the maximum length of a logged message string
}
ulog_message_logging_tagged_s;

typedef __PACKED_STRUCT
{
    uint16_t msg_size;
    uint8_t msg_type = static_cast<uint8_t>(ULogMessageType::PARAMETER);

    uint8_t key_len;
    char key_value_str[255]; ///<  String with the key and value information
}
ulog_message_parameter_s;

typedef __PACKED_STRUCT
{
    uint16_t msg_size;
    uint8_t msg_type = static_cast<uint8_t>(ULogMessageType::FLAG_BITS);

    uint8_t compat_flags[8];
    uint8_t incompat_flags[8];    ///< @see ULOG_INCOMPAT_FLAG_*
    uint64_t appended_offsets[3]; ///< file offset(s) for appended data if
                                  ///< ULOG_INCOMPAT_FLAG0_DATA_APPENDED_MASK is set
}
ulog_message_flag_bits_s;

#define ULOG_FORMAT_GPS_INT "GPS_INT:uint64_t timestamp;uint32_t lon;uint32_t lat;uint32_t alt;"

typedef struct
{
    const uint16_t msg_id;
    const char *msg_name;
    const char *msg_fmt;
} ulog_message_define_s;

enum class ULog_MsgID : uint16_t
{
    GPS_INT
};

static ulog_message_define_s ulog_msg_def_list[] = {
    {0, "GPS_INT", "uint64_t timestamp;uint32_t lon;uint32_t lat;uint32_t alt;"},
};

typedef __PACKED_STRUCT
{
    uint64_t timestamp;
    uint32_t lon;
    uint32_t lat;
    uint32_t alt;
}
ulog_data_gps_int_s;