#pragma once
#include "ex_math.hpp"

#include "bsp.hpp"


#define Motor_RPM (12) //电机最大转速 实测电机最高0.2圈/s =12rpm
// #define Motor_PolesNub (7) //极对数 ???
// #define Motor_Fe (Motor_RPM*Motor_PolesNub/60) //
// #define Wn_min  (M_2PI*Motor_Fe/10) //最小自然频率 = 2pi*Fe/10 = 219.911482 


#define Stepper_Max_RPS (Motor_RPM / 60.0f) //电机最大转速 每秒转的圈数
#define Wn_min  (M_2PI * Stepper_Max_RPS / 10) // 最小带宽 3.30

#define Wn_max  (M_2PI*FOC_VELOCITY_UP_FREQ_HZ/10) //最大自然频率 = 2pi*采样周期/10   2513.2 
#define Ts_min  (3.0/(PLL_ZETA*Wn_max)*1000) //最小收敛时间 Ts = 3/(ζ*Wn_max) = 1.688ms 
#define Ts_max  (3.0/(PLL_ZETA*Wn_min)*1000) //最大收敛时间 Ts = 3/(ζ*Wn_min) =19.2954ms /1286

#define PLL_FREQ_Dt   (1.0/FOC_VELOCITY_UP_FREQ_HZ)//采样周期
#define PLL_ts_temp   (100) //收敛时间 单位ms 
#define PLL_ts  constraint_value(PLL_ts_temp, Ts_min, Ts_max) //约束收敛时间在最小和最大值之间

// #define PLL_ts 1 
#define PLL_ZETA (0.707) //阻尼比
#define PLL_Wn   (3.0/(PLL_ZETA*PLL_ts*0.001)) //223.33hz自然频率 计算公式 wn = 3/(ζ*ts)  4k时ts最低要大于等于1.688ms
#define FOC_PLL_KP    (2*PLL_ZETA*PLL_Wn) //PI参数计算公式 kp = 2ζωn = 310.9548 
#define FOC_PLL_KI    (PLL_Wn*PLL_Wn) //PI参数计算公式 ki = ωn² = 48361.06 



struct PLL_Parameter
{
    PLL_Parameter(double kp, double ki) : foc_pll_kp(kp), foc_pll_ki(ki) {}
    double foc_pll_kp ;
    double foc_pll_ki ;
};



void foc_pll_run(float reg_in, float dt, float *reg_out,float *Angular_velocity, PLL_Parameter *conf);



