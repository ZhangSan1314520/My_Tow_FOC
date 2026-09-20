#include "HAL_System.hpp"
#include "FreeRTOS.h"
#include "task.h"

#define Tim_Number_ofdigits 32

#if Tim_Number_ofdigits == 16
#define nub_reset 0xFFFFFFFFFFFF0000ULL
#elif Tim_Number_ofdigits == 32
#define nub_reset 0xFFFFFFFF00000000ULL
#else
#error "Tim_Number_ofdigits 只能填 16 或 32,当前值不合法"
#endif



uint64_t HAL_System::tick_us = 0; 
uint32_t HAL_System::tick_ms = 0;  

void HAL_System::init(void)
{
    // 启动定时器中断（计数器开始计数）
    HAL_TIM_Base_Start_IT(System_htim);
    System_htim->PeriodElapsedCallback = period_elapsed_callback;
}

uint64_t HAL_System::get_tick_us(void)
{
    // 读取当前 TIM 计数器的低 32 位
    uint32_t current_cnt = __HAL_TIM_GET_COUNTER(System_htim);

    // 拼装 64 位计时器：
    // tick_us 的高 32 位 = 之前的累加值（elapse_cnt × 2^32）
    // tick_us 的低 32 位 = 当前 TIM 计数器值
    // 先把当前的低 32 位清零（& ~0xFFFFFFFF），再或上新的计数器值
    tick_us = (tick_us & (uint64_t)nub_reset) | current_cnt;

    return tick_us;
}



// ============================================================
// 获取当前毫秒数（32 位，由微秒计算得出）
// ============================================================
uint32_t HAL_System::get_tick_ms(void)
{
    // 微秒转毫秒（除以 1000）
    tick_ms = get_tick_us() / 1000;
    return tick_ms;
}

// ============================================================
// 定时器溢出回调函数（每当 TIM 计数器从 0xFFFFFFFF 回到 0 时触发）
// ============================================================
// 注意：此函数运行在中断上下文中！
void HAL_System::period_elapsed_callback(TIM_HandleTypeDef *htim)
{
    tick_us += (uint64_t)1 << Tim_Number_ofdigits;
}


void HAL_System::delay_ms(uint32_t ms)
{
    uint32_t start = get_tick_ms();
    while ((get_tick_ms() - start) < ms) {}
}

