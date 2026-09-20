#pragma once

#include "serial.hpp"
#include "printf.h"
#include "CLI_Module.hpp"
#include "bsp.hpp"
extern Serial sr_console;
extern Serial sr_logger;

class MC_Serial
{
public:
    static void init(void);

private:
    MC_Serial() = delete;
};