#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define UAVCAN_EQUIPMENT_ACTUATOR_COMMAND_MAX_SIZE 4
#define UAVCAN_EQUIPMENT_ACTUATOR_COMMAND_SIGNATURE (0x8D9A6A920C1D616CULL)

#define UAVCAN_EQUIPMENT_ACTUATOR_COMMAND_COMMAND_TYPE_UNITLESS 0  //无量纲控制（如百分比）
#define UAVCAN_EQUIPMENT_ACTUATOR_COMMAND_COMMAND_TYPE_POSITION 1   //位置控制
#define UAVCAN_EQUIPMENT_ACTUATOR_COMMAND_COMMAND_TYPE_FORCE 2  //力控制
#define UAVCAN_EQUIPMENT_ACTUATOR_COMMAND_COMMAND_TYPE_SPEED 3 //速度控制
#define UAVCAN_EQUIPMENT_ACTUATOR_COMMAND_COMMAND_TYPE_PWM 4  //PWM控制


struct uavcan_equipment_actuator_Command {
    uint8_t actuator_id;
    uint8_t command_type;
    float command_value;
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t _uavcan_equipment_actuator_Command_encode(struct uavcan_equipment_actuator_Command* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool _uavcan_equipment_actuator_Command_decode(const CanardRxTransfer* transfer, struct uavcan_equipment_actuator_Command* msg);

static inline uint32_t uavcan_equipment_actuator_Command_encode(struct uavcan_equipment_actuator_Command* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
) {

    return _uavcan_equipment_actuator_Command_encode(msg, buffer
#if CANARD_ENABLE_TAO_OPTION
    , tao
#endif
    );

}

static inline bool uavcan_equipment_actuator_Command_decode(const CanardRxTransfer* transfer, struct uavcan_equipment_actuator_Command* msg) {

    return _uavcan_equipment_actuator_Command_decode(transfer, msg);

}

#if defined(CANARD_DSDLC_INTERNAL)
static inline void __uavcan_equipment_actuator_Command_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_actuator_Command* msg, bool tao);
static inline bool __uavcan_equipment_actuator_Command_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_actuator_Command* msg, bool tao);
void __uavcan_equipment_actuator_Command_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_equipment_actuator_Command* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->actuator_id);
    *bit_ofs += 8;
    canardEncodeScalar(buffer, *bit_ofs, 8, &msg->command_type);
    *bit_ofs += 8;
    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->command_value);
        canardEncodeScalar(buffer, *bit_ofs, 16, &float16_val);
    }
    *bit_ofs += 16;
}

/*
 decode uavcan_equipment_actuator_Command, return true on failure, false on success
*/
bool __uavcan_equipment_actuator_Command_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_equipment_actuator_Command* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->actuator_id);
    *bit_ofs += 8;

    canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->command_type);
    *bit_ofs += 8;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, *bit_ofs, 16, true, &float16_val);
        msg->command_value = canardConvertFloat16ToNativeFloat(float16_val);
    }
    *bit_ofs += 16;

    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_equipment_actuator_Command sample_uavcan_equipment_actuator_Command_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
#endif
#endif
