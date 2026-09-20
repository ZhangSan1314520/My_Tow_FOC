// #include "my_tim.hpp"


// void My_Tim_OC_Callback(TIM_HandleTypeDef *htim)
// {
//     static uint16_t count = 0;
//     static uint32_t start_time, end_time = 0;//求计算时间
//     FOC_Motor* motors[] = {&M1};
//     count = (count % 16000) + 1;
//     for (int i = 0; i < 1; i++)
//     {
//         FOC_Motor* m = motors[i];
//         if (!m->control_init_flag) continue;

//         if (count % FOC_CURRRENT_LOOP_FREQ_DIV == 0)
//         {
//             m->Update_FOC_Current(); //耗时
//             if(m->work_mode == Ia_loop ||m->work_mode == speed_loop || m->work_mode == position_loop)
//             {
//                 start_time = HAL_System::get_tick_us(); //记录开始时间
//                 m->FOC_Current_Loop(); //耗时9us
//                 end_time = HAL_System::get_tick_us(); // 记录结束时间
//                 m->laji = (float)(end_time - start_time);  // 计算时间差
//             }
//         }
//     }
    
// }


// void My_Tim_Callback(TIM_HandleTypeDef *htim)
// {
//     static uint16_t count = 0;
//     static uint32_t start_time, end_time = 0;//求计算时间
//     if ((htim->Instance->CR1 & 0x10) != 0x00U) return; //不是向上计数时，不执行
//     count = (count % 16000) + 1;

//     FOC_Motor* motors[] = { &M1};

//     for (int i = 0; i < 1; i++)
//     {
//         FOC_Motor* m = motors[i];
//         if (!m->control_init_flag) continue;

//         if (count % FOC_VELOCITY_UP_FREQ_DIV == 0)
//         {
//             // start_time = HAL_System::get_tick_us(); //记录开始时间
//             m->Update_Speed_Angle_LPFAndPLL(); //耗时24us 
//             // end_time = HAL_System::get_tick_us(); // 记录结束时间
//             // m->laji = (float)(end_time - start_time);  // 计算时间差
//         }

//         switch (m->work_mode)
//         {
//         case open_loop: // 开环控制
//             if (count % FOC_Open_Loop_FREQ_DIV == 0) 
//             {
//                 // start_time = HAL_System::get_tick_us(); //记录开始时间
//                 m->FOC_Open_Loop1(); //耗时8us
//                 // end_time = HAL_System::get_tick_us(); // 记录结束时间
//                 // m->laji = (float)(end_time - start_time);  // 计算时间差
//             }
//             break;

//         case speed_loop: // 速度控制
//             if (count % FOC_VELOCITY_LOOP_FREQ_DIV == 0)
//             {
//                 // start_time = HAL_System::get_tick_us(); //记录开始时间
//                 m->FOC_Speed_Loop();//耗时
//                 // end_time = HAL_System::get_tick_us(); // 记录结束时间
//                 // m->laji = (float)(end_time - start_time);  // 计算时间差
//             }
//             break;

//         case position_loop: // 位置控制 
//             if (count % FOC_POSITION_LOOP_FREQ_DIV == 0)
//             {
//                 // start_time = HAL_System::get_tick_us(); //记录开始时间
//                 m->FOC_Location_Loop();//耗时
//                 // end_time = HAL_System::get_tick_us(); // 记录结束时间
//                 // m->laji = (float)(end_time - start_time);  // 计算时间差
//             }
//             if (count % FOC_VELOCITY_LOOP_FREQ_DIV == 0)
//             {
//                 m->FOC_Speed_Loop();//耗时
//             }
//             break;

//         case EncoderCalibration: //编码器校准模式
//             if (count % 160 == 0)
//             {
                
//             }
//             break;
//         }
//     }

    
// }





// /*
// void laji()
// {
//     static uint32_t last_time = 0;
//     uint32_t now_time = 0;
//     now_time =  HAL_System::get_tick_us();
//     if (last_time != 0) {
//          M1.laji = (int32_t)(now_time - last_time);  // 单次中断间隔，单位us
//     }
//     last_time = now_time;
// }
// */

// /**
//  * @brief 初始化定时器并设置回调函数
//  * @param htim 定时器句柄指针，指向定时器配置结构体
//  */

// void My_Tim_Init(TIM_HandleTypeDef* htim)
// {
//     uint16_t ARR = __HAL_TIM_GET_AUTORELOAD(htim);// 获取定时器ARR寄存器的值
    
//     __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_4, ARR*0.84f); // 设置占空比为84% 控制My_Tim_OC_Callback回调函数的触发时机
//     __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_5, ARR*0.80f); // 设置占空比为80% 控制电流采样触发时机 和回调函数无关
//     HAL_TIM_OC_Start_IT(htim, TIM_CHANNEL_5); //启动定时器比较中断
//     HAL_TIM_Base_Start_IT(htim); //启动定时器周期中断和启动定时器计数
//     htim->PeriodElapsedCallback = My_Tim_Callback;  // 设置定时器周期结束回调函数
//     htim->OC_DelayElapsedCallback = My_Tim_OC_Callback;// PWM的模式的比较匹配回调函数
//     __HAL_TIM_ENABLE_IT(htim, TIM_IT_CC4); //只打开中断，不启动定时器
//     __HAL_TIM_ENABLE(htim); // 启动定时器
//     HAL_TIM_PWM_Start(htim, TIM_CHANNEL_5);// 启动 TIM1_CH5 用于产生ADC触发事件 ????

// }




