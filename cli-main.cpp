#include <string>
#include <iostream>
#include <csignal>
#include <climits>

#ifdef _MSC_VER
#include <direct.h>
#include <sstream>

#define PATH_MAX MAX_PATH
#define getcwd _getcwd
#else
#include <unistd.h>
#endif

#include "argtable3/argtable3.h"

#ifdef ENABLE_LIBUV
#include "lorawan/storage/listener/uv-listener.h"
#else
#include "udp-listener.h"
#endif

#ifdef ENABLE_SQLITE
#include "identity-service-sqlite.h"
#include "gateway-service-sqlite.h"
#define DEF_DB  "lorawan.db"
#else
#ifdef ENABLE_GEN
#include "lorawan/storage/service/identity-service-gen.h"
#else
#include "identity-service-mem.h"
#endif
#include "lorawan/storage/service/gateway-service-mem.h"
#endif

#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-msg.h"
#include "log.h"
#include "daemonize.h"
#include "lorawan/helper/ip-address.h"

const char *programName = "lorawan-storage";

enum IP_PROTO {
    PROTO_UDP,
    PROTO_TCP,
    PROTO_UDP_N_TCP
};

static std::string IP_PROTO2string(
    enum IP_PROTO value
)
{
    switch(value) {
        case PROTO_UDP:
            return "UDP";
        case PROTO_TCP:
            return "TCP";
        default:
            return "TCP, UDP";
    }
}

// global parameters and descriptors
class CliServiceDescriptorNParams : public Log
{
public:
    StorageListener *server = nullptr;

    enum IP_PROTO proto;
    std::string intf;
    uint16_t port;
    int32_t code;
    uint64_t accessCode;
    bool runAsDaemon;
    int verbose;
#ifdef ENABLE_SQLITE
    std::string db;
#endif
    int32_t retCode;

#ifdef ENABLE_GEN
    std::string passPhrase;
    NETID netid;
#endif

    CliServiceDescriptorNParams()
        : server(nullptr), proto(PROTO_UDP), port(4244), code(0), accessCode(0), verbose(0), retCode(0),
        runAsDaemon(false)
#ifdef ENABLE_GEN
        , netid(0, 0)
#endif

    {

    }

    std::string toString() const {
        std::stringstream ss;
        ss
            << "Service: " << intf << ":" << port << " " << IP_PROTO2string(proto) << ".\n"
            << "Code: " << std::hex << code << ", access code: "  << accessCode << " " << "\n";
        return ss.str();
    }

    std::ostream& strm(
        int level
    ) override {
        return std::cerr;
    }

    void flush() override {
        std::cerr << std::endl;
    }

};

CliServiceDescriptorNParams svc;

static void done() {
    if (svc.server) {
        svc.server->stop();
        delete svc.server;
        svc.server = nullptr;
        std::cerr << MSG_GRACEFULLY_STOPPED << std::endl;
        exit(svc.retCode);
    }
}

static void stop() {
	if (svc.server) {
        svc.server->stop();
	}
}

void signalHandler(int signal)
{
	if (signal == SIGINT) {
		std::cerr << MSG_INTERRUPTED << std::endl;
        done();
	}
}

void setSignalHandler()
{
#ifdef _MSC_VER
#else
	struct sigaction action = {};
	action.sa_handler = &signalHandler;
	sigaction(SIGINT, &action, nullptr);
	sigaction(SIGHUP, &action, nullptr);
#endif
}

void run() {
    auto identityService =
#ifdef ENABLE_SQLITE
        new SqliteIdentityService;
    identityService->init(svc.db, nullptr);
#else
#ifdef ENABLE_GEN
            new GenIdentityService;
    identityService->init(svc.passPhrase, &svc.netid);
#else
        new MemoryIdentityService;
        identityService->init("", nullptr);
#endif
#endif
    auto gatewayService =
#ifdef ENABLE_SQLITE
        new SqliteGatewayService;
    gatewayService->init(svc.db, nullptr);
#else
    new MemoryGatewayService;
    gatewayService->init("", nullptr);
#endif

    auto identitySerialization = new IdentitySerialization(identityService, svc.code, svc.accessCode);
    auto gatewaySerialization = new GatewaySerialization(gatewayService, svc.code, svc.accessCode);
#ifdef ENABLE_LIBUV
    svc.server = new UVListener(identitySerialization, gatewaySerialization);
#else
    svc.server = new UDPListener(identitySerialization, gatewaySerialization);
#endif
    svc.server->setAddress(svc.intf, svc.port);
    svc.server->setLog(svc.verbose, &svc);
    svc.retCode = svc.server->run();
    if (svc.retCode)
        std::cerr << ERR_MESSAGE << svc.retCode << ": " << std::endl;
}

