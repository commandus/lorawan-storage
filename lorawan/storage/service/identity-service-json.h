#ifndef IDENTITY_SERVICE_JSON_H_
#define IDENTITY_SERVICE_JSON_H_ 1

#include "third-party/nlohmann/json.hpp"
#include "identity-service-mem.h"
#include "platform-specific.h"

class JsonIdentityService: public MemoryIdentityService {
private:
    bool load();
    bool store();
protected:
    std::string fileName;
public:
    JsonIdentityService();
    ~JsonIdentityService() override;
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
    int rm(const DEVADDR &addr) override;

    int init(const std::string &dbName, void *db) override;
    void flush() override;
    void done() override;

    /**
     * Return next network address if available
     * @return 0- success, ERR_ADDR_SPACE_FULL- no address available
     */
    int next(NETWORKIDENTITY &retVal) override;
    void setOption(int option, void *value) override;
};

EXPORT_SHARED_C_FUNC IdentityService* makeJsonIdentityService();

#endif
