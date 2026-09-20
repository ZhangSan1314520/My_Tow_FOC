#include "My_CLI.hpp"


FOC_Motor* get_motor_by_arg(const char **arg_list, int argc,int *idx_out)
{
    if (argc <= 0) { printf("缺少电机编号 1-4\r\n"); return NULL; }
    int idx = atoi(arg_list[1]);
    if (idx < 1 || idx > 4) { printf("电机编号错误 %d\r\n", idx);}
    *idx_out = idx; //返回电机编号
    switch (idx) {
        case 1: return &M1;
        // case 2: return &M2;
        // case 3: return &M3;
        // case 4: return &M4;
    }
    return NULL;
}



void start(EmbeddedCli *cli, char *args, void *context) //启动电机
{
    int idx = 0;
    // 获取CLI实例
    CLI_Module *mc = CLI_Module::get_cli_module(cli);
    if (mc == NULL) return;

    // 解析参数
    const char *arg_list[CLI_MODULE_MAX_ARGS_NUM]; //数据存放区 超限只保存前CLI_MODULE_MAX_ARGS_NUM个
    int argc = CLI_Module::init_arg_list(arg_list, args);//返回值argc是除了定义头后 字符串的个数
    FOC_Motor *m = get_motor_by_arg(arg_list, argc,&idx); //获取电机对象
    if (!m) return;
    m->Motor_EN(true); // 打开PWM输出

    printf("启动电机 M%d\r\n", idx);
}

void stop(EmbeddedCli *cli, char *args, void *context) //停止电机
{
    int idx = 0;
    // 获取CLI实例
    CLI_Module *mc = CLI_Module::get_cli_module(cli);
    if (mc == NULL) return;

    // 解析参数
    const char *arg_list[CLI_MODULE_MAX_ARGS_NUM]; //数据存放区 超限只保存前CLI_MODULE_MAX_ARGS_NUM个
    int argc = CLI_Module::init_arg_list(arg_list, args);
    FOC_Motor *m = get_motor_by_arg(arg_list, argc,&idx); //获取电机对象
    if (!m) return;
    m->Motor_EN(false); // 关闭PWM输出
    printf("停止电机 M%d\r\n", idx);
}


void reset(EmbeddedCli *cli, char *args, void *context) //驱动板重启
{
    // 获取CLI实例
    CLI_Module *mc = CLI_Module::get_cli_module(cli);
    if (mc == NULL) return;

    // 解析参数
    const char *arg_list[CLI_MODULE_MAX_ARGS_NUM]; //数据存放区 超限只保存前CLI_MODULE_MAX_ARGS_NUM个
    int argc = CLI_Module::init_arg_list(arg_list, args);

    HAL_NVIC_SystemReset();//复位芯片
    printf("驱动板重启\r\n");

}

// void clear_eeprom(EmbeddedCli *cli, char *args, void *context) //清空指定电机的eeprom标志位
// {
//     int idx = 0;
//     bool clear_flag = false;
//     // 获取CLI实例
//     CLI_Module *mc = CLI_Module::get_cli_module(cli);
//     if (mc == NULL) return;

//     // 解析参数
//     const char *arg_list[CLI_MODULE_MAX_ARGS_NUM]; //数据存放区 超限只保存前CLI_MODULE_MAX_ARGS_NUM个
//     int argc = CLI_Module::init_arg_list(arg_list, args);
//     FOC_Motor *m = get_motor_by_arg(arg_list, argc,&idx); //获取电机对象
//     clear_flag = Motor_Storage::clear(idx);
//     if(clear_flag)printf("清除M%d电机标志位 成功\r\n",idx);
//     else printf("清除M%d电机标志位 失败\r\n",idx);
// }

// void save(EmbeddedCli *cli, char *args, void *context) //参数保存
// {
//     int idx = 0;
//     bool save_flag = false;
//     // 获取CLI实例
//     CLI_Module *mc = CLI_Module::get_cli_module(cli);
//     if (mc == NULL) return;

//     // 解析参数
//     const char *arg_list[CLI_MODULE_MAX_ARGS_NUM]; //数据存放区 超限只保存前CLI_MODULE_MAX_ARGS_NUM个
//     int argc = CLI_Module::init_arg_list(arg_list, args);
//     FOC_Motor *m = get_motor_by_arg(arg_list, argc,&idx); //获取电机对象
//     if (!m) return;
//     save_flag = Motor_Storage::save(idx);
//     if(save_flag) printf("M%d电机参数保存成功\r\n",idx);
//     else printf("M%d电机参数保存失败\r\n",idx);
// }




