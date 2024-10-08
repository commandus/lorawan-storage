#include <cstring>
#include "lorawan/helper/aes-helper.h"
#include "system/crypto/aes.h"

/**
 * @see 4.3.3 MAC Frame Payload Encryption (FRMPayload)
 * @see https://os.mbed.com/teams/Semtech/code/LoRaWAN-lib//file/2426a05fe29e/LoRaMacCrypto.cpp/
 */
void encryptPayload(
    void *payload,
    size_t payloadSize,
    unsigned int frameCounter,
    unsigned char direction,
    const DEVADDR &devAddr,
    const KEY128 &appSKey
)
{
    aes_context aesContext;
    memset(aesContext.ksch, '\0', 240);
    aes_set_key(appSKey.c, SIZE_KEY128, &aesContext);

    uint8_t a[16];
    a[0] = 1;
    a[1] = 0;
    a[2] = 0;
    a[3] = 0;
    a[4] = 0;
    a[5] = direction;	// 1- uplink, 0- downlink
    a[6] = devAddr.c[0];
    a[7] = devAddr.c[1];
    a[8] = devAddr.c[2];
    a[9] = devAddr.c[3];
    a[10] = (frameCounter & 0x00ff);
    a[11] = ((frameCounter >> 8) & 0x00ff);
    a[12] = 0; // frame counter upper Bytes
    a[13] = 0;
    a[14] = 0;

    uint8_t s[16];
    memset(s, 0, 16);

    int size = (int) payloadSize;
    uint16_t ctr = 1;
    uint8_t bufferIndex = 0;
    int8_t *encBuffer = (int8_t *) payload;
    while (size >= 16) {
        a[15] = ctr & 0xff;
        ctr++;
        aes_encrypt(a, s, &aesContext);
        for (int i = 0; i < 16; i++) {
            encBuffer[bufferIndex + i] = encBuffer[bufferIndex + i] ^ s[i];
        }
        size -= 16;
        bufferIndex += 16;
    }

    if (size > 0) {
        a[15] = ctr & 0xff;
        aes_encrypt(a, s, &aesContext);
        for (int i = 0; i < size; i++) {
            encBuffer[bufferIndex + i] = encBuffer[bufferIndex + i] ^ s[i];
        }
    }
}

/**
 * Decrypt Join Accept LoRaWAN message
 * @see 6.2.3 Join-accept message
 */
void decryptJoinAccept(
    void *payload,
    size_t size,
    const KEY128 &key
) {
    aes_context aesContext;
    memset(aesContext.ksch, '\0', 240);
    aes_set_key(key.c, SIZE_KEY128, &aesContext);

    uint8_t a[16];
    memset(a, 0, 16);
    uint8_t s[16];
    memset(s, 0, 16);

    uint8_t *encBuffer = (uint8_t *) payload;
    if (size == 0)
        return;
    size--;
    uint8_t bufferIndex = 1;

    while (size >= 16) {
        aes_encrypt(encBuffer + bufferIndex, encBuffer + bufferIndex, &aesContext);
        size -= 16;
        bufferIndex += 16;
    }
}

/**
 * Decrypt Join Accept LoRaWAN message
 * @see 6.2.3 Join-accept message
 */
void decryptJoinAcceptString(
    std::string &payload,
    const KEY128 &key
)
{
    decryptJoinAccept((void *) payload.c_str(), payload.size(), key);
}

/**
 * Encrypt Join-Accept
 * aes128_decrypt(NwkKey or JSEncKey, JoinNonce | NetID | DevAddr | DLSettings | RxDelay | CFList | MIC).
 * @param frame return value
 * @param key NwkKey or JSEncKey
 */
void encryptJoinAcceptResponse(
    JOIN_ACCEPT_FRAME &frame,
    const KEY128 &key   // NwkKey or JSEncKey
)
{
    aes_context aesContext;
    memset(aesContext.ksch, '\0', 240);
    aes_set_key(key.c, SIZE_KEY128, &aesContext);

    uint8_t a[16];
    memset(a, 0, 16);
    uint8_t s[16];
    memset(s, 0, 16);

    auto e = (uint8_t *) &frame.hdr;

    aes_encrypt(a, s, &aesContext);
    for (int i = 0; i < SIZE_JOIN_ACCEPT_FRAME - 1; i++) {
        e[i] = e[i] ^ s[i];
    }
}

void encryptJoinAcceptCFListResponse(
    JOIN_ACCEPT_FRAME_CFLIST &frame,
    const KEY128 &key   // NwkKey or JSEncKey
)
{
    aes_context aesContext;
    memset(aesContext.ksch, '\0', 240);
    aes_set_key(key.c, SIZE_KEY128, &aesContext);

    uint8_t a[16];
    memset(a, 0, 16);
    uint8_t s[16];
    memset(s, 0, 16);

    auto e = (uint8_t *) &frame.hdr;
    aes_encrypt(a, s, &aesContext);
    for (int i = 0; i < 16; i++) {
        e[i] = e[i] ^ s[i];
    }
    aes_encrypt(a, s, &aesContext);
    for (int i = 16; i < SIZE_JOIN_ACCEPT_FRAME_CFLIST - 1; i++) {
        e[i] = e[i] ^ s[i - 16];
    }
}
