#ifndef UV_LISTENER_H_
#define UV_LISTENER_H_	1

#ifdef _MSC_VER
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

#include <string>
#ifdef _MSC_VER
#else
#include <arpa/inet.h>
#endif


#include "identity-listener.h"

class UVListener : public IdentityListener{
private:
    // libuv handler
    void *uv;
    struct sockaddr servaddr;
public:
    int status;
    explicit UVListener(
        IdentitySerialization *aSerializationWrapper
    );
    virtual ~UVListener();
    void setAddress(
        const std::string &host,
        uint16_t port
    ) override;
    void setAddress(
        uint32_t &ipv4,
        uint16_t port
    ) override;
    int run() override;
    void stop() override;
};

#endif
