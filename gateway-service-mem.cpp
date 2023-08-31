#include "gateway-service-mem.h"
#include "lorawan-types.h"
#include "lorawan-error.h"
#include "lorawan-string.h"

#ifdef ESP_PLATFORM
#include <iostream>
#include "platform-defs.h"
#endif

MemoryGatewayService::MemoryGatewayService()
{

}

MemoryGatewayService::~MemoryGatewayService() = default;

void MemoryGatewayService::clear()
{
    storage.clear();
}

/**
 * request device identifier by network address. Return 0 if success, retval = EUI and keys
 * @param retval device identifier
 * @param devaddr network address
 * @return LORA_OK- success
 */
int MemoryGatewayService::get(
    GatewayIdentity &retVal,
    const GatewayIdentity &request
)
{
#ifdef ENABLE_DEBUG
    std::cerr << "get " << std::endl;
#endif
    auto r = storage.find(request.gatewayId);
    if (r != storage.end()) {
        retVal = r->second;
        return CODE_OK;
    }
    return ERR_CODE_GATEWAY_NOT_FOUND;
}


// List entries
void MemoryGatewayService::list(
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
size_t MemoryGatewayService::size()
{
    return storage.size();
}

void MemoryGatewayService::put(
    const GatewayIdentity &request
)
{
    storage[request.gatewayId] = request;
}

void MemoryGatewayService::rm(
    const GatewayIdentity &request
)
{
    auto r = storage.find(request.gatewayId);
    if (r != storage.end()) {
        storage.erase(r);
    }
}

int MemoryGatewayService::init(
        const std::string &option,
        void *data
)
{
    return CODE_OK;
}

void MemoryGatewayService::flush()
{
}

void MemoryGatewayService::done()
{
    clear();
}
