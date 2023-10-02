#include "gateway-serialization.h"
#include "lorawan-conv.h"
#include "ip-helper.h"

#include <sstream>
#include <cstring>

#include "lorawan-error.h"
#include "ip-address.h"

#ifdef ESP_PLATFORM
#include "platform-defs.h"
#endif

struct in_addr_4 {
    union {
        struct {
            u_char s_b1;
            u_char s_b2;
            u_char s_b3;
            u_char s_b4;
        } S_un_b;
        u_long S_addr;
    } S_un;
};

#ifdef ENABLE_DEBUG
#include <iostream>
#include "lorawan-string.h"
#include "lorawan-msg.h"
#endif

/**
 * Serialize Internet address v4, v6
 * @param retBuf return buffer
 * @param addr address to serialize
 * @return 0 (unknown family), 7 (IPv4, 19(IPv6)
 */
static size_t serializeSocketAddress(
    unsigned char *retBuf,
    const struct sockaddr *addr
)
{
    size_t r = 0;
    switch (addr->sa_family) {
        case AF_INET:
            retBuf[0] = AF_INET;
            {
                struct sockaddr_in *addrIn = (struct sockaddr_in *) addr;
                retBuf[1] = *(unsigned char *) &addrIn->sin_port;
                retBuf[2] = * ((unsigned char *) &addrIn->sin_port + 1);
#ifdef _MSC_VER
                retBuf[3] = addrIn->sin_addr.S_un.S_un_b.s_b1;
                retBuf[4] = addrIn->sin_addr.S_un.S_un_b.s_b2;
                retBuf[5] = addrIn->sin_addr.S_un.S_un_b.s_b3;
                retBuf[6] = addrIn->sin_addr.S_un.S_un_b.s_b4;
#else
                struct in_addr_4 *p4 = (struct in_addr_4 *) &addrIn->sin_addr.s_addr;
                retBuf[3] = p4->S_un.S_un_b.s_b1;
                retBuf[4] = p4->S_un.S_un_b.s_b2;
                retBuf[5] = p4->S_un.S_un_b.s_b3;
                retBuf[6] = p4->S_un.S_un_b.s_b4;
#endif
            }
            r = 7;
            break;
        case AF_INET6:
            retBuf[0] = AF_INET6;
            {
                struct sockaddr_in6 *addrIn = (struct sockaddr_in6 *) addr;
                retBuf[1] = *(unsigned char *) &addrIn->sin6_port;
                retBuf[2] = * ((unsigned char *) &addrIn->sin6_port + 1);

#ifdef _MSC_VER
                retBuf[3] = addrIn->sin6_addr.u.Byte[0];
                retBuf[4] = addrIn->sin6_addr.u.Byte[1];
                retBuf[5] = addrIn->sin6_addr.u.Byte[2];
                retBuf[6] = addrIn->sin6_addr.u.Byte[3];

                retBuf[7] = addrIn->sin6_addr.u.Byte[4];
                retBuf[8] = addrIn->sin6_addr.u.Byte[5];
                retBuf[9] = addrIn->sin6_addr.u.Byte[6];
                retBuf[10] = addrIn->sin6_addr.u.Byte[7];

                retBuf[11] = addrIn->sin6_addr.u.Byte[8];
                retBuf[12] = addrIn->sin6_addr.u.Byte[9];
                retBuf[13] = addrIn->sin6_addr.u.Byte[10];
                retBuf[14] = addrIn->sin6_addr.u.Byte[11];

                retBuf[15] = addrIn->sin6_addr.u.Byte[12];
                retBuf[16] = addrIn->sin6_addr.u.Byte[13];
                retBuf[17] = addrIn->sin6_addr.u.Byte[14];
                retBuf[18] = addrIn->sin6_addr.u.Byte[15];
#else
                retBuf[3] = addrIn->sin6_addr.__in6_u.__u6_addr8[0];
                retBuf[4] = addrIn->sin6_addr.__in6_u.__u6_addr8[1];
                retBuf[5] = addrIn->sin6_addr.__in6_u.__u6_addr8[2];
                retBuf[6] = addrIn->sin6_addr.__in6_u.__u6_addr8[3];

                retBuf[7] = addrIn->sin6_addr.__in6_u.__u6_addr8[4];
                retBuf[8] = addrIn->sin6_addr.__in6_u.__u6_addr8[5];
                retBuf[9] = addrIn->sin6_addr.__in6_u.__u6_addr8[6];
                retBuf[10] = addrIn->sin6_addr.__in6_u.__u6_addr8[7];

                retBuf[11] = addrIn->sin6_addr.__in6_u.__u6_addr8[8];
                retBuf[12] = addrIn->sin6_addr.__in6_u.__u6_addr8[9];
                retBuf[13] = addrIn->sin6_addr.__in6_u.__u6_addr8[10];
                retBuf[14] = addrIn->sin6_addr.__in6_u.__u6_addr8[11];

                retBuf[15] = addrIn->sin6_addr.__in6_u.__u6_addr8[12];
                retBuf[16] = addrIn->sin6_addr.__in6_u.__u6_addr8[13];
                retBuf[17] = addrIn->sin6_addr.__in6_u.__u6_addr8[14];
                retBuf[18] = addrIn->sin6_addr.__in6_u.__u6_addr8[15];
#endif
            }
            r = 19;
            break;
        default:
            break;
    }
    return r;
}

