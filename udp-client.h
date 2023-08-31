#ifndef UDP_CLIENT_H_
#define UDP_CLIENT_H_	1

#ifdef _MSC_VER
#include <BaseTsd.h>
#include <Winsock2.h>
typedef SSIZE_T ssize_t;
#else
#include <netinet/in.h>
typedef int SOCKET;
#endif

#include <string>
#include "gateway-serialization.h"
#include "gateway-client.h"

class UDPClient : public GatewayClient {
private:
    SOCKET sock;
    struct sockaddr addr;
    GetRequest* query;
    int status;
public:
    explicit UDPClient(
        uint32_t ipv4,
        uint16_t port,
        ResponseIntf *onResponse
    );
    explicit UDPClient(
        const std::string &aHost,
        uint16_t port,
        ResponseIntf *onResponse
    );
    ~UDPClient() override;

    /**
     * Prepare to send request
     * @param value
     */
    void request(
        GetRequest* value
    ) override;
    void start() override;
    void stop() override;
};

#endif