int main(int argc, char **argv) {
	struct arg_str *a_interface_n_port = arg_str0(nullptr, nullptr, "ipaddr:port", "Default *:4244");
#ifdef ENABLE_SQLITE
    struct arg_str *a_db = arg_str0("d", "db", "<file>", "database file name. Default " DEF_DB);
#endif
    struct arg_int *a_code = arg_int0("c", "code", "<number>", "Default 42. 0x - hex number prefix");

#ifdef ENABLE_GEN
    struct arg_str *a_pass_phrase = arg_str1("m", "master-key", "<pass-phrase>", "");
    struct arg_str *a_net_id = arg_str0("n", "network-id", "<hex|hex:hex>", "Hexadecimal <network-id> or <net-type>:<net-id>. Default 0");
#endif

    struct arg_str *a_access_code = arg_str0("a", "access", "<hex>", "Default 2a (42 decimal)");
	struct arg_lit *a_daemonize = arg_lit0("d", "daemonize", "run daemon");
	struct arg_lit *a_verbose = arg_litn("v", "verbose", 0, 2,"-v - verbose, -vv - debug");
	struct arg_lit *a_help = arg_lit0("h", "help", "Show this help");
	struct arg_end *a_end = arg_end(20);

	void* argtable[] = { 
		a_interface_n_port,
#ifdef ENABLE_GEN
        a_pass_phrase, a_net_id,
#endif
#ifdef ENABLE_SQLITE
        a_db,
#endif
        a_code, a_access_code, a_verbose, a_daemonize,
		a_help, a_end 
	};

	// verify the argtable[] entries were allocated successfully
	if (arg_nullcheck(argtable) != 0) {
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return ERR_CODE_COMMAND_LINE;
	}
	// Parse the command line as defined by argtable[]
	int nerrors = arg_parse(argc, argv, argtable);

    svc.runAsDaemon = a_daemonize->count > 0;
	svc.verbose = a_verbose->count;

	if (a_interface_n_port->count) {
        splitAddress(svc.intf, svc.port, std::string(*a_interface_n_port->sval));
    } else {
        svc.intf = "*";
        svc.port = 4244;
    }

#ifdef ENABLE_GEN
    if (a_pass_phrase->count)
        svc.passPhrase = *a_pass_phrase->sval;
    if (a_net_id) {
        std::string v = *a_net_id->sval;
        auto p = v.find(':');
        if (p != std::string::npos) {
            svc.netid.set(
                strtoul(v.substr(0, p - 1).c_str(), nullptr, 16),
                strtoul(v.substr(p + 1).c_str(), nullptr, 16)
            );
        } else
            svc.netid.set(strtoul(v.c_str(), nullptr, 16));
    }
#endif

#ifdef ENABLE_SQLITE
    if (a_db->count)
        svc.db = *a_db->sval;
    else
        svc.db = DEF_DB;
#endif
    if (a_code->count)
        svc.code = *a_code->ival;
    else
        svc.code = 42;

    if (a_access_code->count)
        svc.accessCode = strtoull(*a_access_code->sval, nullptr, 16);
    else
        svc.accessCode = 42;

	// special case: '--help' takes precedence over error reporting
	if ((a_help->count) || nerrors) {
		if (nerrors)
			arg_print_errors(stderr, a_end, programName);
		std::cerr << "Usage: " << programName << std::endl;
		arg_print_syntax(stderr, argtable, "\n");
		std::cerr << "LoRaWAN gateway storage example" << std::endl;
		arg_print_glossary(stderr, argtable, "  %-27s %s\n");
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return ERR_CODE_COMMAND_LINE;
	}
	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));

#ifdef _MSC_VER
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    if (svc.runAsDaemon) {
		char workDir[PATH_MAX];
		std::string programPath = getcwd(workDir, PATH_MAX);
		if (svc.verbose)
			std::cerr << MSG_LISTENER_DAEMON_RUN
                      << "(" << programPath << "/" << programName << "). "
                      << MSG_CHECK_SYSLOG << std::endl;
		OPEN_SYSLOG(programName)
        Daemonize daemon(programName, programPath, run, stop, done);
		// CLOSESYSLOG()
	} else {
		setSignalHandler();
        if (svc.verbose > 1) {
            std::cerr << svc.toString() << std::endl;
        }
		run();
		done();
	}
}
