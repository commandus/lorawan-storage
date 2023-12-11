#ifndef PLUGIN_CLIENT_H_
#define PLUGIN_CLIENT_H_	1

#include <string>

#include "identity-service.h"
#include "gateway-service.h"

#ifdef _MSC_VER
#include <Windows.h>
#else
typedef void * HINSTANCE;
#endif

class PluginClient {
private:
    HINSTANCE handleSvc;
    int load(
        const std::string &fileName,
        const std::string &classIdentityName,
        const std::string &classGatewayName
    );
    void unload();
public:
    IdentityService* svcIdentity;
    GatewayService* svcGateway;

    explicit PluginClient(
        const std::string &fileName,
        const std::string &classIdentityName,
        const std::string &classGatewayName
    );
    virtual ~PluginClient();

};

#endif
