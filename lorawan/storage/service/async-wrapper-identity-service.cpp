#include "async-wrapper-identity-service.h"

AsyncWrapperIdentityService::AsyncWrapperIdentityService(IdentityService *value)
    : identityService(value)
{
}

void AsyncWrapperIdentityService::get(
    const DEVADDR &devAddr,
    std::function<void(
        int retCode,
        DEVICEID &retVal
    )> cb
)
{
    DEVICEID v;
    int r = identityService->get(v, devAddr);
    cb(r, v);
}

void AsyncWrapperIdentityService::getNetworkIdentity(
    const DEVEUI &eui,
    std::function<void(
        int retCode,
        NETWORKIDENTITY &retVal
    )> cb
)
{
    NETWORKIDENTITY v;
    int r = identityService->getNetworkIdentity(v, eui);
    cb(r, v);
}

// Add or replace Address = EUI and keys pair
void AsyncWrapperIdentityService::put(
    const DEVADDR &devaddr,
    const DEVICEID &id,
    std::function<void(
        int retCode
    )> cb
)
{
    int r = identityService->put(devaddr, id);
    cb(r);
}

// Remove
void AsyncWrapperIdentityService::rm(
    const DEVADDR &addr,
    std::function<void(
        int retCode
    )> cb
)
{
    int r = identityService->rm(addr);
    cb(r);
}

void AsyncWrapperIdentityService::list(
    size_t offset,
    size_t size,
    std::function<void(
        int retCode,
        std::vector<NETWORKIDENTITY> &retval
    )> cb
)
{
    std::vector<NETWORKIDENTITY> v;
    int r = identityService->list(v, offset, size);
    cb(r, v);
}

// Entries count
void AsyncWrapperIdentityService::size(
    std::function<void(
        size_t size
    )> cb
)
{
    cb(identityService->size());
}

// force save
void AsyncWrapperIdentityService::flush(
    std::function<void(
        int retCode
    )> cb
)
{
    identityService->flush();
    cb(0);
}

// reload
void AsyncWrapperIdentityService::init(
    const std::string &option,
    void *data,
    std::function<void(
        int retCode
    )> cb
)
{
    cb(identityService->init(option, data));
}

// close resources
void AsyncWrapperIdentityService::done(
    std::function<void(
        int retCode
    )> cb
)
{
    identityService->done();
    cb(0);
}

void AsyncWrapperIdentityService::next(
    std::function<void(
        NETWORKIDENTITY &retVal
    )> cb
)
{
    NETWORKIDENTITY r;
    identityService->next(r);
    cb(r);
}

#endif