static size_t deserializeSocketAddress(
    struct sockaddr *addr,
    const unsigned char *retBuf
)
{
    size_t r = 0;
    switch (retBuf[0]) {
        case AF_INET:
            addr->sa_family = AF_INET;
            {
                struct sockaddr_in *addrIn = (struct sockaddr_in *) addr;
                *(unsigned char *) &addrIn->sin_port = retBuf[1];
                *((unsigned char *) &addrIn->sin_port + 1) = retBuf[2];

#ifdef _MSC_VER
                addrIn->sin_addr.S_un.S_un_b.s_b1 = retBuf[3];
                addrIn->sin_addr.S_un.S_un_b.s_b2 = retBuf[4];
                addrIn->sin_addr.S_un.S_un_b.s_b3 = retBuf[5];
                addrIn->sin_addr.S_un.S_un_b.s_b4 = retBuf[6];
#else
                struct in_addr_4 *p4 = (struct in_addr_4 *) &addrIn->sin_addr.s_addr;
                p4->S_un.S_un_b.s_b1 = retBuf[3];
                p4->S_un.S_un_b.s_b2 = retBuf[4];
                p4->S_un.S_un_b.s_b3 = retBuf[5];
                p4->S_un.S_un_b.s_b4 = retBuf[6];
#endif
            }
            r = 7;
            break;
        case AF_INET6:
            addr->sa_family = AF_INET6;
            {
                struct sockaddr_in6 *addrIn = (struct sockaddr_in6 *) addr;
                *(unsigned char *) &addrIn->sin6_port = retBuf[1];
                *((unsigned char *) &addrIn->sin6_port + 1) = retBuf[2];
#ifdef _MSC_VER
                addrIn->sin6_addr.u.Byte[0] = retBuf[3];
                addrIn->sin6_addr.u.Byte[1] = retBuf[4];
                addrIn->sin6_addr.u.Byte[2] = retBuf[5];
                addrIn->sin6_addr.u.Byte[3] = retBuf[6];

                addrIn->sin6_addr.u.Byte[4] = retBuf[7];
                addrIn->sin6_addr.u.Byte[5] = retBuf[8];
                addrIn->sin6_addr.u.Byte[6] = retBuf[9];
                addrIn->sin6_addr.u.Byte[7] = retBuf[10];

                addrIn->sin6_addr.u.Byte[8] = retBuf[11];
                addrIn->sin6_addr.u.Byte[9] = retBuf[12];
                addrIn->sin6_addr.u.Byte[10] = retBuf[13];
                addrIn->sin6_addr.u.Byte[11] = retBuf[14];

                addrIn->sin6_addr.u.Byte[12] = retBuf[15];
                addrIn->sin6_addr.u.Byte[13] = retBuf[16];
                addrIn->sin6_addr.u.Byte[14] = retBuf[17];
                addrIn->sin6_addr.u.Byte[15] = retBuf[18];
#else
                addrIn->sin6_addr.__in6_u.__u6_addr8[0] = retBuf[3];
                addrIn->sin6_addr.__in6_u.__u6_addr8[1] = retBuf[4];
                addrIn->sin6_addr.__in6_u.__u6_addr8[2] = retBuf[5];
                addrIn->sin6_addr.__in6_u.__u6_addr8[3] = retBuf[6];

                addrIn->sin6_addr.__in6_u.__u6_addr8[4] = retBuf[7];
                addrIn->sin6_addr.__in6_u.__u6_addr8[5] = retBuf[8];
                addrIn->sin6_addr.__in6_u.__u6_addr8[6] = retBuf[9];
                addrIn->sin6_addr.__in6_u.__u6_addr8[7] = retBuf[10];

                addrIn->sin6_addr.__in6_u.__u6_addr8[8] = retBuf[11];
                addrIn->sin6_addr.__in6_u.__u6_addr8[9] = retBuf[12];
                addrIn->sin6_addr.__in6_u.__u6_addr8[10] = retBuf[13];
                addrIn->sin6_addr.__in6_u.__u6_addr8[11] = retBuf[14];

                addrIn->sin6_addr.__in6_u.__u6_addr8[12] = retBuf[15];
                addrIn->sin6_addr.__in6_u.__u6_addr8[13] = retBuf[16];
                addrIn->sin6_addr.__in6_u.__u6_addr8[14] = retBuf[17];
                addrIn->sin6_addr.__in6_u.__u6_addr8[15] = retBuf[18];
#endif
            }
            r = 19;
            break;
        default:
            break;
    }
    return r;
}

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
    const unsigned char *buf,
    size_t sz
)
{
    deserialize(buf, sz);
}

