#ifndef ASYNC_WRAPPER_GATEWAY_SERVICE_H_
#define ASYNC_WRAPPER_GATEWAY_SERVICE_H_ 1

#include <functional>
#include "gateway-service.h"

/**
 * Identity service interface
 * Get device identifier and keys by the network address
 */ 
class AsyncWrapperGatewayService {
private:
    GatewayService *gatewayService;
public:
    AsyncWrapperGatewayService();
    void get(
        const GatewayIdentity &request,
        std::function<void(
            int retCode,
            GatewayIdentity &retVal
        )> cb
    )
    {
        GatewayIdentity gatewayIdentity;
        int r = gatewayService->get(gatewayIdentity, request);
        cb(r, gatewayIdentity);
    }

    // Add or replace Address = EUI and keys pair
    void put(
        const GatewayIdentity &identity,
        std::function<void(
            int retCode
        )> cb
    )
    {
        int r = gatewayService->put(identity);
        cb(r);
    }

    // Remove entry
    void rm(
        const GatewayIdentity &identity,
        std::function<void(
            int retCode
        )> cb
    )
    {
        int r = gatewayService->rm(identity);
        cb(r);
    }

    // List entries
    void list(
        size_t offset,
        size_t size,
        std::function<void(
            int retCode,
            std::vector<GatewayIdentity> &retVal
        )> cb
    )
    {
        std::vector<GatewayIdentity> gatewayIdentities;
        int r = gatewayService->list(gatewayIdentities, offset, size);
        cb(r, gatewayIdentities);
    }

    // Entries count
    void size(
        std::function<void(
            size_t size
        )> cb
    )
    {
        int r = gatewayService->size();
        cb(r);
    }

    // force save
    void flush(
        std::function<void(
            int retCode
        )> cb
    )
    {
        gatewayService->flush();
        cb(0);
    }

    // reload
    int init(
        const std::string &option,
        void *data,
        std::function<void(
            int retCode
        )> cb
    )
    {
        cb(gatewayService->init(option, data));
    }

    // close resources
    void done(
        std::function<void(
            int retCode
        )> cb
    )
    {
        gatewayService->done();
        cb(0);
    }
};

#endif
