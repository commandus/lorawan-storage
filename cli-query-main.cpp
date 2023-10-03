#include <string>
#include <iostream>

#include <sstream>
#include <cstring>

#include "argtable3/argtable3.h"

#ifdef ENABLE_LIBUV
#include <uv.h>
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
#define DEF_PORT 4244

class CommandName {
public:
    enum CliGatewayQueryTag s;
    const char* l;
    bool isCommand(
        const char* cmd
    ) const
    {
        if (!cmd)
            return false;
        auto len = strlen(cmd);
        if (len == 1 && s == *cmd)
            return true;
        return (strncmp(cmd, l, len) == 0);
    };
};

static const CommandName commands[9] = {
    { QUERY_GATEWAY_NONE, "none" },             // '0'
    { QUERY_GATEWAY_ADDR, "addr" },             // 'a'
    { QUERY_GATEWAY_ID, "id" },                 // 'A'
    { QUERY_GATEWAY_LIST, "list" },             // 'L'
    { QUERY_GATEWAY_COUNT, "count" },           // 'c'
    { QUERY_GATEWAY_ASSIGN, "assign" },         // 'p'
    { QUERY_GATEWAY_RM, "remove" },             // 'r',
    { QUERY_GATEWAY_FORCE_SAVE, "save" },       // 'f',
    { QUERY_GATEWAY_CLOSE_RESOURCES, "close" }  // 'd'
};

#define COMMANDS_COUNT 9

static const char *commandLongName(
    enum CliGatewayQueryTag tag
)
{
    for (int i = 0; i < COMMANDS_COUNT; i++) {
        if (commands[i].isCommand((char *) &tag))
            return commands[i].l;
    }
    return commands[0].l;
}

static std::string listCommands() {
    std::stringstream ss;
    for (int i = 1; i < COMMANDS_COUNT; i++) {
        ss << "  " << (char ) commands[i].s << "\t" << commands[i].l;
        if (i == 1)
            ss << " (default)";
        ss << "\n";
    }
    return ss.str();
}

static CliGatewayQueryTag isTag(const char *tag) {
    for (int i = 0; i < COMMANDS_COUNT; i++) {
        if (commands[i].isCommand(tag)) {
            return commands[i].s;
        }
    }
    return QUERY_GATEWAY_NONE;
}

// global parameters
class CliGatewayQueryParams {
public:
    CliGatewayQueryTag tag;
    std::vector<GatewayIdentity> query;
    size_t queryPos;
    bool useTcp;
    int verbose;
    std::string intf;
    uint16_t port;
    int32_t code;
    uint64_t accessCode;
    size_t offset;
    size_t size;

    int32_t retCode;

    CliGatewayQueryParams()
        : tag(tag), queryPos(0), useTcp(false), verbose(0), port(DEF_PORT), code(42), accessCode(42), offset(0), size(0),
          retCode(0)
    {

    }

    std::string toString() {
        std::stringstream ss;
        ss
            << "Service: " << intf << ":" << port << " " << (useTcp ? "TCP" : "UDP") << " "
            << "command: " << commandLongName(tag) << ", code: " << std::hex << code << ", access code: "  << accessCode << " "
            << "offset: " << std::dec << offset << ", size: "  << size << "\n";
        for (auto it(query.begin()); it != query.end(); it++) {
            if (it->gatewayId)
                ss << std::hex << it->gatewayId;
            ss << "\t" << sockaddr2string(&it->sockaddr) << "\n";
        }
        return ss.str();
    }
};

static CliGatewayQueryParams params;

class OnResp : public ResponseIntf {
public:
    const std::vector<GatewayIdentity> &query;
    int verbose;
    explicit OnResp(
        const std::vector<GatewayIdentity> &aQuery,
        int aVerbose
    )
        : query(aQuery), verbose(aVerbose)
    {
    }

