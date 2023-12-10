#ifndef PLUGIN_CLIENT_H_
#define PLUGIN_CLIENT_H_	1

#include <string>

#include "identity-service.h"
#include "gateway-service.h"

#ifdef _MSC_VER
#include <Windows.h>
#define PLUGIN_FILE_NAME_SUFFIX ".dll"
#else
#define PLUGIN_FILE_NAME_SUFFIX ".so"
typedef void * HINSTANCE;
#endif

class PluginClient {
private:
    HINSTANCE handleSvc;
    int status;
    int32_t code;
    uint64_t accessCode;
    int load(
        const std::string &fileName,
        const std::string &className
    );
    void unload();
public:
    IdentityService* svcIdentity;
    GatewayService* svcGateway;

    explicit PluginClient(
        const std::string &fileName,
        const std::string &className,
        int32_t code,
        uint64_t accessCode
    );
    virtual ~PluginClient();

};

#endif