void ServiceMessage::ntoh()
{
    code = NTOH4(code);
    accessCode = NTOH8(accessCode);
}

size_t ServiceMessage::deserialize(
    const unsigned char *buf,
    size_t sz
)
{
    if (sz >= 13) {
        memmove(&tag, &buf[0], sizeof(tag));         // 1
        memmove(&code, &buf[1], sizeof(code));       // 4
        memmove(&accessCode, &buf[5], sizeof(accessCode)); // 8
    }
    return 13;
}

size_t ServiceMessage::serialize(
    unsigned char *retBuf
) const
{
    memmove(&retBuf[0], &tag, sizeof(tag));         // 1
    memmove(&retBuf[1], &code, sizeof(code));       // 4
    memmove(&retBuf[5], &accessCode, sizeof(accessCode)); // 8
    return 13;
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
    const uint64_t aId
)
    : ServiceMessage('a', 0, 0), id(aId)
{

}

GatewayIdRequest::GatewayIdRequest(
    const uint64_t aId,
    int32_t code,
    uint64_t accessCode
)
    : ServiceMessage('a', code, accessCode), id(aId)
{

}

GatewayIdRequest::GatewayIdRequest(
    const unsigned char *buf,
    size_t sz
)
{
    deserialize(buf, sz);
}

void GatewayIdRequest::ntoh() {
    ServiceMessage::ntoh();
    id = NTOH8(id);
}

size_t GatewayIdRequest::deserialize(
    const unsigned char *buf,
    size_t sz
)
{
    ServiceMessage::deserialize(buf, sz);   // 13
    memmove(&id, &buf[13], sizeof(id));         // 8
    return 21;
}


size_t GatewayIdRequest::serialize(
    unsigned char *retBuf
) const
{
    ServiceMessage::serialize(retBuf);   // 13
    memmove(&retBuf[13], &id, sizeof(id));         // 8
    return 21;
}

