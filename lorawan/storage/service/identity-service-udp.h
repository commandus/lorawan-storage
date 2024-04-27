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

    // synchronous wrappers
    int get(DEVICEID &retVal, const DEVADDR &request) override;
    int getNetworkIdentity(NETWORKIDENTITY &retVal, const DEVEUI &eui) override;
    int put(const DEVADDR &devAddr, const DEVICEID &id) override;
    int rm(const DEVADDR &devAddr) override;
    int list(std::vector<NETWORKIDENTITY> &retVal, size_t offset, size_t size) override;
    size_t size() override;
    int next(NETWORKIDENTITY &retVal) override;

    // asynchronous calls
    int cGet(const DEVADDR &request) override;
    int cGetNetworkIdentity(const DEVEUI &eui) override;
    int cPut(const DEVADDR &devAddr, const DEVICEID &id) override;
    int cRm(const DEVADDR &devAddr) override;
    int cList(size_t offset, size_t size) override;
    int cSize() override;
    int cNext() override;

    int init(const std::string &addrPort, void *db) override;
    void flush() override;
    void done() override;

    void setOption(int option, void *value) override;
};

EXPORT_SHARED_C_FUNC IdentityService* makeClientUDPIdentityService();

#endif
