#ifndef ASYNC_WRAPPER_IDENTITY_SERVICE_H_
#define ASYNC_WRAPPER_IDENTITY_SERVICE_H_ 1

#include <functional>
#include "identity-service.h"

/**
 * Identity service async wrapper
 */
class AsyncWrapperIdentityService {
private:
    IdentityService *identityService;
public:
    explicit AsyncWrapperIdentityService(IdentityService *value);

    void get(
        const DEVADDR &devAddr,
        std::function<void(
            int retCode,
            DEVICEID &retVal
        )> cb
    );

    void getNetworkIdentity(
        const DEVEUI &eui,
        std::function<void(
            int retCode,
            NETWORKIDENTITY &retVal
        )> cb
    );

    // Add or replace Address = EUI and keys pair
    void put(
        const DEVADDR &devaddr,
        const DEVICEID &id,
        std::function<void(
            int retCode
        )> cb
    );

    // Remove
    void rm(
        const DEVADDR &addr,
        std::function<void(
            int retCode
        )> cb
    );

    void list(
        size_t offset,
        size_t size,
        std::function<void(
            int retCode,
            std::vector<NETWORKIDENTITY> &retval
        )> cb
    );

    // Entries count
    void size(
        std::function<void(
            size_t size
        )> cb
    );

    // force save
    void flush(
        std::function<void(
            int retCode
        )> cb
    );

    // reload
    void init(
        const std::string &option,
        void *data,
        std::function<void(
            int retCode
        )> cb
    );

    // close resources
    void done(
        std::function<void(
            int retCode
        )> cb
    );

    void next(
        std::function<void(
            NETWORKIDENTITY &retVal
        )> cb
    );
};

#endif
