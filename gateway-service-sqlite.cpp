#include "gateway-service-sqlite.h"
#include "lorawan-types.h"
#include "lorawan-error.h"
#include "lorawan-string.h"

#ifdef ESP_PLATFORM
#include <iostream>
#include "platform-defs.h"
#endif

SqliteGatewayService::SqliteGatewayService()
{

}

SqliteGatewayService::~SqliteGatewayService() = default;

void SqliteGatewayService::clear()
{
    storage.clear();
}

/**
 * request device identifier by network address. Return 0 if success, retval = EUI and keys
 * @param retval device identifier
 * @param devaddr network address
 * @return LORA_OK- success
 */
int SqliteGatewayService::get(
        GatewayIdentity &retVal,
        const GatewayIdentity &request
)
{
    auto r = storage.find(request.gatewayId);
    if (r != storage.end()) {
        retVal = r->second;
        return CODE_OK;
    }
    return ERR_CODE_GATEWAY_NOT_FOUND;
}


// List entries
void SqliteGatewayService::list(
        std::vector<GatewayIdentity> &retVal,
        size_t offset,
        size_t size
) {
    size_t c = 0;
    for (auto it(storage.begin()); it != storage.end(); it++) {
        if (c < offset) {
            // skip first
            c++;
            continue;
        }
        c++;
        if (c > size)
            break;
        retVal.push_back(it->second);
    }
}

// Entries count
size_t SqliteGatewayService::size()
{
    return storage.size();
}

void SqliteGatewayService::put(
        const GatewayIdentity &request
)
{
    storage[request.gatewayId] = request;
}

void SqliteGatewayService::rm(
        const GatewayIdentity &request
)
{
    auto r = storage.find(request.gatewayId);
    if (r != storage.end()) {
        storage.erase(r);
    }
}

int SqliteGatewayService::init(
        const std::string &option,
        void *data
)
{
    return CODE_OK;
}

void SqliteGatewayService::flush()
{
}

void SqliteGatewayService::done()
{
    clear();
}
