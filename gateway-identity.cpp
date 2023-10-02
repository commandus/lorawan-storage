#include "gateway-identity.h"
#include <sstream>
#include <iomanip>
#include <cstring>

#include "lorawan-date.h"
#include "ip-address.h"

/**
 * Create empty gateway statistics
 */
GatewayIdentity::GatewayIdentity()
	: gatewayId(0)
{
    memset(&sockaddr, 0, sizeof(sockaddr));
}

/**
 * Copy gateway statistics
 */
GatewayIdentity::GatewayIdentity(
	const GatewayIdentity &value
)
    : gatewayId(value.gatewayId)
{
	memmove(&sockaddr, &value.sockaddr, sockaddr.sa_family ==  AF_INET6 ? sizeof(struct sockaddr_in6) : sizeof(struct sockaddr_in));
}

GatewayIdentity::GatewayIdentity(
    uint64_t aGatewayId
)
    : gatewayId(aGatewayId)
{
    memset(&sockaddr, 0, sizeof(struct sockaddr));
}

GatewayIdentity::GatewayIdentity(
    uint64_t aGatewayId,
    const std::string &addressNport
)
    : gatewayId(aGatewayId)
{
    string2sockaddr(&sockaddr, addressNport);
}

GatewayIdentity::GatewayIdentity(
        uint64_t aGatewayId,
        const std::string &address,
        uint16_t port
)
    : gatewayId(aGatewayId)
{
    string2sockaddr(&sockaddr, address, port);
}

/**
 * 8-bit gateway Identifier
 */
bool GatewayIdentity::operator==(
	GatewayIdentity &rhs
) const {
	return gatewayId == rhs.gatewayId;
}

/**
 * Statistics property names
 */
static const char* STAT_NAMES[13] = {
	"gwid",	// 0 string id
	"addr",	// 1 string address
	"name",	// 2 string name
	"time", // 3 string | UTC time of pkt RX, us precision, ISO 8601 'compact' format
	"lati", // 4 number 
	"long", // 5 number
	"alti", // 6 number
	"rxnb", // 7 number
	"rxok", // 8 number
	"rxfw", // 9 number
	"ackr", // 10 number
	"dwnb", // 11 number
	"txnb" // 12 number
};

/**
 * debug string
 */
std::string GatewayIdentity::toString() const
{
	std::stringstream ss;
	ss << std::hex << gatewayId << " " << sockaddr2string(&sockaddr);
	return ss.str();
}

/**
 * JSON string
 */
std::string GatewayIdentity::toJsonString() const
{
    std::stringstream ss;
    ss << "{"
       << R"("gwid":")" << std::hex << gatewayId
       << R"(", "addr":")" << sockaddr2string(&sockaddr)
       << "}";
    return ss.str();
}

/**
 * Create empty gateway statistics
 */
GatewayStatistic::GatewayStatistic()
    : GatewayIdentity(), errcode(0),
    t(0),					// UTC time of pkt RX, us precision, ISO 8601 'compact' format
    lat(0.0),				// latitude
    lon(0.0),				// longitude
    alt(0),					// altitude, meters, integer
    rxnb(0),				// Number of radio packets received (unsigned integer)
    rxok(0),				// Number of radio packets received with a valid PHY CRC
    rxfw(0),				// Number of radio packets forwarded (unsigned integer)
    ackr(0.0),				// Percentage of upstream datagrams that were acknowledged
    dwnb(0),				// Number of downlink datagrams received (unsigned integer)
    txnb(0)					// Number of packets emitted (unsigned integer)
{
}

/**
 * Copy gateway statistics
 */
GatewayStatistic::GatewayStatistic(
    const GatewayStatistic &value
)
 : GatewayIdentity(value) {
    name = value.name;
    gatewayId = value.gatewayId;
    errcode = value.errcode;
    t = value.t;
    lat = value.lat;
    lon = value.lon;
    alt = value.alt;
    rxnb = value.rxnb;
    rxok = value.rxok;
    rxfw = value.rxfw;
    ackr = value.ackr;
    dwnb = value.dwnb;
    txnb = value.txnb;
}

/**
 * 8-bit gateway Identifier
 */
bool GatewayStatistic::operator==(
        GatewayStatistic &rhs
) const {
    return gatewayId == rhs.gatewayId;
}

/**
 * debug string
 */
std::string GatewayStatistic::toString() const
{
    std::stringstream ss;
    ss << "{"
        << R"("gwid":")" << std::hex << gatewayId << std::dec
        << R"(", "addr":")" << sockaddr2string(&sockaddr)
        << R"(", "name":")" << name
        << R"(", "time":")" << time2string(t)
        << R"(", "lati":)" << std::fixed << std::setprecision(5) << lat
        << ", \"long\":" << std::fixed << std::setprecision(5) << lon
        << ", \"alti\":" << alt
        << ", \"rxnb\":" << rxnb
        << ", \"rxok\":" << rxok
        << ", \"rxfw\":" << rxfw
        << ", \"ackr\":" << std::fixed << std::setprecision(1) << ackr
        << ", \"dwnb\":" << dwnb
        << ", \"txnb\":" << txnb
        << "}";
    return ss.str();
}
