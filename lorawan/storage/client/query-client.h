#ifndef GATEWAY_CLIENT_H
#define GATEWAY_CLIENT_H

#include "lorawan/storage/serialization/identity-serialization.h"
#include "lorawan/storage/serialization/gateway-serialization.h"

class QueryClient;

class ResponseIntf {
public:
    virtual void onIdentityGet(
        QueryClient* client,
        const IdentityGetResponse *response
    ) = 0;
    virtual void onIdentityOperation(
        QueryClient* client,
        const IdentityOperationResponse *response
    ) = 0;
    virtual void onIdentityList(
        QueryClient* client,
        const IdentityListResponse *response
    ) = 0;

    virtual void onGatewayGet(
        QueryClient* client,
        const GatewayGetResponse *response
    ) = 0;
    virtual void onGatewayOperation(
        QueryClient* client,
        const GatewayOperationResponse *response
    ) = 0;
    virtual void onGatewayList(
        QueryClient* client,
        const GatewayListResponse *response
    ) = 0;
    virtual void onError(
        QueryClient* client,
        int32_t code,  // 0- success, != 0- failure (error code)
        int errorCode
    ) = 0;
    // TCP connection lost
    virtual void onDisconnected(
        QueryClient* client
    ) = 0;
};

class QueryClient {
public:
    ResponseIntf* onResponse;

    QueryClient(
        ResponseIntf *aOnResponse
    )
    : onResponse(aOnResponse)
    {

    };

    virtual ~QueryClient() = default;

    /**
     * Prepare to send request
     * @param value
     * @return previous message, NULL if not exists
     */
    virtual ServiceMessage* request(
        ServiceMessage* value
    ) = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
};

#endif
