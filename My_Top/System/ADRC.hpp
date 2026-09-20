#pragma once

// ============================================================
// 自抗扰控制器 (Active Disturbance Rejection Control)
//
// 三层结构:
//   ESO（扩张状态观测器）→ PD 控制律 → 扰动补偿
//
// 参数:
//   wc: 控制器带宽 (rad/s)  越大 → 响应快、刚度高
//   wo: 观测器带宽 (rad/s)  越大 → 跟踪准、抗扰强（建议 wo = 3~5×wc）
//   b0: 系统增益估计        越大 → 控制量偏小（需实测标定）
//   dt: 采样周期 (秒)
//
// 使用:
//   ADRC adrc(30.0, 100.0, 250.0, 0.001);
//   float output = adrc.update(target_rad, current_rad);
//   adrc.reset();
// ============================================================

class ADRC
{
public:
    // ============================================================
    // 构造函数
    // @wc:  控制器带宽
    // @wo:  观测器带宽
    // @b0:  系统增益估计
    // @dt:  采样周期
    // ============================================================
    ADRC(float wc, float wo, float b0, float dt)
        : _wc(wc), _wo(wo), _b0(b0), _dt(dt),
          _z1(0), _z2(0), _z3(0), _last_output(0)
    {}

    // ============================================================
    // 核心更新函数（每控制周期调用一次）
    // @target:  目标值 (弧度)
    // @current: 当前值 (弧度)
    // @return:  控制输出 (频率 Hz，可直接喂 Set_Motor_Frequency)
    // ============================================================
    float update(float target, float current)
    {
        // ── 1. ESO 增益 ──
        float beta1 = 3.0f * _wo;
        float beta2 = 3.0f * _wo * _wo;
        float beta3 = _wo * _wo * _wo;

        // ── 2. 扩张状态观测器 ──
        float error_eso = _z1 - current;

        _z1 += _dt * (_z2 - beta1 * error_eso);                    // 位置估计
        _z2 += _dt * (_z3 + _b0 * _last_output - beta2 * error_eso); // 速度估计
        _z3 += _dt * (-beta3 * error_eso);                         // 总扰动估计

        // ── 3. PD 控制律 ──
        float kp = _wc * _wc;
        float kd = 2.0f * _wc;

        float u0 = kp * (target - _z1) - kd * _z2;

        // ── 4. 扰动补偿 ──
        float u = (u0 - _z3) / _b0;

        // ── 5. 保存输出 ──
        _last_output = u;

        return u;
    }

    // ============================================================
    // 重置内部状态
    // ============================================================
    void reset(void)
    {
        _z1 = 0;
        _z2 = 0;
        _z3 = 0;
        _last_output = 0;
    }

    // ============================================================
    // 读取内部状态（调试用）
    // ============================================================
    float get_z1(void) { return _z1; }   // 位置估计
    float get_z2(void) { return _z2; }   // 速度估计
    float get_z3(void) { return _z3; }   // 扰动估计

private:
    float _wc, _wo, _b0, _dt;     // 参数
    float _z1;                      // 位置估计
    float _z2;                      // 速度估计
    float _z3;                      // 总扰动估计（摩擦力/重力/外部力）
    float _last_output;             // 上一次输出（供 ESO 使用）
};
