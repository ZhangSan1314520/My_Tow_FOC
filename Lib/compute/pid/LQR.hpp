#pragma once 
#include "ex_math.hpp"

// class LQR //不带积分
// {
// public:
//     float _k1;   // 位置误差增益（离线算好）
//     float _k2;   // 速度误差增益（离线算好）
//     float _max;  // 输出上限
//     float _min;  // 输出下限

//     // 构造函数，带默认参数
//     LQR(float k1 = 0.0f, float k2 = 0.0f, float max = 1.0f, float min = -1.0f)
//         : _k1(k1), _k2(k2), _max(max), _min(min), _output(0.0f) {}

//     float update(float pos_error, float vel_error)
//     {
//         // u = -(K1 * pos_error + K2 * vel_error)
//         _output = _k1 * pos_error + _k2 * vel_error;

//         // 限幅
//         if (_output > _max)
//             _output = _max;
//         else if (_output < _min)
//             _output = _min;

//         return _output;
//     }
//     void reset(void)
//     {
//         _output = 0.0f;
//     }

// private:
//     float _output;  // LQR 输出
// };






class LQR //带积分
{
public:
    float _k1;   // 位置误差增益 响应速度
    float _k2;   // 速度误差增益 到达目标刹车力度 防止超调
    float _max;  // 输出上限
    float _min;  // 输出下限
    float _integral;  // 积分项

    // 构造函数，带默认参数
    LQR(float k1 = 0.0f, float k2 = 0.0f, float max = 1.0f, float min = -1.0f)
        : _k1(k1), _k2(k2), _max(max), _min(min), _output(0.0f) {}

    float update(float pos_error, float vel_error)
    {

        if(fabs(pos_error)<deg2rad(5.0f))
        {
            _k1 = 10.0f;
            _integral += pos_error * 0.33f; //频率1k
        }else
        {
            _k1 = 3.5;
            _integral=0;
        }

        if(_integral > 5.0f)
            _integral=5.0f;

        if(_integral < -5.0f)
            _integral=-5.0f;
        _output = _k1 * pos_error + _k2 * vel_error+ 0.1f*_integral;

        // 限幅
        if (_output > _max)
            _output = _max;
        else if (_output < _min)
            _output = _min;

        return _output;
    }
    void reset(void)
    {
        _output = 0.0f;
        _integral=0;

    }

private:
    float _output;  // LQR 输出
};


