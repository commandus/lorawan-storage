#ifndef GATEWAY_TEXT_JSON_SERIALIZATION_H
#define GATEWAY_TEXT_JSON_SERIALIZATION_H

#include <cinttypes>
#include "lorawan/storage/service/gateway-service.h"
#include "lorawan/storage/gateway-identity.h"
#include "lorawan/storage/serialization/gateway-serialization.h"
#include "lorawan/storage/serialization/service-serialization.h"

class GatewayTextJSONSerialization : public GatewaySerialization {
public:
    explicit GatewayTextJSONSerialization(
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
    ) override;
};

#endif
