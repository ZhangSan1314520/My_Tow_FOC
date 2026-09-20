#include "CLI_Module.hpp"

#include <string.h>
#include "printf.h"

// 静态成员初始化：CLI实例指针数组，最多存 CLI_MODULE_INSTANCE_MAX_NUM 个实例
CLI_Module *CLI_Module::_cli_list[] = {};
// 静态成员初始化：当前已创建的CLI实例数量，初始为0
int CLI_Module::_cli_count = 0;

// 初始化一个CLI实例
// name：CLI名称字符串；internal：任务轮询间隔(ms)，最小10ms
bool CLI_Module::init(const char *name, uint16_t internal)
{
    // 名称不能为空，否则初始化失败
    if (name == NULL)
        return false;

    // 将name拷贝到_name，预留4字节给后缀"-CLI"，防止溢出
    strncpy(_name, name, sizeof(_name) - 1 - 4);
    // 在_name末尾追加"-CLI"后缀，形如 "DBG-CLI"
    strcat(_name, "-CLI");

    // 将name拷贝到_invitation，用于命令行提示符前缀
    strncpy(_invitation, name, sizeof(_name) - 1);
    // 在_invitation末尾追加"-CLI > "，形如 "DBG-CLI > "
    strcat(_invitation, "-CLI > ");

    // 设置任务轮询间隔，最小保护为10ms，避免过于频繁调度
    _process_internal = internal < 10 ? 10 : internal;

    // 配置 embedded-cli 库的初始化参数
    EmbeddedCliConfig cli_cfg = {
        .invitation = _invitation,        // 命令行提示符字符串
        .rxBufferSize = 128,              // 接收缓冲区大小(字节)
        .cmdBufferSize = 128,             // 命令解析缓冲区大小(字节)
        .historyBufferSize = 256,         // 历史命令缓冲区大小(字节)
        .maxBindingCount = 32,            // 最多可绑定的命令数量(含内置help，实际可用31条)
        .cliBuffer = NULL,                // 不使用外部静态buffer，让库自行动态分配
        .cliBufferSize = 0,               // 外部buffer大小为0，配合上面NULL使用
        .enableAutoComplete = true};      // 启用Tab自动补全功能

    // 根据配置创建embedded-cli实例，失败返回NULL
    _cli = embeddedCliNew(&cli_cfg);

    // 创建失败则返回false
    if (_cli == NULL)
        return false;

    // 绑定底层单字符输出回调，embedded-cli内部输出字符时调用此函数
    _cli->writeChar = _low_level_write_char;
    // 绑定底层字符串输出回调，embedded-cli内部输出字符串时调用此函数
    _cli->writeString = _low_level_write_string;

    // 创建FreeRTOS任务，用于周期性处理CLI输入/命令执行
    // 栈大小512字，优先级osPriorityNormal，任务参数传入this指针
    if (xTaskCreate(_process_task, _name, 512, this, osPriorityNormal, &_task_handle) != pdPASS)
        return false;   // 任务创建失败，返回false
    // 任务创建后立即挂起，等待 start() 调用后再运行
    vTaskSuspend(_task_handle);

    // 将当前实例注册到全局静态列表，供 get_cli_module 等静态方法查找
    _cli_list[_cli_count++] = this;
    // 标记初始化完成
    _init_ok = true;

    return true;
}

// 启动CLI，恢复之前被挂起的FreeRTOS处理任务
bool CLI_Module::start(void)
{
    // 未初始化则不允许启动
    if (!_init_ok)
        return false;

    // 恢复任务，CLI开始正常运行
    vTaskResume(_task_handle);

    return true;
}

// 绑定底层写数据回调（函数指针版本）
// write_data：用户提供的数据发送函数；args：透传给该函数的用户参数
bool CLI_Module::bind_write_data(void (*write_data)(uint8_t *data, uint16_t size, void *args), void *args)
{
    // 回调函数不能为NULL
    if (write_data == NULL)
        return false;

    // 保存写数据回调函数指针
    _write_data = write_data;
    // 保存透传参数
    _write_data_args = args;

    return true;
}

