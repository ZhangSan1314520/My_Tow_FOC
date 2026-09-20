
#include "FreeRTOS.h"
#include "task.h"
#include "MC_Serial.hpp"
#include "My_Vofa.hpp"
#include "uorb.hpp"
#include "bsp.hpp"
#include "FocMotor_ClosedLoop.hpp"
#include "My_FM24V02A.hpp"

extern TaskHandle_t SystemInitHandle;
extern TaskHandle_t TestHandle;
extern TaskHandle_t Task_VofaRxHandle;
extern TaskHandle_t DroneCANHandle;
extern TaskHandle_t MotorPubHandle;
extern TaskHandle_t CLIHandle;
extern TaskHandle_t VofaTxHandle;

void look_mem(void)
{
    printf("历史最低栈剩余 Sys:%d VofaRx:%d CLI:%d Can:%d Pub:%d VofaTx:%d 自己:%d 堆剩余:%d / %d\r\n",
           uxTaskGetStackHighWaterMark(SystemInitHandle),
           uxTaskGetStackHighWaterMark(Task_VofaRxHandle),
           uxTaskGetStackHighWaterMark(CLIHandle),
           uxTaskGetStackHighWaterMark(DroneCANHandle),
           uxTaskGetStackHighWaterMark(MotorPubHandle),
           uxTaskGetStackHighWaterMark(VofaTxHandle),
           uxTaskGetStackHighWaterMark(NULL), // 自己的任务栈
           xPortGetFreeHeapSize(), configTOTAL_HEAP_SIZE);
}

uint8_t add[]= {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0d,0x10,0x11,0x12,0x16,0x72};
uint8_t data_tx[]= {0x00,0x00,0x80,0xff,0x03,0x00,0x07,0x00,0x00,0xa5,0xa5,0x88,0x00,0xaC,0x02,0x08,0x00};

uint8_t len = sizeof(data_tx)/sizeof(data_tx[0]);
uint8_t data_rx1[20], data_rx2[20], data_rx3[20], data_rx4[20],data_rx5[20];

float DR1 = 0.7;//0.92极限
float DR2 = 0.3;//0.47
float DR3 = 0.8;//0.45
float ARR = 3750;
void test_laji()
{
    vTaskDelay(pdMS_TO_TICKS(300));

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_SET);
    
    __HAL_TIM_SET_COMPARE(&htim20, TIM_CHANNEL_1, (uint16_t)(ARR*DR1));
    __HAL_TIM_SET_COMPARE(&htim20, TIM_CHANNEL_2, (uint16_t)(ARR*DR2));
    __HAL_TIM_SET_COMPARE(&htim20, TIM_CHANNEL_3, (uint16_t)(ARR*DR3));        

    HAL_TIM_PWM_Start(&htim20, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim20, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim20, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Start(&htim20, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim20, TIM_CHANNEL_3);
    HAL_TIMEx_PWMN_Start(&htim20, TIM_CHANNEL_3);
    __HAL_TIM_ENABLE(&htim20); // 使能定时器外设

    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
    __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, (uint16_t)(ARR*DR1));
    __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, (uint16_t)(ARR*DR2));
    __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, (uint16_t)(ARR*DR3));
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim8, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Start(&htim8, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_3);
    HAL_TIMEx_PWMN_Start(&htim8, TIM_CHANNEL_3);
    __HAL_TIM_ENABLE(&htim8); // 使能定时器外设


    vTaskDelay(pdMS_TO_TICKS(100));
    M1.Current_Auto_Calibration(true);
    M2.Current_Auto_Calibration(true);
    vTaskDelay(pdMS_TO_TICKS(100));
    M1.Current_Auto_Calibration(false);
    M2.Current_Auto_Calibration(false);
}

HAL_StatusTypeDef ret1, ret2;

void Task_Test(void *argument)
{
    printf("1-任务启动中\r\n");
    // ret1 = FM24V02A0.write(0x00, add, len); //写入多个寄存器
    // vTaskDelay(pdMS_TO_TICKS(100));
    // ret2 = FM24V02A0.read(0x00, data_rx1, len); //读取多个寄存器
    

    // test_laji();
    while (1)
    {
        // printf("laji:%d\r\n",M1.laji);
        M1.laji+=2;

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}





