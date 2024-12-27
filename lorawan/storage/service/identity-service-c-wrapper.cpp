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
    const DEVADDR a(*devAddr);
    DEVICEID did;
    int r = ((IdentityService *) o)->get(did, a);
    retVal->activation = did.activation;
    retVal->deviceclass = did.deviceclass;
    retVal->devEUI = did.devEUI.u;
    memmove(&retVal->nwkSKey, &did.nwkSKey, sizeof(KEY128));
    memmove(&retVal->appSKey, &did.appSKey, sizeof(KEY128));
    retVal->version = did.version.c;
    retVal->appEUI = did.appEUI.u;
    memmove(&retVal->appKey, &did.appKey, sizeof(KEY128));
    memmove(&retVal->nwkKey, &did.nwkKey, sizeof(KEY128));
    retVal->devNonce = did.devNonce.u;
    memmove(&retVal->joinNonce, &did.joinNonce, sizeof(C_JOINNONCE));
    memmove(&retVal->name, &did.name, sizeof(C_DEVICENAME));
    return r;
}

int c_getNetworkIdentity(
    void *o,
    C_NETWORKIDENTITY *retVal,
    const C_DEVEUI *eui
)
{
    NETWORKIDENTITY nid;
    const DEVEUI devEui(*eui);
    int r = ((IdentityService *) o)->getNetworkIdentity(nid, devEui);

    retVal->devid.activation = nid.devid.activation;
    retVal->devid.deviceclass = nid.devid.deviceclass;
    retVal->devid.devEUI = nid.devid.devEUI.u;
    memmove(&retVal->devid.nwkSKey, &nid.devid.nwkSKey, sizeof(KEY128));
    memmove(&retVal->devid.appSKey, &nid.devid.appSKey, sizeof(KEY128));
    retVal->devid.version = nid.devid.version.c;
    retVal->devid.appEUI = nid.devid.appEUI.u;
    memmove(&retVal->devid.appKey, &nid.devid.appKey, sizeof(KEY128));
    memmove(&retVal->devid.nwkKey, &nid.devid.nwkKey, sizeof(KEY128));
    retVal->devid.devNonce = nid.devid.devNonce.u;
    memmove(&retVal->devid.joinNonce, &nid.devid.joinNonce, sizeof(C_JOINNONCE));
    memmove(&retVal->devid.name, &nid.devid.name, sizeof(C_DEVICENAME));
    return r;
}

int c_put(
    void *o,
    const C_DEVADDR *devaddr,
    const C_DEVICEID *id
)
{
    const DEVADDR a(*devaddr);
    const DEVICEID did(
        static_cast<ACTIVATION>(id->activation),
        static_cast<DEVICECLASS>(id->deviceclass),
        static_cast<DEVEUI>(id->devEUI),
        *(KEY128*) &id->nwkSKey,
        *(KEY128*) &id->appSKey,
        id->version,
        static_cast<DEVEUI>(id->appEUI),
        *(KEY128*) &id->appKey,
        *(KEY128*) &id->nwkKey,
        static_cast<DEVNONCE>(id->devNonce),
        *(JOINNONCE*) &id->joinNonce,
        static_cast<DEVICENAME>(id->name)
    );
    return ((IdentityService *) o)->put(a, did);
}

int c_rm(
    void *o,
    const C_DEVADDR *addr
)
{
    const DEVADDR a(*addr);
    return ((IdentityService *) o)->rm(a);
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
    return r < 0 ? r : (int) v.size();
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
    return r < 0 ? r : (int) v.size();
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