    void onError(
        GatewayClient* client,
        const int32_t code,
        const int errorCode
    ) override {
        std::cerr << "error " << code << ", errno: " << errno << "\n";
        client->stop();
        params.retCode = code;
    }

    void onStatus(
        GatewayClient* client,
        const OperationResponse *response
    ) override {
        if (verbose) {
            std::cerr << "operation successfully completed\n";
        }
        if (response) {
            if (response->code) {
                std::cerr << ERR_MESSAGE << response->code << std::endl;
                client->stop();
            } else {
                if (params.verbose)
                    std::cout << response->toJsonString() << std::endl;
                else
                    std::cout << response->response << std::endl;
                if (!next(client)) {
                    client->stop();
                }
            }
        } else {
            client->stop();
        }
    }

    void onGet(
        GatewayClient* client,
        const GetResponse *response
    ) override {
        if (response) {
            if (response->code) {
                std::cerr << ERR_MESSAGE << response->code << std::endl;
                client->stop();
            } else {
                if (params.verbose)
                    std::cout << response->toJsonString() << std::endl;
                else
                    std::cout << sockaddr2string(&response->response.sockaddr) << std::endl;
                if (!next(client)) {
                    client->stop();
                }
            }
        } else {
            client->stop();
        }
	}

    void onList(
        GatewayClient* client,
        const ListResponse *response
    ) override {
        if (response) {
            if (response->code) {
                std::cerr << ERR_MESSAGE << response->code << std::endl;
                client->stop();
            } else {
                if (params.verbose)
                    std::cout << response->toJsonString() << std::endl;
                else {
                    for (int i = 0; i < response->response; i++) {
                        std::cout << std::hex << response->identities[i].gatewayId
                            << sockaddr2string(&response->identities[i].sockaddr)
                            << std::endl;
                    }
                }
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
        if (verbose) {
            std::cerr << "disconnected \n";
        }
    }

    bool next(
        GatewayClient *client
    ) {
        bool hasNext = params.queryPos < query.size();
        ServiceMessage *req = nullptr;
        if (hasNext) {
            GatewayIdentity gi = params.query[params.queryPos];
            switch (params.tag) {
                case QUERY_GATEWAY_NONE:
                    break;
                case QUERY_GATEWAY_LIST:
                    req = new OperationRequest((char) params.tag, params.offset, params.size, params.code, params.accessCode);
                    break;
                case QUERY_GATEWAY_COUNT:
                    req = new OperationRequest((char) params.tag, params.offset, params.size, params.code, params.accessCode);
                    break;
                case QUERY_GATEWAY_ASSIGN:
                    req = new GatewayIdAddrRequest((char) params.tag, gi, params.code, params.accessCode);
                    break;
                case QUERY_GATEWAY_RM:
                    req = new GatewayIdRequest(gi.gatewayId, params.code, params.accessCode);
                    break;
                case QUERY_GATEWAY_FORCE_SAVE:
                    break;
                case QUERY_GATEWAY_CLOSE_RESOURCES:
                    break;
                default:
                    if (gi.gatewayId == 0)
                        req = new GatewayAddrRequest(gi.sockaddr, params.code, params.accessCode);
                    else
                        req = new GatewayIdRequest(gi.gatewayId, params.code, params.accessCode);
                    break;
            }
        }
        if (req) {
            ServiceMessage *previousMessage = client->request(req);
            if (previousMessage)
                delete previousMessage;
        }
        params.queryPos++;
        if (verbose > 1) {
            std::cerr << "next " << params.queryPos << "\n";
        }
        return hasNext;
    }
};

/**
 * Merge address and identifiers
 * @param query Each item has address or identifier
 * @return true if success
 */
static bool mergeIdAddress(
    std::vector<GatewayIdentity> &query
)
{
    auto pairSize = query.size() / 2;
    int m = 1;
    for (int i = 0; i < pairSize; i++) {
        if (query[i * 2].gatewayId) {
            query[i].gatewayId = query[i * 2].gatewayId;
            memmove(&query[i].sockaddr, &query[(i * 2) + 1].sockaddr, sizeof(struct sockaddr));
        } else {
            memmove(&query[i].sockaddr, &query[(i * 2)].sockaddr, sizeof(struct sockaddr));
            query[i].gatewayId = query[(i * 2) + 1].gatewayId;
        }
    }
    query.resize(pairSize);
    return true;
}

static void run()
{
	OnResp onResp(params.query, params.verbose);
    GatewayClient *client;
#ifdef ENABLE_LIBUV
    client = new UvClient(useTcp, intf, port, &onResp);
#else
    client = new UDPClient(params.intf, params.port, &onResp);
#endif
    // request first address to resolve
    if (!onResp.next(client)) {
        params.retCode = ERR_CODE_PARAM_INVALID;
        return;
    }
    client->start();
    delete client;
}

int main(int argc, char **argv) {
    struct arg_str *a_tag = arg_str0("g", "tag", "<a|A|L|c|p|r|f|d>", "a(default) address by id, A- id by address, L- list, c- count, p- assign id&addr, r- remove");

    struct arg_str *a_query = arg_strn(nullptr, nullptr, "<command | gateway-id | address:port>", 1, 100, "command: a|A|L|c|p|r|f|d, gateway id: 16 hex digits");
    struct arg_str *a_interface_n_port = arg_str0("s", "service", "<ipaddr:port>", "Default localhost:4244");
    struct arg_int *a_code = arg_int0("c", "code", "<number>", "Default 42. 0x - hex number prefix");
    struct arg_str *a_access_code = arg_str0("a", "access", "<hex>", "Default 2a (42 decimal)");
	struct arg_lit *a_tcp = arg_lit0("t", "tcp", "use TCP protocol. Default UDP");
    struct arg_int *a_offset = arg_int0("o", "offset", "<0..>", "list offset. Default 0. ");
    struct arg_int *a_size = arg_int0("z", "size", "<number>", "list size limit. Default 100. ");
    struct arg_lit *a_verbose = arg_litn("v", "verbose", 0, 2,"-v verbose -vv debug");
    struct arg_lit *a_help = arg_lit0("h", "help", "Show this help");
	struct arg_end *a_end = arg_end(20);

	void* argtable[] = { 
		a_query, a_interface_n_port,
        a_code, a_access_code, a_tcp,
        a_offset, a_size,  a_verbose,
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
        params.port = DEF_PORT;
    }

    params.tag = QUERY_GATEWAY_ADDR;

    params.query.reserve(a_query->count);
    for (int i = 0; i < a_query->count; i++) {
        enum CliGatewayQueryTag tag = isTag(a_query->sval[i]);
        if (tag != QUERY_GATEWAY_NONE) {
            params.tag = tag;
            continue;
        }
        std::string a;
        uint16_t p;
        if (splitAddress(a, p, a_query->sval[i])) {
            params.query.emplace_back(GatewayIdentity(0, a, p));
        } else {
            char *last;
            uint64_t id = strtoull(a_query->sval[i], &last, 16);
            if (*last)
                return ERR_CODE_COMMAND_LINE;
            params.query.emplace_back(id);
        }
    }

    if (params.tag == QUERY_GATEWAY_LIST) {
        if (a_offset->count) {
            params.offset = (size_t) *a_offset->ival;
        }
        if (a_size->count) {
            params.size = (size_t) *a_size->ival;
        }
    }

    if (params.tag == QUERY_GATEWAY_ASSIGN) {
        // reorder query
        mergeIdAddress(params.query);
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
        std::cerr << "Commands:\n" << listCommands() << std::endl;
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return ERR_CODE_COMMAND_LINE;
	}
	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    if (params.verbose) {
        std::cerr << params.toString() << std::endl;
    }

#ifdef _MSC_VER
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
	run();
    return params.retCode;
}
