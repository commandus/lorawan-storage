#include <string>
#include <iostream>
#include <sstream>
#include <vector>

#include "argtable3/argtable3.h"
#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-msg.h"
#include "lorawan/storage/serialization/urn-helper.h"
#include "lorawan/lorawan-string.h"

const char *programName = "lorawan-tag";

// i18n
// #include <libintl.h>
// #define _(String) gettext (String)
#define _(String) (String)

// global parameters
class CliTagParams {
public:
    std::string join_eui;
    std::string dev_eui;
    std::string profile_id;
    std::string owner_token;
    std::string serial_number;
    std::vector<std::string> proprietary;
    bool crc;
    bool qr;
    int verbose;
    int32_t retCode;

    CliTagParams()
        : crc(false), qr(false), verbose(0), retCode(0)
    {

    }
};

static CliTagParams params;

static int32_t printURN(
    std::string &retVal,
    const CliTagParams &p
) {
    NETWORKIDENTITY networkIdentity;
    string2DEVEUI(networkIdentity.devid.appEUI, p.join_eui);
    string2DEVEUI(networkIdentity.devid.devEUI, p.dev_eui);
    memmove(networkIdentity.devid.name.c, p.profile_id.c_str(), p.profile_id.size() > 8 ? 8 : p.profile_id.size());
    retVal = NETWORKIDENTITY2URN(
        networkIdentity,
        p.owner_token,
        p.serial_number,
        false,
        p.crc,
        &p.proprietary
    );

    return CODE_OK;
}

static int32_t printQR(
    std::string &retVal,
    const CliTagParams &p
) {
    std::string r;
    int32_t c = printURN(r, p);
    if (!c) {
        retVal = "QR: " + r;
    }
    return c;
}

static void run()
{
    std::string r;
    if (params.qr)
        params.retCode = printQR(r, params);
    else
        params.retCode = printURN(r, params);
    if (params.retCode)
        std::cerr << ERR_MESSAGE << params.retCode << std::endl;
    else
        std::cout << r << std::endl;
}

int main(int argc, char **argv) {
    struct arg_str *a_join_eui = arg_str1("j", "join-eui", _("<hex>"), _("8 bytes (16 hex digits)"));
    struct arg_str *a_dev_eui = arg_str1("d", "dev-eui", _("<hex>"), _("8 bytes (16 hex digits)"));
    struct arg_str *a_profile_id = arg_str1("i", "profile-id", _("<hex>"), _("4 bytes (8 hex digits)"));
    struct arg_str *a_owner_token = arg_str0("o", "owner-token", _("<string>"), _("Owner token"));
    struct arg_str* a_serial_number = arg_str0("s", "serial-number", _("<string>"), _("Serial number"));
    struct arg_str* a_proprietary = arg_strn("p", "proprietary", _("<string>"), 0, 50, _("Proprietary value"));
    struct arg_lit *a_crc = arg_lit0("c", "crc", _("add CRC-16"));
    struct arg_lit *a_qr = arg_lit0("q", "qr", _("print QR code"));
    struct arg_lit *a_verbose = arg_litn("v", "verbose", 0, 2, _("-v verbose -vv debug"));
    struct arg_lit *a_help = arg_lit0("h", "help", _("Show this help"));
	struct arg_end *a_end = arg_end(20);

	void* argtable[] = {
		a_join_eui, a_dev_eui, a_profile_id,
        a_owner_token, a_serial_number, a_proprietary,
        a_crc, a_qr, a_verbose,
		a_help, a_end 
	};

	// verify the argtable[] entries were allocated successfully
	if (arg_nullcheck(argtable) != 0) {
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return ERR_CODE_COMMAND_LINE;
	}
	// Parse the command line as defined by argtable[]
	int errorCount = arg_parse(argc, argv, argtable);

    params.join_eui = *a_join_eui->sval;
    params.dev_eui = *a_dev_eui->sval;
    params.profile_id = *a_profile_id->sval;
    if (a_owner_token->count)
        params.owner_token = *a_owner_token->sval;
    if (a_serial_number->count)
        params.serial_number = *a_serial_number->sval;
    params.crc = a_crc->count > 0;
    params.qr = a_qr->count > 0;
    params.verbose = a_verbose->count;
    for (int i = 0; i < a_proprietary->count; i++) {
        params.proprietary.emplace_back(a_proprietary->sval[i]);
    }

    // special case: '--help' takes precedence over error reporting
	if ((a_help->count) || errorCount) {
		if (errorCount)
			arg_print_errors(stderr, a_end, programName);
		std::cerr << _("Usage: ") << programName << std::endl;
		arg_print_syntax(stderr, argtable, "\n");
		std::cerr << _("Print LoRaWAN device tag") << std::endl;
		arg_print_glossary(stderr, argtable, "  %-27s %s\n");
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return ERR_CODE_COMMAND_LINE;
	}
	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));

    run();

    return params.retCode;
}
