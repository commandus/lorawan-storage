#include "uv-client.h"

#include <uv.h>

#include "lorawan-string.h"
#include "ip-helper.h"

#ifdef _MSC_VER
#include <io.h>
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)
#define write _write
#define close _close 
#else
#include <netinet/in.h>
#include <unistd.h>
#define SOCKET int
#endif

#include "uv-mem.h"
#include "lorawan-error.h"

#ifdef ENABLE_DEBUG
#include <iostream>
#include <cstring>
#include "lorawan-msg.h"
#endif

#define DEF_KEEPALIVE_SECS 60

static void onTCPRead(
	uv_stream_t* strm,
	ssize_t nread,
	const uv_buf_t* buf
) {
    auto client = (UvClient*) strm->data;
    if (nread < 0) {
        if (nread == UV__EOF) {
            client->tcpConnected = false;
            client->onResponse->onDisconnected(client);
            return;
        } else {
#ifdef ENABLE_DEBUG
            std::cerr << ERR_SOCKET_READ << " " << nread << ": " << uv_err_name(nread) << std::endl;
            client->onResponse->onGet(client, false, nullptr);
#endif
        }
    } else {
#ifdef ENABLE_DEBUG
        std::cerr << "Read  " << nread << " bytes: " << hexString(buf->base, nread) << std::endl;
#endif
        GetResponse gr = GetResponse(buf->base, nread);
        gr.ntoh();
        client->onResponse->onGet(client, true, &gr);
    }
    freeBuffer(buf);
}

// https://gist.github.com/Jxck/4305806
static void onWriteEnd(
	uv_write_t* req,
	int status
) {
	if (status < 0) {
#ifdef ENABLE_DEBUG		
		std::cerr << ERR_SOCKET_WRITE << status << std::endl;
#endif
        freeReqData(req);
        if (req)
            free(req);
    }
}

static void onUDPread(
    uv_udp_t *handle,
    ssize_t bytesRead,
    const uv_buf_t *buf,
    const struct sockaddr *addr,
    unsigned flags
) {
    if (bytesRead < 0) {
        if (bytesRead != UV__EOF) {
#ifdef ENABLE_DEBUG
            std::cerr << ERR_SOCKET_READ << " " << bytesRead << ": " << uv_err_name(bytesRead) << std::endl;
#endif
        }
        return;
    }
    auto client = (UvClient*) handle->data;
    if (bytesRead == 0) {
        return;
    } else {
#ifdef ENABLE_DEBUG
        std::cerr << "Read " << bytesRead << " bytes: " << hexString(buf->base, bytesRead) << std::endl;
#endif
    }
    if (client) {
        if (client->onResponse) {
            GetResponse gr = GetResponse(buf->base, bytesRead);
            gr.ntoh();
            client->onResponse->onGet(client, true, &gr);
        }
    }
}

static void onClientUDPSent(
    uv_udp_send_t *req,
    int status
) {
    if (!req)
        return;
    auto *client = (UvClient *) req->data;
    if (status) {
#ifdef ENABLE_DEBUG
        std::cerr << ERR_SOCKET_WRITE << status << ": " << uv_strerror(status) << std::endl;
#endif
        client->onResponse->onGet(client, false, nullptr);
        free(req);
        return;
    }
    free(req);
}

static int sendTcp(
    UvClient* client,
    uv_stream_t* tcp
)
{
    uv_buf_t buf = uv_buf_init((char *) client->dataBuf, client->dataSize);
    auto *write_req = new uv_write_t;
    write_req->data = client;
    int buf_count = 1;
    int r = uv_write(write_req, tcp, &buf, buf_count, onWriteEnd);
    if (r < 0) {
#ifdef ENABLE_DEBUG
        std::cerr << ERR_SOCKET_WRITE << r << ": " << uv_strerror(r) << std::endl;
#endif
        client->onResponse->onGet(client, false, nullptr);
    }
    return r;
}

