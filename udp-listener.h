#ifndef UDP_LISTENER_H_
#define UDP_LISTENER_H_	1

#include <string>
#ifdef _MSC_VER
#include <WinSock2.h>
#else
#include <sys/socket.h>
#endif

#include "gateway-listener.h"

class UDPListener : public GatewayListener {
private:
    struct sockaddr destAddr;
public:
    int status; // ERR_CODE_STOPPED - stop request
    explicit UDPListener(
        GatewaySerialization *aSerializationWrapper
    );
    virtual ~UDPListener();
    void setAddress(
        const std::string &host,
        uint16_t port
    ) override;
    void setAddress(
        uint32_t &ipv4,
        uint16_t port
    ) override;
    int run() override;
    void stop() override;
};

#endif
