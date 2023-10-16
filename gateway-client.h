#ifndef GATEWAY_CLIENT_H
#define GATEWAY_CLIENT_H

class GatewayClient;

class ResponseIntf {
public:
    virtual void onGet(
        GatewayClient* client,
        const GatewayGetResponse *response
    ) = 0;
    virtual void onStatus(
        GatewayClient* client,
        const GatewayOperationResponse *response
    ) = 0;
    virtual void onList(
        GatewayClient* client,
        const GatewayListResponse *response
    ) = 0;
    virtual void onError(
        GatewayClient* client,
        int32_t code,  // 0- success, != 0- failure (error code)
        int errorCode
    ) = 0;
    // TCP connection lost
    virtual void onDisconnected(
        GatewayClient* client
    ) = 0;
};

class GatewayClient {
public:
    ResponseIntf* onResponse;

    GatewayClient(
        ResponseIntf *aOnResponse
    )
    : onResponse(aOnResponse)
    {

    };

    virtual ~GatewayClient() = default;

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
