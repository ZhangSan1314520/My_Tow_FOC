#include "pll.hpp"


/*  wn是自然频率决定了系统的响应速度和带宽  ζ是阻尼比  ts是收敛时间即系统达到稳态的时间 
    ζ≈0.707 时，带宽 ≈ ωn/2pi（近似）
    实测电机最高38.2圈/s =2292rpm  电频率Fe = 2280*7/60=226hz  wn>2pi*Fe/10=142  wn最小限制
    wn<(2pi*4000/10) = 2513.2  wn最大限制
    wn = 3/(ζ*ts) = 3/(0.707*0.002) = 2121.64(Ts最小2ms)
    kp = 2ζωn
    ki = ωn²
    ζ = 0.707

*/
/* 
    PLL_ts = 5ms  →  带宽 ≈ 135Hz  →  不动时波动 0.7 rad/s
    PLL_ts = 10ms →  带宽 ≈ 67Hz   →  波动估计降到 ~0.2 rad/s
    PLL_ts = 20ms →  带宽 ≈ 34Hz   →  更平滑
    采样频率必须 ≥ 带宽频率的 2 倍，否则信号会混叠。（带宽频率的<采样频率/2）
    带宽太低：跟踪不了快速变化的角度，动态响应差
    带宽太高：会把高频噪声也跟踪进来，输出抖动大
*/


/*
reg_in:输入角度 对应数学公式中的 θin
dt:采样周期 两次调用 foc_pll_run 之间的时间间隔，单位为秒
reg_out:输出角度 对应数学公式中的 θout
Angular_velocity:输出角速度 对应数学公式中的 ωout
conf：PI参数配置结构体

*/

//超过实际速度3倍以上需要限制最大速度
void foc_pll_run(float reg_in, float dt, float *reg_out,
                 float *Angular_velocity, PLL_Parameter *conf) {

// volatile float debug_pll_kp = FOC_PLL_KP; 
// volatile float debug_pll_ki = FOC_PLL_KI; 
// volatile float debug_pll_wn = PLL_Wn;
// volatile float debug_PLL_ts = PLL_ts;
// volatile float debug_pll_Ts_min = Ts_min;
// volatile float debug_pll_Ts_max = Ts_max;
// volatile float debug_pll_wn_min = Wn_min;
// volatile float debug_pll_wn_max = Wn_max;
                    

    UTILS_NAN_ZERO(*reg_out);//如果输出角度是非数字(NaN)或无穷大置零
    float delta_theta = reg_in - *reg_out;//计算输入角度与输出角度的误差
    delta_theta = wrap_to_PI(delta_theta);//将误差限制在 -π 到 π 之间
    UTILS_NAN_ZERO(*Angular_velocity);//如果输出角速度是非数字(NaN)或无穷大置零
    // 对应：θ_out[k+1] = θ_out[k] + (ω_out[k] + Kp·Δθ[k]) × Ts
    *reg_out += (*Angular_velocity + conf->foc_pll_kp * delta_theta) * dt; //更新输出角度 310
    *reg_out = wrap_to_PI(*reg_out);//将输出角度限制在 -π 到 π 之间
    *Angular_velocity += conf->foc_pll_ki * delta_theta * dt; //更新输出角速度
}





