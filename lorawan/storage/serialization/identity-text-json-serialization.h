#ifndef IDENTITY_TEXT_JSON_SERIALIZATION_H
#define IDENTITY_TEXT_JSON_SERIALIZATION_H

#ifdef _MSC_VER
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#else
#include <cinttypes>
#endif
#include "lorawan/storage/service/identity-service.h"
#include "lorawan/storage/serialization/identity-serialization.h"
#include "lorawan/storage/serialization/service-serialization.h"

class IdentityTextJSONSerialization : public IdentitySerialization {
public:
    explicit IdentityTextJSONSerialization(
        IdentityService* svc,
        int32_t code,
        uint64_t accessCode
    );

    /**
     * Request IdentityService and return serialized response.
     * @param retBuf buffer to return serialized response
     * @param retSize buffer size
     * @param request serialized request
     * @param sz serialized request size
     * @return IdentityService response size
     */
    size_t query(
        unsigned char* retBuf,
        size_t retSize,
        const unsigned char* request,
        size_t sz
    ) override;
};

#endif
