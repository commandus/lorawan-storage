#include <string>
#include "lorawan-types.h"
#include "lorawan-packet-storage.h"

std::string replaceAll(std::string str, const std::string& from, const std::string& to);

std::string uint64_t2string(const uint64_t &value);

bool isHex(const std::string &value);

/**
 * @brief Return hex string
 * @param buffer buffer
 * @param size buffer size
 * @return hex string
 */
std::string hexString(const void *buffer, size_t size);

/**
 * @brief Return hex string
 * @param data binary data
 * @return string hex
 */
std::string hexString(const std::string &data);

std::string hex2string(const std::string &hex);
std::string toUpperCase(const std::string &value);

std::string DEVICENAME2string(const DEVICENAME &value);
std::string gatewayId2str(uint64_t value);
std::string MHDR2String(const MHDR &value);
std::string MIC2String(uint16_t value);
std::string DEVADDR2string(const DEVADDR &value);
std::string DEVEUI2string(const DEVEUI &value);
std::string KEY2string(const KEY128 &value);
std::string DEVNONCE2string(const DEVNONCE &value);
std::string JOINNONCE2string(const JOINNONCE &value);
std::string JOIN_ACCEPT_FRAME_HEADER2string(const JOIN_ACCEPT_FRAME_HEADER &value);
std::string CFLIST2string(const CFLIST &value);
std::string JOIN_REQUEST_FRAME2string(const JOIN_REQUEST_FRAME &value);
std::string JOIN_ACCEPT_FRAME2string(const JOIN_ACCEPT_FRAME &value);
std::string JOIN_ACCEPT_FRAME_CFLIST2string(const JOIN_ACCEPT_FRAME_CFLIST &value);
std::string DOWNLINK_STORAGE2String(const DOWNLINK_STORAGE &value, int size);
std::string UPLINK_STORAGE2String(const UPLINK_STORAGE &value, int size);
std::string NETID2String(const NETID &value);
std::string activation2string(ACTIVATION value);
std::string MODULATION2String(MODULATION value);
std::string BANDWIDTH2String(BANDWIDTH value);
std::string LORAWAN_VERSION2string(LORAWAN_VERSION value);
std::string deviceclass2string(DEVICECLASS value);

DEVNONCE string2DEVNONCE(const std::string &value);
MTYPE string2mtype(const std::string &value);
std::string mtype2string(MTYPE value);
MHDR string2mhdr(const std::string &value);
std::string mhdr2string(MHDR value);
std::string fctrl2string(const RFM_HEADER*);
std::string mac2string(void *value, uint8_t foptSize, size_t bufferSize);
std::string rfm_header2string(const RFM_HEADER* value);
ACTIVATION string2activation(const std::string &value);
ACTIVATION pchar2activation(const char *value);
MODULATION string2MODULATION(const char *value);
BANDWIDTH string2BANDWIDTH(const char *value);
LORAWAN_VERSION string2LORAWAN_VERSION(const std::string &value);
DEVICECLASS string2deviceclass(const std::string &value);
void string2DEVADDR(DEVADDR &retVal, const std::string &str);
void string2DEVEUI(DEVEUI &retval, const std::string &str);
void string2KEY(KEY128 &retval, const std::string &str);
void string2DEVICENAME(DEVICENAME &retval, const char *str);
void string2JOINNONCE(JOINNONCE &retval, const std::string &value);
void string2NETID(NETID &retVal, const char *str);
void string2FREQUENCY(FREQUENCY &retVal, const char *value);
void string2JOINNONCE(JOINNONCE &retval, const char *value);
void string2APPNONCE(APPNONCE& retval, const std::string& value);
uint64_t string2gatewayId(const std::string& value);
bool string2NETWORKIDENTITY(NETWORKIDENTITY &retVal, const char *identityString);
const std::string& ERR_CODE_TX2string(ERR_CODE_TX code);
ERR_CODE_TX string2ERR_CODE_TX(const std::string &value);
/**
 * Parse data rate identifier e.g."SF7BW125" into bandwidth & spreading factor variables
 * @param bandwidth return bandwidth index
 * @param value LoRa datarate identifier e.g. "SF7BW125"
 * @return spreading factor
 */
SPREADING_FACTOR string2datr(BANDWIDTH &bandwidth, const std::string &value);
/**
 * Return data rate identifier
 * @return LoRa datarate identifier e.g. "SF7BW125"
 */
std::string datr2string(
    SPREADING_FACTOR spreadingFactor,
    BANDWIDTH bandwidth
);

/**
 * @param LoRa LoRa ECC coding rate identifier e.g. "4/6"
 */
CODING_RATE string2codingRate(const std::string &value);

/**
 * Return LoRa ECC coding rate identifier e.g. "4/6"
 * @param codingRate index
 * @return LoRa ECC coding rate identifier e.g. "4/6"
 */
std::string codingRate2string(CODING_RATE codingRate);

std::string SEMTECH_PROTOCOL_METADATA_RX2string(const SEMTECH_PROTOCOL_METADATA_RX &value);
std::string SEMTECH_PROTOCOL_METADATA_TX2string(const SEMTECH_PROTOCOL_METADATA_TX &value);
