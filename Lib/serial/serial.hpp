#pragma once

#include <stdint.h>
#include <stdarg.h>

#include "usart.h"
#if (USE_HAL_UART_REGISTER_CALLBACKS != 1)
#error "请在STM32CubeMX中开启UART的回调函数功能"
#endif

#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

#define SERIAL_NAME_SIZE 16
#define SERIAL_MAX_COUNT 8
#define SERIAL_PRINT_SIZE 200

typedef struct __DataPack_t
{
    uint8_t *pData; // 数据包首地址
    uint16_t size;  // 数据包大小
    uint32_t tick;  // 时间戳
} DataPack_t;

typedef struct __BufferHandle_t
{
    uint8_t *pBuffer; // 数据缓冲区首地址
    uint16_t size;    // 数据缓冲区最大长度
    uint8_t status;   // 数据缓冲区状态
    uint8_t *pData;   // 已有数据首指针
    uint16_t index;   // 已有数据尾指针索引
} BufferHandle_t;

enum class Serial_Mode : uint8_t
{
    None,
    IT,
    DMA
};

enum class Serial_Error : uint8_t
{
    None,
    Init_Error,
    Start_Error,
    RX_Error,
    TX_Error
};


typedef struct
{
    Serial_Mode rx_mode;   // 接收模式（IT、DMA）
    Serial_Mode tx_mode;   // 发送模式（IT、DMA）
    uint16_t rx_buf_size;  // 接收缓冲区大小
    uint16_t tx_buf_size;  // 发送缓冲区大小
    uint16_t rx_pack_size; // 接收数据包大小
    uint16_t tx_pack_size; // 发送数据包大小
    uint16_t rx_queue_len; // 接收队列长度
    uint16_t tx_queue_len; // 发送队列长度
} Serial_Config;

typedef struct
{
    uint16_t error_cnt;          // 串口错误计数
    uint32_t rx_total_size;      // 总接收字节数
    uint32_t tx_total_size;      // 总发送字节数
    uint16_t rx_to_process_size; // 待处理接收字节数
    uint16_t tx_to_process_size; // 待处理发送字节数
} Serial_Info;

class Serial;

typedef struct
{
    UART_HandleTypeDef *uart;
    Serial *serial;
} Serial_Map;

typedef void (*Serial_RX_Msg_Handler)(DataPack_t *pack, Serial *serial, void *args);

class Serial
{
private:
    char _name[SERIAL_NAME_SIZE] = "";             // 串口名称
    bool _ready = false;                           // 串口就绪
    Serial_Error _error = Serial_Error::None;      // 串口错误
    bool _half_cplt = false;                       // DMA半传输标志位
    UART_HandleTypeDef *_uart;                     // HAL库串口句柄
    BufferHandle_t _rx_buf;                        // 接收缓冲区
    BufferHandle_t _tx_buf;                        // 发送缓冲区
    QueueHandle_t _rx_queue;                       // 接收队列
    QueueHandle_t _tx_queue;                       // 发送队列
    SemaphoreHandle_t _tx_src_sem;                 // 发送资源占用信号量
    SemaphoreHandle_t _tx_buf_sem;                 // 发送缓冲区读写信号量
    TaskHandle_t _rx_task_handle;                  // 接收任务句柄
    TaskHandle_t _tx_task_handle;                  // 发送任务句柄
    char _rx_task_name[SERIAL_NAME_SIZE + 3] = ""; // 接收任务名称
    char _tx_task_name[SERIAL_NAME_SIZE + 3] = ""; // 发送任务名称
    Serial_Config _config;                         // 串口配置
    Serial_Info _info;                             // 串口信息

    Serial_RX_Msg_Handler _rx_msg_handler = NULL;
    void *_rx_msg_handler_args = NULL;

public:
    static Serial *sr_list[SERIAL_MAX_COUNT];
    static uint8_t sr_count;

public:
    static Serial *get_serial(UART_HandleTypeDef *uart);
    static bool generate_default_config(Serial_Config *config);

    bool init(const char *name, Serial_Config *config, UART_HandleTypeDef *uart);
    bool init_with_default(const char *name, UART_HandleTypeDef *uart);

    UART_HandleTypeDef *get_uart(void) { return _uart; }

    bool is_ready(void) { return _ready; };

    bool register_rx_msg_handler(Serial_RX_Msg_Handler rx_msg_handler, void *args);
    float get_rx_occupy(void) { return (float)_info.rx_to_process_size / _config.rx_buf_size; };
    float get_tx_occupy(void) { return (float)_info.tx_to_process_size / _config.tx_buf_size; };

    bool send_data(DataPack_t *pack, bool auto_gen_tick = true);
    bool send_data(uint8_t *data, uint16_t size, uint32_t tick = 0);
    bool send_data_from_isr(uint8_t *data, uint16_t size, uint32_t tick = 0);
    bool send_data_fast(uint8_t *data, uint16_t size);
    int print(const char *format, ...);
    int print_from_isr(const char *format, ...);

private:
    bool _init_buf(void);
    bool _init_queue(void);
    bool _init_semaphore(void);
    bool _init_task(void);
    bool _init_callback(void);

    bool _is_tx_busy(bool from_isr = false);
    void _update_info(int16_t rx_size, int16_t tx_size);

    bool _rx_irq_start(void);                // 开启串口空闲接收中断
    bool _tx_irq_start(DataPack_t *tx_pack); // 开启串口发送中断

    void _rx_callback(uint16_t size);
    void _tx_callback(void);
    void _error_callback(void);

    void _rx_task(void);
    void _tx_task(void);

    void _error_handler(Serial_Error error);
};