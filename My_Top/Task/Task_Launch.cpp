
/* FreeRTOS includes */
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "MC_Serial.hpp"
#include "queue.h"
#include "My_Vofa.hpp"


/* Task Handle */
TaskHandle_t SystemInitHandle;
TaskHandle_t TestHandle;
TaskHandle_t Task_VofaRxHandle;
TaskHandle_t  DroneCANHandle;
TaskHandle_t MotorPubHandle;
TaskHandle_t CLIHandle;
TaskHandle_t VofaTxHandle;


/* Task Function */
extern void Task_SystemInit(void *argument);
extern void Task_Test(void *argument);
extern void Task_VofaRx(void *argument);
extern void Task_CLI(void *argument);
extern void Task_DroneCAN(void *argument);
extern void Task_MotorPublish(void *argument);
extern void Task_VofaTx(void *argument);
QueueHandle_t vofaRxQueue;   // VOFA接收队列句柄


void MC_TaskStart(void)
{

    /* 创建 VOFA 接收队列：队列长度 8，每项一帧数据 */
    vofaRxQueue = xQueueCreate(8, sizeof(VofaRxFrame_t));

    /* Task 创建 */
    xTaskCreate(Task_SystemInit, "SystemInit", 256, NULL, osPriorityHigh, &SystemInitHandle); //系统初始化任务
    xTaskCreate(Task_VofaRx, "Task_VofaRx", 256, NULL, osPriorityNormal, &Task_VofaRxHandle); //Vofa解析任务
    xTaskCreate(Task_CLI, "Task_CLI", 256, NULL, osPriorityNormal, &CLIHandle); //CLI任务
    xTaskCreate(Task_DroneCAN, "Task_DroneCAN", 256, NULL, osPriorityNormal, &DroneCANHandle); //DroneCAN 任务
    xTaskCreate(Task_MotorPublish, "MotorPub", 256, NULL, osPriorityLow4, &MotorPubHandle); //uorb发布任务
    xTaskCreate(Task_VofaTx,"VofaTx",  256, NULL, osPriorityLow4, &VofaTxHandle); //Vofa发送任务

    xTaskCreate(Task_Test, "Test", 256, NULL, osPriorityLow, &TestHandle); //测试任务 
}

extern "C" void MC_ScheduleStart(void)
{
    MC_TaskStart();//开始任务
}


