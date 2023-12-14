#ifndef IDENTITY_SERVICE_SQLITE_H_
#define IDENTITY_SERVICE_SQLITE_H_ 1

#include <sqlite3.h>
#include "platform-specific.h"
#include "identity-service.h"

class SqliteIdentityService: public IdentityService {
protected:
    std::string dbName;
    sqlite3 *db;
public:
    SqliteIdentityService();
    ~SqliteIdentityService() override;
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
};

EXPORT_SHARED_C_FUNC IdentityService* makeSqliteIdentityService();

#endif
