
#include "FreeRTOS.h"
#include "task.h"
#include "My_Vofa.hpp"
#include "MC_Serial.hpp"
#include "FocMotor_ClosedLoop.hpp"
#include "uorb.hpp"
#include "task_topics.h"


struct Vafa_data
{ // 解析后的参数值
    uint8_t len;
    float val[VOFA_PARAM_MAX];
    float last_val[VOFA_PARAM_MAX]; // 记录上次的值
    bool changed[VOFA_PARAM_MAX];   // 标记哪个变了
};

/* 同步 last_val = val，用于初始化或写入后 */
void vofa_sync_last(Vafa_data *d)
{
    for (uint8_t i = 0; i < d->len; i++)
    {
        d->last_val[i] = d->val[i];
        d->changed[i] = false;
    }
}

const char *IA_Names[] = {"iq_p", "iq_i", "id_p", "id_i"};
const char *Speed_Names[] = {"speed_p", "speed_i", "speed_d"};
const char *Target_Names[] = {"Target_Iq", "Target_Id", "Target_speed", "Target_location1", "Target_location2"};
const char *Location_Names[] = {"location_p", "location_i", "location_d"};
const char *Button_Names[] = {"motor_selection", "motor_fre", "motor_duty","set_wave"}; // 电机选择12 / 电机频率 / 电机占空比

Vafa_data Vafa_IA;
Vafa_data Vafa_Speed;
Vafa_data Vafa_Target;
Vafa_data Vafa_Location;
Vafa_data Vafa_Button;

void vofa_data_init()
{
    Vafa_IA.len = sizeof(IA_Names) / sizeof(IA_Names[0]);
    Vafa_Speed.len = sizeof(Speed_Names) / sizeof(Speed_Names[0]);
    Vafa_Target.len = sizeof(Target_Names) / sizeof(Target_Names[0]);
    Vafa_Location.len = sizeof(Location_Names) / sizeof(Location_Names[0]);
    Vafa_Button.len = sizeof(Button_Names) / sizeof(Button_Names[0]);
    // 第一次读取后同步 last_val，避免误判 */
    vofa1.vofa_get_batch(&vofa1, IA_Names, Vafa_IA.val, Vafa_IA.len); // 读到的都是0
    vofa1.vofa_get_batch(&vofa1, Speed_Names, Vafa_Speed.val, Vafa_Speed.len);
    vofa1.vofa_get_batch(&vofa1, Target_Names, Vafa_Target.val, Vafa_Target.len);
    vofa1.vofa_get_batch(&vofa1, Location_Names, Vafa_Location.val, Vafa_Location.len);
    vofa1.vofa_get_batch(&vofa1, Button_Names, Vafa_Button.val, Vafa_Button.len);

    vofa_sync_last(&Vafa_IA);
    vofa_sync_last(&Vafa_Speed);
    vofa_sync_last(&Vafa_Target);
    vofa_sync_last(&Vafa_Location);
    vofa_sync_last(&Vafa_Button);
}

/* 模式切换专用函数 */
void vofa_update_mode(Work_Mode *target, Work_Mode default_val , FOC_Motor* motor) // 只在值变了的时候，写入目标到target
{
    static Work_Mode last_mode = (Work_Mode)0; 
    float val = vofa1.get("work_mode", (float)default_val);
    Work_Mode new_mode = (Work_Mode)((int)val);

    if (new_mode != last_mode)
    {
        *target = new_mode;
        last_mode = new_mode;
        if (motor != NULL)
        {
            motor->_target_Id = 0.0f;
            motor->_target_Iq = 0.0f;
            motor->_target_speed = 0.0f;
            motor->motor_duty_a = 0.0f; // 占空比
            motor->motor_duty_b = 0.0f;
            motor->motor_duty_c = 0.0f;
            motor->setPwm(); // 设置占空比
            motor->My_FOC_Motor_Reset();
        }
    }
}



void vofa_update_if_changed(Vafa_data *d, uint8_t idx, float *target,  FOC_Motor* motor)
{
    if (!d->changed[idx]) return;// 只在值变了的时候

    *target = d->val[idx];      // 写入目标

    d->last_val[idx] = d->val[idx]; // 更新last
    d->changed[idx] = false; // 标记无变化

    if (motor == NULL) return; // 如果没有指定电机，不复位
    motor->My_FOC_Motor_Reset();

}




void vofa_detect_changes(Vafa_data *d) // 检测数据的变化 写入标志位到changed
{
    for (uint8_t i = 0; i < d->len; i++)
    {
        d->changed[i] = (d->val[i] != d->last_val[i]);
    }
}

