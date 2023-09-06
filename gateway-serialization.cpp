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

ServiceMessage::ServiceMessage(
    char aTag,
    int32_t aCode,
    uint64_t aAccessCode
)
    : tag(aTag), code(aCode), accessCode(aAccessCode)
{

}

ServiceMessage::ServiceMessage(
    const char *buf,
    size_t sz
)
{
    if (sz >= sizeof(ServiceMessage)) {
        tag = ((ServiceMessage *) buf)->tag;
        code = ((ServiceMessage *) buf)->code;
        accessCode = ((ServiceMessage *) buf)->accessCode;
    }
}

void ServiceMessage::ntoh()
{
    code = NTOH4(code);
    accessCode = NTOH8(accessCode);
}

std::string ServiceMessage::toJsonString() const
{
    return "";
}

GatewayRequest::GatewayRequest()
    : ServiceMessage('a', 0, 0), identity()
{
}

GatewayRequest::GatewayRequest(
    char tag,
    const GatewayIdentity& aIdentity
)
    : ServiceMessage(tag, 0, 0), identity(aIdentity)
{
}

GatewayRequest::GatewayRequest(
    const char *buf,
    size_t sz
)
    : ServiceMessage(buf, sz), identity(((GatewayRequest *) buf)->identity)
{
}

GatewayRequest::GatewayRequest(
    char tag,
    const GatewayIdentity &aIdentity,
    int32_t code,
    uint64_t accessCode
)
    : ServiceMessage(tag, code, accessCode), identity(aIdentity)
{
}

void GatewayRequest::ntoh()
{
    code = NTOH4(code);
    accessCode = NTOH8(accessCode);
    identity.gatewayId = NTOH8(identity.gatewayId);
    sockaddrNtoh(&identity.sockaddr);
}

std::string GatewayRequest::toJsonString() const
{
    std::stringstream ss;
    ss << R"({"identity": ")" << identity.toJsonString() << "\"}";
    return ss.str();
}

OperationRequest::OperationRequest()
    : ServiceMessage('L', 0, 0),
      offset(0), size(0)
{
}

OperationRequest::OperationRequest(
    char tag,
    const GatewayIdentity &identity
)
    : ServiceMessage(tag, 0, 0),
      offset(0), size(0)
{
}

OperationRequest::OperationRequest(
    char tag,
    size_t aOffset,
    size_t aSize,
    int32_t code,
    uint64_t accessCode
)
    : ServiceMessage(tag, code, accessCode), offset(aOffset), size(aSize)
{
}

OperationRequest::OperationRequest(
    const char *buf,
    size_t sz
)
    : ServiceMessage(buf, sz), offset(((OperationRequest *) buf)->offset), size(((OperationRequest *) buf)->size)
{
}

void OperationRequest::ntoh() {
    code = NTOH4(code);
    accessCode = NTOH8(accessCode);
    offset = NTOH8(offset);
    size = NTOH8(size);
}

std::string OperationRequest::toJsonString() const {
    std::stringstream ss;
    ss << R"({"offset": ")" << offset
        << ", \"size\": " << size
        << "\"}";
    return ss.str();
}

GetResponse::GetResponse(
    const GatewayRequest& req
)
    : GatewayRequest(req)
{
}

GetResponse::GetResponse(
    const char* buf,
    size_t sz
)
    : GatewayRequest(buf, sz),
      response(((GetResponse *) buf)->response)
{
}

std::string GetResponse::toJsonString() const {
    std::stringstream ss;
    ss << R"({"request": ")" << GatewayRequest::toJsonString()
        << ", \"response\": " << response.toJsonString() << "\"}";
    return ss.str();
}

void GetResponse::ntoh()
{
    code = NTOH4(code);
    accessCode = NTOH8(accessCode);
    identity.gatewayId = NTOH8(identity.gatewayId);
    sockaddrNtoh(&response.sockaddr);
}

OperationResponse::OperationResponse(
    const OperationRequest& request
)
    : OperationRequest(request)
{
}

OperationResponse::OperationResponse(
    const char *buf,
    size_t sz
)
    : OperationRequest(buf, sz),
        response(((OperationResponse *) buf)->response)
{
}

void OperationResponse::ntoh() {
    NTOH8(response);
}

std::string OperationResponse::toJsonString() const {
    std::stringstream ss;
    ss << R"({"request": ")" << OperationRequest::toJsonString()
       << ", \"response\": " << response << "\"}";
    return ss.str();
}

GatewaySerialization::GatewaySerialization(
    GatewayService *aSvc,
    int32_t aCode,
    uint64_t aAccessCode
)
    : svc(aSvc), code(aCode), accessCode(aAccessCode)
{

}

size_t GatewaySerialization::query(
    char **retBuf,
    const char *request,
    size_t sz
)
{
    if (!svc)
        return 0;
    if (sz < sizeof(GatewayRequest)) {
#ifdef ENABLE_DEBUG
        std::cerr << "Required size " << sizeof(GatewayRequest)  << std::endl;
#endif
        return 0;
    }
    auto pMsg = (ServiceMessage *) request;
    auto gr = (GatewayRequest *) request;
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

    switch (request[0]) {
        case 'a':   // request gateway identifier(with address) by network address. Return 0 if success
        case 'A':   // request gateway address (with identifier) by identifier. Return 0 if success
            {
                auto r = new GetResponse(*gr);
                r->code = svc->get(r->response, r->identity);
                r->ntoh();
                *retBuf = (char *) r;
                return sizeof(GetResponse);
            }
        case 'p':
        {
            auto r = new GetResponse(*gr);
            svc->put(r->identity);
            r->code = CODE_OK;
            r->ntoh();
            *retBuf = (char *) r;
            return sizeof(GetResponse);
        }
        case 'r':
            // Remove entry
        {
            auto r = new GetResponse(*gr);
            svc->rm(r->identity);
            r->code = CODE_OK;
            r->ntoh();
            *retBuf = (char *) r;
            return sizeof(GetResponse);
        }
        case 'L':
            // List entries
        {
            auto r = new GetResponse(*gr);
            std::vector<GatewayIdentity> l;
            svc->list(l, 0, 100);
            r->code = CODE_OK;
            r->ntoh();
            *retBuf = (char *) r;
            return sizeof(GetResponse);
        }
        case 'c':
        {
            auto r = new GetResponse(*gr);
            std::vector<GatewayIdentity> l;
            svc->size();
            r->code = CODE_OK;
            r->ntoh();
            *retBuf = (char *) r;
            return sizeof(GetResponse);
        }
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
