#include "ex_math.hpp"

#include <ctype.h>

bool is_number(const char *str)
{
    if (*str == '-' || *str == '+')
    {
        str++;
    }

    bool has_digit = false;
    bool has_dot = false;

    while (*str)
    {
        if (isdigit(*str))
        {
            has_digit = true;
        }
        else if (*str == '.' && !has_dot)
        {
            has_dot = true;
        }
        else
        {
            return false;
        }
        str++;
    }

    return has_digit;
}

bool is_integer(const char *str)
{
    if (*str == '-' || *str == '+')
    {
        str++;
    }

    bool has_digit = false;

    while (*str)
    {
        if (isdigit(*str))
        {
            has_digit = true;
        }
        else
        {
            return false;
        }
        str++;
    }

    return has_digit;
}

bool is_float(const char *str)
{
    if (str == NULL || *str == '\0')
    {
        return false;
    }

    if (*str == '-' || *str == '+')
    {
        str++;
    }

    bool has_digit = false;
    bool has_dot = false;

    while (*str)
    {
        if (isdigit((unsigned char)*str))
        {
            has_digit = true;
        }
        else if (*str == '.')
        {
            if (has_dot)
            {
                return false;
            }
            has_dot = true;
        }
        else
        {
            return false;
        }
        str++;
    }

    return has_digit;
}