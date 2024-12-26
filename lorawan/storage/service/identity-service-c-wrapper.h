#ifndef IDENTITY_SERVICE_C_WRAPPER_H
#define IDENTITY_SERVICE_C_WRAPPER_H

#include <stdbool.h>
#include <inttypes.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef uint64_t DEVEUI;
typedef uint32_t DEVADDR;
typedef uint8_t ACTIVATION;
typedef uint8_t DEVICECLASS;
typedef uint8_t LORAWAN_VERSION;
typedef uint8_t KEY128[16];
typedef uint16_t DEVNONCE;
typedef uint8_t JOINNONCE[3];
typedef uint8_t NETID[3];
typedef char DEVICENAME[8];


#ifdef __GNUC__
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#else
#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#else
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif
#endif

typedef PACK( struct {
    ACTIVATION activation;	///< activation type: ABP or OTAA
    DEVICECLASS deviceclass;
    DEVEUI devEUI;		    ///< device identifier 8 bytes (ABP device may not store EUI)
    KEY128 nwkSKey;			///< shared session key 16 bytes
    KEY128 appSKey;			///< private key 16 bytes
    LORAWAN_VERSION version;
    // OTAA
    DEVEUI appEUI;			///< OTAA application identifier
    KEY128 appKey;			///< OTAA application private key
    KEY128 nwkKey;          ///< OTAA network key
    DEVNONCE devNonce;      ///< last device nonce
    JOINNONCE joinNonce;    ///< last Join nonce
    DEVICENAME name;
} ) DEVICEID;

typedef PACK( struct {
    DEVADDR devaddr;		///< network address 4 bytes
    DEVICEID devid;         // 91 bytes
} ) NETWORKIDENTITY;

typedef enum NETWORK_IDENTITY_COMPARISON_OPERATOR {
    NICO_NONE = 0,
    NICO_EQ,
    NICO_NE,
    NICO_GT,
    NICO_LT,
    NICO_GE,
    NICO_LE
} NETWORK_IDENTITY_COMPARISON_OPERATOR;

typedef enum NETWORK_IDENTITY_LOGICAL_PRE_OPERATOR {
    NILPO_NONE = 0,
    NILPO_AND,
    NILPO_OR
} NETWORK_IDENTITY_LOGICAL_PRE_OPERATOR;

typedef enum NETWORK_IDENTITY_PROPERTY {
    NIP_NONE = 0,
    NIP_ADDRESS = 1,
    NIP_ACTIVATION = 2,     ///< activation type: ABP or OTAA
    NIP_DEVICE_CLASS = 3,   ///< A, B, C
    NIP_DEVEUI = 4,		    ///< device identifier 8 bytes (ABP device may not store EUI)
    NIP_NWKSKEY = 5,		///< shared session key 16 bytes
    NIP_APPSKEY = 6,        ///< private key 16 bytes
    NIP_LORAWAN_VERSION = 7,
    // OTAA
    NIP_APPEUI = 8,			///< OTAA application identifier
    NIP_APPKEY = 9,			///< OTAA application private key
    NIP_NWKKEY = 10,        ///< OTAA network key
    NIP_DEVNONCE = 11,      ///< last device nonce
    NIP_JOINNONCE = 12,     ///< last Join nonce
    // added for searching
    NIP_DEVICENAME = 13
} NETWORK_IDENTITY_PROPERTY;

typedef PACK( struct {
    uint8_t RX2DataRate: 4;	    ///< downlink data rate that serves to communicate with the end-device on the second receive window (RX2)
    uint8_t RX1DROffset: 3;	    ///< offset between the uplink data rate and the downlink data rate used to communicate with the end-device on the first receive window (RX1)
    uint8_t optNeg: 1;     	    ///< 1.0- RFU, 1.1- optNeg
} ) DLSETTINGS;	        // 1 byte

typedef PACK( struct {
    JOINNONCE joinNonce;   	        //
    NETID netId;   	                //
    DEVADDR devAddr;   	            //
    DLSETTINGS dlSettings;		    // downlink configuration settings
    uint8_t rxDelay;                //
} ) JOIN_ACCEPT_FRAME_HEADER;	    // 3 3 4 1 1 = 12 bytes


typedef PACK( struct {
    enum NETWORK_IDENTITY_LOGICAL_PRE_OPERATOR pre;  ///< and/or previous statement
    enum NETWORK_IDENTITY_PROPERTY property;
    enum NETWORK_IDENTITY_COMPARISON_OPERATOR comparisonOperator;
    uint8_t length; // 0..16
    char filterData[16];
} ) NETWORK_IDENTITY_FILTER;    // 20 bytes long

struct IdentityServiceC {
    void *object;

    int (*c_get)(void *o, DEVICEID &retVal, const DEVADDR &devAddr);
    int (*c_getNetworkIdentity)(void *o, NETWORKIDENTITY &retval, const DEVEUI &eui);
    int (*c_put)(void *o, const DEVADDR &devaddr, const DEVICEID &id);
    int (*c_rm)(void *o, const DEVADDR &addr);
    int (*c_list)(void *o, std::vector<NETWORKIDENTITY> &retVal, uint32_t offset, uint8_t size);
    int (*c_filter)(void *o, std::vector<NETWORKIDENTITY> &retVal, const std::vector<NETWORK_IDENTITY_FILTER> &filters,
        uint32_t offset, uint8_t size);
    size_t (*c_size)(void *o);
    int (*c_next)(void *o, NETWORKIDENTITY &retVal);
    void (*c_flush)(void *o);
    int (*c_init)(void *o, const std::string &option, void *data);
    void (*c_done)(void *o);
    void (*c_setOption)(void *o, int option, void *value);
    NETID *(*c_getNetworkId)(void *o);
    void (*c_setNetworkId)(void *o, const NETID &value);
    int (*c_joinAccept)(void *o, JOIN_ACCEPT_FRAME_HEADER &retVal, NETWORKIDENTITY &networkIdentity);
};

#ifdef __cplusplus
}
#endif

#endif
