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
    return crc16((uint8_t *) urn.c_str(), urn.size());
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
    std::stringstream ss;
    // first mandatory fields
    ss << URN_PREFIX << SCHEMA_ID
        << DEVEUI2string(networkIdentity.devid.appEUI) << DLMT
        << DEVEUI2string(networkIdentity.devid.devEUI) << DLMT;
    // profile id from the name if it is hex number
    std::string profileID = DEVICENAME2string(networkIdentity.devid.name);
    if ((!profileID.empty()) && isHex(profileID)) {
        auto l = profileID.size();
        if (l > 8)
            profileID = profileID.substr(0, 8);
        else
            profileID = std::string(8 - l, '0') + profileID;
        ss << profileID;
    } else
        ss << DEF_PROFILE_ID;

    if (!ownerToken.empty())
        ss << ":O" << ownerToken;
    if (!serialNumber.empty())
        ss << ":S" << serialNumber;
    if (addProprietary) {
        ss << ":PD" << DEVADDR2string(networkIdentity.devaddr)
            << ":PT" << activation2string(networkIdentity.devid.activation)
            << ":PC" << deviceclass2string(networkIdentity.devid.deviceclass)
            << ":PW" << KEY2string(networkIdentity.devid.nwkSKey)
            << ":PS" << KEY2string(networkIdentity.devid.appSKey)
            << ":PV" << LORAWAN_VERSION2string(networkIdentity.devid.version)
            << ":PA" << KEY2string(networkIdentity.devid.appKey)
            << ":PN" << KEY2string(networkIdentity.devid.nwkKey)
            << ":PD" << DEVNONCE2string(networkIdentity.devid.devNonce)
            << ":PJ" << JOINNONCE2string(networkIdentity.devid.joinNonce);
    }
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
