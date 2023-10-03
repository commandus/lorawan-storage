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
#include "uv-listener.h"
#else
#include "udp-listener.h"
#endif

#ifdef ENABLE_SQLITE
#include "gateway-service-sqlite.h"
#else
#include "gateway-service-mem.h"
#endif

#include "lorawan-error.h"
#include "log.h"
#include "daemonize.h"
#include "ip-address.h"

const char *progname = "lorawan-gateway-storage";

#define MSG_DONE            	"Stopped gracefully"
#define MSG_ERROR            	"Error "
#define MSG_INTERRUPTED     	"Interrupted"
#define MSG_DAEMON_STARTED  	"Started"
#define MSG_DAEMON_STARTED_1 	". Check syslog."

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
class CliGatewayServiceDescriptorNParams : public LogIntf
{
public:
    GatewayListener *server = nullptr;

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

    CliGatewayServiceDescriptorNParams()
            : server(nullptr), proto(PROTO_UDP), port(4244), code(0), accessCode(0), verbose(0), retCode(0),
            runAsDaemon(false)
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

CliGatewayServiceDescriptorNParams svc;

static void done() {
    if (svc.server) {
        delete svc.server;
        svc.server = nullptr;
        std::cerr << MSG_DONE << std::endl;
        exit(0);
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
    auto gatewayService =
#ifdef ENABLE_SQLITE
        new SqliteGatewayService;
    gatewayService->init(db, nullptr);
#else
        new MemoryGatewayService;
    gatewayService->init("", nullptr);
#endif

    auto serializationWrapper = new GatewaySerialization(gatewayService, svc.code, svc.accessCode);
#ifdef ENABLE_LIBUV
    server = new UVListener(serializationWrapper);
#else
    svc.server = new UDPListener(serializationWrapper);
#endif
    svc.server->setAddress(svc.intf, svc.port);
    svc.server->setLog(svc.verbose, &svc);
	int r = svc.server->run();
    if (r)
        std::cerr << MSG_ERROR << r << ": " << std::endl;
}

int main(int argc, char **argv) {
	struct arg_str *a_interface_n_port = arg_str0(nullptr, nullptr, "ipaddr:port", "Default *:4244");
#ifdef ENABLE_SQLITE
    struct arg_str *a_db = arg_str1("d", "db", "<file>", "database file name");
#endif
    struct arg_int *a_code = arg_int0("c", "code", "<number>", "Default 42. 0x - hex number prefix");
    struct arg_str *a_access_code = arg_str0("a", "access", "<hex>", "Default 2a (42 decimal)");
	struct arg_lit *a_daemonize = arg_lit0("d", "daemonize", "run daemon");
	struct arg_lit *a_verbose = arg_litn("v", "verbose", 0, 2,"-v - verbose, -vv - debug");
	struct arg_lit *a_help = arg_lit0("h", "help", "Show this help");
	struct arg_end *a_end = arg_end(20);

	void* argtable[] = { 
		a_interface_n_port,
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

#ifdef ENABLE_SQLITE
    if (a_db->count)
        db = *a_db->sval;
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
			arg_print_errors(stderr, a_end, progname);
		std::cerr << "Usage: " << progname << std::endl;
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
		char wd[PATH_MAX];
		std::string progpath = getcwd(wd, PATH_MAX);	
		if (svc.verbose)
			std::cerr << MSG_DAEMON_STARTED << progpath << "/" << progname << MSG_DAEMON_STARTED_1 << std::endl;
		OPEN_SYSLOG(progname)
        Daemonize daemon(progname, progpath, run, stop, done);
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
