#pragma once

#include "tim.h"

#include "bsp.hpp"

class HAL_System
{
public:
    static void init(void);

    static uint64_t get_tick_us(void);
    static uint32_t get_tick_ms(void);
    static void delay_ms(uint32_t ms);

private:
    static uint64_t tick_us;
    static uint32_t tick_ms;
    static uint32_t elapse_cnt;

    static void period_elapsed_callback(TIM_HandleTypeDef *htim);
};

