/*
 * @file cli-helper.h
 */
#ifndef CLI_HELPER_H
#define CLI_HELPER_H     1

#include <string>
#include <vector>

#include "lorawan/lorawan-types.h"
#include "lorawan/storage/serialization/identity-serialization.h"
#include "lorawan/storage/serialization/gateway-serialization.h"

class DeviceOrGatewayIdentity {
public:
    bool hasDevice;
    bool hasGateway;
    GatewayIdentity gid;
    NETWORKIDENTITY nid;
    DeviceOrGatewayIdentity()
        : hasDevice(false), hasGateway(false)
    {};
};

std::string listCommands();

std::string listPlugins();

std::string shortCommandList(char delimiter);

std::string commandLongName(int tag);

GatewayQueryTag isGatewayTag(const char *tag);

IdentityQueryTag isIdentityTag(const char *tag);

/**
 * Merge address and identifiers
 * @param query Each item has address or identifier
 * @return true if success
 */
bool mergeIdAddress(
    std::vector<DeviceOrGatewayIdentity> &query
);

#endif
