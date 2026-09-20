#include "My_FM24V02A.hpp"

My_FM24V02A FM24V02A0 ={};
FOC_Motor_Params Motor_Params1; //写入参数结构体
HAL_StatusTypeDef My_FM24V02A::write(uint16_t address, uint8_t *data, uint16_t size)
{
    if((address+size) > FM24V02A_ADDRESS_MAX) return HAL_ERROR;
    return HAL_I2C_Mem_Write(EEPROM_hi2c,FM24V02A_ADDRESS|0x00, address,I2C_MEMADD_SIZE_16BIT,data, size,100);
}

HAL_StatusTypeDef My_FM24V02A::read(uint16_t address, uint8_t *data, uint16_t size)
{
    if((address+size) > FM24V02A_ADDRESS_MAX) return HAL_ERROR;
    return HAL_I2C_Mem_Read(EEPROM_hi2c,FM24V02A_ADDRESS|0x01, address,I2C_MEMADD_SIZE_16BIT,data, size,100);
}


HAL_StatusTypeDef My_FM24V02A::read_id(uint8_t *data, uint16_t size)
{
    //0x1f 0x17 0x00 0x90 
    return HAL_I2C_Mem_Read(EEPROM_hi2c,FM24V02A_ADDRESS|0x01, FM24V02A_ID,I2C_MEMADD_SIZE_16BIT,data, size,100);
}


FOC_Motor_Params My_FM24V02A::read_motor_params(uint16_t address)
{
    uint8_t rx_data[200]; //接受缓冲区
    FOC_Motor_Params Motor_Params_rx = {};//接受缓冲区
    memset(&Motor_Params_rx, 0, sizeof(Motor_Params_rx)); //清空接受缓冲区
    HAL_StatusTypeDef res = read(address, rx_data, sizeof(Motor_Params_rx));
    memcpy(&Motor_Params_rx,rx_data, sizeof(Motor_Params_rx));

    if(res != HAL_OK || Motor_Params_rx.write_flag != FM24V02A_flag_data)//读取失败 或没写入过
    {
        Motor_Params_rx.write_flag = 0x00;
        return Motor_Params_rx;
    }
    
    return Motor_Params_rx;
}


HAL_StatusTypeDef My_FM24V02A::write_motor_params(uint16_t address, FOC_Motor_Params Motor_Params_tx)
{
    uint8_t tx_data[200]; //发送缓冲区
    Motor_Params_tx.write_flag = FM24V02A_flag_data;
    memcpy(tx_data, &Motor_Params_tx, sizeof(Motor_Params_tx));
    HAL_StatusTypeDef res = write(address, tx_data, sizeof(Motor_Params_tx));
    return res;
}



