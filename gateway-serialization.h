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

enum CliGatewayQueryTag {
    QUERY_GATEWAY_NONE = '0',
    QUERY_GATEWAY_ADDR = 'a',
    QUERY_GATEWAY_ID = 'A',
    QUERY_GATEWAY_LIST = 'L',
    QUERY_GATEWAY_COUNT = 'c',
    QUERY_GATEWAY_ASSIGN = 'p',
    QUERY_GATEWAY_RM = 'r',
    QUERY_GATEWAY_FORCE_SAVE = 'f',
    QUERY_GATEWAY_CLOSE_RESOURCES = 'd'
};

class ServiceMessage {
public:
    char tag;
    int32_t code;  // "account#" in request
    uint64_t accessCode;  // magic number in request, retCode in response, negative is error code
    ServiceMessage();
    ServiceMessage(char tag, int32_t code, uint64_t accessCode);
    ServiceMessage(const unsigned char *buf, size_t sz);
    virtual void ntoh();
    virtual size_t serialize(unsigned char *retBuf) const;
    virtual size_t deserialize(const unsigned char *buf, size_t sz);
    virtual std::string toJsonString() const;
};  // 5 bytes

class GatewayIdRequest : public ServiceMessage {
public:
    uint64_t id;
    GatewayIdRequest();
    explicit GatewayIdRequest(const uint64_t id);
    GatewayIdRequest(char aTag, uint64_t id, int32_t code, uint64_t accessCode);
    GatewayIdRequest(const unsigned char *buf, size_t sz);
    void ntoh() override;
    size_t serialize(unsigned char *retBuf) const override;
    size_t deserialize(const unsigned char *buf, size_t sz) override;
    std::string toJsonString() const override;
};

class GatewayAddrRequest : public ServiceMessage {
public:
    sockaddr addr;
    GatewayAddrRequest();
    explicit GatewayAddrRequest(const GatewayIdentity &identity);
    GatewayAddrRequest(const struct sockaddr &addr, int32_t code, uint64_t accessCode);
    GatewayAddrRequest(const unsigned char *buf, size_t sz);
    void ntoh() override;
    size_t serialize(unsigned char *retBuf) const override;
    size_t deserialize(const unsigned char *buf, size_t sz) override;
    std::string toJsonString() const override;
};

class GatewayIdAddrRequest : public ServiceMessage {
public:
    GatewayIdentity identity;
    GatewayIdAddrRequest();
    explicit GatewayIdAddrRequest(const GatewayIdentity &identity);
    GatewayIdAddrRequest(char aTag, const GatewayIdentity &identity, int32_t code, uint64_t accessCode);
    GatewayIdAddrRequest(const unsigned char *buf, size_t sz);
    void ntoh() override;
    size_t serialize(unsigned char *retBuf) const override;
    size_t deserialize(const unsigned char *buf, size_t sz) override;
    std::string toJsonString() const override;
};

class OperationRequest : public ServiceMessage {
public:
    size_t offset;
    size_t size;
    OperationRequest();
    explicit OperationRequest(char tag);
    OperationRequest(char tag, size_t aOffset, size_t aSize, int32_t code, uint64_t accessCode);
    OperationRequest(const unsigned char *buf, size_t sz);
    void ntoh() override;
    size_t serialize(unsigned char *retBuf) const override;
    size_t deserialize(const unsigned char *buf, size_t sz) override;
    std::string toJsonString() const override;
};

class GetResponse : public ServiceMessage {
public:
    GatewayIdentity response;

    GetResponse() = default;
    GetResponse(const GatewayAddrRequest& request);
    GetResponse(const GatewayIdRequest &request);
    GetResponse(const unsigned char *buf, size_t sz);
    GetResponse(const GatewayIdAddrRequest &request);
    GetResponse(const OperationRequest &request);

    void ntoh() override;
    size_t serialize(unsigned char *retBuf) const override;
    size_t deserialize(const unsigned char *buf, size_t sz) override;
    std::string toJsonString() const override;
};

class OperationResponse : public OperationRequest {
public:
    size_t response;
    OperationResponse();
    explicit OperationResponse(const OperationResponse& resp);
    OperationResponse(const unsigned char *buf, size_t sz);
    OperationResponse(const GatewayIdAddrRequest &request);
    OperationResponse(const OperationRequest &request);
    void ntoh() override;
    size_t serialize(unsigned char *retBuf) const override;
    size_t deserialize(const unsigned char *buf, size_t sz) override;
    std::string toJsonString() const override;
};

class ListResponse : public OperationResponse {
public:
    std::vector<GatewayIdentity> identities;
    ListResponse();
    ListResponse(const ListResponse& resp);
    ListResponse(const unsigned char *buf, size_t sz);
    ListResponse(const OperationRequest & request);
    void ntoh() override;
    size_t serialize(unsigned char *retBuf) const override;
    size_t deserialize(const unsigned char *buf, size_t sz) override;
    std::string toJsonString() const override;
};

/**
 * Return request object or  NULL if packet is invalid
 * @param buf buffer
 * @param sz buffer size
 * @return return NULL if packet is invalid
 */
ServiceMessage* deserialize(
    const unsigned char *buf,
    size_t sz
);

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
     * Request GatewayService and return serializred response.
     * @param retBuf buffer to return serialized response
     * @param retSize buffer size
     * @param request serialized request
     * @param sz serialized request size
     * @return GatewayService response size
     */
    size_t query(
        unsigned char *retBuf,
        size_t retSize,
        const unsigned char *request,
        size_t sz
    );
};

/**
 * Check does it serialized query in the buffer
 * @param buffer buffer to check
 * @param size buffer size
 * @return query tag
 */
enum CliGatewayQueryTag validateQuery(
    const unsigned char *buffer,
    size_t size
);

/**
 * Get query tag in the buffer
 * @param str PChar
 * @return query tag
 */
enum CliGatewayQueryTag getQueryTag(
    const char *str
);

/**
 * Return required size for response
 * @param buffer serialized request
 * @param size buffer size
 * @return size in bytes
 */
size_t responseSizeForRequest(
    const unsigned char *buffer,
    size_t size
);

/**
 * Get size for serialized list
 * @param sz count ofg items
 * @return size in bytes
 */
size_t getMaxListResponseSize(
    size_t sz
);

/**
 * Helper function
 * @param gatewaySerializer
 * @param retBuf return buffer
 * @param retSize buffer size
 * @param buf received packed
 * @param sz received packed size
 * @return 0 if no response
 */
size_t makeResponse(
    GatewaySerialization *gatewaySerializer,
    unsigned char *retBuf,
    size_t retSize,
    const unsigned char *buf,
    size_t sz
);

#endif
