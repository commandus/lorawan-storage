#include <sstream>
#include <iostream>
#include "identity-service-sqlite.h"
#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-string.h"
#include "file-helper.h"
#include "sqlite-helper.h"

#ifdef ESP_PLATFORM
#include <iostream>
#include "platform-defs.h"
#endif

#include "platform-specific.h"

#define FIELD_LIST "activation, deviceclass, deveui, nwkskey, appskey, version, appeui, appkey, nwkkey, devnonce, joinnonce, name, addr"

SqliteIdentityService::SqliteIdentityService()
    : db(nullptr)
{

}

SqliteIdentityService::~SqliteIdentityService() = default;

static void row2DEVICEID(
    DEVICEID &retVal,
    const std::vector<std::string> &row
) {
    retVal.activation = string2activation(row[0]);
    retVal.deviceclass = string2deviceclass(row[1]);
    string2DEVEUI(retVal.devEUI, row[2]);
    string2KEY(retVal.nwkSKey, row[3]);
    string2KEY(retVal.appSKey, row[4]);
    retVal.version = string2LORAWAN_VERSION(row[5]);
    string2DEVEUI(retVal.appEUI, row[6]);
    string2KEY(retVal.appKey, row[7]);
    string2KEY(retVal.nwkKey, row[8]);
    retVal.devNonce = string2DEVNONCE(row[9]);
    string2JOINNONCE(retVal.joinNonce, row[10]);
    string2DEVICENAME(retVal.name, row[11].c_str());
}

/**
 * request device identifier by network address. Return 0 if success, retval = EUI and keys
 * @param retval device identifier
 * @param devaddr network address
 * @return CODE_OK- success
 */
int SqliteIdentityService::get(
    DEVICEID &retVal,
    const DEVADDR &request
)
{
    if (!db)
        return ERR_CODE_DB_DATABASE_NOT_FOUND;
    char *zErrMsg = nullptr;
    std::stringstream statement;
    statement << "SELECT " FIELD_LIST " FROM device WHERE addr = '"
        << DEVADDR2string(request)
        << "'";
    std::vector<std::string> row;
    int r = sqlite3_exec(db, statement.str().c_str(), rowCallback, &row, &zErrMsg);
    if (r != SQLITE_OK) {
        if (zErrMsg) {
            sqlite3_free(zErrMsg);
        }
        return ERR_CODE_DB_SELECT;
    } else {
        if (row.size() < 2)
            return ERR_CODE_BEST_GATEWAY_NOT_FOUND;
    }
    row2DEVICEID(retVal, row);
    return CODE_OK;
}

// List entries
int SqliteIdentityService::list(
    std::vector<NETWORKIDENTITY> &retVal,
    size_t offset,
    size_t size
) {
    if (!db)
        return ERR_CODE_DB_DATABASE_NOT_FOUND;
    char *zErrMsg = nullptr;
    std::stringstream statement;
    statement << "SELECT " FIELD_LIST " FROM device LIMIT " << size << " OFFSET " << offset;
    std::vector<std::vector<std::string>> table;
    int r = sqlite3_exec(db, statement.str().c_str(), tableCallback, &table, &zErrMsg);
    if (r != SQLITE_OK) {
        if (zErrMsg) {
            sqlite3_free(zErrMsg);
        }
        return ERR_CODE_DB_SELECT;
    }
    for (auto row : table) {
        if (row.size() < 2)
            continue;
        NETWORKIDENTITY ni;
        row2DEVICEID(ni.devid, row);
        ni.devaddr = row[12];
        retVal.push_back(ni);
    }
    return CODE_OK;
}

// Entries count
size_t SqliteIdentityService::size()
{
    if (!db)
        return 0;
    char *zErrMsg = nullptr;
    std::string statement = "SELECT count(id) FROM gateway";
    std::vector<std::string> row;
    int r = sqlite3_exec(db, statement.c_str(), rowCallback, &row, &zErrMsg);
    if (r != SQLITE_OK) {
        if (zErrMsg) {
            sqlite3_free(zErrMsg);
        }
        return 0;
    } else {
        if (row.empty())
            return 0;
    }
    return strtoull(row[0].c_str(), nullptr, 10);
}

