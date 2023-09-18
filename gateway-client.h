#ifndef GATEWAY_CLIENT_H
#define GATEWAY_CLIENT_H

class GatewayClient;

class ResponseIntf {
public:
    virtual void onGet(
        GatewayClient* client,
        bool got,
        const GetResponse *retVal
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
            GatewayIdAddrRequest* value
    ) = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
};

#endif
