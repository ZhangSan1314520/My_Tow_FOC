#include "My_CLI.hpp"

CLI_Module cli_console;  // 全局CLI实例

// ============================================================
// 1. 命令处理函数声明
// ============================================================

// ============================================================
// 2. CLI 任务入口
// ============================================================
void Task_CLI(void *argument)
{
    cli_console.init("FOC_ZGJ");                       // CLI终端名称
    cli_console.bind_write_data(&sr_console);       // 绑定串口1

    // 注册命令
    CLI_Module::add_command_to_all("start", "start [idx]", true, NULL, start);//电机启动
    CLI_Module::add_command_to_all("stop", "stop [idx]", true, NULL, stop);//电机停止
    CLI_Module::add_command_to_all("reset", "reset", true, NULL, reset);//驱动板重启
    // CLI_Module::add_command_to_all("clear_eeprom", "clear_eeprom [idx]", true, NULL, clear_eeprom);//清空指定电机的eeprom数据
    // CLI_Module::add_command_to_all("save", "save [idx]", true, NULL, save);//参数保存
    CLI_Module::add_command_to_all("set_zero", "set_zero [idx] 单位°", true, NULL, set_zero);//设置零点
    CLI_Module::add_command_to_all("set_pos", "set_pos [idx] [angle] 单位 °", true, NULL, set_pos);//设置目标位置 范围[-180,180]
    CLI_Module::add_command_to_all("set_speed", "set_speed [idx] [velocity] 单位 rad/s", true, NULL, set_speed);//设置目标速度
    // CLI_Module::add_command_to_all("set_mode", "set_mode [idx] <mode>取值:speed pos open_loop", true, NULL, set_mode); //切换工作模式
    CLI_Module::add_command_to_all("set_pid_id", "set_pid_id [idx] [kp] [ki] [kd]", true, NULL, set_pid_id); //设置PID 电流环id
    CLI_Module::add_command_to_all("set_pid_iq", "set_pid_iq [idx] [kp] [ki] [kd]", true, NULL, set_pid_iq); //设置PID 电流环iq    
    CLI_Module::add_command_to_all("set_pid_v", "set_pid_v [idx] [kp] [ki] [kd]", true, NULL, set_pid_v); //设置PID 速度环  
    CLI_Module::add_command_to_all("set_pid_p", "set_pid_p [idx] [kp] [ki] [kd]", true, NULL, set_pid_p); //设置PID 位置环     
    CLI_Module::add_command_to_all("set_wave", "set_wave [idx] [int]", true, NULL, set_wave);//设置目标波形

    // CLI_Module::add_command_to_all("get_mode", "get_mode [idx] ", true, NULL, get_mode);//获取当前工作模式 
    

    CLI_Module::start_all();  // 启动所有CLI
    printf("CLI栈剩余:%d\r\n", uxTaskGetStackHighWaterMark(NULL));
    vTaskDelete(NULL);
}







