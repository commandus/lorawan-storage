#ifndef IP_ADDRESS_H
#define IP_ADDRESS_H

#include <string>
#ifdef _MSC_VER
#include <WinSock2.h>
#include <ws2tcpip.h>
#else
#define SOCKET int
#include <netinet/in.h>
#endif

/**
 * Split @param address e.g. ADRESS:PORT to @param retAddress and @param retPort
 */
bool splitAddress(
    std::string &retAddress,
    uint16_t &retPort,
    const std::string &address
);

/**
 * Return IP adress:port
 * @return address string
 */
std::string sockaddr2string(
    const struct sockaddr *value
);

/**
 * Trying parseRX I v6 address, then IPv4
 * @param retval return address into struct sockaddr_in6 struct pointer
 * @param value IPv8 or IPv4 address string
 * @return true if success
 */
bool string2sockaddr(
    struct sockaddr *retval,
    const std::string &value
);

#endif
