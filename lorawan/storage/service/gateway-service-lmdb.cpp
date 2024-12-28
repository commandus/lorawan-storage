#include <cstring>

#include "gateway-service-lmdb.h"
#include "lorawan/helper/ip-address.h"
#include "lorawan/lorawan-types.h"
#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-string.h"

#ifdef ESP_PLATFORM
#include <iostream>
#include "platform-defs.h"
#endif

LMDBGatewayService::LMDBGatewayService()
= default;

LMDBGatewayService::~LMDBGatewayService() = default;

void LMDBGatewayService::clear()
{
    storage.clear();
}

/**
 * request device identifier by network address. Return 0 if success, retval = EUI and keys
 * @param retval device identifier
 * @param devaddr network address
 * @return LORA_OK- success
 */
int LMDBGatewayService::get(
    GatewayIdentity &retVal,
    const GatewayIdentity &request
)
{
    if (request.gatewayId) {
        // find out by gateway identifier
        auto r = storage.find(request.gatewayId);
        if (r != storage.end()) {
            retVal = r->second;
            return CODE_OK;
        } else {
            memset(&retVal.sockaddr, 0, sizeof(retVal.sockaddr));
            return ERR_CODE_GATEWAY_NOT_FOUND;
        }
    } else {
        // reverse find out by address
        for (auto & it : storage) {
            if (sameSocketAddress(&request.sockaddr, &it.second.sockaddr)) {
                retVal = it.second;
                return CODE_OK;
            }
        }
        return ERR_CODE_GATEWAY_NOT_FOUND;
    }
}

// List entries
int  LMDBGatewayService::list(
    std::vector<GatewayIdentity> &retVal,
    uint32_t offset,
    uint8_t size
)
{
    size_t o = 0;
    size_t sz = 0;
    for (auto & it : storage) {
        if (o < offset) {
            // skip first
            o++;
            continue;
        }
        sz++;
        if (sz > size)
            break;
        retVal.push_back(it.second);
    }
    return CODE_OK;
}

// Entries count
size_t LMDBGatewayService::size()
{
    return storage.size();
}

int LMDBGatewayService::put(
    const GatewayIdentity &request
)
{
    storage[request.gatewayId] = request;
    return CODE_OK;
}

int LMDBGatewayService::rm(
    const GatewayIdentity &request
)
{
    if (request.gatewayId) {
        // find out by gateway identifier
        auto r = storage.find(request.gatewayId);
        if (r != storage.end()) {
            storage.erase(r);
            return CODE_OK;
        }
    } else {
        // reverse find out by address
        for (auto it(storage.begin()); it != storage.end(); it++) {
            if (sameSocketAddress(&request.sockaddr, &it->second.sockaddr)) {
                storage.erase(it);
                return CODE_OK;
            }
        }
    }
    return ERR_CODE_GATEWAY_NOT_FOUND;
}

int LMDBGatewayService::init(
    const std::string &option,
    void *data
)
{
    return CODE_OK;
}

void LMDBGatewayService::flush()
{
}

void LMDBGatewayService::done()
{
    clear();
}

void LMDBGatewayService::setOption(
    int option,
    void *value
)

{
    // nothing to do
}

EXPORT_SHARED_C_FUNC GatewayService* makeGatewayService5()
{
    return new LMDBGatewayService;
}
