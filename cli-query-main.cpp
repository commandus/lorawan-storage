#include <string>
#include <iostream>

#include <uv.h>

#include "argtable3/argtable3.h"

#ifdef ENABLE_LIBUV
#include "uv-client.h"
#else
#include "udp-client.h"
#endif
#include "lorawan-error.h"
#include "lorawan-msg.h"
#include "log.h"
#include "gateway-identity.h"
#include "ip-address.h"

const char *progname = "lorawan-gateway-query";

enum CliGatewayQueryTag {
    QUERY_GATEWAY_ADDR = 'a',
    QUERY_GATEWAY_ID = 'A',
    QUERY_GATEWAY_LIST = 'L',
    QUERY_GATEWAY_COUNT = 'c',
    QUERY_GATEWAY_ASSIGN = 'p',
    QUERY_GATEWAY_RM = 'r',
    QUERY_GATEWAY_FORCE_SAVE = 'f',
    QUERY_GATEWAY_CLOSE_RESOURCES = 'd'
};

const std::string tags = "aALcprfd";

// global parameters
class CliGatewayQueryParams {
public:
    CliGatewayQueryTag tag;
    std::vector<GatewayIdentity> query;
    size_t queryPos = 0;
    bool useTcp;
    int verbose;
    std::string intf;
    uint16_t port;
    int32_t code;
    uint64_t accessCode;
    std::string queryAddress;
    uint16_t queryPort;
    size_t offset;
    size_t size;
};

static CliGatewayQueryParams params;

class OnResp : public ResponseIntf {
public:
    const std::vector<GatewayIdentity> &query;
    explicit OnResp(
        const std::vector<GatewayIdentity> &aQuery
    )
        : query(aQuery)
    {
    }

	void onGet(
        GatewayClient* client,
        bool got,
        const GetResponse *retVal
    ) override {
        if (got && retVal) {
            if (retVal->code) {
                std::cerr << ERR_MESSAGE << retVal->code << std::endl;
                client->stop();
            } else {
                if (params.verbose)
                    std::cout << retVal->toJsonString() << std::endl;
                else
                    std::cout << sockaddr2string(&retVal->response.sockaddr) << std::endl;
                if (!next(client)) {
                    client->stop();
                }
            }
        } else {
            client->stop();
        }
	}

    void onDisconnected(
        GatewayClient* client
    ) override {
        // retry
        client->request(new GatewayRequest((char) params.tag, query[params.queryPos], params.code, params.accessCode));
    }

    bool next(
        GatewayClient *client
    ) {
        if (params.queryPos >= query.size())
            return false;
        client->request(new GatewayRequest((char) params.tag, query[params.queryPos], params.code, params.accessCode));
        params.queryPos++;
        return true;
    }
};

void run() {
	OnResp onResp(params.query);
    GatewayClient *client;
#ifdef ENABLE_LIBUV
    client = new UvClient(useTcp, intf, port, &onResp);
#else
    client = new UDPClient(params.intf, params.port, &onResp);
#endif
    // request first address to resolve
    if (!onResp.next(client))
        return;
    client->start();
    delete client;
}

int main(int argc, char **argv) {
	struct arg_str *a_query = arg_strn(nullptr, nullptr, "<gateway-id>", 1, 100, "hex, 8 bytes long");
    struct arg_str *a_addr = arg_str0("a", "assign", "<address:port>", "assign address to the gateway");
    struct arg_str *a_interface_n_port = arg_str0("s", "service", "<ipaddr:port>", "Default localhost:4244");
    struct arg_int *a_code = arg_int0("c", "code", "<number>", "Default 42. 0x - hex number prefix");
    struct arg_str *a_access_code = arg_str0("a", "access", "<hex>", "Default 2a (42 decimal)");
	struct arg_lit *a_tcp = arg_lit0("t", "tcp", "use TCP protocol. Default UDP");
    struct arg_str *a_tag = arg_str0("c", "tag", "<a|A|L|c|p|r|f|d>", "a(default) address by id, A- id by address, L- list, c- count, p- assign id&addr, r- remove");
    struct arg_int *a_offset = arg_int0("o", "offset", "<0..>", "list offset. Default 0. ");
    struct arg_int *a_size = arg_int0("z", "size", "<number>", "list size limit. Default 100. ");
    struct arg_lit *a_verbose = arg_litn("v", "verbose", 0, 1,"print errors/warnings");
    struct arg_lit *a_help = arg_lit0("h", "help", "Show this help");
	struct arg_end *a_end = arg_end(20);

	void* argtable[] = { 
		a_query, a_addr, a_interface_n_port,
        a_code, a_access_code, a_tcp,
        a_tag, a_offset, a_size,  a_verbose,
		a_help, a_end 
	};

	// verify the argtable[] entries were allocated successfully
	if (arg_nullcheck(argtable) != 0) {
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return ERR_CODE_COMMAND_LINE;
	}
	// Parse the command line as defined by argtable[]
	int errorCount = arg_parse(argc, argv, argtable);

    params.verbose = a_verbose->count;
    params.useTcp = a_tcp->count > 0;

    if (a_interface_n_port->count) {
        if (!splitAddress(params.intf, params.port, std::string(*a_interface_n_port->sval))) {
            return ERR_CODE_COMMAND_LINE;
        }
    } else {
        params.intf = "localhost";
        params.port = 4244;
    }

    if (a_tag->count) {
        std::string t(*a_tag->sval);
        if (t.empty())
            return ERR_CODE_COMMAND_LINE;
        char tag = t[0];
        if (tags.find(tag) == std::string::npos)
            return ERR_CODE_COMMAND_LINE;
        params.tag = static_cast<CliGatewayQueryTag>(tag);
    } else {
        params.tag = QUERY_GATEWAY_ADDR;
    }

    if (params.tag == QUERY_GATEWAY_LIST) {
        if (a_offset->count) {
            params.offset = (size_t) *a_offset->ival;
        }
        if (a_size->count) {
            params.size = (size_t) *a_size->ival;
        }
    }

    if (params.tag == QUERY_GATEWAY_ID) {
        if (a_addr->count) {
            splitAddress(params.queryAddress, params.queryPort, *a_addr->sval);
        }
    }

    params.query.reserve(a_query->count);

    for (int i = 0; i < a_query->count; i++) {
        params.query.emplace_back(strtoull(a_query->sval[i], nullptr, 16));
    }

    if (a_access_code->count)
        params.accessCode = strtoull(*a_access_code->sval, nullptr, 16);
    else
        params.accessCode = 42;

    if (a_code->count)
        params.code = *a_code->ival;
    else
        params.code = 42;

    // special case: '--help' takes precedence over error reporting
	if ((a_help->count) || errorCount) {
		if (errorCount)
			arg_print_errors(stderr, a_end, progname);
		std::cerr << "Usage: " << progname << std::endl;
		arg_print_syntax(stderr, argtable, "\n");
		std::cerr << "LoRaWAN gateway storage query" << std::endl;
		arg_print_glossary(stderr, argtable, "  %-27s %s\n");
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return ERR_CODE_COMMAND_LINE;
	}
	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));

#ifdef _MSC_VER
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
	run();
}
