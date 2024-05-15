#include <sstream>

#include "nlohmann/json.hpp"

#include "lorawan/storage/serialization/identity-text-json-serialization.h"
#include "lorawan/lorawan-conv.h"

#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-string.h"

#ifdef ESP_PLATFORM
#include "platform-defs.h"
#endif

IdentityTextJSONSerialization::IdentityTextJSONSerialization(
    IdentityService* aSvc,
    int32_t aCode,
    uint64_t aAccessCode
)
    : IdentitySerialization(SKT_TEXT_JSON, aSvc, aCode, aAccessCode)
{

}

static size_t retJs(
    unsigned char* retBuf,
    size_t retSize,
    const nlohmann::json &js
)
{
    std::string s = js.dump();
    auto r = s.size();
    if (r <= retSize) {
        memmove(retBuf, s.c_str(), s.size());
    } else
        r = 0;
    return r;
}

static size_t retStr(
    unsigned char* retBuf,
    size_t retSize,
    const std::string &s
)
{
    auto r = s.size();
    if (r <= retSize) {
        memmove(retBuf, s.c_str(), s.size());
    } else
        r = 0;
    return r;
}

static size_t retError(
    unsigned char* retBuf,
    size_t retSize,
    int errCode
)
{
    nlohmann::json js;
    js["error"] = errCode;
    return retJs(retBuf, retSize, js);
}

size_t IdentityTextJSONSerialization::query(
    unsigned char* retBuf,
    size_t retSize,
    const unsigned char* request,
    size_t sz
)
{
    if (!svc)
        return 0;
    nlohmann::json js = nlohmann::json::parse(request, request + sz);
    if (!js.is_object())
        return 0;
    if (!js.contains("tag"))
        return 0;
    auto jTag = js["tag"];
    if (!jTag.is_string())
        return 0;
    std::string tag = jTag;
    if (tag.empty())
        return 0;
    char t = tag[0];
    switch (t) {
        case 'a':
            // request gateway identifier(with address) by network address
        {
            // get address
            std::string addr;
            if (js.contains("addr")) {
                auto jAddr = js["addr"];
                if (jAddr.is_string()) {
                    addr = jAddr;
                }
            }
            // or eui
            std::string eui;
            if (js.contains("eui")) {
                auto jEui = js["eui"];
                if (jEui.is_string()) {
                    eui = jEui;
                }
            }
            if (addr.empty()) {
                // eui
                DEVEUI devEUI;
                string2DEVEUI(devEUI, eui);
                NETWORKIDENTITY nid;
                int r = svc->getNetworkIdentity(nid, devEUI);
                if (r == CODE_OK) {
                    return retStr(retBuf, retSize, nid.toJsonString());
                } else {
                    return retError(retBuf, retSize, r);
                }
            } else {
                // addr
                DEVADDR a;
                string2DEVADDR(a, addr);
                DEVICEID did;
                int r = svc->get(did, a);
                if (r == CODE_OK)
                    return retStr(retBuf, retSize, did.toJsonString());
                else
                    return retError(retBuf, retSize, r);
            }
        }
            break;
        case 'i':
            // request gateway address (with identifier) by identifier. Return 0 if success
        {
            std::string addr;
            if (js.contains("addr")) {
                auto jAddr = js["addr"];
                if (jAddr.is_string()) {
                    addr = jAddr;
                }
            }
            DEVADDR a;
            string2DEVADDR(a, addr);
            DEVICEID did;
            int r = svc->get(did, a);
            if (r == CODE_OK)
                return retStr(retBuf, retSize, did.toJsonString());
            else
                return retError(retBuf, retSize, r);
        }
            break;
        case 'l': {
            uint32_t offset = 0;
            uint8_t size = 10;
            if (js.contains("offset")) {
                auto jOffset = js["offset"];
                if (jOffset.is_number()) {
                    offset = jOffset;
                }
            }
            if (js.contains("size")) {
                auto jSize = js["size"];
                if (jSize.is_number()) {
                    size = jSize;
                }
            }
            std::vector<NETWORKIDENTITY> nis;
            int r = svc->list(nis, offset, size);
            if (r == CODE_OK) {
                std::stringstream ss;
                bool isFirst = true;
                ss << "[";
                for (auto &ni: nis) {
                    if (isFirst)
                        isFirst = false;
                    else
                        ss << ", ";
                    ss << ni.toJsonString();
                }
                ss << "]";
                return retStr(retBuf, retSize, ss.str());
            } else
                return retError(retBuf, retSize, r);
        }
            break;
        case 'c':
            break;
        case 'n':
            break;
        case 'p':
            break;
        case 'r':
            break;
        case 's':
            break;
        case 'e':
            break;
        default:
            return 0;
    }
        return 0;
}
