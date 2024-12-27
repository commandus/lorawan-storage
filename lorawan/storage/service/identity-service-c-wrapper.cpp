#include <functional>
#include "lorawan/storage/service/identity-service-c-wrapper.h"

#include <cstring>

#include "lorawan/storage/service/identity-service.h"

#ifdef __cplusplus
extern "C"
{
#endif

void *createIdentityServiceC(
    void *instance
)
{
    return instance;
}

void destroyIdentityServiceC(
    void *instance
)
{
    if (instance)
        delete (IdentityService*) instance;
}

int c_get(
    void *o,
    C_DEVICEID *retVal,
    const C_DEVADDR *devAddr
)
{
    return ((IdentityService *) o)->get((DEVICEID &) retVal, (const DEVADDR &) devAddr);
}

int c_getNetworkIdentity(
    void *o,
    C_NETWORKIDENTITY *retVal,
    const C_DEVEUI *eui
)
{
    return ((IdentityService *) o)->getNetworkIdentity((NETWORKIDENTITY &) retVal, (const DEVEUI &) eui);
}

int c_rm(
    void *o,
    const C_DEVADDR *addr
)
{
    return ((IdentityService *) o)->rm((const DEVADDR &) addr);
}

int c_list(
    void *o,
    C_NETWORKIDENTITY retVal[],
    uint32_t offset,
    uint8_t size
)
{
    std::vector<NETWORKIDENTITY> v;
    int r = ((IdentityService *) o)->list(v, offset, size);
    for(auto i = 0; i < v.size(); i++) {
        retVal[i].devaddr = v[i].devaddr.u;
        memmove(&retVal[i].devid.activation, &v[i].devid.activation, sizeof(C_DEVICEID));
    }
    return r;
}

int c_filter(
    void *o,
    C_NETWORKIDENTITY retVal[],
    C_NETWORK_IDENTITY_FILTER filters[],
    size_t filterSize,
    uint32_t offset,
    uint8_t size
)
{
    std::vector<NETWORKIDENTITY> v;
    std::vector<NETWORK_IDENTITY_FILTER> f;
    for (auto i = 0; i < filterSize; i++) {
        f.emplace_back((const NETWORK_IDENTITY_FILTER&) filters[i]);
    }
    int r = ((IdentityService *) o)->filter(v, f, offset, size);
    for(auto i = 0; i < v.size(); i++) {
        retVal[i].devaddr = v[i].devaddr.u;
        memmove(&retVal[i].devid.activation, &v[i].devid.activation, sizeof(C_DEVICEID));
    }
    return r;
}

size_t c_size(void *o)
{
    return ((IdentityService *) o)->size();
}

int c_next(
    void *o,
    C_NETWORKIDENTITY *retVal
)
{
    return ((IdentityService *) o)->next((NETWORKIDENTITY &) retVal);
}

void c_flush(
    void *o
)
{
    ((IdentityService *) o)->flush();
}

int c_init(
    void *o,
    const char *option,
    void *data
)
{
    return ((IdentityService *) o)->init(option, data);
}

void c_done(
    void *o
)
{
    ((IdentityService *) o)->done();
}

void c_setOption(
    void *o,
    int option,
    void *value
)
{
    ((IdentityService *) o)->setOption(option, value);
}

C_NETID *c_getNetworkId(
    void *o
)
{
    return (C_NETID *) &((IdentityService *) o)->getNetworkId()->c;
}

void c_setNetworkId(
    void *o,
    const C_NETID *value
)
{
    NETID n;
    n.c[0] = *value[0];
    n.c[1] = *value[1];
    n.c[2] = *value[2];
    ((IdentityService *) o)->setNetworkId(n);
}

int c_joinAccept(
    void *o,
    C_JOIN_ACCEPT_FRAME_HEADER *retVal,
    C_NETWORKIDENTITY *networkIdentity
)
{
    return ((IdentityService *) o)->joinAccept((JOIN_ACCEPT_FRAME_HEADER &)retVal, (NETWORKIDENTITY &) networkIdentity);
}

#ifdef __cplusplus
}
#endif
