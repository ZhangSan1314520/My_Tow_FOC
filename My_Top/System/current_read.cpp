#include "current_read.hpp"


void PhaseCurrent::ADC_Injected_Init()
{
    filter_ia.init(0.75);
    filter_ib.init(0.75);
    filter_ic.init(0.75);
    avg_filter_ia.init(10);
    avg_filter_ib.init(10);
    avg_filter_ic.init(10);
    if (HAL_ADCEx_InjectedStart(motor_config->hadc) != HAL_OK)                       // 开始 注入组转换
    {
        Error_Handler();
    }
}

void PhaseCurrent::Current_Calibration()//电流零点校准
{
     // 条件：电机不通电（PWM 全为 0，驱动器使能但无输出）
    //   → 相电流 = 0
    //   → 此时 ADC 读到的值就是 offset
    uint32_t adc_val[3] = {0};
    uint32_t JDR1_Temp = 0, JDR2_Temp = 0, JDR3_Temp = 0;
    __HAL_TIM_SET_COMPARE(motor_config->htim, motor_config->ch_hin1, (uint32_t)500); // 设置占空比为0
    __HAL_TIM_SET_COMPARE(motor_config->htim, motor_config->ch_hin2, (uint32_t)500);
    __HAL_TIM_SET_COMPARE(motor_config->htim, motor_config->ch_hin3, (uint32_t)500);

    JDR1_Temp = motor_config->hadc->Instance->JDR1; // 读注入组的ADC的3个通道
    JDR2_Temp = motor_config->hadc->Instance->JDR2;
    JDR3_Temp = motor_config->hadc->Instance->JDR3;



    Zero_IA = (double)(JDR1_Temp) * MOTOR_ADC_RAW_TO_CURR_GAIN; // 取最后一次的值
    Zero_IB = (double)(JDR2_Temp) * MOTOR_ADC_RAW_TO_CURR_GAIN; // 取最后一次的值
    Zero_IC = (double)(JDR3_Temp) * MOTOR_ADC_RAW_TO_CURR_GAIN; // 取最后一次的值

    Zero_IA_avg = avg_filter_ia.filter(Zero_IA); // 平均值滤波
    Zero_IB_avg = avg_filter_ib.filter(Zero_IB); // 平均值滤波
    Zero_IC_avg = avg_filter_ic.filter(Zero_IC); // 平均值滤波
}


void PhaseCurrent::Update_Current() // 更新电机电流
{
    uint32_t adc_val[3] = {0};
    uint32_t vm = 0;
    float _I_noise = 0;
    adc_val[0] = motor_config->hadc->Instance->JDR1; // 读注入组的ADC的3个通道
    adc_val[1] = motor_config->hadc->Instance->JDR2;
    adc_val[2] = motor_config->hadc->Instance->JDR3;
    vm = motor_config->hadc->Instance->JDR4;//未配置通道4

    Primitive_UA = adc_val[0]; // 原始电压A相 0-4096
    Primitive_UB = adc_val[1]; // 原始电压B相
    Primitive_UC = adc_val[2]; // 原始电压C相
    VM = (float)vm*MOTOR_VM_GAIN ; //

    Primitive_IA = (float)Primitive_UA * MOTOR_ADC_RAW_TO_CURR_GAIN; // 原始电流大小 7.955
    Primitive_IB = (float)Primitive_UB * MOTOR_ADC_RAW_TO_CURR_GAIN;
    Primitive_IC = (float)Primitive_UC * MOTOR_ADC_RAW_TO_CURR_GAIN;

    Zero_Offset_IA = Primitive_IA - Zero_IA_avg; // 零点偏移电流大小
    Zero_Offset_IB = Primitive_IB - Zero_IB_avg;
    Zero_Offset_IC = Primitive_IC - Zero_IC_avg;

    // Zero_Offset_IA = Zero_IA_avg - Primitive_IA; // 零点偏移电流大小
    // Zero_Offset_IB = Zero_IB_avg - Primitive_IB;
    // Zero_Offset_IC = Zero_IC_avg - Primitive_IC;

    _I_noise = (Zero_Offset_IA + Zero_Offset_IB + Zero_Offset_IC) / 3.0f; // 求出三相电流之和的平均值（约等于0）
    Zero_Offset_IA -= _I_noise;                                                                         // 对0点偏移电流大小进行 消去共模电流大小的电流
    Zero_Offset_IB -= _I_noise;
    Zero_Offset_IC -= _I_noise;

    IA = Zero_Offset_IA;
    IB = Zero_Offset_IB;
    IC = Zero_Offset_IC;

    // IA_filtered = filter_ia.filter(IA); // 对零点偏移电流进行低通滤波
    // IB_filtered = filter_ib.filter(IB); // 对零点偏移电流进行低通滤波
    // IC_filtered = filter_ic.filter(IC); // 对零点偏移电流进行低通滤波
    IA_filtered = Zero_Offset_IA;
    IB_filtered = Zero_Offset_IB;
    IC_filtered = Zero_Offset_IC;

}
