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

const char *progname = "lorawan-gateway-query";

// global parameters
std::vector<GatewayIdentity> query;
size_t queryPos = 0;
bool useTcp;
int verbose;
std::string intf;
uint16_t port;
int32_t code;
uint64_t accessCode;

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
                if (verbose)
                    std::cout << retVal->toJsonString() << std::endl;
                else
                    std::cout << retVal->toString() << std::endl;
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
        client->request(new GetRequest(query[queryPos], code, accessCode));
    }

    bool next(
        GatewayClient *client
    ) {
        if (queryPos >= query.size())
            return false;
        client->request(new GetRequest(query[queryPos], code, accessCode));
        queryPos++;
        return true;
    }
};

void run() {
	OnResp onResp(query);
    GatewayClient *client;
#ifdef ENABLE_LIBUV
    client = new UvClient(useTcp, intf, port, &onResp);
#else
    client = new UDPClient(intf, port, &onResp);
#endif
    // request first address to resolve
    if (!onResp.next(client))
        return;
    client->start();
    delete client;
}

static void parseAddress(
    std::string &retIntf,
    uint16_t &retPort,
    const std::string &value
) {
    size_t pos = value.find(':');
    if (pos != std::string::npos) {
        retIntf = value.substr(0, pos);
        std::string sport = value.substr(pos + 1);
        retPort = (uint16_t) std::stoi(sport);
    }
}

int main(int argc, char **argv) {
	struct arg_str *a_query = arg_strn(nullptr, nullptr, "<gateway-id>", 1, 100, "hex, 8 bytes long");
    struct arg_str *a_interface_n_port = arg_str0("s", "service", "<ipaddr:port>", "Default localhost:4244");
    struct arg_int *a_code = arg_int0("c", "code", "<number>", "Default 42. 0x - hex number prefix");
    struct arg_str *a_access_code = arg_str0("a", "access", "<hex>", "Default 2a (42 decimal)");
	struct arg_lit *a_tcp = arg_lit0("t", "tcp", "use TCP protocol. Default UDP");
    struct arg_lit *a_verbose = arg_litn("v", "verbose", 0, 1,"print errors/warnings");
    struct arg_lit *a_help = arg_lit0("h", "help", "Show this help");
	struct arg_end *a_end = arg_end(20);

	void* argtable[] = { 
		a_query, a_interface_n_port,
        a_code, a_access_code, a_tcp, a_verbose,
		a_help, a_end 
	};

	// verify the argtable[] entries were allocated successfully
	if (arg_nullcheck(argtable) != 0) {
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return ERR_CODE_COMMAND_LINE;
	}
	// Parse the command line as defined by argtable[]
	int errorCount = arg_parse(argc, argv, argtable);

	verbose = a_verbose->count;
    useTcp = a_tcp->count > 0;

    if (a_interface_n_port->count) {
        parseAddress(intf, port, std::string(*a_interface_n_port->sval));
    } else {
        intf = "localhost";
        port = 4244;
    }

    query.reserve(a_query->count);
    for (int i = 0; i < a_query->count; i++) {
        query.emplace_back(strtoull(a_query->sval[i], nullptr, 16));
    }

    if (a_access_code->count)
        accessCode = strtoull(*a_access_code->sval, nullptr, 16);
    else
        accessCode = 42;

    if (a_code->count)
        code = *a_code->ival;
    else
        code = 42;

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
