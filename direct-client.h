#ifndef DIRECT_CLIENT_H_
#define DIRECT_CLIENT_H_	1

#include "identity-service.h"
#include "gateway-service.h"

/**
 * Abstract class to load specific identity and gateway services
 */
class DirectClient {
public:
    IdentityService* svcIdentity;
    GatewayService* svcGateway;
    DirectClient() = default;
    virtual ~DirectClient() = default;
};

#endif
