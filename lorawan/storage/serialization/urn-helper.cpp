#include <sstream>
#include <iomanip>
#include "lorawan/storage/serialization/urn-helper.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/helper/crc-helper.h"

const char* URN_PREFIX = "LW:";
const char* SCHEMA_ID = "D0:";
const char* DEF_PROFILE_ID = "FFFFFFFF";
const char* DLMT = ":";

static const char FLD_PREFIXES[17] = {
    // mandatory in order
    '0',    // appEui
    '1',    // devEui
    '2',    // profile id (4 bytes)
    // optional with prefix tag
    'C',    // check sum
    'O',    // owner token;
    'S',    // serial number
    // proprietary with prefix tag 'P'
    'D',    // device address
    'T',    // activation
    'C',     // device class
    'W',     // nwkSKey
    'S',    // appSKey
    'V',    // LoRaWAN version
    'A',    // appKey
    'N',    // nwkKey
    'O',    // devNonce
    'J',    // joinNonce
    'X'     // 'command': 'A', 'I', 'L', 'C', 'N', 'P', 'R', 'S', 'E'
};

static uint16_t calcCheckSum(
    const std::string &urn
)
{
    return crc16modbus((uint8_t *) urn.c_str(), urn.size());
}

LorawanIdentificationURN::LorawanIdentificationURN()
    : crc(0xffff), command('\0')
{

}

LorawanIdentificationURN::LorawanIdentificationURN(
    const std::string &urn
)
{
    parse(urn);
}

bool LorawanIdentificationURN::parse(
    const std::string &urn
)
{
    bool r = true;
    size_t p = 0;
    int count = 0;
    while ((p = urn.find(':', p)) != std::string::npos) {
        std::string token = urn.substr(0, p);
        switch (count) {
            case 0:
                if (token != "LW")
                    return false;
                break;
            case 1:
                if (token != "D0")
                    return false;
                break;
            case 2:
                string2DEVEUI(networkIdentity.devid.appEUI, token);
                break;
            case 3:
                string2DEVEUI(networkIdentity.devid.devEUI, token);
                break;
            default:
                // optional
            {
                if (token.empty())
                    continue;
                switch (token[0]) {
                    case 'C':
                        crc = (uint16_t) strtoul(token.c_str() + 1, nullptr, 16);
                        break;
                    case 'O':
                        ownerToken = token.substr(1);
                    case 'S':
                        serialNumber = token.substr(1);
                        break;
                    case 'P':
                        if (token.size() < 2)
                            continue;
                        switch (token[1]) {
                            case 'D':    // device address
                                string2DEVADDR(networkIdentity.devaddr, token.substr(2));
                                break;
                            case 'T':    // activation
                                networkIdentity.devid.activation = string2activation(token.substr(2));
                                break;
                            case 'C':     // device class
                                networkIdentity.devid.setClass(string2deviceclass(token.substr(2)));
                                break;
                            case 'W':     // nwkSKey
                                string2KEY(networkIdentity.devid.nwkSKey, token.substr(2));
                                break;
                            case 'S':    // appSKey
                                string2KEY(networkIdentity.devid.appSKey, token.substr(2));
                                break;
                            case 'V':    // LoRaWAN version
                                networkIdentity.devid.version = string2LORAWAN_VERSION(token.substr(2));
                                break;
                            case 'A':    // appKey
                                string2KEY(networkIdentity.devid.appKey, token.substr(2));
                                break;
                            case 'N':    // nwkKey
                                string2KEY(networkIdentity.devid.nwkKey, token.substr(2));
                                break;
                            case 'O':    // devNonce
                                networkIdentity.devid.devNonce = string2DEVNONCE(token.substr(2));
                                break;
                            case 'J':    // joinNonce
                                string2JOINNONCE(networkIdentity.devid.joinNonce, token.substr(2));
                                break;
                            case 'X':     // 'command': 'A', 'I', 'L', 'C', 'N', 'P', 'R', 'S', 'E'
                                if (token.size() > 2)
                                    command = token[3];
                                else
                                    token = '\0';
                                break;
                            default:
                                break;
                        }
                        break;
                    default:
                        continue;
                }
            }
        }
        p++;
        count++;
    }
    LW:D0:
    return r;
}

std::string LorawanIdentificationURN::toString() const
{
    return NETWORKIDENTITY2URN(networkIdentity, ownerToken, serialNumber, true, false, nullptr);
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
        proprietary.push_back("O" + DEVNONCE2string(networkIdentity.devid.devNonce));
        proprietary.push_back("J" + JOINNONCE2string(networkIdentity.devid.joinNonce));
    }

    return mkURN(networkIdentity.devid.appEUI, networkIdentity.devid.devEUI, pid,
          ownerToken, serialNumber, &proprietary, addCheckSum);
}
