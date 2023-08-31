#include "gateway-serialization.h"
#include "lorawan-conv.h"
#include "ip-helper.h"

#include <sstream>

#include "lorawan-error.h"

#ifdef ESP_PLATFORM
#include "platform-defs.h"
#endif

#ifdef ENABLE_DEBUG
#include <iostream>
#include "lorawan-string.h"
#include "lorawan-msg.h"
#endif

GatewayMessage::GatewayMessage(
    char aTag,
    int32_t aCode,
    uint64_t aAccessCode
)
    : tag(aTag), code(aCode), accessCode(aAccessCode)
{

}

GatewayMessage::GatewayMessage(
    const char *buf,
    size_t sz
)
{
    if (sz >= sizeof(GatewayMessage)) {
        tag = ((GatewayMessage *) buf)->tag;
        code = ((GatewayMessage *) buf)->code;
        accessCode = ((GatewayMessage *) buf)->accessCode;
    }
}

void GatewayMessage::ntoh()
{
    code = NTOH4(code);
    accessCode = NTOH8(accessCode);
}

GetRequest::GetRequest()
    : GatewayMessage('a', 0, 0), request()
{
}

GetRequest::GetRequest(
    const GatewayIdentity& aIdentity
)
    : GatewayMessage('a', 0, 0), request(aIdentity)
{
}

GetRequest::GetRequest(
    const char *buf,
    size_t sz
)
    : GatewayMessage(buf, sz),
    request(((GetRequest *) buf)->request)
{
}

GetRequest::GetRequest(
    const GatewayIdentity &aIdentity,
    int32_t code,
    uint64_t accessCode
)
    : GatewayMessage('a', code, accessCode), request(aIdentity)
{
}

void GetRequest::ntoh()
{
    code = NTOH4(code);
    accessCode = NTOH8(accessCode);
    request.gatewayId = NTOH8(request.gatewayId);
    sockaddrNtoh(&request.sockaddr);
}

std::string GetRequest::toJsonString() const
{
    std::stringstream ss;
    ss << R"({"addr": ")" << request.toString() << "\"}";
    return ss.str();
}

GetResponse::GetResponse(
    const GetRequest& req
)
    : GetRequest(req)
{
}

GetResponse::GetResponse(
    const char* buf,
    size_t sz
)
    : GetRequest(buf, sz),
      response(((GetResponse *) buf)->response)
{
}

std::string GetResponse::toJsonString() const {
    std::stringstream ss;
    ss << R"({"request": ")" << GetRequest::toJsonString()
        << ", \"response\": " << response.toJsonString() << "\"}";
    return ss.str();
}

std::string GetResponse::toString() const {
    return response.toString();
}

void GetResponse::ntoh()
{
    code = NTOH4(code);
    accessCode = NTOH8(accessCode);
    request.gatewayId = NTOH8(request.gatewayId);
    sockaddrNtoh(&response.sockaddr);
}

GatewaySerialization::GatewaySerialization(
    GatewayService *aSvc,
    int32_t aCode,
    uint64_t aAccessCode
)
    : svc(aSvc), code(aCode), accessCode(aAccessCode)
{

}

bool GatewaySerialization::isGetResponse(
    const char *buf,
    size_t sz
)
{
    if (sz < sizeof(GetResponse))
        return false;
    if (buf[0] != 'a')
        return false;
    return true;
}

size_t GatewaySerialization::query(
    char **retBuf,
    const char *request,
    size_t sz
)
{
    if (!svc || sz < sizeof(GatewayMessage))
        return 0;
    auto pMsg = (GatewayMessage *) request;
    switch (request[0]) {
        case 'a':
            if (sz < sizeof(GetRequest)) {
#ifdef ENABLE_DEBUG
                std::cerr << "Required size " << sizeof(GetRequest)  << std::endl;
#endif
                return 0;
            }
            {
                auto gr = (GetRequest *) request;
                gr->ntoh();
                if ((pMsg->code != code) || (pMsg->accessCode != accessCode)) {
#ifdef ENABLE_DEBUG
                    std::cerr << ERR_ACCESS_DENIED
                        << ": " << pMsg->code
                        << "," << pMsg->accessCode
                        << std::endl;
#endif
                    auto r = new GetResponse(*gr);
                    r->code = ERR_CODE_ACCESS_DENIED;
                    r->ntoh();
                    *retBuf = (char *) r;
                    return sizeof(GetResponse);
                }
                auto r = new GetResponse(*gr);
                r->code = svc->get(r->response, r->request);
                r->ntoh();
                *retBuf = (char *) r;
                return sizeof(GetResponse);
            }
        case 'A':
            // request network identity(with address) by network address. Return 0 if success, retval = EUI and keys
            // int getNetworkIdentity(NETWORKIDENTITY &retval, const DEVEUI &eui)
            break;
        case 'p':
            // void put(DEVADDR &devaddr, DEVICEID &id)
            break;
        case 'r':
            // Remove entry
            // void rm(DEVADDR &addr)
            break;
        case 'L':
            // List entries
            // void list(std::vector<NETWORKIDENTITY> &retval, size_t offset, size_t size)
            break;
        case 'c':
            // Entries count
            // size_t size()
            break;
        case 'f':
            // force save
            // void flush()
            break;
        case 'l':
            // reload
            // init(const std::string &option, void *data)
            break;
        case 'd':
            // close resources
            // void stop()
            break;
        case 'I':
            // parse RX list of identifiers, wildcards or regexes and copy found EUI into retval
            // int parseIdentifiers(std::vector<DEVEUI> &retval, const std::vector<std::string> &list, bool useRegex)
            break;
        case 'N':
            // parseRX list of identifiers, wildcards or regexes and copy found EUI into retval
            // int parseNames(std::vector<DEVEUI> &retval, const std::vector<std::string> &list, bool useRegex)
            break;
        case 'C':
            // bool canControlService(const DEVADDR &addr)
            break;
        case 'n':
            // Return next network address if available
            // int next(NETWORKIDENTITY &retval)
            break;
        default:
            break;
    }
    return 0;
}

size_t makeResponse(
    GatewaySerialization *gatewaySerializer,
    char **retBuf,
    const char *buf,
    ssize_t sz
)
{
    size_t retSize;
    if (sz > 0 && gatewaySerializer)	{
        retSize = gatewaySerializer->query(retBuf, buf, sz);
    } else {
        retSize = 0;
        *retBuf = nullptr;
    }
#ifdef ENABLE_DEBUG
    std::cerr << "Response " << retSize << " bytes";
    if (sz > 0)
        std::cerr << ": " << hexString(*retBuf, retSize) << std::endl;
#endif
    return retSize;
}
