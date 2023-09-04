#ifndef GATEWAY_SERIALIZATION_H
#define GATEWAY_SERIALIZATION_H

#ifdef _MSC_VER
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#else
#include <cinttypes>
#endif
#include "gateway-service.h"
#include "gateway-identity.h"

class GatewayMessage {
public:
    char tag;
    int32_t code;  // "account#" in request
    uint64_t accessCode;  // magic number in request, retCode in response, negative is error code
    GatewayMessage() = default;
    GatewayMessage(char tag, int32_t code, uint64_t accessCode);
    GatewayMessage(const char *buf, size_t sz);
    void ntoh();
};  // 5 bytes

class GatewayRequest : public GatewayMessage {
public:
    char tag;
    GatewayIdentity identity;
    GatewayRequest();
    explicit GatewayRequest(char tag, const GatewayIdentity &identity);
    GatewayRequest(char tag, const GatewayIdentity &identity, int32_t code, uint64_t accessCode);
    GatewayRequest(const char *buf, size_t sz);
    void ntoh();
    std::string toJsonString() const;
};

class GetResponse : public GatewayRequest {
public:
    GatewayIdentity response;
    GetResponse() = default;
    explicit GetResponse(const GatewayRequest& request);
    GetResponse(const char *buf, size_t sz);
    void ntoh();
    std::string toString() const;
    std::string toJsonString() const;
};

class GatewaySerialization {
private:
    GatewayService *svc;
    int32_t code;
    uint64_t accessCode;
public:
    explicit GatewaySerialization(
        GatewayService *svc,
        int32_t code,
        uint64_t accessCode
    );
    /**
     * Request GatewayService
     * @param retBuf buffer to return serialized response
     * @param request serialized request
     * @param sz serialized request size
     * @return GatewayService response size
     */
    size_t query(
        char **retBuf,
        const char *request,
        size_t sz
    );

    static bool isGetResponse(
        const char *buf,
        size_t sz
    );
};

/**
 * Helper function
 * @param gatewaySerializer
 * @param retBuf
 * @param buf
 * @param sz
 * @return
 */
size_t makeResponse(
    GatewaySerialization *gatewaySerializer,
    char **retBuf,
    const char *buf,
    ssize_t sz
);

#endif
