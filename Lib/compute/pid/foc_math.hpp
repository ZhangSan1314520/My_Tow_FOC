#pragma once

#include <stdint.h>
#include "ex_math.hpp"


class FOC_Transform
{

public:

    // ============================================================
    // Clarke 克拉克变换
    // ============================================================
    void Clarke(float ia, float ib)
    {
        i_a = ia;
        i_b = ib;

        i_alpha = i_a;

        // i_beta = (i_a + 2*i_b) / √3
        i_beta = i_a * M_SQRT3_3 + i_b * M_2SQRT3_3;
    }



    // ============================================================
    // Park 变换
    // ============================================================
    void Park(float sine, float cosine)
    {
        i_d = i_alpha * cosine + i_beta * sine;

        i_q = -i_alpha * sine + i_beta * cosine;
    }



    // ============================================================
    // Park逆变换
    // ============================================================
    void IPark(float ud, float uq, float sine, float cosine)
    {
        u_d = ud;
        u_q = uq;

        u_alpha = u_d * cosine - u_q * sine;

        u_beta = u_d * sine + u_q * cosine;
    } 



    // ============================================================
    // SVPWM
    // ============================================================
    void SVPWM()
    {
        constexpr float ts = 1.0f;


        float u1 = u_beta;

        float u2 = -(float)(M_SQRT3_2)*u_alpha - 0.5f*u_beta;

        float u3 = (float)(M_SQRT3_2)*u_alpha - 0.5f*u_beta;



        // 确定SVPWM扇区
        uint8_t sector =
        ((uint8_t)(u1 >= 0.0f) << 0) |
        ((uint8_t)(u2 >= 0.0f) << 1) |
        ((uint8_t)(u3 >= 0.0f) << 2);



        // ============================================================
        // 扇区0、7
        // ============================================================
        if(sector == 0 || sector == 7)
        {
            duty_a = 0;
            duty_b = 0;
            duty_c = 0;
        }


        // ============================================================
        // 扇区5
        // ============================================================
        else if(sector == 5) //101
        {
            float t4 = u3;
            float t6 = u1;

            float sum = t4 + t6;


            if(sum > ts)
            {
                k_svpwm = ts / sum;

                t4 *= k_svpwm;
                t6 *= k_svpwm;
            }


            float t7 = (ts - t4 - t6) * 0.5f;


            duty_a = t4 + t6 + t7;
            duty_b = t6 + t7;
            duty_c = t7;
        }


        // ============================================================
        // 扇区1
        // ============================================================
        else if(sector == 1) //001
        {
            float t2 = -u3;
            float t6 = -u2;

            float sum = t2 + t6;


            if(sum > ts)
            {
                k_svpwm = ts / sum;

                t2 *= k_svpwm;
                t6 *= k_svpwm;
            }


            float t7 = (ts - t2 - t6) * 0.5f;


            duty_a = t6 + t7;
            duty_b = t2 + t6 + t7;
            duty_c = t7;
        }

        else if (sector == 3) // 扇区3 011
        {
            float t2 = u1;
            float t3 = u2;
            float sum = t2 + t3;
            if (sum > ts)
            {
                k_svpwm = ts / sum;
                t2 = k_svpwm * t2;
                t3 = k_svpwm * t3;
            }
            float t7 = (ts - t2 - t3) * 0.5f;
            duty_a = t7;
            duty_b = t2 + t3 + t7;
            duty_c = t3 + t7;
        }
        else if (sector == 2) // 扇区2 010
        {
            float t1 = -u1;
            float t3 = -u3;
            float sum = t1 + t3;
            if (sum > ts)
            {
                k_svpwm = ts / sum;
                t1 = k_svpwm * t1;
                t3 = k_svpwm * t3;
            }
            float t7 = (ts - t1 - t3) * 0.5f;
            duty_a = t7;
            duty_b = t3 + t7;
            duty_c = t1 + t3 + t7;
        }
        else if (sector == 6) // 扇区6 111
        {
            float t1 = u2;
            float t5 = u3;
            float sum = t1 + t5;
            if (sum > ts)
            {
                k_svpwm = ts / sum;
                t1 = k_svpwm * t1;
                t5 = k_svpwm * t5;
            }
            float t7 = (ts - t1 - t5) * 0.5f;
            duty_a = t5 + t7;
            duty_b = t7;
            duty_c = t1 + t5 + t7;
        }
        else if (sector == 4) // 扇区4 100
        {
            float t4 = -u2;
            float t5 = -u1;
            float sum = t4 + t5;
            if (sum > ts)
            {
                k_svpwm = ts / sum;
                t4 = k_svpwm * t4;
                t5 = k_svpwm * t5;
            }
            float t7 = (ts - t4 - t5) * 0.5f;
            duty_a = t4 + t5 + t7;
            duty_b = t7;
            duty_c = t5 + t7;
        }

    }




public:


    // ============================================================
    // Clarke输出
    // ============================================================

    float i_alpha = 0;

    float i_beta = 0;



    // ============================================================
    // Park输出
    // ============================================================

    float i_d = 0;

    float i_q = 0;



    // ============================================================
    // IPark输入
    // ============================================================

    float u_d = 0;

    float u_q = 0;



    // ============================================================
    // αβ电压
    // ============================================================

    float u_alpha = 0;

    float u_beta = 0;



    // ============================================================
    // SVPWM输出
    // ============================================================

    float duty_a = 0;

    float duty_b = 0;

    float duty_c = 0;



private:


    float i_a = 0;

    float i_b = 0;


    float k_svpwm = 1.0f;

};