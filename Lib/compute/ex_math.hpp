#pragma once

#include <stdint.h>

#define M_SQRT3_2 0.86602540378443864676372317075294f   //√3/2
#define M_SQRT3_3 0.57735026918962576450914878050196f   //√3/3  
#define M_2SQRT3_3 1.1547005383792515290182975610039f   //2*√3/3 
#define M_2PI 6.283185307179586476925286766559f   //2*pi

#include <math.h>

#include "fast_sin.h"

#define constraint_value(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))
#define min_value(x, y) ((x) < (y) ? (x) : (y))
#define max_value(x, y) ((x) > (y) ? (x) : (y))
#define deg2rad(x) ((x) * (M_PI / 180.0f)) //0-360角度转换为弧度
#define rad2deg(x) ((x) * (180.0f / M_PI)) //弧度转换为0-360角度
#define rad2q31(x) (int32_t)((x) * (float)(2147483648.0f / M_PI))
#define q31_2rad(x) (float)((x) * (float)(M_PI / 2147483648.0f))
#define UTILS_NAN_ZERO(x)   if (isnan(x) || isinf(x)) { x = 0.0; } //如果是非数字(NaN)或无穷大置零

// 表达式语句的值 = 最后一条语句的值
#define My_normalizeAngle(angle) ({ \
    float a__ = fmodf((angle), M_TWOPI); \
    a__ >= 0 ? a__ : (a__ + M_TWOPI); \
}) //将输入的角度限制在0到2π范围内 

#define wrap_to_PI(a) ({ \
    float r__ = fmodf((a) + M_PI, M_2PI); \
    if (r__ < 0) r__ += M_2PI; \
    r__ - M_PI; \
}) //将输入的角度限制在-π到π范围内




enum class FilterType : uint8_t
{
    None,
    Window,
    Lowpass,
    Average
};

class WindowFilter
{
public:
    bool init(uint8_t win_size);
    float filter(float value);
    void clear(void);

    inline uint8_t get_count(void) { return count; }
    inline float get_sum(void) { return sum; }
    inline float get_avg(void) { return avg; }

private:
    uint8_t win_size;
    float *pData;
    uint8_t index;
    uint8_t count;
    float sum;
    float avg;
};

class LowpassFilter
{
public:
    void init(float alpha = 0.2f);
    float filter(float value);
    float filter_with_alpha(float value, float alpha);
    void clear(void);
    void set_alpha(float alpha) { _alpha = alpha; }

private:
    float _alpha;
    float _prev_value;
};

class AvgFilter
{
public:
    bool init(uint8_t win_size);
    float filter(float value);
    void clear(void);

    inline uint8_t get_count(void) { return _count; }
    inline float get_sum(void) { return _sum; }
    inline float get_avg(void) { return _avg; }

private:
    uint8_t _size;
    float *pData;
    uint8_t _index;
    uint8_t _count;
    float _sum;
    float _avg;
    float _max;
    float _min;
};

static inline float polynomial(float x, float *coeff, uint16_t degree)
{
    if (coeff == NULL || degree == 0)
        return 0.f;

    // 按照霍纳法则，从最高次项开始逐步计算
    float result = coeff[degree - 1];
    for (int i = degree - 2; i >= 0; i--)
    {
        result = result * x + coeff[i];
    }

    return result;
}

bool is_number(const char *str);
bool is_integer(const char *str);
bool is_float(const char *str);