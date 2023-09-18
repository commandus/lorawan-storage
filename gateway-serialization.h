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

class ServiceMessage {
public:
    char tag;
    int32_t code;  // "account#" in request
    uint64_t accessCode;  // magic number in request, retCode in response, negative is error code
    ServiceMessage();
    ServiceMessage(char tag, int32_t code, uint64_t accessCode);
    ServiceMessage(const char *buf, size_t sz);
    virtual void ntoh();
    virtual std::string toJsonString() const;
};  // 5 bytes

class GatewayIdRequest : public ServiceMessage {
public:
    uint64_t id;
    GatewayIdRequest();
    explicit GatewayIdRequest(char tag, const uint64_t id);
    GatewayIdRequest(char tag, const uint64_t id, int32_t code, uint64_t accessCode);
    GatewayIdRequest(const char *buf, size_t sz);
    void ntoh() override;

    std::string toJsonString() const override;
};

class GatewayIdAddrRequest : public ServiceMessage {
public:
    GatewayIdentity identity;
    GatewayIdAddrRequest();
    explicit GatewayIdAddrRequest(char tag, const GatewayIdentity &identity);
    GatewayIdAddrRequest(char tag, const GatewayIdentity &identity, int32_t code, uint64_t accessCode);
    GatewayIdAddrRequest(const char *buf, size_t sz);
    void ntoh() override;

    std::string toJsonString() const override;
};

class OperationRequest : public ServiceMessage {
public:
    size_t offset;
    size_t size;
    OperationRequest();
    explicit OperationRequest(char tag, const GatewayIdentity &identity);
    OperationRequest(char tag, size_t aOffset, size_t aSize, int32_t code, uint64_t accessCode);
    OperationRequest(const char *buf, size_t sz);
    void ntoh() override;
    std::string toJsonString() const override;
};

class GetResponse : public GatewayIdAddrRequest {
public:
    GatewayIdentity response;
    GetResponse() = default;
    explicit GetResponse(const GatewayIdAddrRequest& request);
    GetResponse(const char *buf, size_t sz);
    void ntoh() override;
    std::string toJsonString() const override;
};

class OperationResponse : public OperationRequest {
public:
    size_t response;
    OperationResponse();
    explicit OperationResponse(const OperationResponse& resp);
    OperationResponse(const char *buf, size_t sz);
    void ntoh() override;
    std::string toJsonString() const override;
};

class ListResponse : public OperationResponse {
public:
    GatewayIdentity identities[1];
    ListResponse();
    explicit ListResponse(const ListResponse& resp);
    ListResponse(const char *buf, size_t sz);
    void ntoh() override;
    std::string toJsonString() const override;
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
