#include "identity-serialization.h"
#include "lorawan-conv.h"
#include "ip-helper.h"

#include <sstream>
#include <cstring>

#include "lorawan-error.h"
#include "ip-address.h"
#include "lorawan-string.h"

#ifdef ESP_PLATFORM
#include "platform-defs.h"
#endif

#define SIZE_OPERATION_REQUEST 18
#define SIZE_OPERATION_RESPONSE 22
#define SIZE_DEVICE_EUI_REQUEST   21
#define SIZE_DEVICE_ADDR_REQUEST 17
#define SIZE_DEVICE_EUI_ADDR_REQUEST 25

#define SIZE_DEVICE_GET_ADDR_4_RESPONSE 28
#define SIZE_DEVICE_GET_ADDR_6_RESPONSE 40

#ifdef ENABLE_DEBUG
#include <iostream>
#include "lorawan-string.h"
#include "lorawan-msg.h"
#endif

IdentityEUIRequest::IdentityEUIRequest()
    : ServiceMessage(QUERY_IDENTITY_ADDR, 0, 0), eui(0)
{
}

IdentityEUIRequest::IdentityEUIRequest(
    char aTag,
    const DEVEUI &aEUI,
    int32_t code,
    uint64_t accessCode
)
    : ServiceMessage(aTag, code, accessCode), eui(aEUI)
{

}

IdentityEUIRequest::IdentityEUIRequest(
    const unsigned char *buf,
    size_t sz
)
    : ServiceMessage(buf, sz)   //  13
{
    if (sz >= SIZE_DEVICE_EUI_REQUEST) {
        memmove(&eui.u, &buf[13], sizeof(eui.u));     // 8
    }   // 21
}

void IdentityEUIRequest::ntoh() {
    ServiceMessage::ntoh();
    eui.u = NTOH8(eui.u);
}

size_t IdentityEUIRequest::serialize(
    unsigned char *retBuf
) const
{
    ServiceMessage::serialize(retBuf);      // 13
    memmove(&retBuf[13], &eui.u, sizeof(eui.u));  // 8
    return SIZE_DEVICE_EUI_REQUEST;         // 21
}

std::string IdentityEUIRequest::toJsonString() const
{
    std::stringstream ss;
    ss << R"("eui":")" << DEVEUI2string(eui);
    return ss.str();
}

IdentityAddrRequest::IdentityAddrRequest()
    : ServiceMessage(QUERY_IDENTITY_ADDR, 0, 0)
{
    memset(&addr.u, 0, sizeof(addr.u));
}

IdentityAddrRequest::IdentityAddrRequest(
    const unsigned char *buf,
    size_t sz
)
    : ServiceMessage(buf, sz)
{
    if (sz >= SIZE_DEVICE_ADDR_REQUEST) {
        memmove(&addr.u, buf + SIZE_SERVICE_MESSAGE, sizeof(addr.u)); // 4
    }
}

IdentityAddrRequest::IdentityAddrRequest(
    const DEVADDR &aAddr,
    int32_t code,
    uint64_t accessCode
)
    : ServiceMessage(QUERY_IDENTITY_EUI, code, accessCode)
{
    addr.u = aAddr.u;
}

void IdentityAddrRequest::ntoh()
{
    ServiceMessage::ntoh();
    addr.u = NTOH4(addr.u);
}

size_t IdentityAddrRequest::serialize(
    unsigned char *retBuf
) const
{
    ServiceMessage::serialize(retBuf);      // 13
    memmove(retBuf + SIZE_SERVICE_MESSAGE, &addr.u, sizeof(addr.u)); // 4
    return SIZE_DEVICE_ADDR_REQUEST;        // 17
}

std::string IdentityAddrRequest::toJsonString() const
{
    std::stringstream ss;
    ss << R"({"addr": ")" << DEVADDR2string(addr) << "\"}";
    return ss.str();
}

IdentityEUIAddrRequest::IdentityEUIAddrRequest()
    : ServiceMessage(QUERY_IDENTITY_ASSIGN, 0, 0), identity()
{
}

IdentityEUIAddrRequest::IdentityEUIAddrRequest(
    const unsigned char *buf,
    size_t sz
)
    : ServiceMessage(buf, sz)   // 13
{
    if (sz >= SIZE_DEVICE_EUI_ADDR_REQUEST) {
        memmove(&identity.devid.devEUI.u, buf + SIZE_SERVICE_MESSAGE, sizeof(identity.devid.devEUI.u));   // 8
        memmove(&identity.devaddr.u, buf + SIZE_SERVICE_MESSAGE + sizeof(identity.devid.devEUI.u), sizeof(identity.devaddr.u));   // 4
    }   // 25
}

