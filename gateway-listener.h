#ifndef GATEWAY_LISTENER_H
#define GATEWAY_LISTENER_H

#include "gateway-serialization.h"
#include "log-intf.h"

class GatewayListener {
public:
    GatewaySerialization *serializationWrapper;

    explicit GatewayListener(
            GatewaySerialization *aSerializationWrapper
    ) : serializationWrapper(aSerializationWrapper)
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

    virtual ~GatewayListener() = default;
};


#endif
