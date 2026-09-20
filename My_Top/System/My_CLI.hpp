#pragma once  // 保证头文件只被编译一次，防止头文件被重复引用

#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <cstdlib>
#include "CLI_Module.hpp"
#include "MC_Serial.hpp"
#include "FocMotor_ClosedLoop.hpp"


void start(EmbeddedCli *cli, char *args, void *context); //启动电机
void stop(EmbeddedCli *cli, char *args, void *context); //停止电机
void reset(EmbeddedCli *cli, char *args, void *context); //复位电机
void clear_eeprom(EmbeddedCli *cli, char *args, void *context); //清空指定电机的eeprom数据
void save(EmbeddedCli *cli, char *args, void *context); //参数保存
void set_zero(EmbeddedCli *cli, char *args, void *context); //设置0点单位°
void set_pos(EmbeddedCli *cli, char *args, void *context); //设置目标位置
void set_wave(EmbeddedCli *cli, char *args, void *context); //设置目标波形
void set_speed(EmbeddedCli *cli, char *args, void *context); //设置目标速度
void set_mode(EmbeddedCli *cli, char *args, void *context); //切换工作模式
void set_pid_id(EmbeddedCli *cli, char *args, void *context); //设置PID 电流id环  
void set_pid_iq(EmbeddedCli *cli, char *args, void *context); //设置PID 电流iq环  
void set_pid_v(EmbeddedCli *cli, char *args, void *context); //设置PID 速度环
void set_pid_p(EmbeddedCli *cli, char *args, void *context); //设置PID 位置环 
void get_mode(EmbeddedCli *cli, char *args, void *context); //获取当前工作模式 



