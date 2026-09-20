#pragma once

// ============================================================
// 增量式 PID 控制器
//
// 公式:
//   Δu(k) = Kp * (e(k) - e(k-1))
//         + Ki * T * e(k)
//         + (Kd / T) * (e(k) - 2*e(k-1) + e(k-2))
//
//   u(k) = u(k-1) + Δu(k)
//
// 其中 T = dt（采样周期）
// ============================================================


class PID_Increment
{
public:
    float _kp, _ki, _kd;
    PID_Increment(void) //定义时无参的构造函数格式
    {
        _kp = _ki = _kd = _dt = 0.0f;
        _max = _min = 0.0f;
        _e_prev = _e_prev2 = 0.0f;
        _delta_u = 0.0f;
    }

    PID_Increment(float kp, float ki, float kd, float dt, float max, float min) //定义时带参的构造函数格式
    {
        _kp = kp;
        _ki = ki;
        _kd = kd;
        _dt = dt;
        _max = max;
        _min = min;
        _e_prev = _e_prev2 = 0.0f;
        _delta_u = 0.0f;
    }

    // ============================================================
    // 核心更新函数
    // @error: 当前误差 e(k)
    // @return: 限幅后的输出值 u(k)
    // ============================================================
    float update(float error) 
    {                         
        
        // Δu = Kp * (e - e_prev) + Ki * T * e + (Kd / T) * (e - 2*e_prev + e_prev2)
        float p_term = _kp * (error - _e_prev);
        float i_term = _ki * _dt * error;
        float d_term = (_kd / _dt) * (error - 2.0f * _e_prev + _e_prev2);

        _delta_u = p_term + i_term + d_term; // 计算增量

        // 限幅增量输出
        if (_delta_u > _max)
            _delta_u = _max;
        else if (_delta_u < _min)
            _delta_u = _min;

        // 更新误差历史
        _e_prev2 = _e_prev; //更新上上次误差
        _e_prev  = error; //更新上一次误差

        return _delta_u; // 返回增量输出
    }

    // ============================================================
    // 重置内部状态（清零历史误差和累计输出）
    // ============================================================
    void reset(void)
    {
        
        _e_prev  = 0.0f;
        _e_prev2 = 0.0f;
        _delta_u = 0.0f;
    }
private:
    float _dt, _max, _min;
    float _e_prev;      // e(k-1)，上一次误差
    float _e_prev2;     // e(k-2)，上上次误差
    float _delta_u;     // Δu(k)，当前增量
};



