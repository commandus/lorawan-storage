#include <regex>

#include "identity-service-gen.h"
#include "lorawan/lorawan-types.h"
#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-string.h"
#include "key128gen.h"

#ifdef ESP_PLATFORM
#include <iostream>
#include "platform-defs.h"
#endif

#ifdef ENABLE_DEBUG
#include <iostream>
#endif

#define DEFAULT_NETID   0

GenIdentityService::GenIdentityService()
    : maxDevNwkAddr(0), errCode(0)
{

}

GenIdentityService::GenIdentityService(
    const std::string &masterKey
)
    : maxDevNwkAddr(0), errCode(0)
{
    setMasterKey(masterKey);
}

void GenIdentityService::setMasterKey(
    const std::string &masterKey
)
{
    phrase2key((uint8_t *) &key.c, masterKey.c_str(), masterKey.size());
}

GenIdentityService::~GenIdentityService() = default;

void GenIdentityService::clear()
{
    maxDevNwkAddr = 0;
}

/**
 * request device identifier by network address. Return 0 if success, retval = EUI and keys
 * @param retval device identifier
 * @param devaddr network address
 * @return LORA_OK- success
 */
int GenIdentityService::get(
    DEVICEID &retval,
    const DEVADDR &devaddr
)
{
    retval.activation = ABP;	///< activation type: ABP or OTAA
    retval.deviceclass = CLASS_A;
    euiGen((uint8_t *) &retval.devEUI.c, KEY_NUMBER_EUI, (uint8_t *) &key.c, devaddr.u);
    keyGen((uint8_t *) &retval.nwkSKey.c, KEY_NUMBER_NWK, (uint8_t *) &key.c, devaddr.u);
    keyGen((uint8_t *) &retval.appSKey.c, KEY_NUMBER_APP, (uint8_t *) &key.c, devaddr.u);
    retval.version = { 1, 0, 0 };
    string2DEVICENAME(retval.name, DEVADDR2string(devaddr).c_str());
#ifdef ENABLE_DEBUG
        std::cerr << "get " << DEVADDR2string(devaddr)
            << std::endl;
#endif
    return 0;
}

/**
* request network identity(with address) by device EUI. Return 0 if success, retval = EUI and keys
* @param retval network identity(with address)
* @param eui device EUI
* @return LORA_OK- success
*/
int GenIdentityService::getNetworkIdentity(
    NETWORKIDENTITY &retval,
    const DEVEUI &eui
) {
    return ERR_CODE_DEVICE_EUI_NOT_FOUND;
}

// List entries
int GenIdentityService::list(
    std::vector<NETWORKIDENTITY> &retVal,
    size_t offset,
    size_t size
) {
    uint32_t a = offset;
    size_t sz = netid.size();
    for (int i = 0; i < size; i++) {
        if (a > sz)
            break;
        NETWORKIDENTITY v;
        v.devaddr = DEVADDR(netid, a);
        euiGen((uint8_t *) &v.devid.devEUI.c, KEY_NUMBER_EUI, (uint8_t *) &key.c, v.devaddr.u);
        keyGen((uint8_t *) &v.devid.nwkSKey.c, KEY_NUMBER_NWK, (uint8_t *) &key.c, v.devaddr.u);
        keyGen((uint8_t *) &v.devid.appSKey.c, KEY_NUMBER_APP, (uint8_t *) &key.c, v.devaddr.u);
        v.devid.version = { 1, 0, 0 };
        string2DEVICENAME(v.devid.name, DEVADDR2string(v.devaddr).c_str());
        retVal.push_back(v);
        a++;
    }
    return CODE_OK;
}

// Entries count
size_t GenIdentityService::size()
{
    return netid.size();
}

int GenIdentityService::put(
    const DEVADDR &devaddr,
    const DEVICEID &id
)
{
    return CODE_OK;
}

int GenIdentityService::rm(
    const DEVADDR &addr
)
{
    return CODE_OK;
}

int GenIdentityService::init(
    const std::string &option,
    void *data
)
{
    setMasterKey(option);
    if (data)
        netid.set(*(NETID*)data);
    else
        netid.set(DEFAULT_NETID);   // set default network id
    return CODE_OK;
}

void GenIdentityService::flush()
{
}

void GenIdentityService::done()
{

}

/**
  * Return next network address if available
  * @return 0- success, ERR_ADDR_SPACE_FULL- no address available
  */
int GenIdentityService::next(
    NETWORKIDENTITY &retval
)
{
    return ERR_CODE_ADDR_SPACE_FULL;
}

/**
  * Return next available network address
  * @return 0- success, ERR_ADDR_SPACE_FULL- no address available
  */
int GenIdentityService::nextBruteForce(
    NETWORKIDENTITY &retVal
)
{
    return ERR_CODE_ADDR_SPACE_FULL;
}

extern "C" IdentityService* makeGenIdentityService()
{
    return new GenIdentityService;
}