void set_zero(EmbeddedCli *cli, char *args, void *context) //设置0点单位°
{
    int idx = 0;
    char *end;
    // 获取CLI实例
    CLI_Module *mc = CLI_Module::get_cli_module(cli);
    if (mc == NULL) return;
    // 解析参数
    const char *arg_list[CLI_MODULE_MAX_ARGS_NUM]; //数据存放区 超限只保存前CLI_MODULE_MAX_ARGS_NUM个
    int argc = CLI_Module::init_arg_list(arg_list, args);
    FOC_Motor *m = get_motor_by_arg(arg_list, argc,&idx); //获取电机对象
    if (!m) return;
    float temp = strtof(arg_list[2], &end); //将字符串转换为浮点数
    if(end == arg_list[2]) 
    {
        printf("参数格式错误\r\n");
        return;
    }
    m->zero_offset = wrap_to_PI(m->theta_no_offic - deg2rad(temp));
    printf("电机M%d 设置当前角度为%.2f度\r\n",idx,temp);
}


 

void set_pos(EmbeddedCli *cli, char *args, void *context) //设置目标位置
{
    int idx = 0;
    char *end;
    // 获取CLI实例
    CLI_Module *mc = CLI_Module::get_cli_module(cli);
    if (mc == NULL) return;
    // 解析参数
    const char *arg_list[CLI_MODULE_MAX_ARGS_NUM]; //数据存放区 超限只保存前CLI_MODULE_MAX_ARGS_NUM个
    int argc = CLI_Module::init_arg_list(arg_list, args);
    FOC_Motor *m = get_motor_by_arg(arg_list, argc,&idx); //获取电机对象
    if (!m) return;
    float temp = strtof(arg_list[2], &end); //将字符串转换为浮点数
    if(end == arg_list[2]) 
    {
        printf("参数格式错误\r\n");
        return;
    }
    m->_target_location2 = temp;
    printf("电机M%d 目标位置设置%.2f\r\n",idx,temp);
}


void set_speed(EmbeddedCli *cli, char *args, void *context) //设置目标速度
{
    int idx = 0;
    char *end;
    // 获取CLI实例
    CLI_Module *mc = CLI_Module::get_cli_module(cli);
    if (mc == NULL) return;
    // 解析参数
    const char *arg_list[CLI_MODULE_MAX_ARGS_NUM]; //数据存放区 超限只保存前CLI_MODULE_MAX_ARGS_NUM个
    int argc = CLI_Module::init_arg_list(arg_list, args);
    FOC_Motor *m = get_motor_by_arg(arg_list, argc,&idx); //获取电机对象
    if (!m) return;
    float temp = strtof(arg_list[2], &end); //将字符串转换为浮点数
    if(end == arg_list[2]) 
    {
        printf("参数格式错误\r\n");
        return;
    }
    m->_target_speed = temp;
    printf("电机M%d 目标速度设置%.2f\r\n",idx,temp);
}

void set_wave(EmbeddedCli *cli, char *args, void *context) //设置目标波形
{
    int idx = 0;
    char *end;
    // 获取CLI实例
    CLI_Module *mc = CLI_Module::get_cli_module(cli);
    if (mc == NULL) return;
    // 解析参数
    const char *arg_list[CLI_MODULE_MAX_ARGS_NUM]; //数据存放区 超限只保存前CLI_MODULE_MAX_ARGS_NUM个
    int argc = CLI_Module::init_arg_list(arg_list, args);
    FOC_Motor *m = get_motor_by_arg(arg_list, argc,&idx); //获取电机对象
    if (!m) return;
    float temp = strtof(arg_list[2], &end); //将字符串转换为浮点数
    if(end == arg_list[2]) 
    {
        printf("参数格式错误\r\n");
        return;
    }
    m->wave_mode = temp;
    
    switch ((uint8_t)m->wave_mode)
    {
    case 0:
        printf("无波形\r\n");
        break;
    case 1:
        printf("方波\r\n");
        break;
    case 2:
        printf("正弦波\r\n");
        break;
    default:
        printf("无波形\r\n");
        break;
    }
}

