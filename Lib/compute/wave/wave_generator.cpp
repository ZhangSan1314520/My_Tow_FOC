#include "wave_generator.hpp"

void WaveGenerator::_update_status(uint16_t delta_tick)
{
    // 周期开始时更新最新配置，避免运行过程中参数突变
    if (_current_tick == 0)
    {
        _wave_type = _new_wave_type;      // 更新波形类型
        _period_tick = _new_period_tick;  // 更新周期
        _gain = _new_gain;                // 更新幅值
        _offset = _new_offset;            // 更新偏置
    }

    // 更新当前计数，超过周期后重新开始
    _current_tick = (_current_tick < _period_tick) ? ((_current_tick + delta_tick) % _period_tick) : 0;
}

float WaveGenerator::_generate_sine_wave(void) // 正弦波计算
{
    // 正弦波公式：y = A*sin(wt)+offset
    float ret_val = _gain * fast_sin(_current_tick * M_2PI / _period_tick) + _offset;

    return ret_val;
}

float WaveGenerator::_generate_square_wave(void) // 方波计算
{
    float ret_val;

    // 前半周期输出负幅值，后半周期输出正幅值
    if (_current_tick < (_period_tick / 2))
    {
        ret_val = -_gain + _offset;  // 方波低电平
    }
    else
    {
        ret_val = _gain + _offset;   // 方波高电平
    }

    return ret_val;
}

float WaveGenerator::_generate_triangle_wave(void) // 三角波计算
{
    float ret_val;

    // 前半周期上升，后半周期下降
    if (_current_tick < (_period_tick / 2))
    {
        ret_val = _gain * (4.0f * _current_tick / _period_tick - 1.0f) + _offset;  // 上升段
    }
    else
    {
        ret_val = _gain * (4.0f * (_period_tick - _current_tick) / _period_tick - 1.0f) + _offset;  // 下降段
    }

    return ret_val;
}

float WaveGenerator::_generate_sawtooth_wave(void) // 锯齿波计算
{
    // 锯齿波从 -gain 线性变化到 gain
    float ret_val = _gain * (2.0f * _current_tick / _period_tick - 1.0f) + _offset;

    return ret_val;
}

float WaveGenerator::_generate_step_wave(void) // 阶梯波计算
{
    // 没有配置阶梯表时输出0
    if (_step_table == nullptr || _step_table_size == 0)
    {
        return 0.0f;
    }

    // 根据当前周期位置计算查表索引
    int index = _current_tick / (_period_tick / _step_table_size);

    return _gain * _step_table[index] + _offset;  // 输出阶梯波
}

float WaveGenerator::_generate_const_wave(void) // 恒定值计算
{
    return _gain;  // 恒定输出幅值
}

float WaveGenerator::next(uint16_t delta_tick)
{
    float ret_val = 0.0f;

    _update_status(delta_tick);  // 更新当前波形状态

    // 根据当前波形类型生成对应输出
    switch (_wave_type)
    {
    case WaveType::NONE:
        ret_val = 0.0f;  // 无输出
        break;

    case WaveType::SINE:
        ret_val = _generate_sine_wave();  // 正弦波
        break;

    case WaveType::SQUARE:
        ret_val = _generate_square_wave();  // 方波
        break;

    case WaveType::TRIANGLE:
        ret_val = _generate_triangle_wave();  // 三角波
        break;

    case WaveType::SAWTOOTH:
        ret_val = _generate_sawtooth_wave();  // 锯齿波
        break;

    case WaveType::STEP:
        ret_val = _generate_step_wave();  // 阶梯波
        break;

    case WaveType::CONST:
        ret_val = _generate_const_wave();  // 恒定值
        break;

    default:
        ret_val = 0.0f;  // 异常类型默认输出0
        break;
    }

    return ret_val;
}