// 绑定底层写数据回调（Serial对象版本，内部用lambda封装serial->send_data）
bool CLI_Module::bind_write_data(Serial *serial)
{
    // serial对象不能为NULL
    if (serial == NULL)
        return false;

    // 保存serial指针，供 get_serial() 查询和 get_cli_module(Serial*) 匹配
    _serial = serial;
    // 透传参数也指向serial，lambda内通过此指针调用send_data
    _write_data_args = serial;

    //改成（有重试，等TX空闲再发）
    _write_data = [](uint8_t *data, uint16_t size, void *args)
    {
        Serial *serial = (Serial *)args;
        // 最多重试100次，每次等1ms，直到TX空闲
        for (uint8_t i = 0; i < 100; i++) {
            if (serial->send_data(data, size)) return; // 发送成功则返回
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    };

    return true;
}

// embedded-cli底层单字符输出回调（静态函数，被库回调）
// 根据embeddedCli指针找到对应的CLI_Module实例，再调用其写数据接口
void CLI_Module::_low_level_write_char(EmbeddedCli *embeddedCli, char c)
{
    // 通过embeddedCli指针反查CLI_Module实例
    CLI_Module *mcli = get_cli_module(embeddedCli);
    // 找不到实例则直接返回
    if (mcli == NULL)
        return;

    // 发送单个字符（取地址转为uint8_t*，长度为1）
    mcli->_write_data((uint8_t *)&c, 1, mcli->_write_data_args);
}

// embedded-cli底层字符串输出回调（静态函数，被库回调）
// 根据embeddedCli指针找到对应的CLI_Module实例，再调用其写数据接口
void CLI_Module::_low_level_write_string(EmbeddedCli *embeddedCli, const char *str)
{
    // 通过embeddedCli指针反查CLI_Module实例
    CLI_Module *mcli = get_cli_module(embeddedCli);
    // 找不到实例则直接返回
    if (mcli == NULL)
        return;

    // 发送字符串，长度用strnlen限制最大为CLI_MODULE_PRINT_BUFFER_MAX_SIZE，防止越界
    mcli->_write_data((uint8_t *)str, strnlen(str, CLI_MODULE_PRINT_BUFFER_MAX_SIZE), mcli->_write_data_args);
}

// embedded-cli底层单字符接收函数（静态函数）
// 将外部接收到的字符喂给embedded-cli库
void CLI_Module::_low_level_receive_char(EmbeddedCli *embeddedCli, char c)
{
    // embeddedCli为NULL则不处理
    if (embeddedCli == NULL)
        return;

    // 将字符送入embedded-cli的接收队列，库内部完成行编辑和命令解析
    embeddedCliReceiveChar(embeddedCli, c);
}

// 接收外部数据（如串口DMA收到的一帧），逐字节喂给embedded-cli
bool CLI_Module::receive_data(uint8_t *data, uint16_t size)
{
    // 数据指针或长度无效则返回false
    if (data == NULL || size == 0)
        return false;

    // 未初始化则不处理
    if (!_init_ok)
        return false;

    // 逐字节将数据送入embedded-cli接收处理
    for (uint16_t i = 0; i < size; i++)
    {
        _low_level_receive_char(_cli, *(char *)(data + i));
    }

    return true;
}

// 按索引查找CLI_Module实例（静态方法）
CLI_Module *CLI_Module::get_cli_module(int index)
{
    // 索引越界则返回NULL
    if (index < 0 || index >= _cli_count)
        return NULL;

    return _cli_list[index];
}

// 按embeddedCli指针查找对应的CLI_Module实例（静态方法）
CLI_Module *CLI_Module::get_cli_module(EmbeddedCli *cli)
{
    // cli指针为NULL则无需查找
    if (cli == NULL)
        return NULL;

    // 遍历所有已注册实例，找到内部_cli指针匹配的那个
    for (uint8_t i = 0; i < _cli_count; i++)
    {
        if (_cli_list[i]->_cli == cli)
            return _cli_list[i];
    }

    // 未找到则返回NULL
    return NULL;
}

// 按Serial指针查找对应的CLI_Module实例（静态方法）
// 用于在串口回调中直接通过serial找到对应的CLI实例
CLI_Module *CLI_Module::get_cli_module(Serial *serial)
{
    // serial指针为NULL则无需查找
    if (serial == NULL)
        return NULL;

    // 遍历所有已注册实例，找到绑定了该serial的那个
    for (uint8_t i = 0; i < _cli_count; i++)
    {
        if (_cli_list[i]->_serial == serial)
            return _cli_list[i];
    }

    // 未找到则返回NULL
    return NULL;
}

// 向所有已注册的CLI实例批量添加同一条命令（静态方法）
// 用于一次性将某命令注册到所有CLI通道
bool CLI_Module::add_command_to_all(CliCommandBinding cmd_binding)
{
    bool ret = true;

    // 遍历所有实例，逐个添加命令；任意一个失败则最终返回false
    for (uint8_t i = 0; i < _cli_count; i++)
    {
        ret &= _cli_list[i]->add_command(cmd_binding);
    }

    return ret;
}

// 向所有CLI实例批量添加命令（参数展开版本，内部构造CliCommandBinding后转发）
bool CLI_Module::add_command_to_all(const char *name,
                                    const char *help,
                                    bool tokenizeArgs,
                                    void *context,
                                    void (*binding)(EmbeddedCli *cli, char *args, void *context))
{
    // 用传入的各字段构造embedded-cli所需的命令绑定结构体
    CliCommandBinding cmd_binding = {.name = name,           // 命令名称字符串，如"set_speed"
                                     .help = help,           // 命令帮助说明，显示在help列表中
                                     .tokenizeArgs = tokenizeArgs, // 是否自动分词，true则args已按空格分割
                                     .context = context,     // 透传给binding回调的用户上下文指针
                                     .binding = binding};    // 命令触发时执行的回调函数
    // 转发给结构体版本的add_command_to_all
    return add_command_to_all(cmd_binding);
}

// 启动所有已注册的CLI实例（静态方法）
bool CLI_Module::start_all(void)
{
    bool ret = true;

    // 遍历所有实例，逐个调用start()；任意一个失败则最终返回false
    for (uint8_t i = 0; i < _cli_count; i++)
    {
        ret &= _cli_list[i]->start();
    }

    return ret;
}

// 子命令分发执行（预留接口，当前实现为空）
// 可在此根据args解析子命令名称，查找subcmd_cfg表并调用对应处理函数
void CLI_Module::execute_subcmd(CLI_Module *mcli, const char *args, void *context)
{
    // uint16_t arg_cnt = embeddedCliGetTokenCount(args);   // 获取参数token数量（已注释）
    // cli_subcmd_cfg_t *subcmd_cfg = (cli_subcmd_cfg_t *)context;  // 子命令配置表（已注释）
}

// 解析命令参数列表，将args字符串中的各token填入arg_list数组
// arg_list[0]留空("")，arg_list[1]起依次存各参数；返回实际参数数量
int CLI_Module::init_arg_list(const char **arg_list, const char *args)
{
    // 获取args中的token数量（embeddedCliGetTokenCount不含命令名本身，即参数个数）
    int arg_count = embeddedCliGetTokenCount(args); // 返回除第一个字符串的个数
    // arg_list[0]置空字符串，对应"命令名"占位（不使用）
    arg_list[0] = "";
    // 限制参数数量不超过CLI_MODULE_MAX_ARGS_NUM，防止数组越界
    if (arg_count >= CLI_MODULE_MAX_ARGS_NUM) arg_count = CLI_MODULE_MAX_ARGS_NUM;
    // 从索引1开始填充各参数token（embeddedCliGetToken索引从1起）
    for (int i = 1; i <= arg_count; i++) //
    {
        arg_list[i] = embeddedCliGetToken(args, i);
    }
    return arg_count;
}

// FreeRTOS任务函数：周期性调用embeddedCliProcess处理CLI输入与命令执行
void CLI_Module::_process_task(void *argument)
{
    // 将void*参数还原为CLI_Module实例指针
    CLI_Module *cli_module = (CLI_Module *)argument;
    if (cli_module != NULL)
    {
        // 任务启动时打印欢迎信息，格式如 "DBG-CLI DBG-CLI Start!"
        // cli_module->print("%s%s Start!\r\n", cli_module->_invitation, cli_module->_name);
    }
    while (1)
    {
        // 处理CLI内部状态：回显输入字符、执行已完成的命令、输出提示符等
        embeddedCliProcess(cli_module->_cli);
        // 按配置的间隔延时，避免CPU满载轮询
        vTaskDelay(pdMS_TO_TICKS(cli_module->_process_internal));
    }
}

// 格式化打印输出到CLI绑定的串口（类printf用法）
// 内部动态申请堆内存做格式化缓冲，完成后立即释放
int CLI_Module::print(const char *format, ...)
{
    // 未绑定写数据回调则无法输出，返回-1
    if (_write_data == NULL)
        return -1;

    // 从FreeRTOS堆申请格式化缓冲区，大小为CLI_MODULE_PRINT_BUFFER_MAX_SIZE
    uint8_t *print_buf = (uint8_t *)pvPortMalloc(CLI_MODULE_PRINT_BUFFER_MAX_SIZE);

    // 申请失败则返回-1
    if (print_buf == NULL)
        return -1;

    // 初始化可变参数列表
    va_list args;
    va_start(args, format);
    // 将格式化结果写入print_buf，最多写CLI_MODULE_PRINT_BUFFER_MAX_SIZE字节，返回实际长度
    int len = vsnprintf((char *)print_buf, CLI_MODULE_PRINT_BUFFER_MAX_SIZE, format, args);
    // 清理可变参数列表
    va_end(args);

    // 调用底层写数据接口将格式化后的内容发送出去
    _write_data(print_buf, len, _write_data_args);

    // 释放堆内存，防止内存泄漏
    vPortFree(print_buf);

    // 返回实际格式化写入的字节数
    return len;
}

// 向当前CLI实例添加一条命令绑定
bool CLI_Module::add_command(CliCommandBinding cmd_binding)
{
    // 未初始化则不允许添加
    if (!_init_ok)
        return false;

    // 命令名或回调函数为NULL则视为无效命令
    if (cmd_binding.name == NULL || cmd_binding.binding == NULL)
        return false;

    // 调用embedded-cli库接口将命令绑定注册到该CLI实例
    // 注意：总数受 maxBindingCount 限制（当前配置为16，含内置help实际可用15条）
    return embeddedCliAddBinding(_cli, cmd_binding);
}
