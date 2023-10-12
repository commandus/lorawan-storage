#ifndef GATEWAY_SERVICE_GEN_H_
#define GATEWAY_SERVICE_GEN_H_ 1

#include "gateway-service.h"
#include "sqlite3.h"

class SqliteGatewayService: public GatewayService {
protected:
    sqlite3 *db;
public:
    SqliteGatewayService();
    ~SqliteGatewayService() override;
    int get(GatewayIdentity &retVal, const GatewayIdentity &request) override;
    // List entries
    int list(std::vector<GatewayIdentity> &retVal, size_t offset, size_t size) override;
    // Entries count
    size_t size() override;
    int put(const GatewayIdentity &request) override;
    int rm(const GatewayIdentity &addr) override;

    int init(const std::string &option, void *data) override;
    void flush() override;
    void done() override;
};

#endif
