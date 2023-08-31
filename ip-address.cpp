#include "ip-address.h"

#include <sstream>

/**
 * @return address string
 */
std::string sockaddr2string(
    const struct sockaddr *value
) {
    char buf[INET6_ADDRSTRLEN];
    int port;
    switch (value->sa_family) {
        case AF_INET:
            if (inet_ntop(AF_INET, &((struct sockaddr_in *) value)->sin_addr, buf, sizeof(buf)) == nullptr)
                return "";
            port = ntohs(((struct sockaddr_in *) value)->sin_port);
            break;
        case AF_INET6:
            if (inet_ntop(AF_INET6, &((struct sockaddr_in6 *) value)->sin6_addr, buf, sizeof(buf)) == nullptr) {
                return "";
            }
            port = ntohs(((struct sockaddr_in6 *) value)->sin6_port);
            break;
        case AF_UNSPEC:
            return "embedded";
        default:
            return "";
    }
    std::stringstream ss;
    ss << buf << ":" << port;
    return ss.str();
}

/**
 * ADRESS:PORT
 */
static bool splitAddress(
    std::string &retAddress,
    int &retPort,
    const std::string &address
)
{
    size_t pos = address.find_last_of(':');
    if (pos == std::string::npos)
        return false;
    retAddress = address.substr(0, pos);
    std::string p(address.substr(pos + 1));
    retPort = atoi(p.c_str());
    return true;
}

/**
 * Trying parseRX I v6 address, then IPv4
 * @param retval return address into struct sockaddr_in6 struct pointer
 * @param value IPv8 or IPv4 address string
 * @return true if success
 */
bool string2sockaddr(
    struct sockaddr *retval,
    const std::string &value
)
{
    std::string address;
    int port;
    if (!splitAddress(address, port, value))
        return false;
    bool r = inet_pton(AF_INET6, address.c_str(), &((struct sockaddr_in6 *) retval)->sin6_addr) == 1;
    if (r) {
        ((struct sockaddr_in6*) retval)->sin6_family = AF_INET6;
        ((struct sockaddr_in6*) retval)->sin6_port = htons(port);
    } else {
        r = inet_pton(AF_INET, address.c_str(), &((struct sockaddr_in *) retval)->sin_addr) == 1;
        if (r) {
            ((struct sockaddr_in*) retval)->sin_family = AF_INET;
            ((struct sockaddr_in*) retval)->sin_port = htons(port);
        }
    }
    return r;
}
