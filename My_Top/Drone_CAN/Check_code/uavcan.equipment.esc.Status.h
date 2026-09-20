#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define UAVCAN_EQUIPMENT_ESC_STATUS_MAX_SIZE 14
#define UAVCAN_EQUIPMENT_ESC_STATUS_SIGNATURE (0xA9AF28AEA2FBB254ULL)
#define UAVCAN_EQUIPMENT_ESC_STATUS_ID 1034

struct uavcan_equipment_esc_Status {
    uint32_t error_count;
    float voltage;
    float current;
    float temperature;
    int32_t rpm;
    uint8_t power_rating_pct;
    uint8_t esc_index;
};

#ifdef __cplusplus
extern "C"
{
#endif

static inline uint32_t uavcan_equipment_esc_Status_encode(struct uavcan_equipment_esc_Status* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
) {
#if CANARD_ENABLE_TAO_OPTION
    (void)tao;
#endif
    uint32_t bit_ofs = 0;

    canardEncodeScalar(buffer, bit_ofs, 32, &msg->error_count);
    bit_ofs += 32;

    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->voltage);
        canardEncodeScalar(buffer, bit_ofs, 16, &float16_val);
    }
    bit_ofs += 16;

    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->current);
        canardEncodeScalar(buffer, bit_ofs, 16, &float16_val);
    }
    bit_ofs += 16;

    {
        uint16_t float16_val = canardConvertNativeFloatToFloat16(msg->temperature);
        canardEncodeScalar(buffer, bit_ofs, 16, &float16_val);
    }
    bit_ofs += 16;

    canardEncodeScalar(buffer, bit_ofs, 18, &msg->rpm);
    bit_ofs += 18;

    canardEncodeScalar(buffer, bit_ofs, 7, &msg->power_rating_pct);
    bit_ofs += 7;

    canardEncodeScalar(buffer, bit_ofs, 5, &msg->esc_index);
    bit_ofs += 5;

    return (bit_ofs + 7) / 8;
}

static inline bool uavcan_equipment_esc_Status_decode(const CanardRxTransfer* transfer, struct uavcan_equipment_esc_Status* msg) {
    (void)transfer;
    uint32_t bit_ofs = 0;

    canardDecodeScalar(transfer, bit_ofs, 32, false, &msg->error_count);
    bit_ofs += 32;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, bit_ofs, 16, true, &float16_val);
        msg->voltage = canardConvertFloat16ToNativeFloat(float16_val);
    }
    bit_ofs += 16;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, bit_ofs, 16, true, &float16_val);
        msg->current = canardConvertFloat16ToNativeFloat(float16_val);
    }
    bit_ofs += 16;

    {
        uint16_t float16_val;
        canardDecodeScalar(transfer, bit_ofs, 16, true, &float16_val);
        msg->temperature = canardConvertFloat16ToNativeFloat(float16_val);
    }
    bit_ofs += 16;

    canardDecodeScalar(transfer, bit_ofs, 18, true, &msg->rpm);
    bit_ofs += 18;

    canardDecodeScalar(transfer, bit_ofs, 7, false, &msg->power_rating_pct);
    bit_ofs += 7;

    canardDecodeScalar(transfer, bit_ofs, 5, false, &msg->esc_index);
    bit_ofs += 5;

    return false; /* success */
}

#ifdef __cplusplus
}
#endif
