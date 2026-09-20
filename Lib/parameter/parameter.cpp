#include "parameter.hpp"

void Parameter::init(int index, param_init_t pi)
{
    init(index, &pi);
}

void Parameter::init(int index, param_init_t *pi)
{
    _index = index;
    _id = pi->id;
    _type = pi->type;
    _dest_ptr = pi->dest_ptr;
    _default_value = pi->default_value;
    _min_value = pi->min_value;
    _max_value = pi->max_value;
    _step = pi->step;
#if PARAM_USE_CN_DESCRIPTION
    _description = pi->desc_cn;
#else
    _description = pi->description;
#endif
}

void Parameter::update(float value)
{
    _value = constraint_value(value, _min_value, _max_value);
}

bool Parameter::load(void)
{
    if (_dest_ptr == nullptr)
        return false;

    switch (_type)
    {
    case Param_Type::U8: /* uint8_t type */
        *(uint8_t *)_dest_ptr = (uint8_t)_value;
        break;
    case Param_Type::I8: /* int8_t type */
        *(int8_t *)_dest_ptr = (int8_t)_value;
        break;
    case Param_Type::U16: /* uint16_t type */
        *(uint16_t *)_dest_ptr = (uint16_t)_value;
        break;
    case Param_Type::I16: /* int16_t type */
        *(int16_t *)_dest_ptr = (int16_t)_value;
        break;
    case Param_Type::U32: /* uint32_t type */
        *(uint32_t *)_dest_ptr = (uint32_t)_value;
        break;
    case Param_Type::I32: /* int32_t type */
        *(int32_t *)_dest_ptr = (int32_t)_value;
        break;
    case Param_Type::U64: /* uint64_t type */
        *(uint64_t *)_dest_ptr = (uint64_t)_value;
        break;
    case Param_Type::I64: /* int64_t type */
        *(int64_t *)_dest_ptr = (int64_t)_value;
        break;
    case Param_Type::F32: /* float type */
        *(float *)_dest_ptr = (float)_value;
        break;
    case Param_Type::F64: /* double type */
        *(double *)_dest_ptr = (double)_value;
        break;
    default:
        return false;
    }

    return true;
}

bool Parameter::extract(void)
{
    if (_dest_ptr == nullptr)
        return false;

    switch (_type)
    {
    case Param_Type::U8: /* uint8_t type */
        _value = (float)(*(uint8_t *)_dest_ptr);
        break;
    case Param_Type::I8: /* int8_t type */
        _value = (float)(*(int8_t *)_dest_ptr);
        break;
    case Param_Type::U16: /* uint16_t type */
        _value = (float)(*(uint16_t *)_dest_ptr);
        break;
    case Param_Type::I16: /* int16_t type */
        _value = (float)(*(int16_t *)_dest_ptr);
        break;
    case Param_Type::U32: /* uint32_t type */
        _value = (float)(*(uint32_t *)_dest_ptr);
        break;
    case Param_Type::I32: /* int32_t type */
        _value = (float)(*(int32_t *)_dest_ptr);
        break;
    case Param_Type::U64: /* uint64_t type */
        _value = (float)(*(uint64_t *)_dest_ptr);
        break;
    case Param_Type::I64: /* int64_t type */
        _value = (float)(*(int64_t *)_dest_ptr);
        break;
    case Param_Type::F32: /* float type */
        _value = (float)(*(float *)_dest_ptr);
        break;
    case Param_Type::F64: /* double type */
        _value = (float)(*(double *)_dest_ptr);
        break;
    default:
        return false;
    }

    return true;
}

void Parameter::update_with_default(void)
{
    update(_default_value);
}