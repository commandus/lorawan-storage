#ifndef IDENTITY_SERVICE_H_
#define IDENTITY_SERVICE_H_ 1

#include <map>
#include <vector>

#include "lorawan/lorawan-types.h"
#include "lorawan/storage/client/response-client.h"

/**
 * Identity service interface
 * Get device identifier and keys by the network address
 */ 
class IdentityService {
protected:
    // LoraWAN network identifier
    NETID netid;
    const ResponseClient *responseClient;
public:
    IdentityService();
    IdentityService(const ResponseClient *responseClient);
    IdentityService(const IdentityService &value);
    virtual ~IdentityService();

    /**
    * synchronous request device identifier(w/o address) by network address. Return 0 if success, retVal = EUI and keys
    * @param retVal device identifier
    * @param devAddr network address
    * @return LORA_OK- success
    */
    virtual int get(DEVICEID &retVal, const DEVADDR &devAddr) = 0;

    /**
    * synchronous request network identity(with address) by network address. Return 0 if success, retval = EUI and keys
    * @param retval network identity(with address)
    * @param eui device EUI
    * @return CODE_OK- success
    */
    virtual int getNetworkIdentity(NETWORKIDENTITY &retval, const DEVEUI &eui) = 0;

    /**
     * synchronous add or replace Address = EUI and keys pair
     * @param devaddr
     * @param id
     * @return
     */
    virtual int put(const DEVADDR &devaddr, const DEVICEID &id) = 0;

    /**
     * synchronous remove entry
     * @param addr
     * @return
     */
    virtual int rm(const DEVADDR &addr) = 0;

    /**
     * synchronous list entries
     * @param retVal return values
     * @param offset 0..
     * @param size 0- all
     */
    virtual int list(
        std::vector<NETWORKIDENTITY> &retVal,
        size_t offset,
        size_t size
    ) = 0;

    /**
     * synchronous entries count
     * @return
     */
    virtual size_t size() = 0;

    /**
     * synchronous force save
     */
    virtual void flush() = 0;

    /**
     * Initialize
     * @param option
     * @param data
     * @return
     */
    virtual int init(const std::string &option, void *data) = 0;

    /**
     * Finalize. Close resources
     */
    virtual void done() = 0;

    /**
     * Set options
     * @param option 0- masterkey 1- code 2- accesscode
     * @param value 0- string 1- int32_t 2- uint64_t
     */
    virtual void setOption(int option, void *value) = 0;

    virtual NETID *getNetworkId();

    virtual void setNetworkId(
        const NETID &value
    );

    /**
     * synchronous call.
     * Return next network address if available
     * @return 0- success, ERR_ADDR_SPACE_FULL- no address available
     */
    virtual int next(
        NETWORKIDENTITY &retVal
    ) = 0;

    int joinAccept(
        JOIN_ACCEPT_FRAME_HEADER &retVal,
        NETWORKIDENTITY &networkIdentity
    );
};

#endif
