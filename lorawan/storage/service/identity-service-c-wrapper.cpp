#include <functional>
#include <cstring>
#if defined(_MSC_VER) || defined(__MINGW32__)
#include <WinSock2.h>
#endif
#include "lorawan/storage/service/identity-service-c-wrapper.h"
#include <lorawan/lorawan-string.h>
#include "lorawan/storage/service/identity-service.h"
#include "lorawan/storage/service/identity-service-mem.h"
#include "lorawan/storage/service/identity-service-udp.h"

#ifdef ENABLE_GEN
#include "identity-service-gen.h"
#endif
#ifdef ENABLE_JSON
#include "identity-service-json.h"
#endif
#ifdef ENABLE_SQLITE
#include "identity-service-sqlite.h"
#endif
#ifdef ENABLE_LMDB
#include "identity-service-lmdb.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

EXPORT_SHARED_C_FUNC void *createIdentityServiceC(
    void *instance
)
{
    return instance;
}

EXPORT_SHARED_C_FUNC void* makeIdentityServiceC(
    C_IDENTITY_SERVICE_IMPL impl
)
{
    switch (impl) {
#ifdef ENABLE_GEN
        case CISI_GEN:
            return makeIdentityService();
#endif
#ifdef ENABLE_JSON
        case CISI_JSON:
            return makeIdentityService1();
#endif
        case CISI_MEM:
            return makeIdentityService2();
#ifdef ENABLE_SQLITE
        case CISI_SQLITE:
            return makeIdentityService3();
#endif
        case CISI_UDP:
            return makeIdentityService4();
#ifdef ENABLE_LMDB
        case CISI_LMDB:
            return makeIdentityService5();
#endif
        default:
            return nullptr;
    }
}

EXPORT_SHARED_C_FUNC void destroyIdentityServiceC(
    void *instance
)
{
    if (instance)
        delete (IdentityService*) instance;
}

static void DEVICEID2C_DEVICEID(
    C_DEVICEID *retVal,
    const DEVICEID &did
)
{
    retVal->activation = did.id.activation;
    retVal->deviceclass = did.id.deviceclass;
    retVal->devEUI = did.id.devEUI.u;
    memmove(&retVal->nwkSKey, &did.id.nwkSKey, sizeof(KEY128));
    memmove(&retVal->appSKey, &did.id.appSKey, sizeof(KEY128));
    retVal->version = did.id.version.c;
    retVal->appEUI = did.id.appEUI.u;
    memmove(&retVal->appKey, &did.id.appKey, sizeof(KEY128));
    memmove(&retVal->nwkKey, &did.id.nwkKey, sizeof(KEY128));
    retVal->devNonce = did.id.devNonce.u;
    memmove(&retVal->joinNonce, &did.id.joinNonce, sizeof(C_JOINNONCE));
    memmove(&retVal->name, &did.id.name, sizeof(C_DEVICENAME));
}

static void NETWORKIDENTITY2C_NETWORKIDENTITY(
    C_NETWORKIDENTITY *retVal,
    const NETWORKIDENTITY &nid
)
{
    retVal->devaddr = nid.value.devaddr.u;
    retVal->devid.activation = nid.value.devid.id.activation;
    retVal->devid.deviceclass = nid.value.devid.id.deviceclass;
    retVal->devid.devEUI = nid.value.devid.id.devEUI.u;
    memmove(&retVal->devid.nwkSKey, &nid.value.devid.id.nwkSKey, sizeof(KEY128));
    memmove(&retVal->devid.appSKey, &nid.value.devid.id.appSKey, sizeof(KEY128));
    retVal->devid.version = nid.value.devid.id.version.c;
    retVal->devid.appEUI = nid.value.devid.id.appEUI.u;
    memmove(&retVal->devid.appKey, &nid.value.devid.id.appKey, sizeof(KEY128));
    memmove(&retVal->devid.nwkKey, &nid.value.devid.id.nwkKey, sizeof(KEY128));
    retVal->devid.devNonce = nid.value.devid.id.devNonce.u;
    memmove(&retVal->devid.joinNonce, &nid.value.devid.id.joinNonce, sizeof(C_JOINNONCE));
    memmove(&retVal->devid.name, &nid.value.devid.id.name, sizeof(C_DEVICENAME));
}

