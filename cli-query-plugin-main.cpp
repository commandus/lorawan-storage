#include <string>
#include <iostream>

#include <sstream>

#include "argtable3/argtable3.h"

#include "cli-helper.h"

#include "lorawan/storage/client/service-client.h"
#include "lorawan/storage/client/plugin-client.h"

#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-msg.h"
#include "lorawan/lorawan-string.h"
#include "log.h"
#include "lorawan/helper/ip-address.h"

const char *programName = "lorawan-query-plugin";

// global parameters
class CliQueryParams {
public:
    char tag;
    std::vector<DeviceOrGatewayIdentity> query;
    size_t queryPos;
    int verbose;
    std::string pluginFilePath;
    std::string pluginIdentityClassName;
    std::string pluginGatewayClassName;
    std::string svcName;
    size_t offset;
    size_t size;

    int32_t retCode;
    std::string masterKey;

    CliQueryParams()
        : tag(QUERY_GATEWAY_NONE), queryPos(0), verbose(0), offset(0), size(0),
          retCode(0)
    {

    }

    std::string toString() {
        std::stringstream ss;
        if (svcName.empty())
            ss << "Plugin: " << pluginFilePath << ":" << pluginIdentityClassName;
        else
            ss << "Direct service: " << svcName;
        ss << " "
            << "command: " << commandLongName(tag)
            << ", offset: " << std::dec << offset << ", size: "  << size << "\n";
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


static CliQueryParams params;

#define DEF_PLUGIN  "json"
#define DEF_MASTERKEY   "masterkey"

static void run()
{
    DirectClient *c;
    if (params.svcName.empty())
        c = new PluginClient(params.pluginFilePath, params.pluginIdentityClassName, params.pluginGatewayClassName);
    else
        c = new ServiceClient(params.svcName);
    if (!c->svcIdentity || !c->svcGateway) {
        std::cerr << ERR_MESSAGE << ERR_CODE_LOAD_PLUGINS_FAILED << ": "
            << ERR_LOAD_PLUGINS_FAILED
            << params.pluginFilePath
            << ":" << params.pluginIdentityClassName << ":" << params.pluginGatewayClassName
            << std::endl;
        params.retCode = ERR_CODE_LOAD_PLUGINS_FAILED;
        return;
    }
    // 0- pass master key to generate keys
    c->svcIdentity->setOption(0, &params.masterKey);

    switch (params.tag) {
        case QUERY_IDENTITY_LIST: {
            std::vector<NETWORKIDENTITY> nids;
            c->svcIdentity->list(nids, params.offset, params.size);
            for (auto &it: nids) {
                std::cout
                    << DEVADDR2string(it.devaddr) << "\t"
                    << it.devid.toString()
                    << std::endl;
            }
        }
            break;
        case QUERY_IDENTITY_COUNT:
            std::cout << c->svcIdentity->size() << std::endl;
            break;
        case QUERY_IDENTITY_ASSIGN:
            for (auto &it: params.query) {
                c->svcIdentity->put(it.nid.devaddr, it.nid.devid);
            }
            break;
        case QUERY_IDENTITY_RM:
            for (auto &it: params.query) {
                c->svcIdentity->rm(it.nid.devaddr);
            }
            break;
        case QUERY_IDENTITY_FORCE_SAVE:
            c->svcIdentity->flush();
            break;
        case QUERY_IDENTITY_CLOSE_RESOURCES:
            c->svcIdentity->done();
            break;
        case QUERY_GATEWAY_LIST: {
            std::vector <GatewayIdentity> gids;
            c->svcGateway->list(gids, params.offset, params.size);
            for (auto &it: gids) {
                std::cout
                    << sockaddr2string(&it.sockaddr) << "\t"
                    << gatewayId2str(it.gatewayId)
                    << std::endl;
            }
        }
            break;
        case QUERY_GATEWAY_COUNT:
            std::cout << c->svcGateway->size() << std::endl;
            break;
        case QUERY_GATEWAY_ASSIGN:
            for (auto &it: params.query) {
                c->svcGateway->put(it.gid);
            }
            break;
        case QUERY_GATEWAY_RM:
            for (auto &it: params.query) {
                c->svcGateway->rm(it.gid);
            }
            break;
        case QUERY_GATEWAY_FORCE_SAVE:
            c->svcGateway->flush();
            break;
        case QUERY_GATEWAY_CLOSE_RESOURCES:
            c->svcGateway->done();
            break;
        case QUERY_IDENTITY_EUI:
        case QUERY_IDENTITY_ADDR:
        case QUERY_GATEWAY_ID:
        case QUERY_GATEWAY_ADDR:
            for (auto &it: params.query) {
                if (it.hasDevice) {
                    c->svcIdentity->get(it.nid.devid, it.nid.devaddr);
                    if (!it.nid.devaddr.empty())
                        std::cout << DEVADDR2string(it.nid.devaddr);
                    std::cout << "\t" << it.nid.devid.toString() << "\n";
                }
                if (it.hasGateway) {
                    c->svcGateway->get(it.gid, it.gid);
                    if (it.gid.gatewayId)
                        std::cout << std::hex << it.gid.gatewayId;
                    std::cout << "\t" << sockaddr2string(&it.gid.sockaddr) << "\n";
                }
            }
            break;
        default:
            break;
    }
    delete c;
}

int main(int argc, char **argv) {
    std::string shortCL = shortCommandList('|');
    struct arg_str *a_query = arg_strn(nullptr, nullptr, "<command | id | address", 1, 100,
        shortCL.c_str());
    struct arg_str *a_plugin_file_n_class = arg_str0("p", "plugin", "<plugin>", "Default " DEF_PLUGIN);
	struct arg_int *a_offset = arg_int0("o", "offset", "<0..>", "list offset. Default 0. ");
    struct arg_int *a_size = arg_int0("z", "size", "<number>", "list size limit. Default 10. ");
    struct arg_str* a_pass_phrase = arg_str0("m", "masterkey", "<pass-phrase>", "Default " DEF_MASTERKEY);
    struct arg_lit *a_verbose = arg_litn("v", "verbose", 0, 2,"-v verbose -vv debug");
    struct arg_lit *a_help = arg_lit0("h", "help", "Show this help");
	struct arg_end *a_end = arg_end(20);

	void* argtable[] = {
        a_query, a_plugin_file_n_class,
        a_offset, a_size, a_pass_phrase, a_verbose,
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

    // try load shared library
    if (!splitFileClass(params.pluginFilePath, params.pluginIdentityClassName, params.pluginGatewayClassName,
        (a_plugin_file_n_class->count ? std::string(*a_plugin_file_n_class->sval) : DEF_PLUGIN))) {
        if (ServiceClient::hasStaticPlugin(*a_plugin_file_n_class->sval)) {
            // "load" from static by name: "json", "gen", "mem", "sqlite"
            params.svcName = *a_plugin_file_n_class->sval;
        } else
            return ERR_CODE_COMMAND_LINE;
    }

    if (a_pass_phrase->count)
        params.masterKey = *a_pass_phrase->sval;
    else
        params.masterKey = DEF_MASTERKEY;

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

   // special case: '--help' takes precedence over error reporting
	if ((a_help->count) || errorCount) {
		if (errorCount)
			arg_print_errors(stderr, a_end, programName);
		std::cerr << "Usage: " << programName << std::endl;
		arg_print_syntax(stderr, argtable, "\n");
		std::cerr << "LoRaWAN storage query" << std::endl;
		arg_print_glossary(stderr, argtable, "  %-27s %s\n");
        std::cerr << "Commands:\n" << listCommands() << std::endl;
        std::cerr << "Plugins:\n" << listPlugins() << std::endl;
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
