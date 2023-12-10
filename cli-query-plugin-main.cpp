#include <string>
#include <iostream>

#include <sstream>
#include <cstring>

#include "argtable3/argtable3.h"

#include "cli-helper.h"

#include "plugin-client.h"
#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-msg.h"
#include "lorawan/lorawan-string.h"
#include "log.h"
#include "gateway-identity.h"
#include "ip-address.h"
#include "ip-helper.h"

const char *programName = "lorawan-query-plugin";

// global parameters
class CliQueryParams {
public:
    char tag;
    std::vector<DeviceOrGatewayIdentity> query;
    size_t queryPos;
    int verbose;
    std::string pluginFilePath;
    std::string pluginClassName;
    int32_t code;
    uint64_t accessCode;
    size_t offset;
    size_t size;

    int32_t retCode;

    CliQueryParams()
        : tag(QUERY_GATEWAY_NONE), queryPos(0), verbose(0), code(42), accessCode(42), offset(0), size(0),
          retCode(0)
    {

    }

    std::string toString() {
        std::stringstream ss;
        ss
            << "Plugin: " << pluginFilePath << ":" << pluginClassName << " "
            << "command: " << commandLongName(tag) << ", code: " << std::hex << code << ", access code: "  << accessCode << " "
            << "offset: " << std::dec << offset << ", size: "  << size << "\n";
        for (auto & it : query) {
            if (it.hasDevice) {
                if (!it.nid.devaddr.empty())
                    ss << DEVADDR2string(it.nid.devaddr);
                ss << "\t" << it.nid.devid.toString() << "\n";
            }
            if (it.hasGateway) {
                if (it.gid.gatewayId)
                    ss << std::hex << it.gid.gatewayId;
                ss << "\t" << sockaddr2string(&it.gid.sockaddr) << "\n";
            }
        }
        return ss.str();
    }
};

/**
 * Split @param address e.g. FILE:CLASS to @param retFile and @param retClass
 */
static bool splitFileClass(
    std::string& retFile,
    std::string& retClass,
    const std::string& value
)
{
    size_t pos = value.find_last_of(':');
    if (pos == std::string::npos)
        return false;
    retFile = value.substr(0, pos);
    retClass = value.substr(pos + 1);
    return true;
}


static CliQueryParams params;

#define DEF_PLUGIN  "storage-gen:Gen"

static void run()
{

    PluginClient c(params.pluginFilePath, params.pluginClassName, params.code, params.accessCode);
    if (!c.svcIdentity || !c.svcGateway) {
        params.retCode = ERR_CODE_INSUFFICIENT_PARAMS;
        return;
    }

    for (auto& it : params.query) {
        if (it.hasDevice) {
            c.svcIdentity->get(it.nid.devid, it.nid.devaddr);
            if (!it.nid.devaddr.empty())
                std::cout << DEVADDR2string(it.nid.devaddr);
            std::cout << "\t" << it.nid.devid.toString() << "\n";
        }
        if (it.hasGateway) {
            c.svcGateway->get(it.gid, it.gid);
            if (it.gid.gatewayId)
                std::cout << std::hex << it.gid.gatewayId;
            std::cout << "\t" << sockaddr2string(&it.gid.sockaddr) << "\n";
        }
    }

}

int main(int argc, char **argv) {
    std::string shortCL = shortCommandList('|');
    struct arg_str *a_query = arg_strn(nullptr, nullptr, "<command | gateway-id | plugin:class>", 1, 100,
        shortCL.c_str());
    struct arg_str *a_plugin_file_n_class = arg_str0("s", "service", "<plugin-file:class-prefix>", "Default " DEF_PLUGIN);
    struct arg_int *a_code = arg_int0("c", "code", "<number>", "Default 42. 0x - hex number prefix");
    struct arg_str *a_access_code = arg_str0("a", "access", "<hex>", "Default 2a (42 decimal)");
	struct arg_int *a_offset = arg_int0("o", "offset", "<0..>", "list offset. Default 0. ");
    struct arg_int *a_size = arg_int0("z", "size", "<number>", "list size limit. Default 10. ");
    struct arg_lit *a_verbose = arg_litn("v", "verbose", 0, 2,"-v verbose -vv debug");
    struct arg_lit *a_help = arg_lit0("h", "help", "Show this help");
	struct arg_end *a_end = arg_end(20);

	void* argtable[] = { 
		a_query, a_plugin_file_n_class,
        a_code, a_access_code,
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

    if (!splitFileClass(params.pluginFilePath, params.pluginClassName, 
        (a_plugin_file_n_class->count ? std::string(*a_plugin_file_n_class->sval) : DEF_PLUGIN)))
        return ERR_CODE_COMMAND_LINE;

    params.tag = QUERY_IDENTITY_ADDR;

    params.query.reserve(a_query->count);
    bool queryHasIdentity = false;
    bool queryHasGateway = false;
    for (int i = 0; i < a_query->count; i++) {
        enum IdentityQueryTag identityQueryTag = isIdentityTag(a_query->sval[i]);
        if (identityQueryTag != QUERY_IDENTITY_NONE) {
            params.tag = identityQueryTag;
            queryHasIdentity = true;
            continue;
        }
        enum GatewayQueryTag gatewayTag = isGatewayTag(a_query->sval[i]);
        if (gatewayTag != QUERY_GATEWAY_NONE) {
            params.tag = gatewayTag;
            queryHasGateway = true;
            continue;
        }
        if (queryHasIdentity) {
            DeviceOrGatewayIdentity id;
            id.hasDevice = true;
            switch (params.tag) {
                case QUERY_IDENTITY_ADDR:
                    string2DEVEUI(id.nid.devid.devEUI, a_query->sval[i]);
                    break;
                case QUERY_IDENTITY_EUI:
                    string2DEVADDR(id.nid.devaddr, a_query->sval[i]);
                    break;
                default:
                    if (!string2NETWORKIDENTITY(id.nid, a_query->sval[i])) {
                        return ERR_CODE_PARAM_INVALID;
                    }
            }
            params.query.push_back(id);
        }
        if (queryHasGateway) {
            std::string a;
            uint16_t p;
            DeviceOrGatewayIdentity id;
            id.hasGateway = true;
            if (splitAddress(a, p, a_query->sval[i])) {
                // IP address
                string2sockaddr(&id.gid.sockaddr, a, p);
            } else {
                char *last;
                id.gid.gatewayId = strtoull(a_query->sval[i], &last, 16);
                if (*last)
                    return ERR_CODE_COMMAND_LINE;
            }
            params.query.push_back(id);
        }
    }

    if (params.tag == QUERY_IDENTITY_LIST || params.tag == QUERY_GATEWAY_LIST) {
        DeviceOrGatewayIdentity id;
        if (params.query.empty()) {
            params.query.push_back(id);
        }
        if (a_offset->count) {
            params.offset = (size_t) *a_offset->ival;
        }
        if (a_size->count)
            params.size = (size_t) *a_size->ival;
        else
            params.size = 10;
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
			arg_print_errors(stderr, a_end, programName);
		std::cerr << "Usage: " << programName << std::endl;
		arg_print_syntax(stderr, argtable, "\n");
		std::cerr << "LoRaWAN storage query" << std::endl;
		arg_print_glossary(stderr, argtable, "  %-27s %s\n");
        std::cerr << "Commands:\n" << listCommands() << std::endl;
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return ERR_CODE_COMMAND_LINE;
	}
	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    if (params.verbose) {
        std::cerr << params.toString() << std::endl;
    }

	run();
    return params.retCode;
}
