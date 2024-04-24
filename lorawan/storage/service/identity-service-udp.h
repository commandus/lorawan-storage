#ifndef IDENTITY_SERVICE_MEM_H_
#define IDENTITY_SERVICE_MEM_H_ 1

#include "identity-service.h"
#include "platform-specific.h"
#include "lorawan/storage/client/query-client.h"
#include "cli-helper.h"
#include "lorawan/lorawan-msg.h"

class ClientUDPIdentityService: public IdentityService {
public:
    std::string addr;
    uint16_t port;
    int32_t code;  // "account#" in request
    uint64_t accessCode;  // magic number in request, retCode in response, negative is error code
    QueryClient *client;
    int verbose;
    int32_t retCode;

    ClientUDPIdentityService();
    ~ClientUDPIdentityService() override;
    int get(DEVICEID &retVal, const DEVADDR &request) override;
    // List entries
    int list(std::vector<NETWORKIDENTITY> &retVal, size_t offset, size_t size) override;
    // Entries count
    size_t size() override;

    /**
    * request network identity(with address) by network address. Return 0 if success, retval = EUI and keys
    * @param retval network identity(with address)
    * @param eui device EUI
    * @return LORA_OK- success
    */
    int getNetworkIdentity(NETWORKIDENTITY &retVal, const DEVEUI &eui) override;

    int put(const DEVADDR &devAddr, const DEVICEID &id) override;
    int rm(const DEVADDR &devAddr) override;

    /**
     * Initialize connection to the service
     * @param addrPort e.g. 127.0.0.1:4244
     * @param db not used
     * @return CODE_OK if success
     */
    int init(const std::string &addrPort, void *db) override;
    void flush() override;
    void done() override;

    /**
     * Return next network address if available
     * @return 0- success, ERR_ADDR_SPACE_FULL- no address available
     */
    int next(NETWORKIDENTITY &retVal) override;
    void setOption(int option, void *value) override;

};

EXPORT_SHARED_C_FUNC IdentityService* makeClientUDPIdentityService();

#endif
