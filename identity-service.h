#ifndef IDENTITY_SERVICE_H_
#define IDENTITY_SERVICE_H_ 1

#include <map>
#include <vector>
#include "lorawan-types.h"

/**
 * Identity service interface
 * Get device identifier and keys by the network address
 */ 
class IdentityService {
protected:
    // LoraWAN network identifier
    NETID netid;
public:
    IdentityService();
    IdentityService(const IdentityService &value);
    virtual ~IdentityService();
    /**
    * request device identifier(w/o address) by network address. Return 0 if success, retVal = EUI and keys
    * @param retVal device identifier
    * @param devAddr network address
    * @return LORA_OK- success
    */
    virtual int get(DEVICEID &retVal, DEVADDR &devAddr) = 0;

    /**
    * request network identity(with address) by network address. Return 0 if success, retval = EUI and keys
    * @param retval network identity(with address)
    * @param eui device EUI
    * @return LORA_OK- success
    */
    virtual int getNetworkIdentity(NETWORKIDENTITY &retval, const DEVEUI &eui) = 0;

    // Add or replace Address = EUI and keys pair
    virtual void put(DEVADDR &devaddr, DEVICEID &id) = 0;

    // Remove entry
    virtual void rm(DEVADDR &addr) = 0;

    /**
     * List entries
     * @param retval return values
     * @param offset 0..
     * @param size 0- all
     */

    virtual void list(
        std::vector<NETWORKIDENTITY> &retval,
        size_t offset,
        size_t size) = 0;

    // Entries count
    virtual size_t size() = 0;

    // force save
    virtual void flush() = 0;

    // reload
    virtual int init(const std::string &option, void *data) = 0;

    // close resources
    virtual void done() = 0;

    // parseRX list of identifiers, wildcards or regexes and copy found EUI into retval
    virtual int parseIdentifiers(
        std::vector<DEVEUI> &retval,
        const std::vector<std::string> &list,
        bool useRegex
    ) = 0;

    // parseRX list of names, wildcards or regexes and copy found EUI into retval
    virtual int parseNames(
        std::vector<DEVEUI> &retval,
        const std::vector<std::string> &list,
        bool useRegex
    ) = 0;

    virtual bool canControlService(
        const DEVADDR &addr
    ) = 0;

    int joinAccept(
        JOIN_ACCEPT_FRAME_HEADER &retval,
        NETWORKIDENTITY &NETWORKIDENTITY
    );

    NETID* getNetworkId();
    void setNetworkId(
        const NETID &value
    );

    /**
     * Return next network address if available
     * @return 0- success, ERR_ADDR_SPACE_FULL- no address available
     */
    virtual int next(NETWORKIDENTITY &retval) = 0;
};

#endif
