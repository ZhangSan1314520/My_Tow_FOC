#pragma once

#include "ex_math.hpp"
#include "fast_sin.h"

enum class WaveType
{
    NONE,       // 无波形输出
    SINE,       // 正弦波
    SQUARE,     // 方波
    TRIANGLE,   // 三角波
    SAWTOOTH,   // 锯齿波
    STEP,       // 步进波
    CONST,      // 常值波
};

class WaveGenerator
{
public:
    WaveGenerator()
    {
        _wave_type = WaveType::NONE;      // 默认无波形
        _period_tick = 1000;              // 默认周期
        _gain = 1.0f;                     // 默认幅值
        _offset = 0.0f;                   // 默认偏置

        _new_wave_type = WaveType::NONE;  // 待更新波形
        _new_period_tick = 1000;          // 待更新周期
        _new_gain = 1.0f;                 // 待更新幅值
        _new_offset = 0.0f;               // 待更新偏置

        _current_tick = 0;                // 当前周期位置清零
    }

    WaveGenerator(WaveType type, uint16_t period_tick = 1000, float gain = 1.0f, float offset = 0.0f)
    {
        _wave_type = type;                // 设置当前波形类型
        _period_tick = period_tick;       // 设置周期
        _gain = gain;                     // 设置幅值
        _offset = offset;                 // 设置偏置

        _new_wave_type = type;            // 保存待更新波形
        _new_period_tick = period_tick;   // 保存待更新周期
        _new_gain = gain;                  // 保存待更新幅值
        _new_offset = offset;              // 保存待更新偏置

        _current_tick = 0;                // 周期位置清零
    }

    void change_wave_type(WaveType type) { _new_wave_type = type; }  // 修改波形类型

    void reset_tick(void) { _current_tick = 0; }  // 重置当前周期位置

    void set_period_tick(uint16_t period_tick) { _new_period_tick = period_tick; }  // 设置周期
    void set_gain(float gain) { _new_gain = gain; }                                  // 设置幅值
    void set_offset(float offset) { _new_offset = offset; }                          // 设置偏置

    uint16_t get_current_tick(void) { return _current_tick; }  // 获取当前tick
    uint16_t get_period_tick(void) { return _new_period_tick; } // 获取周期
    float get_gain(void) { return _new_gain; }                  // 获取幅值
    float get_offset(void) { return _new_offset; }              // 获取偏置

    void add(uint16_t delta_tick = 5) { _update_status(delta_tick); }  // 仅更新波形状态
    float next(uint16_t delta_tick = 5);                              // 更新并输出波形值

    void config_step(const float *step_table, uint16_t step_table_size)
    {
        _step_table = step_table;              // 设置阶梯波数据表
        _step_table_size = step_table_size;    // 设置阶梯数量
    }

protected:
    uint16_t _current_tick;       // 当前周期计数
    uint16_t _period_tick;        // 当前周期长度
    float _gain;                  // 当前幅值
    float _offset;                // 当前偏置

    uint16_t _new_period_tick;    // 待更新周期
    float _new_gain;              // 待更新幅值
    float _new_offset;            // 待更新偏置

private:
    WaveType _wave_type;          // 当前波形类型
    WaveType _new_wave_type;      // 待更新波形类型

    const float *_step_table = NULL;  // 阶梯波数据表
    uint16_t _step_table_size = 0;    // 阶梯波数量

    void _update_status(uint16_t delta_tick);  // 更新波形状态

    float _generate_sine_wave(void);      // 生成正弦波
    float _generate_square_wave(void);     // 生成方波
    float _generate_triangle_wave(void);   // 生成三角波
    float _generate_sawtooth_wave(void);   // 生成锯齿波
    float _generate_step_wave(void);       // 生成阶梯波
    float _generate_const_wave(void);      // 生成常值波
};