std::string GatewayIdRequest::toJsonString() const
{
    std::stringstream ss;
    ss << R"("gwid":")" << std::hex << id;
    return ss.str();
}

GatewayAddrRequest::GatewayAddrRequest()
    : ServiceMessage('A', 0, 0)
{
    memset(&addr, 0, sizeof(addr));
}

GatewayAddrRequest::GatewayAddrRequest(
    const GatewayIdentity& aIdentity
)
    : ServiceMessage('A', 0, 0)
{
    memset(&addr, 0, sizeof(addr));
}

GatewayAddrRequest::GatewayAddrRequest(
    const unsigned char *buf,
    size_t sz
)
{
    deserialize(buf, sz);
}

GatewayAddrRequest::GatewayAddrRequest(
    const struct sockaddr &aAddr,
    int32_t code,
    uint64_t accessCode
)
    : ServiceMessage(tag, code, accessCode)
{
    memmove(&addr, &aAddr, sizeof(addr));
}

void GatewayAddrRequest::ntoh()
{
    ServiceMessage::ntoh();
    sockaddrNtoh(&addr);
}

size_t GatewayAddrRequest::deserialize(
    const unsigned char *buf,
    size_t sz
)
{
    ServiceMessage::deserialize(buf, sz);   // 13
    size_t r = deserializeSocketAddress(&addr, &buf[21]); // 0, 7, 19
    return 21 + r;                // up to 40 bytes
}

size_t GatewayAddrRequest::serialize(
    unsigned char *retBuf
) const
{
    ServiceMessage::serialize(retBuf);   // 13
    size_t r = serializeSocketAddress(&retBuf[21], &addr); // 0, 7, 19
    return 21 + r;                // up to 40 bytes
}

std::string GatewayAddrRequest::toJsonString() const
{
    std::stringstream ss;
    ss << R"({"addr": ")" << sockaddr2string(&addr) << "\"}";
    return ss.str();
}

////////////////////////////////////////////////

GatewayIdAddrRequest::GatewayIdAddrRequest()
    : ServiceMessage('p', 0, 0), identity()
{
}

GatewayIdAddrRequest::GatewayIdAddrRequest(
    const GatewayIdentity& aIdentity
)
    : ServiceMessage('A', 0, 0), identity(aIdentity)
{
}

GatewayIdAddrRequest::GatewayIdAddrRequest(
    const unsigned char *buf,
    size_t sz
)
{
    deserialize(buf, sz);
}

GatewayIdAddrRequest::GatewayIdAddrRequest(
    const GatewayIdentity &aIdentity,
    int32_t code,
    uint64_t accessCode
)
    : ServiceMessage(tag, code, accessCode), identity(aIdentity)
{
}

void GatewayIdAddrRequest::ntoh()
{
    ServiceMessage::ntoh();
    identity.gatewayId = NTOH8(identity.gatewayId);
    sockaddrNtoh(&identity.sockaddr);
}

size_t GatewayIdAddrRequest::deserialize(
    const unsigned char *buf,
    size_t sz
)
{
    ServiceMessage::deserialize(buf, sz);   // 13
    memmove(&identity.gatewayId, &buf[13], sizeof(identity.gatewayId));
    size_t r = deserializeSocketAddress(&identity.sockaddr, &buf[21]); // 0, 7, 19
    return 21 + r;                // up to 40 bytes
}

size_t GatewayIdAddrRequest::serialize(
        unsigned char *retBuf
) const
{
    ServiceMessage::serialize(retBuf);   // 13
    memmove(&retBuf[13], &identity.gatewayId, sizeof(identity.gatewayId));
    size_t r = serializeSocketAddress(&retBuf[21], &identity.sockaddr); // 0, 7, 19
    return 21 + r;                // up to 40 bytes
}