// void set_mode(EmbeddedCli *cli, char *args, void *context) //设置工作模式
// {
//     int idx = 0;
//     char *end;
//     Work_Mode mode_temp = null_mode;
//     // 获取CLI实例
//     CLI_Module *mc = CLI_Module::get_cli_module(cli);
//     if (mc == NULL) return;
//     // 解析参数
//     const char *arg_list[CLI_MODULE_MAX_ARGS_NUM]; //数据存放区 超限只保存前CLI_MODULE_MAX_ARGS_NUM个
//     int argc = CLI_Module::init_arg_list(arg_list, args);
//     FOC_Motor *m = get_motor_by_arg(arg_list, argc,&idx); //获取电机对象
//     if (!m) return;

//     if (strcmp(arg_list[2], "speed") == 0) {
//         mode_temp = speed;
//     } else if (strcmp(arg_list[2], "pos") == 0) {
//         mode_temp = position;
//     } else if (strcmp(arg_list[2], "open_loop") == 0) {
//         mode_temp = open_loop;
//     } else if (strcmp(arg_list[2], "calib") == 0) {
//         mode_temp = EncoderCalibration;
//     }else {
//         printf("未知模式\r\n");
//         return;
//     }
//     m->work_mode = mode_temp;

//     m->My_FOC_Motor_Reset(); //重置电机

//     printf("电机M%d 工作模式设置%s\r\n",idx,arg_list[2]);


//     if(mode_temp == EncoderCalibration)
//     {
//         printf("电机M%d 校准模式：启动开环全速\r\n", idx);
//         vTaskDelay(pdMS_TO_TICKS(2000));

//         printf("电机M%d 开始 ANLC 校准，保持转动...\r\n", idx);
//         bool ok = m->_encoder->KTH7111_ANLC_Calibration(60);
//         m->work_mode = open_loop;//切回开环控制
//         vTaskDelay(pdMS_TO_TICKS(10));
//         m->Motor_EN(false);
//         m->updown_duty = 0.0;//停止电机
//         if (ok) {
//             printf("电机M%d ANLC 校准成功！参数已保存至MTP\r\n", idx);
//         } else {
//             printf("电机M%d ANLC 校准失败\r\n", idx);
//         }
//     }

// }



void set_pid_id(EmbeddedCli *cli, char *args, void *context) //设置PID 电流id环  
{
    int idx = 0;
    char *end1,*end2,*end3;
    // 获取CLI实例
    CLI_Module *mc = CLI_Module::get_cli_module(cli);
    if (mc == NULL) return;
    // 解析参数
    const char *arg_list[CLI_MODULE_MAX_ARGS_NUM]; //数据存放区 超限只保存前CLI_MODULE_MAX_ARGS_NUM个
    int argc = CLI_Module::init_arg_list(arg_list, args);
    FOC_Motor *m = get_motor_by_arg(arg_list, argc,&idx); //获取电机对象 
    if (!m) return;
    float kp = strtof(arg_list[2], &end1); //将字符串转换为浮点数
    float ki = strtof(arg_list[3], &end2); //将字符串转换为浮点数
    float kd = strtof(arg_list[4], &end3); //将字符串转换为浮点数
    if(end1 == arg_list[2] || end2 == arg_list[3] || end3 == arg_list[4])
    {
        printf("参数格式错误\r\n");
        return;
    }
    m->_pid_id->_kp = kp;
    m->_pid_id->_ki = ki;
    m->_pid_id->_kd = kd;
    m->My_FOC_Motor_Reset(); //重置电机
    printf("电机M%d 电流环id pid设置 kp:%.3f ki:%.3f kd:%.3f \r\n",idx,kp,ki,kd);
}



void set_pid_iq(EmbeddedCli *cli, char *args, void *context) //设置PID 电流iq环  
{
    int idx = 0;
    char *end1,*end2,*end3;
    // 获取CLI实例
    CLI_Module *mc = CLI_Module::get_cli_module(cli);
    if (mc == NULL) return;
    // 解析参数
    const char *arg_list[CLI_MODULE_MAX_ARGS_NUM]; //数据存放区 超限只保存前CLI_MODULE_MAX_ARGS_NUM个
    int argc = CLI_Module::init_arg_list(arg_list, args);
    FOC_Motor *m = get_motor_by_arg(arg_list, argc,&idx); //获取电机对象 
    if (!m) return;
    float kp = strtof(arg_list[2], &end1); //将字符串转换为浮点数
    float ki = strtof(arg_list[3], &end2); //将字符串转换为浮点数
    float kd = strtof(arg_list[4], &end3); //将字符串转换为浮点数
    if(end1 == arg_list[2] || end2 == arg_list[3] || end3 == arg_list[4])
    {
        printf("参数格式错误\r\n");
        return;
    }
    m->_pid_iq->_kp = kp;
    m->_pid_iq->_ki = ki;
    m->_pid_iq->_kd = kd;
    m->My_FOC_Motor_Reset(); //重置电机
    printf("电机M%d 电流环iq pid设置 kp:%.3f ki:%.3f kd:%.3f \r\n",idx,kp,ki,kd);
}



