#pragma once

#include <stdint.h>

#include "ex_math.hpp"

#define PARAM_USE_CN_DESCRIPTION 1

enum class Param_Type : uint8_t
{
    NONE = 0,
    U8 = 1,
    I8 = 2,
    U16 = 3,
    I16 = 4,
    U32 = 5,
    I32 = 6,
    U64 = 7,
    I64 = 8,
    F32 = 9,
    F64 = 10,
};

typedef struct
{
    const char *id;
    Param_Type type;
    void *dest_ptr;
    float default_value;
    float min_value;
    float max_value;
    float step;
    const char *description;
    const char *desc_cn;
} param_init_t;

class Parameter
{
public:
    void init(int index, param_init_t pi);
    void init(int index, param_init_t *pi);
    void update(float value);
    bool load(void);
    bool extract(void);

    void update_with_default(void);

    int get_index(void) { return _index; }
    const char *get_id(void) { return _id; }
    Param_Type get_type(void) { return _type; }
    float get_value(void) { return _value; }
    const char *get_description(void) { return _description; }
    float get_default(void) { return _default_value; }
    float get_min(void) { return _min_value; }
    float get_max(void) { return _max_value; }
    float get_step(void) { return _step; }

private:
    int _index;
    const char *_id;
    Param_Type _type;
    float _value;
    void *_dest_ptr;
    float _default_value;
    float _min_value;
    float _max_value;
    float _step;
    const char *_description;
};