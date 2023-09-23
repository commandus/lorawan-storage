#ifndef GATEWAY_CLIENT_H
#define GATEWAY_CLIENT_H

class GatewayClient;

class ResponseIntf {
public:
    virtual void onGet(
        GatewayClient* client,
        const GetResponse *response
    ) = 0;
    virtual void onStatus(
        GatewayClient* client,
        const OperationResponse *response
    ) = 0;
    virtual void onList(
        GatewayClient* client,
        const ListResponse *response
    ) = 0;
    virtual void onError(
        GatewayClient* client,
        int32_t code  // 0- success, != 0- failure (error code)
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
     */
    virtual void request(
        ServiceMessage* value
    ) = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
};

#endif
