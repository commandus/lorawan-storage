#include "udp-listener.h"

#ifdef ESP_PLATFORM
#include "platform-defs.h"
#endif

#ifdef ENABLE_DEBUG
#include <iostream>
#endif
#ifdef _MSC_VER
#include <iostream>
#endif

#ifdef _MSC_VER
    #include <ws2tcpip.h>
    #include <io.h>
    #define write _write
    #define close closesocket
#else
    #define SOCKET int
    #ifdef ESP_PLATFORM
        #include "esp_log.h"
        #include "esp_netif.h"

        #include "lwip/err.h"
        #include "lwip/sockets.h"
        #include "lwip/sys.h"
        #include <lwip/netdb.h>
    #else
        #include <netinet/in.h>
        #include <cstring>
        #include <unistd.h>
    #endif
#endif

#include "lorawan-string.h"
#include "lorawan-error.h"
#include "ip-helper.h"

#define DEF_KEEPALIVE_SECS 60

/**
 * @see https://habr.com/ru/post/340758/
 * @see https://github.com/Mityuha/grpc_async/blob/master/grpc_async_server.cc
 */
UDPListener::UDPListener(
    GatewaySerialization *aSerializationWrapper
)
    : GatewayListener(aSerializationWrapper), destAddr({}), status(CODE_OK)
{
}

// http://stackoverflow.com/questions/25615340/closing-libuv-handles-correctly
void UDPListener::stop()
{
    status = ERR_CODE_STOPPED;
}

void UDPListener::setAddress(
    const std::string &host,
    uint16_t port
)
{
    int addr_family = isAddrStringIPv6(host.c_str()) ? AF_INET6 : AF_INET;
    if (addr_family == AF_INET) {
        auto *destAddrIP4 = (struct sockaddr_in *) &destAddr;
        destAddrIP4->sin_addr.s_addr = htonl(INADDR_ANY);
        destAddrIP4->sin_family = AF_INET;
        destAddrIP4->sin_port = htons(port);
    } else {
        auto *destAddrIP6 = (struct sockaddr_in6 *) &destAddr;
        memset(&destAddrIP6->sin6_addr, 0, sizeof(destAddrIP6->sin6_addr));
        destAddrIP6->sin6_family = AF_INET6;
        destAddrIP6->sin6_port = htons(port);
    }
}

void UDPListener::setAddress(
    uint32_t &ipv4,
    uint16_t port
)
{
    struct sockaddr_in *a = (struct sockaddr_in *) &destAddr;
    a->sin_family = AF_INET;
    a->sin_addr.s_addr = ipv4;
    a->sin_port = htons(port);
}

int UDPListener::run()
{
#ifdef ENABLE_DEBUG	
	std::cerr << "Run.." << std::endl;
#endif
    char rxBuf[256];

    int proto = isIPv6(&destAddr) ? IPPROTO_IPV6 : IPPROTO_IP;
    int af = isIPv6(&destAddr) ? AF_INET6 : AF_INET;

    int r = CODE_OK;
    while (status != ERR_CODE_STOPPED) {
        SOCKET sock = socket(af, SOCK_DGRAM, proto);
        if (sock == INVALID_SOCKET) {
#ifdef ENABLE_DEBUG
            std::cerr << "Unable to create socket, error " << errno << std::endl;
#endif
#ifdef _MSC_VER
            std::cerr << "Unable to create socket, error " << WSAGetLastError() << std::endl;
#endif
            r = ERR_CODE_SOCKET_CREATE;
            break;
        }
#ifdef ENABLE_DEBUG
        std::cerr << "Socket created " << std::endl;
#endif

        int enable = 1;
        setsockopt(sock, IPPROTO_IP, IP_PKTINFO, (const char*) &enable, sizeof(enable));

        if (af == AF_INET6) {
            // Note that by default IPV6 binds to both protocols, it must be disabled
            // if both protocols used at the same time (used in CI)
            int opt = 1;
            setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*) &opt, sizeof(opt));
            setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, (const char*) &opt, sizeof(opt));
        }

        // Set timeout
        struct timeval timeout{1, 0};
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof timeout);

        if (bind(sock, (struct sockaddr *) &destAddr, sizeof(destAddr)) < 0) {
#ifdef ENABLE_DEBUG
            std::cerr << "Socket unable to bind, errno " << errno << std::endl;
#endif
        }
#ifdef ENABLE_DEBUG
        std::cerr << "Socket bound " << std::endl;
#endif
        struct sockaddr_storage source_addr{}; // Large enough for both IPv4 or IPv6
        socklen_t socklen = sizeof(source_addr);

        while (status != ERR_CODE_STOPPED) {
            int len = recvfrom(sock, rxBuf, sizeof(rxBuf) - 1, 0, (struct sockaddr *) &source_addr, &socklen);
            // Error occurred during receiving
            if (len < 0) {
                if (errno == EAGAIN)    // timeout occurs
                    continue;
#ifdef ENABLE_DEBUG
                std::cerr << "recvfrom error "
                    << len << ": " << strerror(errno)
                    << " , errno "
                    << errno << ": " << strerror(errno) << std::endl;
#endif
                continue;
            } else {
                // Data received
#ifdef ENABLE_DEBUG
                std::cerr << "Received " << len << " bytes" << std::endl;
#endif
                char *r;
                size_t sz = makeResponse(serializationWrapper, &r, rxBuf, len);
                if (r && (sz > 0)) {
                    if (sendto(sock, r, (int) sz, 0, (struct sockaddr *) &source_addr, sizeof(source_addr)) < 0) {
#ifdef ENABLE_DEBUG
                        std::cerr << "Error occurred during sending " << errno << std::endl;
#endif
                    }
                    free(r);
                }
            }
        }

#ifdef ENABLE_DEBUG
        std::cerr << "Shutting down socket and restarting.." << std::endl;
#endif
        shutdown(sock, 0);
        close(sock);
    }
#ifdef ENABLE_DEBUG
	std::cerr << "Run." << std::endl;
#endif
    return r;
}

UDPListener::~UDPListener()
{
	stop();
}
