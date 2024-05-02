#include "sync-query-client.h"

SyncQueryClient::SyncQueryClient()
    : QueryClient(&rc)
{
};

SyncQueryClient::~SyncQueryClient() = default;

ServiceMessage* SyncQueryClient::request(
    ServiceMessage* value
)
{

}

void SyncQueryClient::start()
{

}

void SyncQueryClient::stop()
{

}
