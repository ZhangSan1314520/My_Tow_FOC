#pragma once
#include "FocMotor_ClosedLoop.hpp"

class My_Tim
{
public:
    My_Tim(FOC_Motor* motor)
        : _motor(motor)
        {
            if (_num < 4) { _list[_num++] = this; }   // 构造即注册（Encoder 是 init 注册，等价）
        }
    void init();   // 初始化定时器并设置回调函数

    //静态变量(无My_Tim* this,TIM_HandleTypeDef *htim)
    static void My_Tim_Callback(TIM_HandleTypeDef *htim);     // 周期中断：静态桥接（给 HAL 用）
    static void My_Tim_OC_Callback(TIM_HandleTypeDef *htim);  // OC 匹配中断：静态桥接（给 HAL 用）

private:
    void Tim_Callback();   // 真正的周期处理（原 My_Tim_Callback 内容）
    void OC_Callback();    // 真正的 OC 处理（原 My_Tim_OC_Callback 内容）

    FOC_Motor*         _motor;  // 本实例的电机

    uint16_t _count = 0;    // 周期计数
    uint16_t _oc_count = 0; // OC 计数

    static My_Tim* _list[4];   // 实例列表（桥接函数按 htim 匹配用）
    static uint8_t _num;       // 已注册实例数
};

extern My_Tim M1_TIM;   // M1 的定时器实例
extern My_Tim M2_TIM;   // M1 的定时器实例
