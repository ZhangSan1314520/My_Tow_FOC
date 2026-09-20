#pragma once

#include "ex_math.hpp"

#include "pid.hpp"

#define DPID_STEP_MAX_NUM 5
#define DPID_SLOPE_MAX_NUM 3
#define DPID_POLY_MAX_NUM 5

enum class DPID_FuncType : uint8_t
{
    COMMON, // 普通PID
    POLY,   // 多项式
    STEP,   // 阶梯函数
    SLOPE   // 斜坡函数
};

typedef struct
{
    float kp;
    float ki;
    float kd;
    float dt;
    float max;
    float min;
    float i_max;
    float kff;
} dpid_param_t;

typedef struct
{
    float prev_error;
    float error;
    float nudge;
    float output;
    float P;
    float I;
    float D;
    float FF;
} dpid_info_t;

typedef struct
{
    float kp;
    float ki;
    float kd;
} dpid_common_param_t;

typedef struct
{
    dpid_common_param_t param;
    float dt;
    float max;
    float min;
    float i_max;
} dpid_common_config_t;

typedef struct
{
    float min_error;
    float max_error;
    float kp;
    float ki;
    float kd;
} dpid_step_param_t;

typedef struct
{
    dpid_step_param_t default_param;
    dpid_step_param_t param[DPID_STEP_MAX_NUM];
    int param_num = DPID_STEP_MAX_NUM;
    float dt;
    float min;
    float max;
    float i_max;
} dpid_step_config_t;

typedef struct
{

} dpid_slope_config_t;

typedef struct
{
    float coef[DPID_POLY_MAX_NUM];
    uint8_t num = DPID_POLY_MAX_NUM;
} dpid_poly_coef_t;

typedef struct
{
    dpid_poly_coef_t coef_kp;
    dpid_poly_coef_t coef_ki;
    dpid_poly_coef_t coef_kd;
    float dt;
    float min;
    float max;
    float i_max;
} dpid_poly_config_t;

class DynamicPID
{
    friend class MC_Param;

public:
    void init(DPID_FuncType type);

    bool config_common_func(const dpid_common_config_t *cfg);
    bool config_step_func(const dpid_step_config_t *cfg, float *determinant = NULL, bool absolute = true);
    bool config_slope_func(const dpid_slope_config_t *cfg);
    bool config_poly_func(const dpid_poly_config_t *cfg);

    void change_func_type(DPID_FuncType type);

    float update(float error, float nudge = 0.0f);
    void reset(void);

    bool config_ploy_coef_p(float *_ploy_coef_p, uint8_t _ploy_n);
    bool config_ploy_coef_i(float *_ploy_coef_i, uint8_t _ploy_n);
    bool config_ploy_coef_d(float *_ploy_coef_d, uint8_t _ploy_n);

    float get_kp(void) { return _param.kp; }
    float get_ki(void) { return _param.ki; }
    float get_kd(void) { return _param.kd; }
    float get_dt(void) { return _param.dt; }
    float get_max(void) { return _param.max; }
    float get_min(void) { return _param.min; }
    float get_i_max(void) { return _param.i_max; }

    float get_P(void) { return _info.P; }
    float get_I(void) { return _info.I; }
    float get_D(void) { return _info.D; }
    float get_error(void) { return _info.error; }
    float get_nudge(void) { return _info.nudge; }
    float get_output(void) { return _info.output; }

    dpid_param_t &get_pid_param(void) { return _param; }
    dpid_info_t &get_pid_info(void) { return _info; }

    dpid_common_config_t &get_common_cfg(void) { return _common_cfg; }
    dpid_step_config_t &get_step_cfg(void) { return _step_cfg; }
    dpid_slope_config_t &get_slope_cfg(void) { return _slope_cfg; }
    dpid_poly_config_t &get_poly_cfg(void) { return _poly_cfg; }

private:
    DPID_FuncType _func_type = DPID_FuncType::COMMON;
    float *_determinant = NULL;
    bool _absolute = true;

    /* Common Function Variables */
    dpid_common_config_t _common_cfg;

    /* Step Function Variables */
    dpid_step_config_t _step_cfg;

    /* Slope Function Variables */
    dpid_slope_config_t _slope_cfg;

    /* Poly Function Variables */
    dpid_poly_config_t _poly_cfg;

    /* DPID Variables */
    dpid_param_t _param;
    dpid_info_t _info;

    void _update_params(void);
    void _update_params_by_common_func(void);
    void _update_params_by_step_func(void);
    void _update_params_by_slope_func(void);
    void _update_params_by_poly_func(void);

    void _update_I(void);
    void _reset_I(void);
};