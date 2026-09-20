#include "dynamic_pid.hpp"

#include <string.h>

void DynamicPID::init(DPID_FuncType type)
{
    _func_type = type;

    reset();
}

bool DynamicPID::config_common_func(const dpid_common_config_t *cfg)
{
    if (cfg == NULL)
        return false;

    _common_cfg = *cfg;

    return true;
}

bool DynamicPID::config_step_func(const dpid_step_config_t *cfg, float *determinant, bool absolute)
{
    if (cfg == NULL)
        return false;

    _step_cfg = *cfg;
    _determinant = determinant;
    _absolute = absolute;

    return true;
}

bool DynamicPID::config_slope_func(const dpid_slope_config_t *cfg)
{
    if (cfg == NULL)
        return false;

    _slope_cfg = *cfg;

    return true;
}

bool DynamicPID::config_poly_func(const dpid_poly_config_t *cfg)
{
    if (cfg == NULL)
        return false;

    _poly_cfg = *cfg;

    return true;
}

void DynamicPID::change_func_type(DPID_FuncType type)
{
    _func_type = type;
    reset();
}

float DynamicPID::update(float error, float nudge)
{
    _info.error = error;
    _info.nudge = nudge;

    _update_params();

    // Proportional term
    _info.P = _param.kp * error;

    // Integral term
    // _integral += error * _dt;
    _update_I();

    // Derivative term
    float derivative = (error - _info.prev_error) / _param.dt;
    _info.D = _param.kd * derivative;

    // Feedforward term
    _info.FF = _param.kff * (derivative + error);

    // Calculate output
    _info.output = _info.P + _info.I - _info.D + nudge;

    // Clamp output
    if (_info.output > _param.max)
        _info.output = _param.max;
    else if (_info.output < _param.min)
        _info.output = _param.min;

    // Save error to previous error
    _info.prev_error = error;

    return _info.output;
}

void DynamicPID::reset(void)
{
    memset(&_info, 0, sizeof(_info));
    // _reset_I();
    // _info.prev_error = 0;
    // _info.P = _info.I = _info.D = _info.error = _info.nudge = _info.output = 0.0f;
}

void DynamicPID::_update_I(void)
{
    if ((_info.I > 0.0f && _info.error < 0.0f) ||
        (_info.error > 0.0f && _info.I < 0.0f))
    {
        _info.I += _param.ki * _info.error * _param.dt * 1.4f;
    }
    else
    {
        _info.I += _param.ki * _info.error * _param.dt;
    }
    _info.I = constraint_value(_info.I, -_param.i_max, _param.i_max);
}

void DynamicPID::_reset_I(void)
{
    _info.I = 0.0f;
}

void DynamicPID::_update_params(void)
{
    switch (_func_type)
    {
    case DPID_FuncType::COMMON:
        _update_params_by_common_func();
        break;
    case DPID_FuncType::STEP:
        _update_params_by_step_func();
        break;
    case DPID_FuncType::SLOPE:
        _update_params_by_slope_func();
        break;
    case DPID_FuncType::POLY:
        _update_params_by_poly_func();
        break;
    default:
        break;
    }
}

void DynamicPID::_update_params_by_common_func(void)
{
    _param.kp = _common_cfg.param.kp;
    _param.ki = _common_cfg.param.ki;
    _param.kd = _common_cfg.param.kd;
}

void DynamicPID::_update_params_by_step_func(void)
{
    float deter_val;

    _param.dt = _step_cfg.dt;
    _param.max = _step_cfg.max;
    _param.min = _step_cfg.min;
    _param.i_max = _step_cfg.i_max;

    deter_val = (_determinant == NULL) ? (_info.error) : (*_determinant);

    if (_absolute)
    {
        deter_val = (deter_val >= 0.0f) ? (deter_val) : (-deter_val);
    }

    for (int i = 0; i < _step_cfg.param_num; i++)
    {
        if ((deter_val >= _step_cfg.param[i].min_error) &&
            (deter_val < _step_cfg.param[i].max_error))
        {
            _param.kp = _step_cfg.param[i].kp;
            _param.ki = _step_cfg.param[i].ki;
            _param.kd = _step_cfg.param[i].kd;
            return;
        }
    }

    _param.kp = _step_cfg.default_param.kp;
    _param.ki = _step_cfg.default_param.ki;
    _param.kd = _step_cfg.default_param.kd;
}

void DynamicPID::_update_params_by_slope_func(void)
{
}

void DynamicPID::_update_params_by_poly_func(void)
{
    // _param.kp = polynomial(_info.error, _poly_coef[0], _poly_n[0]);
    // _param.ki = polynomial(_info.error, _poly_coef[1], _poly_n[1]);
    // _param.kd = polynomial(_info.error, _poly_coef[2], _poly_n[2]);
    _param.kp = polynomial(_info.error, _poly_cfg.coef_kp.coef, _poly_cfg.coef_kp.num);
    _param.ki = polynomial(_info.error, _poly_cfg.coef_ki.coef, _poly_cfg.coef_ki.num);
    _param.kd = polynomial(_info.error, _poly_cfg.coef_kd.coef, _poly_cfg.coef_kd.num);
}
