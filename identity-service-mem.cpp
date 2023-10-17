#include <sstream>
#include <iostream>
#include "identity-service-mem.h"
#include "lorawan-error.h"
#include "lorawan-string.h"
#include "file-helper.h"

#ifdef ESP_PLATFORM
#include <iostream>
#include "platform-defs.h"
#endif

MemoryIdentityService::MemoryIdentityService() = default;

MemoryIdentityService::~MemoryIdentityService() = default;

/**
 * request device identifier by network address. Return 0 if success, retval = EUI and keys
 * @param retval device identifier
 * @param devaddr network address
 * @return CODE_OK- success
 */
int MemoryIdentityService::get(
    DEVICEID &retVal,
    const DEVADDR &request
)
{
    auto r = storage.find(request);
    if (r != storage.end())
        retVal = r->second;
    else
        return ERR_CODE_GATEWAY_NOT_FOUND;
    return CODE_OK;
}

// List entries
int MemoryIdentityService::list(
    std::vector<NETWORKIDENTITY> &retVal,
    size_t offset,
    size_t size
) {
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
        retVal.push_back(NETWORKIDENTITY(it.first, it.second));
    }
    return CODE_OK;
}

// Entries count
size_t MemoryIdentityService::size()
{
    return storage.size();
}

/**
* request network identity(with address) by network address. Return 0 if success, retval = EUI and keys
* @param retval network identity(with address)
* @param eui device EUI
* @return CODE_OK- success
*/
int MemoryIdentityService::getNetworkIdentity(
    NETWORKIDENTITY &retval,
    const DEVEUI &eui
)
{
    return CODE_OK;
}

/**
 * UPSERT SQLite >= 3.24.0
 * @param request gateway identifier or address
 * @return 0- success
 */
int MemoryIdentityService::put(
    const DEVADDR &devAddr,
    const DEVICEID &id
)
{
    storage[devAddr] = id;
    return CODE_OK;
}

int MemoryIdentityService::rm(
    const DEVADDR &addr
)
{
    // find out by gateway identifier
    auto r = storage.find(addr);
    if (r != storage.end()) {
        storage.erase(r);
        return CODE_OK;
    }
    return ERR_CODE_DEVICE_ADDRESS_NOTFOUND;
}

int MemoryIdentityService::init(
    const std::string &databaseName,
    void *database
)
{
    return CODE_OK;
}

void MemoryIdentityService::flush()
{
}

void MemoryIdentityService::done()
{
    storage.clear();
}

/**
 * Return next network address if available
 * @return 0- success, ERR_CODE_ADDR_SPACE_FULL- no address available
 */
int MemoryIdentityService::next(
    NETWORKIDENTITY &retval
)
{
    return ERR_CODE_ADDR_SPACE_FULL;
}
