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

#define SIZE_SERVICE_MESSAGE   13
#define SIZE_OPERATION_REQUEST 18
#define SIZE_OPERATION_RESPONSE 22
#define SIZE_GATEWAY_ID_REQUEST   21
#define SIZE_GATEWAY_ADDR_4_REQUEST 20
#define SIZE_GATEWAY_ASSIGN_ID_ADDR_4_REQUEST 28

#define SIZE_GATEWAY_GET_ADDR_4_RESPONSE 28
#define SIZE_GATEWAY_GET_ADDR_6_RESPONSE 40

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
    if (!retBuf) {
        switch (addr->sa_family) {
            case AF_INET:
                return 7;
            case AF_INET6:
                return 19;
        }
        return 0;
    }
    switch (addr->sa_family) {
        case AF_INET:
            retBuf[0] = AF_INET;
            {
                auto *addrIn = (struct sockaddr_in *) addr;
                retBuf[1] = *(unsigned char *) &addrIn->sin_port;
                retBuf[2] = * ((unsigned char *) &addrIn->sin_port + 1);
#ifdef _MSC_VER
                retBuf[3] = addrIn->sin_addr.S_un.S_un_b.s_b1;
                retBuf[4] = addrIn->sin_addr.S_un.S_un_b.s_b2;
                retBuf[5] = addrIn->sin_addr.S_un.S_un_b.s_b3;
                retBuf[6] = addrIn->sin_addr.S_un.S_un_b.s_b4;
#else
                auto *p4 = (struct in_addr_4 *) &addrIn->sin_addr.s_addr;
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
                auto *addrIn = (struct sockaddr_in6 *) addr;
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
    const unsigned char *retBuf,
    size_t sz
)
{
    size_t r = 0;
    if (sz < 1)
        return r;
    switch (retBuf[0]) {
        case AF_INET:
            if (sz < 7)
                return 0;
            addr->sa_family = AF_INET;
            {
                auto *addrIn = (struct sockaddr_in *) addr;
                *(unsigned char *) &addrIn->sin_port = retBuf[1];
                *((unsigned char *) &addrIn->sin_port + 1) = retBuf[2];

#ifdef _MSC_VER
                addrIn->sin_addr.S_un.S_un_b.s_b1 = retBuf[3];
                addrIn->sin_addr.S_un.S_un_b.s_b2 = retBuf[4];
                addrIn->sin_addr.S_un.S_un_b.s_b3 = retBuf[5];
                addrIn->sin_addr.S_un.S_un_b.s_b4 = retBuf[6];
#else
                auto *p4 = (struct in_addr_4 *) &addrIn->sin_addr.s_addr;
                p4->S_un.S_un_b.s_b1 = retBuf[3];
                p4->S_un.S_un_b.s_b2 = retBuf[4];
                p4->S_un.S_un_b.s_b3 = retBuf[5];
                p4->S_un.S_un_b.s_b4 = retBuf[6];
#endif
            }
            r = 7;
            break;
        case AF_INET6:
            if (sz < 19)
                return 0;
            addr->sa_family = AF_INET6;
            {
                auto *addrIn = (struct sockaddr_in6 *) addr;
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
    if (sz >= SIZE_SERVICE_MESSAGE) {
        memmove(&tag, &buf[0], sizeof(tag));         // 1
        memmove(&code, &buf[1], sizeof(code));       // 4
        memmove(&accessCode, &buf[5], sizeof(accessCode)); // 8
    }   // 13 bytes
}

void ServiceMessage::ntoh()
{
    code = NTOH4(code);
    accessCode = NTOH8(accessCode);
}

size_t ServiceMessage::serialize(
    unsigned char *retBuf
) const
{
    if (retBuf) {
        memmove(&retBuf[0], &tag, sizeof(tag));         // 1
        memmove(&retBuf[1], &code, sizeof(code));       // 4
        memmove(&retBuf[5], &accessCode, sizeof(accessCode)); // 8
    }
    return SIZE_SERVICE_MESSAGE;                        // 13
}

std::string ServiceMessage::toJsonString() const
{
    return "";
}

GatewayIdRequest::GatewayIdRequest()
    : ServiceMessage('a', 0, 0), id(0)
{
}

/*
GatewayIdRequest::GatewayIdRequest(
    const uint64_t aId
)
    : ServiceMessage('a', 0, 0), id(aId)
{

}
*/

GatewayIdRequest::GatewayIdRequest(
    char aTag,
    const uint64_t aId,
    int32_t code,
    uint64_t accessCode
)
    : ServiceMessage(aTag, code, accessCode), id(aId)
{

}

GatewayIdRequest::GatewayIdRequest(
    const unsigned char *buf,
    size_t sz
)
    : ServiceMessage(buf, sz)   //  13
{
    if (sz >= SIZE_GATEWAY_ID_REQUEST) {
        memmove(&id, &buf[13], sizeof(id));     // 8
    }   // 21
}

void GatewayIdRequest::ntoh() {
    ServiceMessage::ntoh();
    id = NTOH8(id);
}

size_t GatewayIdRequest::serialize(
    unsigned char *retBuf
) const
{
    ServiceMessage::serialize(retBuf);      // 13
    memmove(&retBuf[13], &id, sizeof(id));  // 8
    return SIZE_GATEWAY_ID_REQUEST;         // 21
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

/*
GatewayAddrRequest::GatewayAddrRequest(
    const GatewayIdentity& aIdentity
)
    : ServiceMessage('A', 0, 0)
{
    memset(&addr, 0, sizeof(addr));
}
 */

GatewayAddrRequest::GatewayAddrRequest(
    const unsigned char *buf,
    size_t sz
)
    : ServiceMessage(buf, sz)
{
    if (sz >= SIZE_SERVICE_MESSAGE) {
        size_t r = deserializeSocketAddress(&addr, &buf[SIZE_SERVICE_MESSAGE], sz - SIZE_SERVICE_MESSAGE); // 0, 7, 19
        // SIZE_SERVICE_MESSAGE + r;        // IPv4: 20 IPv6: 32
    }
}

size_t GatewayAddrRequest::serializedSize() const
{
    if (isIPv6(&addr))
        return SIZE_SERVICE_MESSAGE + 3 + 4;        // IPv4: 20 IPv6: 32
    else
        return SIZE_SERVICE_MESSAGE + 3 + 16;        // IPv4: 20 IPv6: 32
}

GatewayAddrRequest::GatewayAddrRequest(
    const struct sockaddr &aAddr,
    int32_t code,
    uint64_t accessCode
)
    : ServiceMessage('A', code, accessCode)
{
    memmove(&addr, &aAddr, sizeof(addr));
}

void GatewayAddrRequest::ntoh()
{
    ServiceMessage::ntoh();
    sockaddrNtoh(&addr);
}

size_t GatewayAddrRequest::serialize(
    unsigned char *retBuf
) const
{
    ServiceMessage::serialize(retBuf);      // 13
    size_t r = serializeSocketAddress(&retBuf[SIZE_SERVICE_MESSAGE], &addr); // 0, 7, 19
    return SIZE_SERVICE_MESSAGE + r;        // IPv4: 20 IPv6: 32
}

std::string GatewayAddrRequest::toJsonString() const
{
    std::stringstream ss;
    ss << R"({"addr": ")" << sockaddr2string(&addr) << "\"}";
    return ss.str();
}

GatewayIdAddrRequest::GatewayIdAddrRequest()
    : ServiceMessage('p', 0, 0), identity()
{
}

/*
GatewayIdAddrRequest::GatewayIdAddrRequest(
    const GatewayIdentity& aIdentity
)
    : ServiceMessage('A', 0, 0), identity(aIdentity)
{
}
*/

GatewayIdAddrRequest::GatewayIdAddrRequest(
    const unsigned char *buf,
    size_t sz
)
    : ServiceMessage(buf, sz)   // 13
{
    if (sz >= SIZE_GATEWAY_ID_REQUEST) {
        memmove(&identity.gatewayId, &buf[SIZE_SERVICE_MESSAGE], sizeof(identity.gatewayId));   // 8
        deserializeSocketAddress(&identity.sockaddr, &buf[SIZE_GATEWAY_ID_REQUEST],
                                            sz - SIZE_GATEWAY_ID_REQUEST); // 0, 7, 19
    }
}

size_t GatewayIdAddrRequest::serializedSize() const
{
    return SIZE_GATEWAY_ID_REQUEST + 3 + (isIPv6(&identity.sockaddr) ? 16 : 4);     // IPv4: 28 IPv6: 40
}

GatewayIdAddrRequest::GatewayIdAddrRequest(
    char aTag,
    const GatewayIdentity &aIdentity,
    int32_t code,
    uint64_t accessCode
)
    : ServiceMessage(aTag, code, accessCode), identity(aIdentity)
{
}

void GatewayIdAddrRequest::ntoh()
{
    ServiceMessage::ntoh();
    identity.gatewayId = NTOH8(identity.gatewayId);
    sockaddrNtoh(&identity.sockaddr);
}


size_t GatewayIdAddrRequest::serialize(
    unsigned char *retBuf
) const
{
    ServiceMessage::serialize(retBuf);      // 13
    memmove(&retBuf[SIZE_SERVICE_MESSAGE], &identity.gatewayId, sizeof(identity.gatewayId)); // 8
    size_t r = serializeSocketAddress(&retBuf[SIZE_GATEWAY_ID_REQUEST], &identity.sockaddr); // 0, 7, 19
    return SIZE_GATEWAY_ID_REQUEST + r;     // IPv4: 28 IPv6: 40
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

/*
OperationRequest::OperationRequest(
    char tag
)
    : ServiceMessage(tag, 0, 0),
      offset(0), size(0)
{
}
*/

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
    : ServiceMessage(buf, sz)   // 13
{
    if (sz >= SIZE_OPERATION_REQUEST) {
        memmove(&offset, &buf[13], sizeof(offset));     // 4
        memmove(&size, &buf[17], sizeof(size));         // 1
    }   // 18
}

void OperationRequest::ntoh() {
    ServiceMessage::ntoh();
    offset = NTOH4(offset);
}

size_t OperationRequest::serialize(
    unsigned char *retBuf
) const
{
    ServiceMessage::serialize(retBuf);                  // 13
    if (retBuf) {
        memmove(&retBuf[13], &offset, sizeof(offset));  // 4
        memmove(&retBuf[17], &size, sizeof(size));      // 1
    }
    return SIZE_OPERATION_REQUEST;                      // 18
}

std::string OperationRequest::toJsonString() const {
    std::stringstream ss;
    ss << R"({"offset": )" << offset
        << ", \"size\": " << (int) size
        << "}";
    return ss.str();
}

GetResponse::GetResponse(
    const GatewayAddrRequest& req
)
    : ServiceMessage(req), response()
{
}

GetResponse::GetResponse(
    const GatewayIdRequest &request
)
    : response(request.id)
{
    tag = request.tag;
    code = request.code;
    accessCode = request.accessCode;
}

/*
GetResponse::GetResponse(
    const GatewayIdAddrRequest &request
)
    : response(request.identity.gatewayId, request.identity.sockaddr)
{
    tag = request.tag;
    code = request.code;
    accessCode = request.accessCode;
}
*/

GetResponse::GetResponse(
    const unsigned char* buf,
    size_t sz
)
    : ServiceMessage(buf, sz)   // 13
{
    if (sz >= SIZE_SERVICE_MESSAGE + sizeof(uint64_t)) {
        memmove(&response.gatewayId, buf + SIZE_SERVICE_MESSAGE, sizeof(uint64_t)); // 8
        deserializeSocketAddress(&response.sockaddr, buf + SIZE_SERVICE_MESSAGE + sizeof(uint64_t), sz - SIZE_SERVICE_MESSAGE - sizeof(uint64_t))
               + SIZE_SERVICE_MESSAGE + sizeof(uint64_t); // IPv4 28 IPv6 40
    }
}

size_t GetResponse::serializedSize() const
{
    return SIZE_SERVICE_MESSAGE + sizeof(uint64_t) + 3 + (isIPv6(&response.sockaddr) ? 16 : 4); // IPv4 28 IPv6 40
}

std::string GetResponse::toJsonString() const {
    std::stringstream ss;
    ss << response.toJsonString();
    return ss.str();
}

void GetResponse::ntoh()
{
    ServiceMessage::ntoh();
    response.gatewayId = NTOH8(response.gatewayId);
    sockaddrNtoh(&response.sockaddr);
}

size_t GetResponse::serialize(
    unsigned char *retBuf
) const
{
    ServiceMessage::serialize(retBuf);                   // 13
    memmove(retBuf + SIZE_SERVICE_MESSAGE, &response.gatewayId, sizeof(uint64_t));     // 8
    return serializeSocketAddress(retBuf + SIZE_SERVICE_MESSAGE + 8, &response.sockaddr)     // 0, 7, 19
        + SIZE_SERVICE_MESSAGE + 8; // IPv4 28 IPv6 40
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
    : OperationRequest(buf, sz) // 18
{
    if (sz >= SIZE_OPERATION_RESPONSE) {
        memmove(&response, buf + SIZE_OPERATION_REQUEST, sizeof(response)); // 4
    }   // 22
}

OperationResponse::OperationResponse(
    const GatewayIdAddrRequest &request
)
    : OperationRequest(request.tag, 0, 0, request.code, request.accessCode)
{
}

OperationResponse::OperationResponse(
    const OperationRequest &request
)
    : OperationRequest(request)
{

}

void OperationResponse::ntoh() {
    OperationRequest::ntoh();
    response = NTOH4(response);
}

size_t OperationResponse::serialize(
    unsigned char *retBuf
) const
{
    OperationRequest::serialize(retBuf);                                // 18
    if (retBuf)
        memmove(retBuf + SIZE_OPERATION_REQUEST, &response,
                sizeof(response));                                      // 4
    return SIZE_OPERATION_RESPONSE;                                     // 22
}

std::string OperationResponse::toJsonString() const {
    std::stringstream ss;
    ss << R"({"request": )" << OperationRequest::toJsonString()
       << ", \"response\": " << response << "}";
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
    : OperationResponse(buf, sz)       // 22
{
    deserialize(buf, sz);
    size_t ofs = SIZE_OPERATION_RESPONSE;
    response = 0;
    while (true) {
        if (ofs >= sz)
            break;
        GatewayIdentity gi;
        memmove(&gi.gatewayId, &buf[ofs], sizeof(uint64_t));  // 8
        ofs += 8;
        ofs += deserializeSocketAddress(&gi.sockaddr, &buf[ofs], sz - ofs); // 0, 7, 19
        identities.push_back(gi);
        response++;
    }
}

size_t ListResponse::serializedSize() const
{
    size_t ofs = SIZE_OPERATION_RESPONSE;
    for (auto id : identities) {
        ofs += 8 + 3 + (isIPv6(&id.sockaddr) ? 16 : 4); // 0, 7, 19
    }
    return ofs;
}

ListResponse::ListResponse(
    const OperationRequest &request
)
    : OperationResponse(request)
{
}

void ListResponse::ntoh()
{
    OperationResponse::ntoh();
    for (auto it(identities.begin()); it != identities.end(); it++) {
        it->gatewayId = NTOH8(it->gatewayId);
        sockaddrNtoh(&it->sockaddr);
    }
}

/**
 * IP v4 sizes:
 *  1   22 + 15 = 37
 *  2   22 + 2 * 15 = 52
 *  ..
 *  10   22 + 10 * 15 = 172
 *  18   22 + 18 * 15 = 292
 * IP v6 sizes:
 *  1   22 + 27 = 49
 *  2   22 + 2 * 27 = 76
 *  ..
 *  10   22 + 10 * 27 = 292
 * @param retBuf
 * @return
 */
size_t ListResponse::serialize(
    unsigned char *retBuf
) const
{
    size_t ofs = OperationResponse::serialize(retBuf);   // 22
    if (retBuf) {
        for (auto it(identities.begin()); it != identities.end(); it++) {
            memmove(&retBuf[ofs], &it->gatewayId, sizeof(uint64_t));  // 8
            ofs += 8;
            ofs += serializeSocketAddress(&retBuf[ofs], &it->sockaddr);   // 0, 7, 19
        }
    } else {
        for (auto it(identities.begin()); it != identities.end(); it++) {
            ofs += 8;
            ofs += serializeSocketAddress(nullptr, &it->sockaddr);   // 0, 7, 19
        }
    }
    return ofs;
}

std::string ListResponse::toJsonString() const {
    std::stringstream ss;
    ss << R"({"result": )" << OperationResponse::toJsonString()
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

size_t ListResponse::shortenList2Fit(
    size_t serializedSize
) {
    size_t r = this->serialize(nullptr);
    while(!identities.empty() && r > serializedSize) {
        identities.erase(identities.end() - 1);
        r = this->serialize(nullptr);
    }
    return r;
}

GatewaySerialization::GatewaySerialization(
    GatewayService *aSvc,
    int32_t aCode,
    uint64_t aAccessCode
)
    : svc(aSvc), code(aCode), accessCode(aAccessCode)
{

}

/**
 * Get size for serialized list
 * @param sz count ofg items
 * @return size in bytes
 */
size_t getMaxListResponseSize(
    size_t sz
)
{
    return SIZE_OPERATION_RESPONSE + sz * (sizeof(uint64_t) + 19);
}

static size_t getListResponseSize(
    const std::vector<GatewayIdentity> &list
)
{
    size_t r = SIZE_OPERATION_RESPONSE;
    for (const auto & it : list) {
        r += sizeof(uint64_t) + serializeSocketAddress(nullptr, &it.sockaddr);
    }
    return r;
}

size_t GatewaySerialization::query(
    unsigned char *retBuf,
    size_t retSize,
    const unsigned char *request,
    size_t sz
)
{
    if (!svc)
        return 0;
    if (sz < SIZE_SERVICE_MESSAGE)
        return 0;
    ServiceMessage *pMsg = deserialize((const unsigned char *) request, sz);
    if (!pMsg) {
#ifdef ENABLE_DEBUG
        std::cerr << "Wrong message" << std::endl;
#endif
        return 0;   // unknown request
    }
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
        *retBuf = r->serialize(retBuf);
        delete pMsg;
        return sizeof(OperationResponse);
    }

    ServiceMessage *r = nullptr;
    switch (request[0]) {
        case 'a':   // request gateway identifier(with address) by network address. Return 0 if success
            {
                auto gr = (GatewayIdRequest *) pMsg;
                r = new GetResponse(*gr);
                memmove(&((GetResponse*) r)->response.sockaddr, &gr->id, sizeof(gr->id));
                svc->get(((GetResponse*) r)->response, ((GetResponse*) r)->response);
                break;
            }
        case 'A':   // request gateway address (with identifier) by identifier. Return 0 if success
            {
                auto gr = (GatewayAddrRequest *) pMsg;
                r = new GetResponse(*gr);
                memmove(&((GetResponse*) r)->response.sockaddr, &gr->addr, sizeof(struct sockaddr));
                r->code = svc->get(((GetResponse*) r)->response, ((GetResponse*) r)->response);
                break;
            }
        case 'p':   // assign (put) gateway address to the gateway by identifier
            {
                auto gr = (GatewayIdAddrRequest *) pMsg;
                r = new OperationResponse(*gr);
                int errCode = svc->put(gr->identity);
                ((OperationResponse *) r)->response = errCode;
                if (errCode == 0)
                    ((OperationResponse *) r)->size = 1;    // count of placed entries
                break;
            }
        case 'r':   // Remove entry
            {
                auto gr = (GatewayIdAddrRequest *) pMsg;
                r = new OperationResponse(*gr);
                int errCode = svc->rm(gr->identity);
                ((OperationResponse *) r)->response = errCode;
                if (errCode == 0)
                    ((OperationResponse *) r)->size = 1;    // count of deleted entries
                break;
            }
        case 'L':   // List entries
        {
            auto gr = (OperationRequest *) pMsg;
            r = new ListResponse(*gr);
            svc->list(((ListResponse *) r)->identities, gr->offset, gr->size);
            size_t idSize = ((ListResponse *) r)->identities.size();
            size_t serSize = getListResponseSize(((ListResponse *) r)->identities);
            if (serSize > retSize) {
                serSize = ((ListResponse *) r)->shortenList2Fit(serSize);
                if (serSize > retSize) {
                    delete r;
                    r = nullptr;
                }
                break;
            }
            break;
        }
        case 'c':   // count
        {
            auto gr = (OperationRequest *) pMsg;
            r = new OperationResponse(*gr);
            r->tag = gr->tag;
            r->code = CODE_OK;
            r->accessCode = gr->accessCode;
            ((OperationResponse *) r)->response = svc->size();
            break;
        }
        case 'f':   // force save
            break;
        case 'd':   // close resources
            break;
        default:
            break;
    }
    delete pMsg;
    size_t rsize = 0;
    if (r) {
        r->ntoh();
        rsize = r->serialize(retBuf);
        delete r;
    }
    return rsize;
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
            if (sz < SIZE_GATEWAY_ID_REQUEST)
                return QUERY_GATEWAY_NONE;
            return QUERY_GATEWAY_ADDR;
        case 'A':   // request gateway address (with identifier) by identifier.
            if (sz < SIZE_GATEWAY_ADDR_4_REQUEST)
                return QUERY_GATEWAY_NONE;
            return QUERY_GATEWAY_ID;
        case 'p':   // assign (put) gateway address to the gateway by identifier
            if (sz < SIZE_GATEWAY_ADDR_4_REQUEST)
                return QUERY_GATEWAY_NONE;
            return QUERY_GATEWAY_ASSIGN;
        case 'r':   // Remove entry
            if (sz < SIZE_GATEWAY_ADDR_4_REQUEST)
                return QUERY_GATEWAY_NONE;
            return QUERY_GATEWAY_RM;
        case 'L':   // List entries
            if (sz < SIZE_OPERATION_REQUEST)
                return QUERY_GATEWAY_NONE;
            return QUERY_GATEWAY_LIST;
        case 'c':   // list count
            if (sz < SIZE_OPERATION_REQUEST)
                return QUERY_GATEWAY_NONE;
            return QUERY_GATEWAY_COUNT;
        case 'f':   // force save
            if (sz < SIZE_OPERATION_REQUEST)
                return QUERY_GATEWAY_NONE;
            return QUERY_GATEWAY_FORCE_SAVE;
        case 'd':   // close resources
            if (sz < SIZE_OPERATION_REQUEST)
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
        case QUERY_GATEWAY_ADDR:    // request gateway identifier(with address) by network address.
        case QUERY_GATEWAY_ID:      // request gateway address (with identifier) by identifier.
            return SIZE_GATEWAY_GET_ADDR_6_RESPONSE;    // IPv4: 28 IPv6: 40
        case QUERY_GATEWAY_LIST:    // List entries
            {
                OperationRequest lr(buffer, size);
                return getMaxListResponseSize(lr.size);
            }
        default:
            break;
    }
    return SIZE_OPERATION_RESPONSE; // OperationResponse
}

size_t makeResponse(
    GatewaySerialization *gatewaySerializer,
    unsigned char *retBuf,
    size_t retSize,
    const unsigned char *buf,
    size_t sz
)
{
    size_t rSize;
    if (sz > 0 && gatewaySerializer)	{
        rSize = gatewaySerializer->query(retBuf, retSize, buf, sz);
    } else {
        rSize = 0;
    }
#ifdef ENABLE_DEBUG
    std::cerr << "Response " << rSize << " bytes";
    if (sz > 0)
        std::cerr << ": " << hexString(retBuf, rSize) << std::endl;
#endif
    return rSize;
}

/**
 * Return request object or NULL if packet is invalid
 * @param buf buffer
 * @param sz buffer size
 * @return return NULL if packet is invalid
 */
ServiceMessage* deserialize(
    const unsigned char *buf,
    size_t sz
)
{
    if (sz < SIZE_SERVICE_MESSAGE)
        return nullptr;
    ServiceMessage *r;
    switch (buf[0]) {
        case 'a':   // request gateway identifier(with address) by network address. Return 0 if success
            if (sz < SIZE_GATEWAY_ID_REQUEST)
                return nullptr;
            r = new GatewayIdRequest(buf, sz);
            break;
        case 'A':   // request gateway address (with identifier) by identifier. Return 0 if success
            if (sz < SIZE_GATEWAY_ADDR_4_REQUEST)
                return nullptr;
            r = new GatewayAddrRequest(buf, sz);
            break;
        case 'p':   // assign (put) gateway address to the gateway by identifier
            if (sz < SIZE_GATEWAY_ASSIGN_ID_ADDR_4_REQUEST)
                return nullptr;
            r = new GatewayIdAddrRequest(buf, sz);
            break;
        case 'r':   // Remove entry
            if (sz < SIZE_GATEWAY_ID_REQUEST)   // it can contain id only(no address)
                return nullptr;
            r = new GatewayIdAddrRequest(buf, sz);
            break;
        case 'L':   // List entries
            if (sz < SIZE_OPERATION_REQUEST)
                return nullptr;
            r = new OperationRequest(buf, sz);
            break;
        case 'c':   // count
            if (sz < SIZE_OPERATION_REQUEST)
                return nullptr;
            r = new OperationRequest(buf, sz);
            break;
        case 'f':   // force save
            if (sz < SIZE_OPERATION_REQUEST)
                return nullptr;
            r = new OperationRequest(buf, sz);
            break;
        case 'd':   // close resources
            if (sz < SIZE_OPERATION_REQUEST)
                return nullptr;
            r = new OperationRequest(buf, sz);
            break;
        default:
            r = nullptr;
    }
    if (r)
        r->ntoh();
    return r;
}

const char* tag2string(
    enum CliGatewayQueryTag value
)
{
    switch (value) {
        case QUERY_GATEWAY_ADDR:
            return "address";
        case QUERY_GATEWAY_ID:
            return "identifier";
        case QUERY_GATEWAY_LIST:
            return "list";
        case QUERY_GATEWAY_COUNT:
            return "count";
        case QUERY_GATEWAY_ASSIGN:
            return "assign";
        case QUERY_GATEWAY_RM:
            return "remove";
        case QUERY_GATEWAY_FORCE_SAVE:
            return "save";
        case QUERY_GATEWAY_CLOSE_RESOURCES:
            return "close";
        default:
            return "";
    }
}
