// #pragma once
// #include "DcMotor_ClosedLoop.hpp"

// #define EEPROM_Write_Starting_Address 0x0020 //eeprom的写入起始地址
// #define Motor_Address_Space 64 //单个电机空间地址大小


// #define MOTOR1_MAGIC  0x1111u   // 2 字节
// #define MOTOR2_MAGIC  0x2222u
// #define MOTOR3_MAGIC  0x3333u
// #define MOTOR4_MAGIC  0x4444u

// #define MAGIC_BASE_ADDR  0x0000 //标志的起始地址
// #define MAGIC_SIZE       2     //写入过标志占用2字节

// #pragma pack(push, 1) // 保存当前对齐设置后 设置为1字节对齐

// struct MotorSave_t { 
//     float    speed_kp;     // 速度环 P
//     float    speed_ki;     // 速度环 I
//     float    speed_kd;     // 速度环 D
//     float    pos_kp;       // 位置环 P
//     float    pos_ki;       // 位置环 I
//     float    pos_kd;       // 位置环 D
//     float    zero_offset;  // 零位偏移 (角度)
//     float motor_freq;      //电机频率
// };

// #pragma pack(pop) //恢复默认4字节对齐

// class Motor_Storage
// {
// public:
//     static bool save(uint8_t motor_id); 
//     static bool load(uint8_t motor_id);
//     static bool clear(uint8_t motor_id); //清除指定电机的 EEPROM 标志位

// private:

//     static uint16_t motor_addr(uint8_t id);//分配电机的写入起始地址
//     static DC_Motor* get_motor_ptr(uint8_t id); //返回电机全局对象
//     static uint16_t  motor_magic(uint8_t id);//返回电机写入标志的数据
//     static uint16_t  motor_magic_addr(uint8_t id);//分配电机标志的写入起始地址
// };
