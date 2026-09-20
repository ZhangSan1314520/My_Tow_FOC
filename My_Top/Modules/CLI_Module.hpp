#pragma once

#include <stdint.h>

#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "serial.hpp"
#include "embedded_cli.h"

#ifndef CLI_MODULE_INSTANCE_MAX_NUM
#define CLI_MODULE_INSTANCE_MAX_NUM 5
#endif

#ifndef CLI_MODULE_PRINT_BUFFER_MAX_SIZE
#define CLI_MODULE_PRINT_BUFFER_MAX_SIZE 200
#endif

#define CLI_MODULE_MAX_ARGS_NUM 10

enum class CLI_Status : uint8_t
{
    ArgsError,
    Success,
    Fail
};

class CLI_Module;

typedef CLI_Status (*cli_subcmd_func_t)(CLI_Module *mcli, const char **args, int argc);

typedef struct
{
    const char *name;
    cli_subcmd_func_t func;
} cli_subcmd_cfg_t;

class CLI_Module
{
private:
    static CLI_Module *_cli_list[CLI_MODULE_INSTANCE_MAX_NUM];
    static int _cli_count;

public:
    static CLI_Module *get_cli_module(int index);
    static CLI_Module *get_cli_module(EmbeddedCli *cli);
    static CLI_Module *get_cli_module(Serial *serial);
    static int get_cli_count(void) { return _cli_count; };

    static bool add_command_to_all(CliCommandBinding cmd_binding);
    static bool add_command_to_all(const char *name,
                                   const char *help,
                                   bool tokenizeArgs,
                                   void *context,
                                   void (*binding)(EmbeddedCli *cli, char *args, void *context));
    static bool start_all(void);
    static void execute_subcmd(CLI_Module *mcli, const char *args, void *context);

    static int init_arg_list(const char **arg_list, const char *args);

public:
    bool init(const char *name = "DBG", uint16_t internal = 10);
    bool start(void);
    bool bind_write_data(void (*write_data)(uint8_t *data, uint16_t size, void *args), void *args = NULL);
    bool bind_write_data(Serial *serial);
    bool receive_data(uint8_t *data, uint16_t size);
    int print(const char *format, ...);
    bool add_command(CliCommandBinding cmd_binding);

    Serial *get_serial(void) { return _serial; }

private:
    bool _init_ok = false;

    void (*_write_data)(uint8_t *data, uint16_t size, void *args) = NULL;
    void *_write_data_args = NULL;

    uint16_t _process_internal;

    char _name[10];
    char _invitation[20];

    EmbeddedCli *_cli = NULL;

    Serial *_serial = NULL;

    TaskHandle_t _task_handle;

    static void _process_task(void *argument);
    static void _low_level_write_char(EmbeddedCli *embeddedCli, char c);
    static void _low_level_receive_char(EmbeddedCli *embeddedCli, char c);
    static void _low_level_write_string(EmbeddedCli *embeddedCli, const char *str);
};

#define FOR_EACH_CLI(method)                              \
    for (int i = 0; i < CLI_Module::get_cli_count(); i++) \
    {                                                     \
        CLI_Module::get_cli_module(i)->method;            \
    }
