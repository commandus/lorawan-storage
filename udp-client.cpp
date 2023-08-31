#include "udp-client.h"

#ifdef _MSC_VER
#include <io.h>
#include <WS2tcpip.h>
#define close closesocket
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#ifdef ENABLE_DEBUG
#include <iostream>
#include <cstring>
#include "lorawan-msg.h"
#endif

#include "lorawan-string.h"
#include "lorawan-error.h"
#include "ip-helper.h"

#define DEF_KEEPALIVE_SECS 60

void UDPClient::stop()
{
    status = ERR_CODE_STOPPED;
}

UDPClient::UDPClient(
    uint32_t ipv4,
    uint16_t aPort,
    ResponseIntf *aOnResponse
)
    : GatewayClient(aOnResponse), query(nullptr), status(CODE_OK)
{
    auto *a = (struct sockaddr_in *) &addr;
    a->sin_addr.s_addr = ipv4;
    a->sin_port = htons(aPort);
}

/**
 */
UDPClient::UDPClient(
    const std::string &aHost,
    uint16_t aPort,
    ResponseIntf *aOnResponse
)
    : GatewayClient(aOnResponse), query(nullptr), status(CODE_OK)
{
    memset(&addr, 0, sizeof(addr));
    if (isAddrStringIPv6(aHost.c_str())) {
        auto *a = (struct sockaddr_in6 *) &addr;
        inet_pton(AF_INET, aHost.c_str(), a);
        a->sin6_port = htons(aPort);
        a->sin6_scope_id = 0;
    } else {
        auto *a = (struct sockaddr_in *) &addr;
        inet_pton(AF_INET6, aHost.c_str(), a);
        a->sin_port = htons(aPort);
    }
}

UDPClient::~UDPClient()
{
    stop();
}

void UDPClient::request(
    GetRequest* value
)
{
    query = value;
}

void UDPClient::start() {
    status = CODE_OK;
    int af = isIPv6(&addr) ? AF_INET6 : AF_INET;
    int proto = isIPv6(&addr) ? IPPROTO_IPV6 : IPPROTO_IP;

    while (status != ERR_CODE_STOPPED) {
        sock = socket(af, SOCK_DGRAM, proto);
        if (sock == INVALID_SOCKET) {
            status = ERR_CODE_SOCKET_CREATE;
#ifdef ENABLE_DEBUG
            std::cerr << ERR_SOCKET_CREATE << " " << errno << ": " << strerror(errno) << std::endl;
#endif
            onResponse->onGet(this, false, nullptr);
            break;
        }

        // Set timeout
        struct timeval timeout{ 1, 0 };
        setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (const char *) &timeout, sizeof timeout);
        while (status != ERR_CODE_STOPPED) {
            if (!query)
                break;
            query->ntoh();
            ssize_t sz = sendto(sock, (const char*) query, sizeof(GetRequest), 0, (struct sockaddr *)&addr, sizeof(addr));
            if (sz < 0) {
                status = ERR_CODE_SOCKET_WRITE;
#ifdef ENABLE_DEBUG
                std::cerr << ERR_SOCKET_WRITE << " " << errno << ": " << strerror(errno) << std::endl;
#endif
                onResponse->onGet(this, false, nullptr);
                break;
            }
#ifdef ENABLE_DEBUG
            std::cerr << MSG_SENT << sz << MSG_BYTES << std::endl;
#endif
            struct sockaddr_storage srcAddress{}; // Large enough for both IPv4 or IPv6
            socklen_t socklen = sizeof(srcAddress);
            char rxBuf[256];
            ssize_t len = recvfrom(sock, rxBuf, sizeof(rxBuf), 0, (struct sockaddr *)&srcAddress, &socklen);

            if (len < 0) {  // Error occurred during receiving
#ifdef ENABLE_DEBUG
                std::cerr << ERR_SOCKET_READ << " " << errno << ": " << strerror(errno) << std::endl;
#endif
                status = ERR_CODE_SOCKET_READ;
                onResponse->onGet(this, false, nullptr);
                break;
            } else {
#ifdef ENABLE_DEBUG
                std::cerr << MSG_RECEIVED << len << " " << MSG_BYTES << std::endl;
#endif
                GetResponse gr = GetResponse(rxBuf, len);
                gr.ntoh();
                onResponse->onGet(this, true, &gr);
            }
        }

        if (sock != -1) {
            shutdown(sock, 0);
            close(sock);
        }
    }
}
