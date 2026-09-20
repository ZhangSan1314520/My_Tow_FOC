#include "bsp.hpp"

//const 不可以被修改
// ---------- M1 电机 ----------
const MotorPWM_Config M1_PWM = {
    .htim      = &htim20,            
    .Tim_base  = TIM20,
    .ch_hin1   = TIM_CHANNEL_1,
    .ch_hin2 = TIM_CHANNEL_2,
    .ch_hin3 = TIM_CHANNEL_3,
    .huart = Encoder1_huart,
    .hadc = &hadc1,
    .Motor_EN_PORT = M1_ENABLE_GPIO_Port,
    .Motor_EN_PIN = M1_ENABLE_Pin,
    .Motor_CAL_EN_PORT = M1_CAL_GPIO_Port,
    .Motor_CAL_EN_PIN = M1_CAL_Pin,
};


const MotorPWM_Config M2_PWM = {
    .htim      = &htim8,            
    .Tim_base  = TIM8,
    .ch_hin1   = TIM_CHANNEL_1,
    .ch_hin2 = TIM_CHANNEL_2,
    .ch_hin3 = TIM_CHANNEL_3,
    .huart = Encoder2_huart,
    .hadc = &hadc4,
    .Motor_EN_PORT = M2_ENABLE_GPIO_Port,
    .Motor_EN_PIN = M2_ENABLE_Pin,
    .Motor_CAL_EN_PORT = M2_CAL_GPIO_Port,
    .Motor_CAL_EN_PIN = M2_CAL_Pin,
};




