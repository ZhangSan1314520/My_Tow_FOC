#include "FreeRTOS.h"
#include "task.h"
#include "uorb.hpp"
#include "task_topics.h"
#include "FocMotor_ClosedLoop.hpp"

static void fill_telem(motor_telem_t *dst, FOC_Motor *m) // 电机数据填充转存
{

    // 实际位置/速度
    dst->theta_zero = m->theta_zero; //电角度零位
    dst->theta = m->theta; //电机电角度
    dst->theta_no_offic = m->theta_no_offic; //电机未偏移的原始弧度
    dst->theta_m = m->theta_m;                //电机原始弧度
    dst->theta_m_speed = m->theta_m_speed;        //电机原始弧度速度
    dst->theta_deg_final = m->theta_deg_final;        //最终的电机角度 (度)
    dst->reg_final = m->reg_final;              //最终的电机角度 (弧度)
    dst->Angular_velocity_final = m->Angular_velocity_final; //最终的角速度
    dst->Now_Id = m->Now_Id; //实际Id
    dst->Now_Iq = m->Now_Iq; //实际Iq
    dst->u_d = m->u_d; //输出ud轴电压
    dst->u_q = m->u_q; //输出q轴电压

    // 目标位置/速度
    dst->_target_location2 = m->_target_location2;      //目标位置 (度)
    dst->_target_speed = m->_target_speed;          //目标速度
    dst->_target_Id = m->_target_Id;      //目标Id
    dst->_target_Iq = m->_target_Iq;      //目标Iq

    //ABC相占空比
    dst->motor_duty_a = m->motor_duty_a;       //电机占空比
    dst->motor_duty_b = m->motor_duty_b;
    dst->motor_duty_c = m->motor_duty_c;

    dst->_IA = m->_IA;       //电机三相电流
    dst->_IB = m->_IB;
    dst->_IC = m->_IC;

    dst->Zero_IA = m->_phase_current->Zero_IA;       //电机三相电流零点偏移
    dst->Zero_IB = m->_phase_current->Zero_IB;
    dst->Zero_IC = m->_phase_current->Zero_IC;

    dst->Zero_IA_avg = m->_phase_current->Zero_IA_avg;       //电机三相电流零点偏移平均值
    dst->Zero_IB_avg = m->_phase_current->Zero_IB_avg;
    dst->Zero_IC_avg = m->_phase_current->Zero_IC_avg;

    //laji
    dst->theta_m_offic = m->theta_m_offic; //电机原始弧度偏移角度

}

void Task_MotorPublish(void *argument)
{
    uorb_handle_t topic = uorb_advertise(UORB_TOPIC_MOTOR, sizeof(motor_telem_t) * 4); //注册发布者
    motor_telem_t buf[4];
    TickType_t last_wake_time=xTaskGetTickCount(); //获取当前系统时间戳
    while (1)
    {
        fill_telem(&buf[0], &M1);
        fill_telem(&buf[1], &M2);
        uorb_publish(topic, buf);//发布数据 (没有指定给谁发，放在公共区域里)
        //运行耗时10us左右 
        vTaskDelayUntil(&last_wake_time,pdMS_TO_TICKS(5)); //严格按5ms发布一次
    }
}



