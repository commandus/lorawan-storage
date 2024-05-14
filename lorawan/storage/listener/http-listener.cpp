#include <cstring>
#include <sstream>
#include <iostream>

#include <microhttpd.h>

#include "http-listener.h"

#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-conv.h"
#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-msg.h"
#include "lorawan/helper/ip-helper.h"

#define MHD_START_FLAGS 	MHD_USE_POLL | MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_SUPPRESS_DATE_NO_CLOCK | MHD_USE_TCP_FASTOPEN | MHD_USE_TURBO

#define NUMBER_OF_THREADS CPU_COUNT

// Caution: version may be different, if microhttpd dependency not compiled, revise version humber
#if MHD_VERSION <= 0x00096600
#define MHD_Result int
#endif
#ifndef MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_CREDENTIALS
#define MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_CREDENTIALS "Access-Control-Allow-Credentials"
#endif
#ifndef MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_METHODS
#define MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_METHODS "Access-Control-Allow-Methods"
#endif
#ifndef MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_HEADERS
#define MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_HEADERS "Access-Control-Allow-Headers"
#endif
#ifndef MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN
#define MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN "Access-Control-Allow-Origin"
#endif

HTTPListener::HTTPListener(
    IdentitySerialization* aIdentitySerialization,
    GatewaySerialization* aSerializationWrapper
)
    : StorageListener(aIdentitySerialization, aSerializationWrapper), 
    log(nullptr), verbose(0), flags(MHD_START_FLAGS),
    threadCount(1), connectionLimit(32768), descriptor(nullptr)
{
}

HTTPListener::~HTTPListener()
{
    stop();
}

void HTTPListener::setLog(
    int aVerbose,
    Log* aLog
)
{
    verbose = aVerbose;
    log = aLog;
}

void HTTPListener::stop()
{
    if (descriptor) {
        MHD_stop_daemon((struct MHD_Daemon *) descriptor);
        descriptor = nullptr;
    }
}

void HTTPListener::setAddress(
    const std::string& host,
    uint16_t aPort
)
{
    port = aPort;
}

void HTTPListener::setAddress(
    uint32_t& ipv4,
    uint16_t aPort
)
{
    port = aPort;
}

const static char* CT_JSON = "text/javascript;charset=UTF-8";
const static char* HDR_CORS_ORIGIN = "*";
const static char* HDR_CORS_METHODS = "GET,HEAD,OPTIONS,POST,PUT,DELETE";
const static char* HDR_CORS_HEADERS = "Authorization, Access-Control-Allow-Headers, Access-Control-Allow-Origin, "
"Origin, Accept, X-Requested-With, Content-Type, Access-Control-Request-Method, Access-Control-Request-Headers";

const static char* MSG_HTTP_ERROR = "Error";
const static char* MSG401 = "Unauthorized";
const static char* MSG501 = "Not implemented";

const static char* MSG500 = "Internal error";

static void addCORS(MHD_Response *response) {
    MHD_add_response_header(response, MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN, HDR_CORS_ORIGIN);
    // MHD_add_response_header(response, MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_CREDENTIALS, HDR_CORS_CREDENTIALS);
    MHD_add_response_header(response, MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_METHODS, HDR_CORS_METHODS);
    MHD_add_response_header(response, MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_HEADERS, HDR_CORS_HEADERS);
}

static MHD_Result httpError(
    struct MHD_Connection *connection,
    int code
)
{
    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(MSG_HTTP_ERROR), (void *) MSG_HTTP_ERROR, MHD_RESPMEM_PERSISTENT);
    addCORS(response);
    MHD_Result r = MHD_queue_response(connection, code, response);
    MHD_destroy_response(response);
    return r;
}

static MHD_Result httpError404(
    struct MHD_Connection *connection
)
{
    int hc = MHD_HTTP_NOT_FOUND;
    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(MSG401), (void *) MSG401, MHD_RESPMEM_PERSISTENT);
    addCORS(response);
    MHD_Result r = MHD_queue_response(connection, hc, response);
    MHD_destroy_response(response);
    return r;
}

class RequestContext {
public:
    std::string url;
    std::string postData;
};

static void *uri_logger_callback(
        void *cls,
        const char *uri
)
{
    auto c = (HTTPListener *) cls;
    if (c->verbose) {
        std::cout << uri << std::endl;
    }
    return nullptr;
}

static MHD_Result request_callback(
	void *cls,			// HTTPListener
	struct MHD_Connection *connection,
	const char *url,
	const char *method,
	const char *version,
	const char *upload_data,
	size_t *upload_data_size,
	void **ptr
)
{
	struct MHD_Response *response;
	MHD_Result ret;

    if (!*ptr) {
		// do never respond on first call
		*ptr = new RequestContext;
		return MHD_YES;
	}

    if (strcmp(method, "OPTIONS") == 0) {
        response = MHD_create_response_from_buffer(strlen(MSG500), (void *) MSG500, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, CT_JSON);
        addCORS(response);
        MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
        return MHD_YES;
    }

    auto *requestCtx = (RequestContext *) *ptr;
    if (*upload_data_size != 0) {
        requestCtx->postData += std::string(upload_data, *upload_data_size);
        *upload_data_size = 0;
        return MHD_YES;
    }

    requestCtx->url = url;

    int hc;
    if (strcmp(method, "DELETE") == 0) {
        hc = MHD_HTTP_NOT_IMPLEMENTED;
        response = MHD_create_response_from_buffer(strlen(MSG501), (void *) MSG501, MHD_RESPMEM_PERSISTENT);
    } else {
        // get instance
        auto *l = (HTTPListener *) cls;
        unsigned char rb[4096];
        size_t sz;
        if (l->identitySerialization) {
            sz = l->identitySerialization->query(&rb[0], sizeof(rb),
            (const unsigned char *) requestCtx->postData.c_str(), requestCtx->postData.size());
            if (sz == 0) {
                if (l->gatewaySerialization) {
                    sz = l->gatewaySerialization->query(&rb[0], sizeof(rb),
                        (const unsigned char *) requestCtx->postData.c_str(), requestCtx->postData.size());
                }
            }
        }
        if (sz == 0) {
            hc = MHD_HTTP_INTERNAL_SERVER_ERROR;
            response = MHD_create_response_from_buffer(strlen(MSG500), (void *) MSG500, MHD_RESPMEM_PERSISTENT);
        } else {
            hc = MHD_HTTP_OK;
            response = MHD_create_response_from_buffer(sz, (void *) &rb, MHD_RESPMEM_MUST_COPY);
        }
    }
    MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, CT_JSON);
    addCORS(response);
	ret = MHD_queue_response(connection, hc, response);
	MHD_destroy_response(response);
    delete requestCtx;
    *ptr = nullptr;
	return ret;
}

int HTTPListener::run()
{
    struct MHD_Daemon *d = MHD_start_daemon(
        flags, port, nullptr, nullptr,
        &request_callback, this,
        MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int) 30,  // 30s timeout
        MHD_OPTION_THREAD_POOL_SIZE, threadCount,
        MHD_OPTION_URI_LOG_CALLBACK, &uri_logger_callback, this,
        MHD_OPTION_CONNECTION_LIMIT, connectionLimit,
        MHD_OPTION_END
    );
    descriptor = (void *) d;
    return CODE_OK;
}