/**
* request network identity(with address) by network address. Return 0 if success, retval = EUI and keys
* @param retval network identity(with address)
* @param eui device EUI
* @return CODE_OK- success
*/
int SqliteIdentityService::getNetworkIdentity(
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
int SqliteIdentityService::put(
    const DEVADDR &devAddr,
    const DEVICEID &id
)
{
    if (!db)
        return ERR_CODE_DB_DATABASE_NOT_FOUND;
    char *zErrMsg = nullptr;
    std::stringstream statement;

    statement << "INSERT INTO device(" FIELD_LIST ") VALUES ('"
              << activation2string(id.activation) << "', "
              << "'" << deviceclass2string(id.deviceclass) << "', "
              << "'" << DEVEUI2string(id.devEUI) << "', "
              << "'" << KEY2string(id.nwkSKey) << "', "
              << "'" << KEY2string(id.appSKey) << "', "
              << "'" << LORAWAN_VERSION2string(id.version) << "', "
              << "'" << DEVEUI2string(id.appEUI) << "', "
              << "'" << KEY2string(id.appKey) << "', "
              << "'" << KEY2string(id.nwkKey) << "', "
              << "'" << DEVNONCE2string(id.devNonce) << "', "
              << "'" << JOINNONCE2string(id.joinNonce) << "', "
              << "'" << DEVICENAME2string(id.name) << "', "
              << "'" << DEVADDR2string(devAddr)
              << "') ON CONFLICT(addr) DO UPDATE SET "
                 "activation=excluded.activation, deviceclass=excluded.deviceclass, deveui=excluded.deveui, "
                 "nwkskey=excluded.nwkskey, appskey=excluded.appskey, version=excluded.version, "
                 "appeui=excluded.appeui, appkey=excluded.appkey, nwkkey=excluded.nwkkey, "
                 "devnonce=excluded.devnonce, joinnonce=excluded.joinnonce, name=excluded.name";
    int r = sqlite3_exec(db, statement.str().c_str(), nullptr, nullptr, &zErrMsg);
    if (r != SQLITE_OK) {
        if (zErrMsg) {
            sqlite3_free(zErrMsg);
        }
        return ERR_CODE_DB_INSERT;
    }
    return CODE_OK;
}

int SqliteIdentityService::rm(
    const DEVADDR &addr
)
{
    if (!db)
        return ERR_CODE_DB_DATABASE_NOT_FOUND;
    char *zErrMsg = nullptr;
    std::stringstream statement;
    statement << "DELETE FROM device WHERE addr = '" << DEVADDR2string(addr) << "'";
    int r = sqlite3_exec(db, statement.str().c_str(), nullptr, nullptr, &zErrMsg);
    std::cerr << statement.str() << std::endl;
    if (r != SQLITE_OK) {
        if (zErrMsg) {
            sqlite3_free(zErrMsg);
        }
        return ERR_CODE_DB_EXEC;
    }
    return CODE_OK;
}

/**
 * "CREATE DATABASE IF NOT EXISTS \"device\" USE \"db_name\"",
 */
static std::string SCHEMA_STATEMENT[] {
    "CREATE TABLE \"device\" (\"addr\" TEXT NOT NULL PRIMARY KEY, \"activation\" TEXT, \"deviceclass\" TEXT, \"deveui\" TEXT, \"nwkskey\" TEXT, \"appskey\" TEXT, \"version\" TEXT, \"appeui\" TEXT, \"appkey\" TEXT, \"nwkkey\" TEXT, \"devnonce\" TEXT, \"joinnonce\" TEXT, \"name\" TEXT)",
    "CREATE INDEX \"device_key_deveui\" ON \"device\" (\"deveui\")"
};

static int createDatabaseFile(
    const std::string &fileName
)
{
    sqlite3 *db;
    int r = sqlite3_open_v2(fileName.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
    if (r)
        return r;
    char *zErrMsg = nullptr;
    for (auto s : SCHEMA_STATEMENT) {
        r = sqlite3_exec(db, s.c_str(), nullptr, nullptr, &zErrMsg);
        if (r != SQLITE_OK) {
            if (zErrMsg) {
                sqlite3_free(zErrMsg);
            }
            break;
        }
    }
    sqlite3_close(db);
    return r;
}

int SqliteIdentityService::init(
    const std::string &databaseName,
    void *database
)
{
    dbName = databaseName;
    if (database) {
        // use external db
        db = (sqlite3 *) database;
        return CODE_OK;
    }
    if (!file::fileExists(dbName)) {
        int r = createDatabaseFile(dbName);
        if (r)
            return r;
    }
    int r = sqlite3_open(dbName.c_str(), &db);
    if (r) {
        db = nullptr;
        return ERR_CODE_DB_DATABASE_OPEN;
    }
    // validate objects
    r = sqlite3_exec(db, "SELECT * FROM device WHERE addr = ''", nullptr, nullptr, nullptr);
    if (r != SQLITE_OK) {
        int r = createDatabaseFile(dbName);
        if (r)
            return r;
    }
    return CODE_OK;
}

void SqliteIdentityService::flush()
{
    // re-open database file
    // external db closed
    if (db)
        sqlite3_close(db);
    sqlite3_open(dbName.c_str(), &db);
}

void SqliteIdentityService::done()
{
    int r = sqlite3_close(db);
    db = nullptr;
}

/**
 * Return next network address if available
 * @return 0- success, ERR_CODE_ADDR_SPACE_FULL- no address available
 */
int SqliteIdentityService::next(
    NETWORKIDENTITY &retval
)
{
    return ERR_CODE_ADDR_SPACE_FULL;
}

EXPORT_SHARED_C_FUNC IdentityService* makeSqliteIdentityService()
{
    return new SqliteIdentityService;
}
