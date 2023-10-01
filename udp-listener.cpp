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

#ifdef _MSC_VER
#define SOCKET_ERRNO WSAGetLastError()
#define ERR_TIMEOUT WSAETIMEDOUT
#else
#define SOCKET_ERRNO errno
#define ERR_TIMEOUT EAGAIN
#endif

/**
 * @see https://habr.com/ru/post/340758/
 * @see https://github.com/Mityuha/grpc_async/blob/master/grpc_async_server.cc
 */
UDPListener::UDPListener(
    GatewaySerialization *aSerializationWrapper
)
    : GatewayListener(aSerializationWrapper), destAddr({}), status(CODE_OK), verbose(0), log(nullptr)
{
}

void UDPListener::setLog(
    int aVerbose,
    LogIntf *aLog
)
{
    verbose = aVerbose;
    log = aLog;
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
    if (log && verbose > 1) {
        log->strm(LOG_INFO) << "Run..";
        log->flush();
    }
    char rxBuf[256];

    int proto = isIPv6(&destAddr) ? IPPROTO_IPV6 : IPPROTO_IP;
    int af = isIPv6(&destAddr) ? AF_INET6 : AF_INET;

    int r = CODE_OK;
    while (status != ERR_CODE_STOPPED) {
        SOCKET sock = socket(af, SOCK_DGRAM, proto);
        if (sock == INVALID_SOCKET) {
            if (log) {
                log->strm(LOG_ERR) << "Unable to create socket, error " << SOCKET_ERRNO;
                log->flush();
            }
            r = ERR_CODE_SOCKET_CREATE;
            break;
        }
#ifdef _MSC_VER
        if (log && verbose > 1) {
            log->strm(LOG_INFO) << "Socket created ";
            log->flush();
        }
#endif
        int enable = 1;
        if (setsockopt(sock, IPPROTO_IP, IP_PKTINFO, (const char*) &enable, sizeof(enable))) {
            if (log) {
                log->strm(LOG_ERR) << "Socket unable to enable receive packet information, error " << SOCKET_ERRNO;
                log->flush();
            }
        }

        if (af == AF_INET6) {
            // Note that by default IPV6 binds to both protocols, it must be disabled
            // if both protocols used at the same time (used in CI)
            int opt = 1;
            setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*) &opt, sizeof(opt));
            setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, (const char*) &opt, sizeof(opt));
        }

        // Set timeout
#ifdef _MSC_VER
        DWORD timeout = 1000;   // ms
#else
        struct timeval timeout { 1, 0 };
#endif
        if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof timeout)) {
            if (log) {
                log->strm(LOG_ERR) << "Socket unable to set timeout, error " << SOCKET_ERRNO;
                log->flush();
            }
        }

        if (bind(sock, (struct sockaddr *) &destAddr, sizeof(destAddr)) < 0) {
            if (log) {
                log->strm(LOG_ERR) << "Socket unable to bind, error " << SOCKET_ERRNO;
                log->flush();
            }
        }
        if (log && verbose > 1) {
            log->strm(LOG_INFO) << "Socket bound ";
            log->flush();
        }

        struct sockaddr_storage source_addr{}; // Large enough for both IPv4 or IPv6
        socklen_t socklen = sizeof(source_addr);

        while (status != ERR_CODE_STOPPED) {
            int len = recvfrom(sock, rxBuf, sizeof(rxBuf) - 1, 0, (struct sockaddr *) &source_addr, &socklen);
            // Error occurred during receiving
            if (len < 0) {
                if (SOCKET_ERRNO == ERR_TIMEOUT) {    // timeout occurs
                    continue;
                }
                if (log) {
                    log->strm(LOG_ERR) << "Receive error " << SOCKET_ERRNO;
                    log->flush();
                }
                continue;
            } else {
                // Data received
                if (log && verbose > 1) {
                    log->strm(LOG_INFO) << "Received " << len << " bytes: " << hexString(rxBuf, len);
                    log->flush();
                }
                char *r;
                size_t sz = makeResponse(serializationWrapper, &r, rxBuf, len);
                if (r && (sz > 0)) {
                    if (sendto(sock, r, (int) sz, 0, (struct sockaddr *) &source_addr, sizeof(source_addr)) < 0) {
                        if (log) {
                            log->strm(LOG_ERR) << "Error occurred during sending " << SOCKET_ERRNO;
                            log->flush();
                        }
                    }
                    free(r);
                } else {
                    if (log && verbose) {
                        log->strm(LOG_ERR) << "Invalid request: " << hexString(rxBuf, len) << " (" << len << " bytes)";
                        log->flush();
                    }
                }
            }
        }
        if (log && verbose > 1) {
            log->strm(LOG_INFO) << "Shutting down socket and restarting..";
            log->flush();
        }
        shutdown(sock, 0);
        close(sock);
    }
    if (log && verbose > 1) {
        log->strm(LOG_INFO) << "Run.";
        log->flush();
    }
    return r;
}

UDPListener::~UDPListener()
{
	stop();
}
