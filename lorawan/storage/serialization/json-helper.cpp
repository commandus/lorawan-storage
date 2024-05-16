#include "lorawan/storage/serialization/json-helper.h"

size_t retJs(
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

 size_t retStr(
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

size_t retStatusCode(
    unsigned char* retBuf,
    size_t retSize,
    int errCode
)
{
    nlohmann::json js;
    js["code"] = errCode;
    return retJs(retBuf, retSize, js);
}
