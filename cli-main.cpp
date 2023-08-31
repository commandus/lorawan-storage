#include <string>
#include <iostream>
#include <csignal>
#include <climits>

#ifdef _MSC_VER
#include <direct.h>
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

const char *progname = "lorawan-gateway-storage";

#define MSG_DONE            	"Stopped gracefully"
#define MSG_ERROR            	"Error "
#define MSG_INTERRUPTED     	"Interrupted"
#define MSG_DAEMON_STARTED  	"Started"
#define MSG_DAEMON_STARTED_1 	". Check syslog."

GatewayListener *server = nullptr;
std::string intf;
uint16_t port;
int32_t code;
uint64_t accessCode;
#ifdef ENABLE_SQLITE
std::string db;
#endif

static void done() {
  if (server) {
    delete server;
    server = nullptr;
    std::cerr << MSG_DONE << std::endl;
    exit(0);
  }
}

static void stop() {
	if (server) {
		server->stop();
	}
}

void signalHandler(int signal)
{
	if (signal == SIGINT) {
		std::cerr << MSG_INTERRUPTED << std::endl;
        done();
	}
}

#ifdef _MSC_VER
// TODO
void setSignalHandler()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
}

#else
void setSignalHandler()
{
	struct sigaction action = {};
	action.sa_handler = &signalHandler;
	sigaction(SIGINT, &action, nullptr);
	sigaction(SIGHUP, &action, nullptr);
}
#endif

void run() {
    auto gatewayService =
#ifdef ENABLE_SQLITE
        new SqliteGatewayService;
    gatewayService->init(db, nullptr);
#else
        new MemoryGatewayService;
    gatewayService->init("", nullptr);
#endif

    auto serializationWrapper = new GatewaySerialization(gatewayService, code, accessCode);
#ifdef ENABLE_LIBUV
    server = new UVListener(serializationWrapper);
#else
    server = new UDPListener(serializationWrapper);
#endif
    server->setAddress(intf, port);
	int r = server->run();
    if (r)
        std::cerr << MSG_ERROR << r << ": " << std::endl;
}

static void parseAddress(
    std::string &retHost,
    uint16_t &retPort,
    const std::string &value
) {
    size_t pos = value.find(':');
    if (pos != std::string::npos) {
        retHost = value.substr(0, pos);
        std::string sport = value.substr(pos + 1);
        retPort = (uint16_t) std::stoi(sport);
    }
}

int main(int argc, char **argv) {
	struct arg_str *a_interface_n_port = arg_str0(nullptr, nullptr, "ipaddr:port", "Default *:4242");
#ifdef ENABLE_SQLITE
    struct arg_str *a_db = arg_str1("d", "db", "<file>", "database file name");
#endif
    struct arg_int *a_code = arg_int0("c", "code", "<number>", "Default 42. 0x - hex number prefix");
    struct arg_str *a_access_code = arg_str0("a", "access", "<hex>", "Default 2a (42 decimal)");
	struct arg_lit *a_daemonize = arg_lit0("d", "daemonize", "run daemon");
	struct arg_lit *a_verbose = arg_litn("v", "verbose", 0, 1,"print errors/warnings");
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

	bool runAsDaemon = a_daemonize->count > 0;
	int verbose = a_verbose->count;

	if (a_interface_n_port->count) {
        parseAddress(intf, port, std::string(*a_interface_n_port->sval));
    } else {
        intf = "*";
        port = 4242;
    }

#ifdef ENABLE_SQLITE
    if (a_db->count)
        db = *a_db->sval;
#endif
    if (a_code->count)
        code = *a_code->ival;
    else
        code = 42;

    if (a_access_code->count)
        accessCode = strtoull(*a_access_code->sval, nullptr, 16);
    else
        accessCode = 42;

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

	if (runAsDaemon) {
		char wd[PATH_MAX];
		std::string progpath = getcwd(wd, PATH_MAX);	
		if (verbose)
			std::cerr << MSG_DAEMON_STARTED << progpath << "/" << progname << MSG_DAEMON_STARTED_1 << std::endl;
		OPEN_SYSLOG(progname)
        Daemonize daemon(progname, progpath, run, stop, done);
		// CLOSESYSLOG()
	} else {
		setSignalHandler();
		run();
		done();
	}
}
