
#include "FreeRTOS.h"
#include "task.h"
#include "ex_math.hpp"
#include "HAL_System.hpp"
#include "MC_Serial.hpp"
#include "FocMotor_ClosedLoop.hpp"
#include "My_Vofa.hpp"
#include "uorb.hpp"
#include "my_tim.hpp"
#include "Motor_Storage.hpp"

void Task_SystemInit(void *argument)
{
    uorb_init(); //初始化uORB消息中间件
    My_Canard_Init(); //初始化Canard协议栈
    HAL_System::init(); //初始化计时器 
    MC_Serial::init(); //初始化串口
    Vofa_Init(Vofa_huart); //初始化Vofa 
    M1_TIM.init(); //初始化定时器绑定中断
    M2_TIM.init(); //初始化定时器绑定中断
    M1.My_FOC_Motor_Init(); //初始化电机1
    M2.My_FOC_Motor_Init(); //初始化电机2
    // // Motor_Storage::load(1);//读eeprom的数据传回给电机
    // // Motor_Storage::load(2);//读eeprom的数据传回给电机
    HAL_ADC_Start(VM_adc);   // 启动ADC转换测母线电压
    printf("Sys_init栈剩余:%d\r\n", uxTaskGetStackHighWaterMark(NULL));
    vTaskDelete(NULL); //删除任务
}








