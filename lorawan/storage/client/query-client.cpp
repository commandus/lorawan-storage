#include "query-client.h"

QueryClient::QueryClient(
    ResponseIntf *aOnResponse
)
    : onResponse(aOnResponse)
{
};

QueryClient::~QueryClient() = default;
