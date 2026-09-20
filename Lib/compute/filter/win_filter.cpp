#include "ex_math.hpp"

#include "FreeRTOS.h"

bool WindowFilter::init(uint8_t win_size)
{
    if (win_size == 0)
        return false;

    sum = 0.f;
    this->win_size = win_size;
    index = 0;
    count = 0;

    pData = (float *)pvPortMalloc(sizeof(float) * win_size);
    if (pData == NULL)
        return false;

    return true;
}

float WindowFilter::filter(float value)
{
    count = count < win_size ? count + 1 : win_size;
    sum = sum - pData[index] + value;
    avg = sum / count;
    pData[index] = value;
    index = (index + 1) % win_size;

    return avg;
}

inline void WindowFilter::clear(void)
{
    index = 0;
    count = 0;
    sum = 0.f;
}