static void C_NETWORKIDENTITY2NETWORKIDENTITY(
    NETWORKIDENTITY &retVal,
    C_NETWORKIDENTITY *nid
)
{
    retVal.value.devaddr.u = nid->devaddr;
    retVal.value.devid.id.activation = (ACTIVATION) nid->devid.activation;
    retVal.value.devid.id.deviceclass = (DEVICECLASS) nid->devid.deviceclass;
    retVal.value.devid.id.devEUI.u = nid->devid.devEUI;
    memmove(&retVal.value.devid.id.nwkSKey, &nid->devid.nwkSKey, sizeof(KEY128));
    memmove(&retVal.value.devid.id.appSKey, &nid->devid.appSKey, sizeof(KEY128));
    retVal.value.devid.id.version.c = nid->devid.version;
    retVal.value.devid.id.appEUI.u = nid->devid.appEUI;
    memmove(&retVal.value.devid.id.appKey, &nid->devid.appKey, sizeof(KEY128));
    memmove(&retVal.value.devid.id.nwkKey, &nid->devid.nwkKey, sizeof(KEY128));
    retVal.value.devid.id.devNonce.u = nid->devid.devNonce;
    memmove(&retVal.value.devid.id.joinNonce, &nid->devid.joinNonce, sizeof(C_JOINNONCE));
    memmove(&retVal.value.devid.id.name, &nid->devid.name, sizeof(C_DEVICENAME));
}

static void C_NETWORK_IDENTITY_FILTER2NETWORK_IDENTITY_FILTER(
    NETWORK_IDENTITY_FILTER &retVal,
    C_NETWORK_IDENTITY_FILTER *filter
)
{
    retVal.pre = (NETWORK_IDENTITY_LOGICAL_PRE_OPERATOR) filter->pre;
    retVal.property = (NETWORK_IDENTITY_PROPERTY) filter->property;
    retVal.comparisonOperator = (NETWORK_IDENTITY_COMPARISON_OPERATOR) filter->comparisonOperator;
    retVal.length = filter->length;
    memmove(retVal.filterData, filter->filterData, sizeof(retVal.filterData));
}

static void JOIN_ACCEPT_FRAME_HEADER2C_JOIN_ACCEPT_FRAME_HEADER(
    C_JOIN_ACCEPT_FRAME_HEADER *retVal,
    const JOIN_ACCEPT_FRAME_HEADER &hdr
)
{
    retVal->joinNonce[0] = hdr.joinNonce.c[0];
    retVal->joinNonce[1] = hdr.joinNonce.c[1];
    retVal->joinNonce[2] = hdr.joinNonce.c[2];
    retVal->netId[0] = hdr.netId.c[0];
    retVal->netId[1] = hdr.netId.c[1];
    retVal->netId[2] = hdr.netId.c[2];
    retVal->devAddr = hdr.devAddr.u;
    retVal->dlSettings.c = hdr.dlSettings.c;		    // downlink configuration settings
    retVal->rxDelay = hdr.rxDelay;
}

EXPORT_SHARED_C_FUNC int c_get(
    void *o,
    C_DEVICEID *retVal,
    const C_DEVADDR *devAddr
)
{
    const DEVADDR a(*devAddr);
    DEVICEID did;
    int r = ((IdentityService *) o)->get(did, a);
    DEVICEID2C_DEVICEID(retVal, did);
    return r;
}

EXPORT_SHARED_C_FUNC int c_getNetworkIdentity(
    void *o,
    C_NETWORKIDENTITY *retVal,
    const C_DEVEUI *eui
)
{
    NETWORKIDENTITY nid;
    const DEVEUI devEui(*eui);
    int r = ((IdentityService *) o)->getNetworkIdentity(nid, devEui);
    NETWORKIDENTITY2C_NETWORKIDENTITY(retVal, nid);
    return r;
}