IdentityEUIAddrRequest::IdentityEUIAddrRequest(
    char aTag,
    const NETWORKIDENTITY &aIdentity,
    int32_t code,
    uint64_t accessCode
)
    : ServiceMessage(aTag, code, accessCode), identity(aIdentity)
{
}

void IdentityEUIAddrRequest::ntoh()
{
    ServiceMessage::ntoh();
    identity.devid.devEUI.u = NTOH8(identity.devid.devEUI.u);
    identity.devaddr.u = NTOH4(identity.devaddr.u);
}

size_t IdentityEUIAddrRequest::serialize(
    unsigned char *retBuf
) const
{
    ServiceMessage::serialize(retBuf);      // 13
    memmove(retBuf + SIZE_SERVICE_MESSAGE, &identity.devid.devEUI.u, sizeof(identity.devid.devEUI.u)); // 8
    memmove(retBuf + SIZE_SERVICE_MESSAGE + sizeof(identity.devid.devEUI.u), &identity.devaddr.u, sizeof(identity.devaddr.u)); // 4
    return SIZE_DEVICE_EUI_ADDR_REQUEST;     // 25
}

std::string IdentityEUIAddrRequest::toJsonString() const
{
    std::stringstream ss;
    ss << R"({"identity": ")" << identity.toJsonString() << "\"}";
    return ss.str();
}

