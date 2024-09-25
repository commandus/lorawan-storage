#include "lorawan/wrapper/connector-identity-serialization.h"

#include "lorawan/storage/serialization/identity-binary-serialization.h"
#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-string.h"

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

EXPORT_SHARED_C_FUNC int connectorIdentityVersion()
{
    return 1;
}

EXPORT_SHARED_C_FUNC int binaryIdentityEUIRequest(
    char *retBuf,
    size_t bufSize,
    char aTag,
    uint64_t aEUI,
    int32_t code,
    uint64_t accessCode
)
{
    if (bufSize < SIZE_DEVICE_EUI_REQUEST)
        return ERR_CODE_INSUFFICIENT_MEMORY;
    IdentityEUIRequest r(aTag, DEVEUI(aEUI), code, accessCode);
    return (int) r.serialize(reinterpret_cast<unsigned char *>(retBuf));
}

EXPORT_SHARED_C_FUNC int binaryIdentityAddrRequest(
    char *retBuf,
    size_t bufSize,
    char aTag,
    uint32_t addr,
    int32_t code,
    uint64_t accessCode
)
{
    if (bufSize < SIZE_DEVICE_ADDR_REQUEST)
        return ERR_CODE_INSUFFICIENT_MEMORY;
    IdentityAddrRequest r(aTag, DEVADDR(addr), code, accessCode);
    return (int) r.serialize(reinterpret_cast<unsigned char *>(retBuf));
}

EXPORT_SHARED_C_FUNC int binaryIdentityAssignRequest(
    char *retBuf,
    size_t bufSize,

    char aTag,
    // NETWORKIDENTITY
    uint32_t addr,
    char *activation,   	///< activation type: ABP or OTAA
    char *deviceClass,      ///< A, B, C
    char *devEUI,		    ///< device identifier 8 bytes (ABP device may not store EUI)
    char *nwkSKey,			///< shared session key 16 bytes
    char *appSKey,			///< private key 16 bytes
    char *version,
    // OTAA
    char *appEUI,			   ///< OTAA application identifier
    char *appKey,			   ///< OTAA application private key
    char *nwkKey,              ///< OTAA network key
    uint16_t devNonce,     ///< last device nonce
    uint32_t joinNonce,    ///< last Join nonce
    // added for searching
    char *name,

    int32_t code,
    uint64_t accessCode
)
{
    if (bufSize < SIZE_ASSIGN_REQUEST)
        return ERR_CODE_INSUFFICIENT_MEMORY;
    NETWORKIDENTITY identity;
    identity.devaddr = DEVADDR(addr);
    identity.devid.activation = string2activation(activation);   	// activation type: ABP or OTAA
    identity.devid.deviceclass = string2deviceclass(deviceClass); // A, B, C
    string2DEVEUI(identity.devid.devEUI, devEUI);		        // device identifier 8 bytes (ABP device may not store EUI)
    string2KEY(identity.devid.nwkSKey, nwkSKey); //shared session key 16 bytes
    string2KEY(identity.devid.appSKey, appSKey); // private key 16 bytes
    identity.devid.version = string2LORAWAN_VERSION(version);
    // OTAA
    string2DEVEUI(identity.devid.appEUI, devEUI); // OTAA application identifier
    string2KEY(identity.devid.appKey, appKey); // OTAA application private key
    string2KEY(identity.devid.nwkKey, nwkKey); // OTAA network key
    identity.devid.devNonce = DEVNONCE((uint16_t) devNonce);     // last device nonce
    identity.devid.joinNonce = JOINNONCE((uint32_t) joinNonce); // last Join nonce
    // added for searching
    string2DEVICENAME(identity.devid.name, name);

    IdentityAssignRequest r(aTag, identity, code, accessCode);
    return (int) r.serialize(reinterpret_cast<unsigned char *>(retBuf));
}
