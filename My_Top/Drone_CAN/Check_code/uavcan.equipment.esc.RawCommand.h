#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_MAX_SIZE 36
#define UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_SIGNATURE (0x217F5C87D7EC951DULL)
#define UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_ID 1030

struct uavcan_equipment_esc_RawCommand {
    struct { uint8_t len; int16_t data[20]; } cmd;
};

#ifdef __cplusplus
extern "C"
{
#endif

static inline bool uavcan_equipment_esc_RawCommand_decode(const CanardRxTransfer* transfer, struct uavcan_equipment_esc_RawCommand* msg) {
    (void)transfer;
    uint32_t bit_ofs = 0;

    // TAO (tail array optimization): 数组长度由剩余 bits 推算
    msg->cmd.len = ((transfer->payload_len * 8) - bit_ofs) / 14;

    if (msg->cmd.len > 20) {
        return true; /* invalid value */
    }

    for (size_t i = 0; i < msg->cmd.len; i++) {
        canardDecodeScalar(transfer, bit_ofs, 14, true, &msg->cmd.data[i]);
        bit_ofs += 14;
    }

    return false; /* success */
}

#ifdef __cplusplus
}
#endif