std::string GatewayIdAddrRequest::toJsonString() const
{
    std::stringstream ss;
    ss << R"({"identity": ")" << identity.toJsonString() << "\"}";
    return ss.str();
}

////////////////////////////////////////////////
OperationRequest::OperationRequest()
    : ServiceMessage('L', 0, 0),
      offset(0), size(0)
{
}

OperationRequest::OperationRequest(
    char tag
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
    const unsigned char *buf,
    size_t sz
)
{
    deserialize(buf, sz);
}

void OperationRequest::ntoh() {
    ServiceMessage::ntoh();
    offset = NTOH8(offset);
    size = NTOH8(size);
}

size_t OperationRequest::deserialize(
    const unsigned char *buf,
    size_t sz
)
{
    ServiceMessage::deserialize(buf, sz);   // 13
    memmove(&offset, &buf[13], sizeof(offset));         // 8
    memmove(&offset, &buf[21], sizeof(offset));         // 8
    return 29;
}

size_t OperationRequest::serialize(
    unsigned char *retBuf
) const
{
    ServiceMessage::serialize(retBuf);   // 13
    memmove(&retBuf[13], &offset, sizeof(offset));         // 8
    memmove(&retBuf[21], &offset, sizeof(offset));         // 8
    return 29;
}

std::string OperationRequest::toJsonString() const {
    std::stringstream ss;
    ss << R"({"offset": ")" << offset
        << ", \"size\": " << size
        << "\"}";
    return ss.str();
}

GetResponse::GetResponse(
    const GatewayAddrRequest& req
)
    : GatewayAddrRequest(req)
{
}

GetResponse::GetResponse(
    const unsigned char* buf,
    size_t sz
)
{
    deserialize(buf, sz);
}

std::string GetResponse::toJsonString() const {
    std::stringstream ss;
    ss << R"({"request": ")" << GatewayAddrRequest::toJsonString()
        << ", \"response\": " << response.toJsonString() << "\"}";
    return ss.str();
}

void GetResponse::ntoh()
{
    GatewayAddrRequest::ntoh();
}

size_t GetResponse::deserialize(
    const unsigned char *buf,
    size_t sz
)
{
    return GatewayAddrRequest::deserialize(buf, sz);   // up to 40 bytes
}

size_t GetResponse::serialize(
    unsigned char *retBuf
) const
{
    return GatewayAddrRequest::serialize(retBuf);   // up to 40 bytes
}

OperationResponse::OperationResponse()
    : OperationRequest(), response(0)
{
}

OperationResponse::OperationResponse(
    const OperationResponse& resp
)
    : OperationRequest(resp)
{
}

OperationResponse::OperationResponse(
    const unsigned char *buf,
    size_t sz
)
{
    deserialize(buf, sz);
}

void OperationResponse::ntoh() {
    OperationRequest::ntoh();
    response = NTOH8(response);
}

size_t OperationResponse::deserialize(
    const unsigned char *buf,
    size_t sz
)
{
    OperationRequest::deserialize(buf, sz);   // 29
    memmove(&response, &buf[29], sizeof(response));         // 8
    return 37;
}

size_t OperationResponse::serialize(
    unsigned char *retBuf
) const
{
    OperationRequest::serialize(retBuf);   // 29
    memmove(&retBuf[29], &response, sizeof(response));         // 8
    return 37;
}

std::string OperationResponse::toJsonString() const {
    std::stringstream ss;
    ss << R"({"request": ")" << OperationRequest::toJsonString()
       << ", \"response\": " << response << "\"}";
    return ss.str();
}

ListResponse::ListResponse()
    : OperationResponse()
{
}

ListResponse::ListResponse(
    const ListResponse& resp
)
    : OperationResponse(resp)
{
}

ListResponse::ListResponse(
    const unsigned char *buf,
    size_t sz
)
{
    deserialize(buf, sz);
}

void ListResponse::ntoh()
{
    OperationResponse::ntoh();
    for (auto i = 0; i < response; i++) {
        identities[i].gatewayId = NTOH8(identities[i].gatewayId);
        sockaddrNtoh(&identities[i].sockaddr);
    }
}