static void onConnect(
	uv_connect_t* req,
	int status
)
{
	if (status < 0) {
#ifdef ENABLE_DEBUG		
		std::cerr << ERR_SOCKET_CONNECT << status << ": " << uv_strerror(status) << std::endl;
#endif
        if (status != -125)
		    return;
	}
    auto client = (UvClient*) req->data;
    client->tcpConnected = true;
    uv_read_start(req->handle, allocBuffer, onTCPRead);
    sendTcp((UvClient*) req->data, req->handle);
#ifdef ENABLE_DEBUG
    std::cerr << MSG_CONNECTED << std::endl;
#endif
}

void UvClient::init()
{
    loop = uv_default_loop();
    if (useTcp) {
        // TCP
        tcpConnected = false;
        uv_tcp_init(loop, &tcpSocket);
        uv_tcp_keepalive(&tcpSocket, 1, DEF_KEEPALIVE_SECS);
        tcpSocket.data = this;
    } else {
        // UDP
        uv_udp_init(loop, &udpSocket);
        udpSocket.data = this;
        int r = uv_udp_recv_start(&udpSocket, allocBuffer, onUDPread);
        if (r < 0) {
#ifdef ENABLE_DEBUG
            std::cerr << ERR_SOCKET_READ << r << ": " << uv_strerror(r) << std::endl;
            return;
#endif
        }
    }
}

void UvClient::stop()
{
    if (useTcp) {
        // TCP
        tcpConnected = false;
    }
    if (!loop)
        return;
    uv_stop(loop);
    int result = uv_loop_close(loop);
    if (result == UV_EBUSY) {
        uv_walk(loop, [](uv_handle_t* handle, void* arg) {
            if (!uv_is_closing(handle))
                uv_close(handle, nullptr);
            }, nullptr);
        uv_run(loop, UV_RUN_DEFAULT);
        uv_loop_close(loop);
    }
}

/**
 */
UvClient::UvClient(
    bool aUseTcp,
    const std::string &aHost,
    uint16_t aPort,
	ResponseIntf *aOnResponse
)
	: IdentityClient(aOnResponse), useTcp(aUseTcp), status(0), tcpConnected(false)
{
    int r;
    if (isAddrStringIPv6(aHost.c_str()))
        uv_ip6_addr(aHost.c_str(), aPort, (sockaddr_in6*) &serverAddress);
    else
        uv_ip4_addr(aHost.c_str(), aPort, (sockaddr_in*) &serverAddress);
    if (r)
        status = ERR_CODE_ADDR_OUT_OF_RANGE;
    init();
}

void UvClient::query(
	void* buf,
	size_t size
)
{
#ifdef ENABLE_DEBUG
    std::cerr << "Query " << size << " bytes";
    if (size > 0)
        std::cerr <<": " << hexString(buf, size);
    std::cerr << std::endl;
#endif
	int r;
    dataBuf = buf;
    dataSize = size;
	if (useTcp) {
        // TCP
        if (!tcpConnected) {
            connectReq.data = this;
            r = uv_tcp_connect(&connectReq, &tcpSocket, (const sockaddr *) &serverAddress, onConnect);
        } else {
            r = sendTcp(this, connectReq.handle);
        }
	} else {
        // UDP
        auto *sendReq = (uv_udp_send_t *) malloc(sizeof(uv_udp_send_t));
        uv_buf_t uvBuf = uv_buf_init((char *) buf, size);
        sendReq->data = this;
        r = uv_udp_send(sendReq, &udpSocket, &uvBuf, 1, (const struct sockaddr *) &serverAddress, onClientUDPSent);
    }
	if (r) {
#ifdef ENABLE_DEBUG
		std::cerr << ERR_MESSAGE << r << ": " << uv_strerror(r) << std::endl;
#endif
		status = ERR_CODE_SOCKET_CREATE;
        onResponse->onGet(this, false, nullptr);
		return;
	}
	status = CODE_OK;
}

UvClient::~UvClient()
{
    stop();
}

void UvClient::request(
	GetRequest* value
)
{
    value->ntoh();
	query(value, sizeof(GetRequest));
}

void UvClient::start() {
    uv_run(loop, UV_RUN_DEFAULT);
}