void Task_VofaRx(void *argument)
{ 
    static float Motor_Select = 0.0; // 电机选择1-4
    static uint8_t last_idx = 0; // 上一次的电机选择
    VofaRxFrame_t frame;   // 队列接收缓冲区

    vofa_data_init();
    
    while (1)
    {
        xQueueReceive(vofaRxQueue, &frame, portMAX_DELAY); // 从队列中获取数据
        /* 把队列数据喂给 vofa1（复用原有解析逻辑） */
        memcpy(vofa1.rx_buf, frame.buf, frame.len);
        vofa1.rx_len = frame.len;
        vofa1.parse();// 解析收到的帧

        vofa1.vofa_get_batch(&vofa1, IA_Names, Vafa_IA.val, Vafa_IA.len); // 读取IA_Names参数值
        vofa1.vofa_get_batch(&vofa1, Speed_Names, Vafa_Speed.val, Vafa_Speed.len); // 读取Speed_Names参数值
        vofa1.vofa_get_batch(&vofa1, Target_Names, Vafa_Target.val, Vafa_Target.len); // 读取Target_Names参数值
        vofa1.vofa_get_batch(&vofa1, Location_Names, Vafa_Location.val, Vafa_Location.len); // 读取Location_Names参数值
        vofa1.vofa_get_batch(&vofa1, Button_Names, Vafa_Button.val, Vafa_Button.len);       // 读取Button_Names参数值

        vofa_detect_changes(&Vafa_IA); // 检测数据的变化 写入标志位到changed
        vofa_detect_changes(&Vafa_Speed);
        vofa_detect_changes(&Vafa_Target);
        vofa_detect_changes(&Vafa_Location);
        vofa_detect_changes(&Vafa_Button);

        
        vofa_update_if_changed(&Vafa_Button, 0, &Motor_Select, 0);
        
        uint8_t idx = (uint8_t)Motor_Select;  // 1=M1, 2=M2, 3=M3, 4=M4
        if (idx != last_idx) //电机切换后           
        {                                   
            vofa_sync_last(&Vafa_IA);  //同步last_val = val且标记无变化 
            vofa_sync_last(&Vafa_Speed); 
            vofa_sync_last(&Vafa_Target);
            vofa_sync_last(&Vafa_Location); 
            vofa_sync_last(&Vafa_Button);
            last_idx = idx;
        }
        if (idx < 1 || idx > 2)
        {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;                        // 跳过本轮，不退出任务
        }

        FOC_Motor* m = NULL;
        switch (idx)
        {
            case 1: m = &M1; break; // M1
            case 2: m = &M2; break; // M2
        }
        switch (m->work_mode) 
        {
            case Ia_loop: // 电流环模式
                vofa_update_if_changed(&Vafa_IA,  0, &m->_pid_iq->_kp, m);//
                vofa_update_if_changed(&Vafa_IA,  1, &m->_pid_iq->_ki, m);
                vofa_update_if_changed(&Vafa_IA,  2, &m->_pid_id->_kp, m);
                vofa_update_if_changed(&Vafa_IA,  3, &m->_pid_id->_ki, m);
                vofa_update_if_changed(&Vafa_Target, 0, &m->_target_Iq, NULL); // 
                vofa_update_if_changed(&Vafa_Target, 1, &m->_target_Id, NULL); // 
                break;                
            case speed_loop: // 速度闭环模式
                vofa_update_if_changed(&Vafa_Speed,  0, &m->_pid_spd->_kp, m);//速度PID
                vofa_update_if_changed(&Vafa_Speed,  1, &m->_pid_spd->_ki, m);
                vofa_update_if_changed(&Vafa_Speed,  2, &m->_pid_spd->_kd, m);
                vofa_update_if_changed(&Vafa_Target,  2, &m->_target_speed, NULL); //目标速度
                break;
            case position_loop: // 位置闭环模式
                vofa_update_if_changed(&Vafa_Location, 0, &m->_pid_loc->_kp, m);//位置PID
                vofa_update_if_changed(&Vafa_Location, 1, &m->_pid_loc->_ki, m);
                vofa_update_if_changed(&Vafa_Location, 2, &m->_pid_loc->_kd, m);
                vofa_update_if_changed(&Vafa_Target,  4, &m->_target_location2, NULL); //目标位置-180°~+180°
                vofa_update_if_changed(&Vafa_Speed,  0, &m->_pid_spd->_kp, m);//速度PID
                vofa_update_if_changed(&Vafa_Speed,  1, &m->_pid_spd->_ki, m);
                vofa_update_if_changed(&Vafa_Speed,  2, &m->_pid_spd->_kd, m);
                break;
        }
        vofa_update_mode(&m->work_mode, null_mode, m); //电机模式选择      
        vofa_update_if_changed(&Vafa_Button, 3, &m->wave_mode, m); //设置波形类型
    }
}





// ===== Vofa 发送任务 =====
void Task_VofaTx(void *argument)
{
    uorb_handle_t topic = uorb_subscribe(UORB_TOPIC_MOTOR); // 订阅motor_state话题
    motor_telem_t buf[4];

    while (1)
    {
        uorb_copy(topic, buf);  // 阻塞等新数据

        // Vofa_SendFireWater_VA(Vofa_huart,21,
        //     // M1
        //     buf[0].theta_m, buf[0].theta_deg_final,buf[0].Angular_velocity_final, 
        //     buf[0]._target_location2, buf[0]._target_speed,999.0f,
        //     // M2
        //     buf[1].theta_m, buf[1].theta_deg_final,buf[1].Angular_velocity_final,
        //     buf[1]._target_location2, buf[1]._target_speed,999.0f,
        //     // M3
        //     buf[2].theta_m, buf[2].theta_deg_final,buf[2].Angular_velocity_final,
        //     buf[2]._target_location2, buf[2]._target_speed,999.0f,
        //     // M4
        //     buf[3].theta_m, buf[3].theta_deg_final,buf[3].Angular_velocity_final,
        //     buf[3]._target_location2, buf[3]._target_speed,999.0f

        // );

        Vofa_SendFireWater_VA(Vofa_huart,19,
            
            buf[0]._target_Iq,
            buf[0].Now_Iq,
            3.14f, 
            buf[0]._target_Id,
            buf[0].Now_Id,
            3.14f,
            buf[0].Zero_IA,
            buf[0].Zero_IB,
            buf[0].Zero_IC,
            3.14f,
            buf[0]._IA,
            buf[0]._IB,
            buf[0]._IC,
            3.14f,
            buf[0].theta_zero,
            buf[0].filtered_speed,
            buf[0].Angular_velocity_final,
            // buf[0].theta_deg_final,
            buf[0].theta_no_offic,
            3.14f

        );
    }
}

