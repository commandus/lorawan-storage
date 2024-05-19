#include <sstream>
#include <iomanip>
#include "lorawan/storage/serialization/urn-helper.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/helper/crc-helper.h"

const char* URN_PREFIX = "LW:";
const char* SCHEMA_ID = "D0:";
const char* DEF_PROFILE_ID = "FFFFFFFF";
const char* DLMT = ":";

static uint16_t calcCheckSum(
    const std::string &urn
)
{
    return crc16modbus((uint8_t *) urn.c_str(), urn.size());
}

std::string mkURN(
    const DEVEUI &appEui,
    const DEVEUI &devEui,
    const PROFILEID &profileId,
    const std::string &ownerToken,
    const std::string &serialNumber,
    const std::vector<std::string> *extraProprietary,
    bool addCheckSum
)
{
    std::stringstream ss;
    // first mandatory fields
    ss << URN_PREFIX << SCHEMA_ID
       << DEVEUI2string(appEui) << DLMT
       << DEVEUI2string(devEui) << DLMT;
    // profile id from the name if it is hex number
    ss << std::hex << std::setw(8) << std::setfill('0') << profileId.u;
    if (!ownerToken.empty())
        ss << ":O" << ownerToken;
    if (!serialNumber.empty())
        ss << ":S" << serialNumber;
    if (extraProprietary) {
        for (auto &p : *extraProprietary) {
            ss << ":P" << p;
        }
    }
    std::string r = toUpperCase(ss.str());
    if (addCheckSum) {
        ss << ":C" << std::hex << std::setw(4) << std::setfill('0') << calcCheckSum(r);
        r = toUpperCase(ss.str());
    }
    return r;
}

std::string NETWORKIDENTITY2URN(
    const NETWORKIDENTITY &networkIdentity,
    const std::string &ownerToken,
    const std::string &serialNumber,
    bool addProprietary,
    bool addCheckSum,
    const std::vector<std::string> *extraProprietary
)
{
    PROFILEID pid(DEVICENAME2string(networkIdentity.devid.name));
    std::vector<std::string> proprietary;
    if (extraProprietary) {
        proprietary = *extraProprietary;
    }
    if (addProprietary) {
        proprietary.push_back("D" + DEVADDR2string(networkIdentity.devaddr));
        proprietary.push_back("T" + activation2string(networkIdentity.devid.activation));
        proprietary.push_back("C" + deviceclass2string(networkIdentity.devid.deviceclass));
        proprietary.push_back("W" + KEY2string(networkIdentity.devid.nwkSKey));
        proprietary.push_back("S" + KEY2string(networkIdentity.devid.appSKey));
        proprietary.push_back("V" + LORAWAN_VERSION2string(networkIdentity.devid.version));
        proprietary.push_back("A" + KEY2string(networkIdentity.devid.appKey));
        proprietary.push_back("N" + KEY2string(networkIdentity.devid.nwkKey));
        proprietary.push_back("D" + DEVNONCE2string(networkIdentity.devid.devNonce));
        proprietary.push_back("J" + JOINNONCE2string(networkIdentity.devid.joinNonce));
    }

    return mkURN(networkIdentity.devid.appEUI, networkIdentity.devid.devEUI, pid,
          ownerToken, serialNumber, &proprietary, addCheckSum);
}
