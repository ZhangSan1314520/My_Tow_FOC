// #include "Motor_Storage.hpp"
// #include "XBLW24C64.hpp" 

// // ============================================================
// bool Motor_Storage::save(uint8_t motor_id) //写电机数据到eeprom
// {
//     // ① 拿电机对象指针 
//     DC_Motor *m = get_motor_ptr(motor_id);
//     if (m == nullptr) {
//         // 编号非法（0 或 >4）—— 不写 EEPROM 直接拒绝
//         return false;
//     }

//     // ② 打包当前参数到 MotorSave_t
//     MotorSave_t s;                              // 栈上 32 字节局部变量
//     s.speed_kp    = m->pid_speed_incparam->_kp;  // 速度环 P
//     s.speed_ki    = m->pid_speed_incparam->_ki;  // 速度环 I
//     s.speed_kd    = m->pid_speed_incparam->_kd;  // 速度环 D
//     s.pos_kp      = m->pid_location->_kp;        // 位置环 P
//     s.pos_ki      = m->pid_location->_ki;        // 位置环 I
//     s.pos_kd      = m->pid_location->_kd;        // 位置环 D
//     s.zero_offset = m->zero_offset;              // 零位/角度偏移 (角度)
//     s.motor_freq  = m->motor_freq;

//     uint16_t addr = motor_addr(motor_id); //获取该电机的分配的起始地址
//     if (!XBLW24C64::write(addr,  //写电机数据
//                             (const uint8_t *)&s,
//                             sizeof(s)) )
//     {
//         return false;
//     }
//     uint16_t magic = motor_magic(motor_id);//获取电机写入标志的数据
//     return XBLW24C64::write(motor_magic_addr(motor_id), //写电机标志表示写过了
//                             (const uint8_t *)&magic,
//                             MAGIC_SIZE);
// }

// bool Motor_Storage::load(uint8_t motor_id) //读eeprom的数据传回给电机
// {
//     DC_Motor *m = get_motor_ptr(motor_id);
//     if (m == nullptr) {
//         return false;
//     }

//     uint16_t magic_read = 0x0000;
//     if (!XBLW24C64::read(motor_magic_addr(motor_id), //读对应地址的标志数据
//                         (uint8_t *)&magic_read,
//                         MAGIC_SIZE)) {
//         return false;
//     }
//     if(magic_read != motor_magic(motor_id))
//     {
//         return false;//没有数据写入过
//     }


//     // ② 读出 32 字节到 MotorSave_t
//     MotorSave_t s;
//     uint16_t addr = motor_addr(motor_id); //获取该电机的分配的起始地址
//     if (!XBLW24C64::read(addr,
//                          (uint8_t *)&s,
//                          sizeof(s))) {
//         // I²C 失败
//         return false;
//     }

//     // ③ 反填到 DC_Motor
//     m->pid_speed_incparam->_kp = s.speed_kp;
//     m->pid_speed_incparam->_ki = s.speed_ki;
//     m->pid_speed_incparam->_kd = s.speed_kd;
//     m->pid_location->_kp       = s.pos_kp;
//     m->pid_location->_ki       = s.pos_ki;
//     m->pid_location->_kd       = s.pos_kd;
//     m->zero_offset             = s.zero_offset;
//     m->motor_freq              = s.motor_freq;

//     return true;
// }


// bool Motor_Storage::clear(uint8_t motor_id) //清除指定电机的 EEPROM 标志位
// {
//     uint16_t erase = 0x0000;

//     // 100 = 全部清除
//     if (motor_id == 100) {
//         bool ok = true;
//         for (uint8_t i = 1; i <= 4; i++) {
//             if (!XBLW24C64::write(motor_magic_addr(i),
//                                   (const uint8_t *)&erase,
//                                   MAGIC_SIZE)) {
//                 ok = false;
//             }
//         }
//         return ok;
//     }

//     // 单个清除
//     if (motor_id < 1 || motor_id > 4) return false;
//     return XBLW24C64::write(motor_magic_addr(motor_id), //给标志位写入0x0000
//                             (const uint8_t *)&erase,
//                             MAGIC_SIZE);
// }







// uint16_t Motor_Storage::motor_addr(uint8_t id) //分配电机的写入起始地址
// {
//     return (EEPROM_Write_Starting_Address + (id - 1) * Motor_Address_Space); 
// }

// DC_Motor* Motor_Storage::get_motor_ptr(uint8_t id)
// {
//     switch (id) {
//         case 1: return &M1;
//         case 2: return &M2;
//         case 3: return &M3;
//         case 4: return &M4;
//         default: return nullptr;
//     }
// }



// uint16_t Motor_Storage::motor_magic(uint8_t id) {  //返回电机写入标志的地址
//     switch (id) {
//         case 1: return MOTOR1_MAGIC;
//         case 2: return MOTOR2_MAGIC;
//         case 3: return MOTOR3_MAGIC;
//         case 4: return MOTOR4_MAGIC;
//         default: return 0;
//     }
// }


// uint16_t Motor_Storage::motor_magic_addr(uint8_t id) //分配电机标志的写入起始地址
// {
//     return (MAGIC_BASE_ADDR + (id - 1) * MAGIC_SIZE);
// }



