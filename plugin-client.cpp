#include "plugin-client.h"

#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-conv.h"
#include "lorawan/lorawan-msg.h"

#ifdef _MSC_VER
#define dlopen(fileName, opt) LoadLibraryA(fileName)
#define dlclose FreeLibrary
#define dlsym GetProcAddress
#else
#include <dlfcn.h>
#include <algorithm>
#endif


#ifdef ENABLE_DEBUG
#include <iostream>
#include <cstring>
#include "lorawan-msg.h"
#endif

typedef IdentityService*(*makeIdentityServiceFunc)();
typedef GatewayService*(*makeGatewayServiceFunc)();

const std::string MAKE_FUNC_PREFIX = "make";
const std::string MAKE_FUNC_IDENTITY_SUFFIX = "IdentityService";
const std::string MAKE_FUNC_GATEWAY_SUFFIX = "GatewayService";


int PluginClient::load(
    const std::string &fileName,
    const std::string &className
)
{
    std::string makeIdentityClass = MAKE_FUNC_PREFIX + className + MAKE_FUNC_IDENTITY_SUFFIX;
    std::string makeGatewayClass = MAKE_FUNC_PREFIX + className + MAKE_FUNC_GATEWAY_SUFFIX;
    handleSvc = dlopen(fileName.c_str(), RTLD_LAZY);
    if (handleSvc) {
        makeIdentityServiceFunc fI = (makeIdentityServiceFunc) dlsym(handleSvc, makeIdentityClass.c_str());
        makeGatewayServiceFunc fG = (makeGatewayServiceFunc) dlsym(handleSvc, makeGatewayClass.c_str());
        if (fI && fG) {
            svcIdentity = fI();
            svcGateway = fG();
            return CODE_OK;
        }
    }
    return ERR_CODE_LOAD_PLUGINS_FAILED;
}

void PluginClient::unload()
{
    delete svcIdentity;
    svcIdentity = nullptr;
    delete svcGateway;
    svcGateway = nullptr;

    if (handleSvc) {
        dlclose(handleSvc);
        handleSvc = 0;
    }
}

PluginClient::PluginClient(
    const std::string &fileName,
    const std::string &className,
    int32_t aCode,
    uint64_t aAccessCode
)
	: handleSvc(0), svcIdentity(nullptr), svcGateway(nullptr),
    status(CODE_OK), code(aCode), accessCode(aAccessCode)
{
    load(fileName, className);
}

PluginClient::~PluginClient()
{
    unload();
}
