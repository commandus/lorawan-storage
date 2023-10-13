#ifndef GATEWAY_SERIALIZATION_H
#define GATEWAY_SERIALIZATION_H

#ifdef _MSC_VER
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#else
#include <cinttypes>
#endif
#include "identity-service.h"
#include "gateway-identity.h"
#include "service-serialization.h"

enum CliIdentityQueryTag {
    QUERY_IDENTITY_ADDR = 'a',
    QUERY_IDENTITY_EUI = 'i',
    QUERY_IDENTITY_LIST = 'l',
    QUERY_IDENTITY_COUNT = 'c',
    QUERY_IDENTITY_ASSIGN = 'p',
    QUERY_IDENTITY_RM = 'r',
    QUERY_IDENTITY_FORCE_SAVE = 's',
    QUERY_IDENTITY_CLOSE_RESOURCES = 'e'
};

class IdentityEUIRequest : public ServiceMessage {
public:
    DEVEUI eui; // 8 bytes
    IdentityEUIRequest();
    IdentityEUIRequest(char aTag, const DEVEUI &aEUI, int32_t code, uint64_t accessCode);
    IdentityEUIRequest(const unsigned char *buf, size_t sz);
    void ntoh() override;
    size_t serialize(unsigned char *retBuf) const override;
    std::string toJsonString() const override;
};

class IdentityAddrRequest : public ServiceMessage {
public:
    DEVADDR addr;   // 4 bytes
    IdentityAddrRequest();
    IdentityAddrRequest(const DEVADDR &addr, int32_t code, uint64_t accessCode);
    IdentityAddrRequest(const unsigned char *buf, size_t sz);
    void ntoh() override;
    size_t serialize(unsigned char *retBuf) const override;
    std::string toJsonString() const override;
};

class IdentityEUIAddrRequest : public ServiceMessage {
public:
    NETWORKIDENTITY identity;
    IdentityEUIAddrRequest();
    // explicit IdentityEUIAddrRequest(const DeviceIdentity &identity);
    IdentityEUIAddrRequest(char aTag, const NETWORKIDENTITY &identity, int32_t code, uint64_t accessCode);
    IdentityEUIAddrRequest(const unsigned char *buf, size_t sz);
    void ntoh() override;
    size_t serialize(unsigned char *retBuf) const override;
    std::string toJsonString() const override;
};

class OperationRequest : public ServiceMessage {
public:
    uint32_t offset;
    uint8_t size;
    OperationRequest();
    OperationRequest(char tag, size_t aOffset, size_t aSize, int32_t code, uint64_t accessCode);
    OperationRequest(const unsigned char *buf, size_t sz);
    void ntoh() override;
    size_t serialize(unsigned char *retBuf) const override;
    std::string toJsonString() const override;
};

class GetResponse : public ServiceMessage {
public:
    NETWORKIDENTITY response;

    GetResponse() = default;
    explicit GetResponse(const IdentityAddrRequest& request);
    explicit GetResponse(const IdentityEUIRequest &request);
    GetResponse(const unsigned char *buf, size_t sz);
    void ntoh() override;
    size_t serialize(unsigned char *retBuf) const override;
    size_t serializedSize() const;
    std::string toJsonString() const override;
};

class OperationResponse : public OperationRequest {
public:
    uint32_t response;
    OperationResponse();
    OperationResponse(const OperationResponse& resp);
    OperationResponse(const unsigned char *buf, size_t sz);
    explicit OperationResponse(const IdentityEUIAddrRequest &request);
    explicit OperationResponse(const OperationRequest &request);
    void ntoh() override;
    size_t serialize(unsigned char *retBuf) const override;
    std::string toJsonString() const override;
};

class ListResponse : public OperationResponse {
public:
    std::vector<NETWORKIDENTITY> identities;
    ListResponse();
    ListResponse(const ListResponse& resp);
    ListResponse(const unsigned char *buf, size_t sz);
    explicit ListResponse(const OperationRequest &request);
    void ntoh() override;
    size_t serializedSize() const;
    size_t serialize(unsigned char *retBuf) const override;
    std::string toJsonString() const override;
    size_t shortenList2Fit(size_t serializedSize);
};

class IdentitySerialization {
private:
    IdentityService *svc;
    int32_t code;
    uint64_t accessCode;
public:
    explicit IdentitySerialization(
        IdentityService *svc,
        int32_t code,
        uint64_t accessCode
    );
    /**
     * Request IdentityService and return serializred response.
     * @param retBuf buffer to return serialized response
     * @param retSize buffer size
     * @param request serialized request
     * @param sz serialized request size
     * @return IdentityService response size
     */
    size_t query(
        unsigned char *retBuf,
        size_t retSize,
        const unsigned char *request,
        size_t sz
    );
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

/**
 * Check does it serialized query in the buffer
 * @param buffer buffer to check
 * @param size buffer size
 * @return query tag
 */
enum CliIdentityQueryTag validateIdentityQuery(
    const unsigned char *buffer,
    size_t size
);

/**
 * Return required size for response
 * @param buffer serialized request
 * @param size buffer size
 * @return size in bytes
 */
size_t responseSizeForIdentityRequest(
    const unsigned char *buffer,
    size_t size
);

const char* identityTag2string(enum CliIdentityQueryTag value);

#endif
