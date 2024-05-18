#ifndef LORAWAN_STORAGE_URN_HELPER_H
#define LORAWAN_STORAGE_URN_HELPER_H

/**
 * TR005 LoRaWAN Device Identification QR Codes
 * @see https://resources.lora-alliance.org/document/tr005-lorawan-device-identification-qr-codes
 */
#include "lorawan/lorawan-types.h"

std::string NETWORKIDENTITY2URN(
    const NETWORKIDENTITY &networkIdentity,
    bool proprietary = false
);

#endif
