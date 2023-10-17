#ifndef GATEWAY_LISTENER_H
#define GATEWAY_LISTENER_H

#include "identity-serialization.h"
#include "gateway-serialization.h"
#include "log-intf.h"

class StorageListener {
public:
    IdentitySerialization *identitySerialization;
    GatewaySerialization *gatewaySerialization;

    explicit StorageListener(
        IdentitySerialization *aIdentitySerialization,
        GatewaySerialization *aSerializationWrapper
    ) : identitySerialization(aIdentitySerialization), gatewaySerialization(aSerializationWrapper)
    {

    }

    virtual void setAddress(
        const std::string &host,
        uint16_t port
    ) = 0;

    virtual void setAddress(
        uint32_t &ipv4,
        uint16_t port
    ) = 0;

    virtual int run() = 0;

    virtual void stop() = 0;

    virtual void setLog(int verbose, Log *log) = 0;

    virtual ~StorageListener() = default;
};


#endif
