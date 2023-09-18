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

ServiceMessage::ServiceMessage()
    : tag(0), code(0), accessCode(0)
{

}

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

GatewayIdRequest::GatewayIdRequest()
    : ServiceMessage('a', 0, 0), id(0)
{
}

GatewayIdRequest::GatewayIdRequest(
    char tag,
    const uint64_t aId
)
    : ServiceMessage(tag, 0, 0), id(aId)
{

}

GatewayIdRequest::GatewayIdRequest(
    char tag,
    const uint64_t aId,
    int32_t code,
    uint64_t accessCode
)
    : ServiceMessage(tag, code, accessCode), id(aId)
{

}

GatewayIdRequest::GatewayIdRequest(
    const char *buf,
    size_t sz
)
    : ServiceMessage(buf, sz), id(((GatewayIdRequest *) buf)->id)
{
}

void GatewayIdRequest::ntoh() {
    code = NTOH4(code);
    accessCode = NTOH8(accessCode);
    id = NTOH8(id);
}

std::string GatewayIdRequest::toJsonString() const
{
    std::stringstream ss;
    ss << R"("gwid":")" << std::hex << id;
    return ss.str();
}

GatewayIdAddrRequest::GatewayIdAddrRequest()
    : ServiceMessage('a', 0, 0), identity()
{
}

GatewayIdAddrRequest::GatewayIdAddrRequest(
    char tag,
    const GatewayIdentity& aIdentity
)
    : ServiceMessage(tag, 0, 0), identity(aIdentity)
{
}

GatewayIdAddrRequest::GatewayIdAddrRequest(
    const char *buf,
    size_t sz
)
    : ServiceMessage(buf, sz), identity(((GatewayIdAddrRequest *) buf)->identity)
{
}

GatewayIdAddrRequest::GatewayIdAddrRequest(
    char tag,
    const GatewayIdentity &aIdentity,
    int32_t code,
    uint64_t accessCode
)
    : ServiceMessage(tag, code, accessCode), identity(aIdentity)
{
}

void GatewayIdAddrRequest::ntoh()
{
    code = NTOH4(code);
    accessCode = NTOH8(accessCode);
    identity.gatewayId = NTOH8(identity.gatewayId);
    sockaddrNtoh(&identity.sockaddr);
}

std::string GatewayIdAddrRequest::toJsonString() const
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
    const GatewayIdAddrRequest& req
)
    : GatewayIdAddrRequest(req)
{
}

GetResponse::GetResponse(
    const char* buf,
    size_t sz
)
    : GatewayIdAddrRequest(buf, sz),
      response(((GetResponse *) buf)->response)
{
}

std::string GetResponse::toJsonString() const {
    std::stringstream ss;
    ss << R"({"request": ")" << GatewayIdAddrRequest::toJsonString()
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
    response = NTOH8(response);
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
    if (sz < sizeof(ServiceMessage)) {
#ifdef ENABLE_DEBUG
        std::cerr << MSG_REQUIRED_SIZE << sizeof(GatewayIdAddrRequest)  << std::endl;
#endif
        return 0;
    }
    auto pMsg = (ServiceMessage *) request;
    if ((pMsg->code != code) || (pMsg->accessCode != accessCode)) {
#ifdef ENABLE_DEBUG
        std::cerr << ERR_ACCESS_DENIED
                        << ": " << pMsg->code
                        << "," << pMsg->accessCode
                        << std::endl;
#endif
        auto r = new OperationResponse;
        r->code = ERR_CODE_ACCESS_DENIED;
        r->ntoh();
        *retBuf = (char *) r;
        return sizeof(OperationResponse);
    }

    switch (request[0]) {
        case 'a':   // request gateway identifier(with address) by network address. Return 0 if success
        case 'A':   // request gateway address (with identifier) by identifier. Return 0 if success
            {
                if (sz < sizeof(GatewayIdAddrRequest)) {
#ifdef ENABLE_DEBUG
                    std::cerr << MSG_REQUIRED_SIZE << sizeof(GatewayIdAddrRequest)  << std::endl;
#endif
                    return 0;
                }

                auto gr = (GatewayIdAddrRequest *) request;
                gr->ntoh();
                auto r = new GetResponse(*gr);
                r->code = svc->get(r->response, r->identity);
                r->ntoh();
                *retBuf = (char *) r;
                return sizeof(GetResponse);
            }
        case 'p':   // assign (put) gateway address to the gateway by identifier
        {
            if (sz < sizeof(GatewayIdAddrRequest)) {
#ifdef ENABLE_DEBUG
                std::cerr << MSG_REQUIRED_SIZE << sizeof(GatewayIdAddrRequest)  << std::endl;
#endif
                return 0;
            }
            auto gr = (GatewayIdAddrRequest *) request;
            gr->ntoh();
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
            if (sz < sizeof(GatewayIdRequest)) {
#ifdef ENABLE_DEBUG
                std::cerr << MSG_REQUIRED_SIZE << sizeof(GatewayIdRequest)  << std::endl;
#endif
                return 0;
            }
            auto gr = (GatewayIdRequest *) request;
            gr->ntoh();
            auto r = new OperationResponse;
            svc->rm(gr->id);
            r->code = CODE_OK;
            r->ntoh();
            *retBuf = (char *) r;
            return sizeof(OperationResponse);
        }
        case 'L':
            // List entries
        {
            if (sz < sizeof(OperationRequest)) {
#ifdef ENABLE_DEBUG
                std::cerr << MSG_REQUIRED_SIZE << sizeof(OperationRequest)  << std::endl;
#endif
                return 0;
            }
            auto gr = (OperationRequest *) request;
            gr->ntoh();
            auto r = new OperationResponse(*gr);
            std::vector<GatewayIdentity> l;
            svc->list(l, gr->offset, gr->size);
            r->response = l.size();
            r->code = CODE_OK;
            r->ntoh();
            *retBuf = (char *) r;
            return sizeof(GetResponse);
        }
        case 'c':
        {
            if (sz < sizeof(OperationRequest)) {
#ifdef ENABLE_DEBUG
                std::cerr << MSG_REQUIRED_SIZE << sizeof(OperationRequest)  << std::endl;
#endif
                return 0;
            }
            auto gr = (OperationRequest *) request;
            gr->ntoh();
            auto r = new OperationResponse;
            r->response = svc->size();
            r->code = CODE_OK;
            r->ntoh();
            *retBuf = (char *) r;
            return sizeof(OperationResponse);
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