void set_pid_v(EmbeddedCli *cli, char *args, void *context) //设置PID 速度环  
{
    int idx = 0;
    char *end1,*end2,*end3;
    // 获取CLI实例
    CLI_Module *mc = CLI_Module::get_cli_module(cli);
    if (mc == NULL) return;
    // 解析参数
    const char *arg_list[CLI_MODULE_MAX_ARGS_NUM]; //数据存放区 超限只保存前CLI_MODULE_MAX_ARGS_NUM个
    int argc = CLI_Module::init_arg_list(arg_list, args);
    FOC_Motor *m = get_motor_by_arg(arg_list, argc,&idx); //获取电机对象 
    if (!m) return;
    float kp = strtof(arg_list[2], &end1); //将字符串转换为浮点数
    float ki = strtof(arg_list[3], &end2); //将字符串转换为浮点数
    float kd = strtof(arg_list[4], &end3); //将字符串转换为浮点数
    if(end1 == arg_list[2] || end2 == arg_list[3] || end3 == arg_list[4])
    {
        printf("参数格式错误\r\n");
        return;
    }
    m->_pid_spd->_kp = kp;
    m->_pid_spd->_ki = ki;
    m->_pid_spd->_kd = kd;
    m->My_FOC_Motor_Reset(); //重置电机
    printf("电机M%d 速度pid设置 kp:%.3f ki:%.3f kd:%.3f \r\n",idx,kp,ki,kd);
}


void set_pid_p(EmbeddedCli *cli, char *args, void *context) //设置PID 位置环  
{
    int idx = 0;
    char *end1,*end2,*end3;
    // 获取CLI实例
    CLI_Module *mc = CLI_Module::get_cli_module(cli);
    if (mc == NULL) return;
    // 解析参数
    const char *arg_list[CLI_MODULE_MAX_ARGS_NUM]; //数据存放区 超限只保存前CLI_MODULE_MAX_ARGS_NUM个
    int argc = CLI_Module::init_arg_list(arg_list, args);
    FOC_Motor *m = get_motor_by_arg(arg_list, argc,&idx); //获取电机对象
    if (!m) return;
    float kp = strtof(arg_list[2], &end1); //将字符串转换为浮点数
    float ki = strtof(arg_list[3], &end2); //将字符串转换为浮点数
    float kd = strtof(arg_list[4], &end3); //将字符串转换为浮点数
    if(end1 == arg_list[2] || end2 == arg_list[3] || end3 == arg_list[4])
    {
        printf("参数格式错误\r\n");
        return;
    }
    m->_pid_loc->_kp = kp;
    m->_pid_loc->_ki = ki;
    m->_pid_loc->_kd = kd;
    m->My_FOC_Motor_Reset(); //重置电机
    printf("电机M%d 位置pid设置 kp:%.3f ki:%.3f kd:%.3f \r\n",idx,kp,ki,kd);
}


void get_mode(EmbeddedCli *cli, char *args, void *context) //获取当前工作模式
{
    int idx = 0;
    char *end;
    // 获取CLI实例
    CLI_Module *mc = CLI_Module::get_cli_module(cli);
    if (mc == NULL) return;
    // 解析参数
    const char *arg_list[CLI_MODULE_MAX_ARGS_NUM]; //数据存放区 超限只保存前CLI_MODULE_MAX_ARGS_NUM个
    int argc = CLI_Module::init_arg_list(arg_list, args);
    FOC_Motor *m = get_motor_by_arg(arg_list, argc,&idx); //获取电机对象
    if (!m) return;
    Work_Mode temp = m->work_mode;
    switch (temp)
    {
    case open_loop:
        printf("电机M%d 当前模式为开环控制模式\r\n",idx);
        break;
    case speed_loop:
        printf("电机M%d 当前模式为速度控制模式\r\n",idx);
        break;
    case position_loop:
        printf("电机M%d 当前模式为位置控制模式\r\n",idx);
        break;    
    default:
        printf("电机M%d 未知模式\r\n",idx);
        break;
    }
    
}


