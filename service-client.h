#ifndef SERVICE_CLIENT_H_
#define SERVICE_CLIENT_H_	1

#include <string>

#include "direct-client.h"

class ServiceClient : public DirectClient{
public:
    explicit ServiceClient(
        const std::string &name
    );
    static bool hasService(const std::string &name);
    virtual ~ServiceClient();
};

#endif
