#include <sstream>
#include <iostream>
#include "lorawan/storage/service/identity-service-lmdb.h"
#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/helper/file-helper.h"
#include "lorawan/storage/serialization/identity-binary-serialization.h"

#ifdef ESP_PLATFORM
#include <iostream>
#include "platform-defs.h"
#endif

LMDBIdentityService::LMDBIdentityService() = default;

LMDBIdentityService::~LMDBIdentityService() = default;

/**
 * request device identifier by network address. Return 0 if success, retval = EUI and keys
 * @param retval device identifier
 * @param devaddr network address
 * @return CODE_OK- success
 */
int LMDBIdentityService::get(
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
int LMDBIdentityService::list(
    std::vector<NETWORKIDENTITY> &retVal,
    uint32_t offset,
    uint8_t size
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
        retVal.emplace_back(it.first, it.second);
    }
    return CODE_OK;
}

// Entries count
size_t LMDBIdentityService::size()
{
    return storage.size();
}

/**
* request network identity(with address) by network address. Return 0 if success, retval = EUI and keys
* @param retval network identity(with address)
* @param eui device EUI
* @return CODE_OK- success
*/
int LMDBIdentityService::getNetworkIdentity(
    NETWORKIDENTITY &retVal,
    const DEVEUI &eui
)
{
    for(auto & it : storage) {
        if (it.second.devEUI.u == eui.u) {
            retVal.devaddr = it.first;
            retVal.devid = it.second;
            return CODE_OK;
        }
    }
    return ERR_CODE_GATEWAY_NOT_FOUND;
}

/**
 * UPSERT SQLite >= 3.24.0
 * @param request gateway identifier or address
 * @return 0- success
 */
int LMDBIdentityService::put(
    const DEVADDR &devAddr,
    const DEVICEID &id
)
{
    storage[devAddr] = id;
    return CODE_OK;
}

int LMDBIdentityService::rm(
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

int LMDBIdentityService::init(
    const std::string &databaseName,
    void *database
)
{
    return CODE_OK;
}

void LMDBIdentityService::flush()
{
}

void LMDBIdentityService::done()
{
    storage.clear();
}

/**
 * Return next network address if available
 * @return 0- success, ERR_CODE_ADDR_SPACE_FULL- no address available
 */
int LMDBIdentityService::next(
    NETWORKIDENTITY &retval
)
{
    return ERR_CODE_ADDR_SPACE_FULL;
}

void LMDBIdentityService::setOption(
    int option,
    void *value
)

{
    // nothing to do
}

EXPORT_SHARED_C_FUNC IdentityService* makeLMDBIdentityService()
{
    return new LMDBIdentityService;
}

// ------------------- asynchronous imitation -------------------
int LMDBIdentityService::cGet(const DEVADDR &request)
{
    IdentityGetResponse r;
    r.response.devaddr = request;
    get(r.response.devid, request);
    if (responseClient)
        responseClient->onIdentityGet(nullptr, &r);
    return CODE_OK;
}

int LMDBIdentityService::cGetNetworkIdentity(const DEVEUI &eui)
{
    IdentityGetResponse r;
    getNetworkIdentity(r.response, eui);
    if (responseClient)
        responseClient->onIdentityGet(nullptr, &r);
    return CODE_OK;
}

int LMDBIdentityService::cPut(const DEVADDR &devAddr, const DEVICEID &id)
{
    IdentityOperationResponse r;
    r.response = put(devAddr, id);
    if (responseClient)
        responseClient->onIdentityOperation(nullptr, &r);
    return CODE_OK;
}

int LMDBIdentityService::cRm(const DEVADDR &devAddr)
{
    IdentityOperationResponse r;
    r.response = rm(devAddr);
    if (responseClient)
        responseClient->onIdentityOperation(nullptr, &r);
    return CODE_OK;
}

int LMDBIdentityService::cList(
    uint32_t offset,
    uint8_t size
)
{
    IdentityListResponse r;
    r.response = list(r.identities, offset, size);
    r.size = (uint8_t) r.identities.size();
    if (responseClient)
        responseClient->onIdentityList(nullptr, &r);
   return CODE_OK;
}

int LMDBIdentityService::filter(
    std::vector<NETWORKIDENTITY> &retVal,
    const std::vector<NETWORK_IDENTITY_FILTER> &filters,
    uint32_t offset,
    uint8_t size
)
{
    size_t o = 0;
    size_t sz = 0;
    for (auto & it : storage) {
        if (!isIdentityFilteredV2(it.first, it.second, filters))
            continue;
        if (o < offset) {
            // skip first
            o++;
            continue;
        }
        sz++;
        if (sz > size)
            break;
        retVal.emplace_back(it.first, it.second);
    }
    return CODE_OK;
}

int LMDBIdentityService::cFilter(
    const std::vector<NETWORK_IDENTITY_FILTER> &filters,
    uint32_t offset,
    uint8_t size
)
{
    IdentityListResponse r;
    r.response = filter(r.identities, filters, offset, size);
    r.size = (uint8_t) r.identities.size();
    if (responseClient)
        responseClient->onIdentityList(nullptr, &r);
    return CODE_OK;
}

int LMDBIdentityService::cSize()
{
    IdentityOperationResponse r;
    r.size = (uint8_t) size();
    if (responseClient)
        responseClient->onIdentityOperation(nullptr, &r);
    return CODE_OK;
}

int LMDBIdentityService::cNext()
{
    IdentityGetResponse r;
    next(r.response);
    if (responseClient)
        responseClient->onIdentityGet(nullptr, &r);
    return CODE_OK;
}

EXPORT_SHARED_C_FUNC IdentityService* makeIdentityService5()
{
    return new LMDBIdentityService;
}
