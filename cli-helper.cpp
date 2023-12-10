#include <sstream>
#include "cli-helper.h"

std::string listCommands() {
    std::stringstream ss;
    const std::string &cs = identityCommandSet();
    for (auto i = 0; i < cs.size(); i++) {
        auto c = cs[i];
        ss << "  " << c << "\t" << identityTag2string((enum IdentityQueryTag) c);
        if (c == QUERY_IDENTITY_ADDR)
            ss << " (default)";
        ss << "\n";
    }
    for (auto i = 0; i < cs.size(); i++) {
        auto c = gatewayCommandSet()[i];
        ss << "  " << c << "\t" << gatewayTag2string((enum GatewayQueryTag) c);
        ss << "\n";
    }
    return ss.str();
}

std::string shortCommandList(char delimiter) {
    std::stringstream ss;
    ss << "command: ";
    const std::string &cs = identityCommandSet();
    for (auto i = 0; i < cs.size(); i++) {
        ss << cs[i] << delimiter;
    }
    for (auto i = 0; i < cs.size(); i++) {
        ss << cs[i] << delimiter;
    }
    ss << cs[cs.size() - 1];
    ss << ", gateway id: 16 hex digits";
    return ss.str();
}

std::string commandLongName(int tag)
{
    std::string r = identityTag2string((enum IdentityQueryTag) tag);
    if (r.empty())
        r = gatewayTag2string((enum GatewayQueryTag) tag);
    return r;
}

GatewayQueryTag isGatewayTag(const char *tag) {
    if (!tag)
        return QUERY_GATEWAY_NONE;
    const std::string &cs = gatewayCommandSet();
    auto len = strlen(tag);
    if (len == 1) {
        if (cs.find(*tag) != std::string::npos) {
            return (GatewayQueryTag) *tag;
        }
    } else {
        for (auto it : cs) {
            if (strcmp(gatewayTag2string((GatewayQueryTag) it), tag) == 0)
                return (GatewayQueryTag) it;
        }
    }
    return QUERY_GATEWAY_NONE;
}

IdentityQueryTag isIdentityTag(const char *tag) {
    if (!tag)
        return QUERY_IDENTITY_NONE;
    const std::string &cs = identityCommandSet();
    auto len = strlen(tag);
    if (len == 1) {
        if (cs.find(*tag) != std::string::npos) {
            return (IdentityQueryTag) *tag;
        }
    } else {
        for (auto it : cs) {
            if (strcmp(identityTag2string((IdentityQueryTag) it), tag) == 0)
                return (IdentityQueryTag) it;
        }
    }
    return QUERY_IDENTITY_NONE;
}

/**
 * Merge address and identifiers
 * @param query Each item has address or identifier
 * @return true if success
 */
bool mergeIdAddress(
    std::vector<DeviceOrGatewayIdentity> &query
)
{
    auto pairSize = query.size() / 2;
    for (int i = 0; i < pairSize; i++) {
        if (query[i * 2].gid.gatewayId) {
            query[i].gid.gatewayId = query[i * 2].gid.gatewayId;
            memmove(&query[i].gid.sockaddr, &query[(i * 2) + 1].gid.sockaddr, sizeof(struct sockaddr));
        } else {
            memmove(&query[i].gid.sockaddr, &query[(i * 2)].gid.sockaddr, sizeof(struct sockaddr));
            query[i].gid.gatewayId = query[(i * 2) + 1].gid.gatewayId;
        }
    }
    query.resize(pairSize);
    return true;
}