EXPORT_SHARED_C_FUNC int c_put(
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

EXPORT_SHARED_C_FUNC int c_rm(
    void *o,
    const C_DEVADDR *addr
)
{
    const DEVADDR a(*addr);
    return ((IdentityService *) o)->rm(a);
}

EXPORT_SHARED_C_FUNC int c_list(
    void *o,
    C_NETWORKIDENTITY retVal[],
    uint32_t offset,
    uint8_t size
)
{
    std::vector<NETWORKIDENTITY> v;
    int r = ((IdentityService *) o)->list(v, offset, size);
    if (r >= 0) {
        for (auto i = 0; i < v.size(); i++) {
            retVal[i].devaddr = v[i].value.devaddr.u;
            NETWORKIDENTITY2C_NETWORKIDENTITY(&retVal[i], v[i]);
        }
    }
    return r < 0 ? r : (int) v.size();
}

EXPORT_SHARED_C_FUNC int c_filter(
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
        NETWORK_IDENTITY_FILTER nif;
        C_NETWORK_IDENTITY_FILTER2NETWORK_IDENTITY_FILTER(nif, &filters[i]);
        f.emplace_back(nif);
    }
    int r = ((IdentityService *) o)->filter(v, f, offset, size);
    for(auto i = 0; i < v.size(); i++) {
        retVal[i].devaddr = v[i].value.devaddr.u;
        NETWORKIDENTITY2C_NETWORKIDENTITY(&retVal[i], v[i]);

    }
    return r < 0 ? r : (int) v.size();
}

EXPORT_SHARED_C_FUNC int c_filterExpression(
    void *o,
    C_NETWORKIDENTITY retVal[],
    const char *filterExpression,
    size_t filterExpressionSize,
    uint32_t offset,
    uint8_t size
)
{
    std::vector<NETWORKIDENTITY> v;
    std::vector<NETWORK_IDENTITY_FILTER> f;

    string2NETWORK_IDENTITY_FILTERS(f, filterExpression, filterExpressionSize);
    int r = ((IdentityService *) o)->filter(v, f, offset, size);
    for(auto i = 0; i < v.size(); i++) {
        retVal[i].devaddr = v[i].value.devaddr.u;
        NETWORKIDENTITY2C_NETWORKIDENTITY(&retVal[i], v[i]);
    }
    return r < 0 ? r : (int) v.size();
}

EXPORT_SHARED_C_FUNC size_t c_size(void *o)
{
    return ((IdentityService *) o)->size();
}

EXPORT_SHARED_C_FUNC int c_next(
    void *o,
    C_NETWORKIDENTITY *retVal
)
{
    NETWORKIDENTITY nid;
    int r = ((IdentityService *) o)->next(nid);
    NETWORKIDENTITY2C_NETWORKIDENTITY(retVal, nid);
    return r;
}

EXPORT_SHARED_C_FUNC void c_flush(
    void *o
)
{
    ((IdentityService *) o)->flush();
}

EXPORT_SHARED_C_FUNC int c_init(
    void *o,
    const char *option,
    void *data
)
{
    return ((IdentityService *) o)->init(option, data);
}

EXPORT_SHARED_C_FUNC void c_done(
    void *o
)
{
    ((IdentityService *) o)->done();
}

EXPORT_SHARED_C_FUNC void c_setOption(
    void *o,
    int option,
    void *value
)
{
    ((IdentityService *) o)->setOption(option, value);
}

EXPORT_SHARED_C_FUNC C_NETID *c_getNetworkId(
    void *o
)
{
    return (C_NETID *) &((IdentityService *) o)->getNetworkId()->c;
}

EXPORT_SHARED_C_FUNC void c_setNetworkId(
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

EXPORT_SHARED_C_FUNC int c_joinAccept(
    void *o,
    C_JOIN_ACCEPT_FRAME_HEADER *retVal,
    C_NETWORKIDENTITY *networkIdentity
)
{
    NETWORKIDENTITY nid;
    C_NETWORKIDENTITY2NETWORKIDENTITY(nid, networkIdentity);
    JOIN_ACCEPT_FRAME_HEADER path;
    int r = ((IdentityService *) o)->joinAccept(path, nid);
    JOIN_ACCEPT_FRAME_HEADER2C_JOIN_ACCEPT_FRAME_HEADER(retVal, path);
    return r;
}

#ifdef __cplusplus
}
#endif