OperationRequest::OperationRequest()
    : ServiceMessage(QUERY_GATEWAY_LIST, 0, 0),
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
static size_t getMaxGatewayListResponseSize(
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
        case QUERY_GATEWAY_ADDR:   // request gateway identifier(with address) by network address. Return 0 if success
            {
                auto gr = (GatewayIdRequest *) pMsg;
                r = new GetResponse(*gr);
                memmove(&((GetResponse*) r)->response.sockaddr, &gr->id, sizeof(gr->id));
                svc->get(((GetResponse*) r)->response, ((GetResponse*) r)->response);
                break;
            }
        case QUERY_GATEWAY_ID:   // request gateway address (with identifier) by identifier. Return 0 if success
            {
                auto gr = (GatewayAddrRequest *) pMsg;
                r = new GetResponse(*gr);
                memmove(&((GetResponse*) r)->response.sockaddr, &gr->addr, sizeof(struct sockaddr));
                r->code = svc->get(((GetResponse*) r)->response, ((GetResponse*) r)->response);
                break;
            }
        case QUERY_GATEWAY_ASSIGN:   // assign (put) gateway address to the gateway by identifier
            {
                auto gr = (GatewayIdAddrRequest *) pMsg;
                r = new OperationResponse(*gr);
                int errCode = svc->put(gr->identity);
                ((OperationResponse *) r)->response = errCode;
                if (errCode == 0)
                    ((OperationResponse *) r)->size = 1;    // count of placed entries
                break;
            }
        case QUERY_GATEWAY_RM:   // Remove entry
            {
                auto gr = (GatewayIdAddrRequest *) pMsg;
                r = new OperationResponse(*gr);
                int errCode = svc->rm(gr->identity);
                ((OperationResponse *) r)->response = errCode;
                if (errCode == 0)
                    ((OperationResponse *) r)->size = 1;    // count of deleted entries
                break;
            }
        case QUERY_GATEWAY_LIST:   // List entries
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
        case QUERY_GATEWAY_COUNT:   // count
        {
            auto gr = (OperationRequest *) pMsg;
            r = new OperationResponse(*gr);
            r->tag = gr->tag;
            r->code = CODE_OK;
            r->accessCode = gr->accessCode;
            ((OperationResponse *) r)->response = svc->size();
            break;
        }
        case QUERY_GATEWAY_FORCE_SAVE:   // force save
            break;
        case QUERY_GATEWAY_CLOSE_RESOURCES:   // close resources
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
 * Check does it serialized query in the buffer
 * @param buffer buffer to check
 * @param size buffer size
 * @return query tag
 */
enum CliGatewayQueryTag validateGatewayQuery(
    const unsigned char *buffer,
    size_t size
)
{
    switch (buffer[0]) {
        case QUERY_GATEWAY_ADDR:   // request gateway identifier(with address) by network address.
            if (size < SIZE_DEVICE_EUI_REQUEST)
                return QUERY_GATEWAY_NONE;
            return QUERY_GATEWAY_ADDR;
        case QUERY_GATEWAY_ID:   // request gateway address (with identifier) by identifier.
            if (size < SIZE_DEVICE_ADDR_REQUEST)
                return QUERY_GATEWAY_NONE;
            return QUERY_GATEWAY_ID;
        case QUERY_GATEWAY_ASSIGN:   // assign (put) gateway address to the gateway by identifier
            if (size < SIZE_DEVICE_ADDR_REQUEST)
                return QUERY_GATEWAY_NONE;
            return QUERY_GATEWAY_ASSIGN;
        case QUERY_GATEWAY_RM:   // Remove entry
            if (size < SIZE_DEVICE_ADDR_REQUEST)
                return QUERY_GATEWAY_NONE;
            return QUERY_GATEWAY_RM;
        case QUERY_GATEWAY_LIST:   // List entries
            if (size < SIZE_OPERATION_REQUEST)
                return QUERY_GATEWAY_NONE;
            return QUERY_GATEWAY_LIST;
        case QUERY_GATEWAY_COUNT:   // list count
            if (size < SIZE_OPERATION_REQUEST)
                return QUERY_GATEWAY_NONE;
            return QUERY_GATEWAY_COUNT;
        case QUERY_GATEWAY_FORCE_SAVE:   // force save
            if (size < SIZE_OPERATION_REQUEST)
                return QUERY_GATEWAY_NONE;
            return QUERY_GATEWAY_FORCE_SAVE;
        case QUERY_GATEWAY_CLOSE_RESOURCES:   // close resources
            if (size < SIZE_OPERATION_REQUEST)
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
size_t responseSizeForGatewayRequest(
    const unsigned char *buffer,
    size_t size
)
{
    enum CliGatewayQueryTag tag = validateGatewayQuery(buffer, size);
    switch (tag) {
        case QUERY_GATEWAY_ADDR:    // request gateway identifier(with address) by network address.
        case QUERY_GATEWAY_ID:      // request gateway address (with identifier) by identifier.
            return SIZE_DEVICE_GET_ADDR_6_RESPONSE;    // IPv4: 28 IPv6: 40
        case QUERY_GATEWAY_LIST:    // List entries
            {
                OperationRequest lr(buffer, size);
                return getMaxGatewayListResponseSize(lr.size);
            }
        default:
            break;
    }
    return SIZE_OPERATION_RESPONSE; // OperationResponse
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
        case QUERY_GATEWAY_ADDR:   // request gateway identifier(with address) by network address. Return 0 if success
            if (sz < SIZE_DEVICE_EUI_REQUEST)
                return nullptr;
            r = new GatewayIdRequest(buf, sz);
            break;
        case QUERY_GATEWAY_ID:   // request gateway address (with identifier) by identifier. Return 0 if success
            if (sz < SIZE_DEVICE_ADDR_REQUEST)
                return nullptr;
            r = new GatewayAddrRequest(buf, sz);
            break;
        case QUERY_GATEWAY_ASSIGN:   // assign (put) gateway address to the gateway by identifier
            if (sz < SIZE_DEVICE_EUI_ADDR_REQUEST)
                return nullptr;
            r = new GatewayIdAddrRequest(buf, sz);
            break;
        case QUERY_GATEWAY_RM:   // Remove entry
            if (sz < SIZE_DEVICE_EUI_REQUEST)   // it can contain id only(no address)
                return nullptr;
            r = new GatewayIdAddrRequest(buf, sz);
            break;
        case QUERY_GATEWAY_LIST:   // List entries
            if (sz < SIZE_OPERATION_REQUEST)
                return nullptr;
            r = new OperationRequest(buf, sz);
            break;
        case QUERY_GATEWAY_COUNT:   // count
            if (sz < SIZE_OPERATION_REQUEST)
                return nullptr;
            r = new OperationRequest(buf, sz);
            break;
        case QUERY_GATEWAY_FORCE_SAVE:   // force save
            if (sz < SIZE_OPERATION_REQUEST)
                return nullptr;
            r = new OperationRequest(buf, sz);
            break;
        case QUERY_GATEWAY_CLOSE_RESOURCES:   // close resources
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

const char* gatewayTag2string(
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
