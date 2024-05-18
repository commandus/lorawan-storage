#include <sstream>
#include "lorawan/storage/serialization/urn-helper.h"
#include "lorawan/lorawan-string.h"

const char* URN_PREFIX = "LW:";
const char* SCHEMA_ID = "D0:";
const char* DEF_PROFILE_ID = "FFFFFFFF";
const char* DLMT = ":";

std::string NETWORKIDENTITY2URN(
    const NETWORKIDENTITY &networkIdentity,
    bool proprietary
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
        ss << profileID << DLMT;
    } else
        ss << DEF_PROFILE_ID;

    if (proprietary) {
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
    std::string r = toUpperCase(ss.str());
    return r;
}
