// #include "MotorController.hpp"
#include "MC_Serial.hpp"

Serial sr_logger;
Serial sr_console;

static void handle_console_msg(DataPack_t *pack, Serial *serial, void *args);
static void handle_logger_msg(DataPack_t *pack, Serial *serial, void *args);

void MC_Serial::init(void)
{
    /* vofa+串口初始化 */
    sr_logger.init_with_default("logger", Vofa_huart);
    sr_logger.register_rx_msg_handler(handle_logger_msg, NULL);

    /* 控制台打印串口初始化 */
    sr_console.init_with_default("console", CLI_huart);
    sr_console.register_rx_msg_handler(handle_console_msg, NULL);
}

extern "C" int printf_(const char *format, ...) // 替代标准printf，将格式化输出重定向到控制台串口
{
    // extern Serial sr_console;
    uint8_t *print_buf = (uint8_t *)pvPortMalloc(SERIAL_PRINT_SIZE);

    if (print_buf == NULL)
        return -1;

    va_list args;
    va_start(args, format);
    int len = vsnprintf((char *)print_buf, SERIAL_PRINT_SIZE, format, args);
    va_end(args);

    bool status;
    for (uint8_t i = 0; i < 100; i++)
    {
        status = sr_console.send_data(print_buf, len);
        if (status)
            break;
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    if (!status)
        len = -1;

    vPortFree(print_buf);

    return len;
}

extern "C" int println(const char *format, ...) // 替代标准printf，将格式化输出重定向到控制台串口
{
    // extern Serial sr_console;
    uint8_t *print_buf = (uint8_t *)pvPortMalloc(SERIAL_PRINT_SIZE);

    if (print_buf == NULL)
        return -1;

    va_list args;
    va_start(args, format);
    int len = vsnprintf((char *)print_buf, SERIAL_PRINT_SIZE, format, args);
    va_end(args);

    bool status;
    for (uint8_t i = 0; i < 100; i++)
    {
        status = sr_console.send_data(print_buf, len);
        if (status)
            break;
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    status &= sr_console.send_data((uint8_t *)"\r\n", 2);
    len += 2;
    if (!status)
        len = -1;

    vPortFree(print_buf);

    return len;
}

static void handle_console_msg(DataPack_t *pack, Serial *serial, void *args) // 用于实现交互式命令行界面 打印等操作
{
    CLI_Module *mcli = CLI_Module::get_cli_module(&sr_console);

    if (mcli == NULL)
        return;

    mcli->receive_data(pack->pData, pack->size);
}

static void handle_logger_msg(DataPack_t *pack, Serial *serial, void *args) //串口2 VoFA的回调
{
    CLI_Module *mcli = CLI_Module::get_cli_module(&sr_console);

    if (mcli == NULL)
        return;
    // vofa1.on_recv(pack->pData, pack->size);//转发给自定义解析任务
    mcli->receive_data(pack->pData, pack->size);
    

}