size_t ListResponse::deserialize(
    const unsigned char *buf,
    size_t sz
)
{
    size_t ofs = OperationRequest::deserialize(buf, sz);   // 29
    for (auto i = 0; i < response; i++) {
        memmove(&identities[i].gatewayId, &buf[ofs], sizeof(uint64_t));  // 8
        ofs += 8;
        ofs += deserializeSocketAddress(&identities[i].sockaddr, &buf[ofs]);
    }
    return ofs;
}

size_t ListResponse::serialize(
    unsigned char *retBuf
) const
{
    size_t ofs = OperationRequest::serialize(retBuf);   // 29
    for (auto i = 0; i < response; i++) {
        memmove(&retBuf[ofs], &identities[i].gatewayId, sizeof(uint64_t));  // 8
        ofs += 8;
        ofs += serializeSocketAddress(&retBuf[ofs], &identities[i].sockaddr);
    }
    return ofs;
}

std::string ListResponse::toJsonString() const {
    std::stringstream ss;
    ss << R"({"result": ")" << OperationResponse::toJsonString()
       << ", \"gateways\": [";
    bool isFirst = true;
    for (auto i = 0; i < response; i++) {
        if (isFirst)
            isFirst = false;
        else
            ss << ", ";
        ss << identities[i].toJsonString();
    }
    ss <<  "]}";
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

size_t getListResponseSize(
    size_t sz
)
{
    return sz > 0 ? sizeof(ListResponse) + ((sz - 1) * sizeof(GatewayIdentity))
        : sizeof(OperationResponse);
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
        std::cerr << MSG_REQUIRED_SIZE << sizeof(GatewayAddrRequest)  << std::endl;
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
                if (sz < sizeof(GatewayAddrRequest)) {
#ifdef ENABLE_DEBUG
                    std::cerr << MSG_REQUIRED_SIZE << sizeof(GatewayAddrRequest)  << std::endl;
#endif
                    return 0;
                }

                auto gr = (GatewayAddrRequest *) request;
                gr->ntoh();
                auto r = new GetResponse(*gr);
                r->code = svc->get(r->response, r->response);
                r->ntoh();
                *retBuf = (char *) r;
                return sizeof(GetResponse);
            }
        case 'p':   // assign (put) gateway address to the gateway by identifier
        {
            if (sz < sizeof(GatewayAddrRequest)) {
#ifdef ENABLE_DEBUG
                std::cerr << MSG_REQUIRED_SIZE << sizeof(GatewayAddrRequest)  << std::endl;
#endif
                return 0;
            }
            auto gr = (GatewayIdAddrRequest *) request;
            gr->ntoh();
            svc->put(gr->identity);
            auto r = new OperationResponse;
            r->code = CODE_OK;
            r->ntoh();
            *retBuf = (char *) r;
            return sizeof(OperationResponse);
        }
        case 'r':   // Remove entry
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
        case 'L':   // List entries
        {
            if (sz < sizeof(OperationRequest)) {
#ifdef ENABLE_DEBUG
                std::cerr << MSG_REQUIRED_SIZE << sizeof(OperationRequest)  << std::endl;
#endif
                return 0;
            }
            auto gr = (OperationRequest *) request;
            gr->ntoh();

            std::vector<GatewayIdentity> l;
            svc->list(l, gr->offset, gr->size);

            size_t sz = l.size();
            size_t size = getListResponseSize(sz);
            *retBuf = (char *) malloc(size);
            ListResponse *r = (ListResponse *) *retBuf;
            r->tag = gr->tag;
            r->code = CODE_OK;
            r->accessCode = gr->accessCode;
            r->offset = gr->offset;
            r->size = gr->size;
            r->response = sz;
            int i = 0;
            for (auto it(l.begin()); it != l.end(); it++) {
                r->identities[i] = *it;
                i++;
            }
            r->ntoh();
            return size;
        }
        case 'c':   // count
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
        case 'f':   // force save
            // void flush()
            break;
        case 'd':   // close resources
            // void stop()
            break;
        default:
            break;
    }
    return 0;
}

