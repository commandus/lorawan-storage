#ifndef LORAWAN_STORAGE_CRC_HELPER_H
#define LORAWAN_STORAGE_CRC_HELPER_H

#include "cinttypes"

uint16_t crc16(
    const uint8_t *data,
    size_t size
);

#endif
