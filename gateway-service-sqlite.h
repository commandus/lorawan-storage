#ifndef GATEWAY_SERVICE_GEN_H_
#define GATEWAY_SERVICE_GEN_H_ 1

#include <vector>
#include <mutex>
#include <map>
#include "gateway-service.h"

class SqliteGatewayService: public GatewayService {
protected:
    std::map<uint64_t, GatewayIdentity> storage;
    void clear();
public:
    SqliteGatewayService();
    ~SqliteGatewayService() override;
    int get(GatewayIdentity &retVal, const GatewayIdentity &request) override;
    // List entries
    void list(std::vector<GatewayIdentity> &retVal, size_t offset, size_t size) override;
    // Entries count
    size_t size() override;
    void put(const GatewayIdentity &request) override;
    void rm(const GatewayIdentity &addr) override;

    int init(const std::string &option, void *data) override;
    void flush() override;
    void done() override;
};

#endif