/**
 * Get query tag in the buffer
 * @param str PChar
 * @return query tag
 */
enum CliGatewayQueryTag getQueryTag(
    const char *str
)
{
    if (str) {
        switch (*str) {
            case 'a':   // request gateway identifier(with address) by network address.
                return QUERY_GATEWAY_ADDR;
            case 'A':   // request gateway address (with identifier) by identifier.
                return QUERY_GATEWAY_ID;
            case 'p':   // assign (put) gateway address to the gateway by identifier
                return QUERY_GATEWAY_ASSIGN;
            case 'r':   // Remove entry
                return QUERY_GATEWAY_RM;
            case 'L':   // List entries
                return QUERY_GATEWAY_LIST;
            case 'c':   // list count
                return QUERY_GATEWAY_COUNT;
            case 'f':   // force save
                return QUERY_GATEWAY_FORCE_SAVE;
            case 'd':   // close resources
                return QUERY_GATEWAY_CLOSE_RESOURCES;
            default:
                break;
        }
    }
    return QUERY_GATEWAY_NONE;
}

/**
 * Check does it serialized query in the buffer
 * @param buffer buffer to check
 * @param size buffer size
 * @return query tag
 */
enum CliGatewayQueryTag validateQuery(
    const unsigned char *buffer,
    size_t sz
)
{
    switch (buffer[0]) {
        case 'a':   // request gateway identifier(with address) by network address.
            if (sz < sizeof(GatewayAddrRequest))
                return QUERY_GATEWAY_NONE;
            return QUERY_GATEWAY_ADDR;
        case 'A':   // request gateway address (with identifier) by identifier.
            if (sz < sizeof(GatewayAddrRequest))
                return QUERY_GATEWAY_NONE;
            return QUERY_GATEWAY_ID;
        case 'p':   // assign (put) gateway address to the gateway by identifier
            if (sz < sizeof(GatewayAddrRequest))
                return QUERY_GATEWAY_NONE;
            return QUERY_GATEWAY_ASSIGN;
        case 'r':   // Remove entry
            if (sz < sizeof(GatewayIdRequest))
                return QUERY_GATEWAY_NONE;
            return QUERY_GATEWAY_RM;
        case 'L':   // List entries
            if (sz < sizeof(OperationRequest))
                return QUERY_GATEWAY_NONE;
            return QUERY_GATEWAY_LIST;
        case 'c':   // list count
            if (sz < sizeof(OperationRequest))
                return QUERY_GATEWAY_NONE;
            return QUERY_GATEWAY_COUNT;
        case 'f':   // force save
            if (sz < sizeof(OperationRequest))
                return QUERY_GATEWAY_NONE;
            return QUERY_GATEWAY_FORCE_SAVE;
        case 'd':   // close resources
            if (sz < sizeof(OperationRequest))
                return QUERY_GATEWAY_NONE;
            return QUERY_GATEWAY_CLOSE_RESOURCES;
    default:
            break;
    }
    return QUERY_GATEWAY_NONE;
}

/**
 * Return required size for response
 * @param buffer serialized request
 * @param size buffer size
 * @return size in bytes
 */
size_t responseSizeForRequest(
    const unsigned char *buffer,
    size_t size
)
{
    enum CliGatewayQueryTag tag = validateQuery(buffer, size);
    switch (tag) {
        case QUERY_GATEWAY_ADDR:   // request gateway identifier(with address) by network address.
        case QUERY_GATEWAY_ID:   // request gateway address (with identifier) by identifier.
            return sizeof(GetResponse);
        case QUERY_GATEWAY_LIST:   // List entries
            return getListResponseSize(((OperationRequest *) buffer)->size);
        default:
            break;
    }
    return sizeof(OperationResponse);
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
