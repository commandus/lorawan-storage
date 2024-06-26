#include "ip-helper.h"

#include "lorawan/lorawan-conv.h"

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <WinSock2.h>
#include <WS2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

bool isAddrStringIPv6(
    const char * value
) {
    struct in6_addr result = {};
    return inet_pton(AF_INET6, value, &result) == 1;
}

bool isIPv6(
    const struct sockaddr *addr
) {
    return addr->sa_family == AF_INET6;
}

bool isIP(
    const struct sockaddr *addr
)
{
    return addr->sa_family == AF_INET || addr->sa_family == AF_INET6;
}

void sockaddrNtoh(
    struct sockaddr *addr
)
{
    switch (addr->sa_family)
    {
        case AF_INET6: {
            auto *a = (sockaddr_in6 *) addr;
            NTOH2(a->sin6_port);
        }
        break;
        case AF_INET: {
            auto *a = (sockaddr_in *) addr;
            NTOH2(a->sin_port);
        }
        default:
            break;
    }
}
