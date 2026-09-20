#include "ex_math.hpp"

void LowpassFilter::init(float alpha)
{
    _alpha = constraint_value(alpha, 0.0f, 1.0f);
    _prev_value = 0.0f;
}

float LowpassFilter::filter(float value) // 低通滤波器算法
{
    /* y=ax+(1-a)*y0 -> y=y0+a(x-y0) */
    /* x 是当前输入值（value），y0 是上一个输出值（prev_value）, 
    a 是滤波系数（alpha） ,y 是当前输出值（ret） */
    float ret = _prev_value + _alpha * (value - _prev_value);
    _prev_value = ret;
    return ret;
}

float LowpassFilter::filter_with_alpha(float value, float alpha)
{
    float ret = _prev_value + alpha * (value - _prev_value);
    _prev_value = ret;
    return ret;
}

void LowpassFilter::clear(void)
{
    _prev_value = 0.0f;
}