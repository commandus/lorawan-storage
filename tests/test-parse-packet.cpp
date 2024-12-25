#include <cassert>
#include <sstream>
#include <iostream>
#include "lorawan/lorawan-string.h"

static void testIdentityProp()
{
    NETWORK_IDENTITY_PROPERTY p = string2NETWORK_IDENTITY_PROPERTY("version");
    assert(p == NIP_LORAWAN_VERSION);
    p = string2NETWORK_IDENTITY_PROPERTY("ver");
    assert(p == NIP_NONE);
    p = string2NETWORK_IDENTITY_PROPERTY("name");
    assert(p == NIP_DEVICENAME);
}

std::string NETWORK_IDENTITY_FILTERS2string(
    std::vector<NETWORK_IDENTITY_FILTER> &filters
) {
    bool isFirst = true;
    std::stringstream statement;
    for (auto &f: filters) {
        statement << NETWORK_IDENTITY_FILTER2string(f, isFirst) << ' ';
        isFirst = false;
    }
    return statement.str();
}

static void testFilter() {
    std::vector<NETWORK_IDENTITY_FILTER> filters;
    std::string expression;
    int r;

    /*
    expression = "version <= 1 or ";
    r = string2NETWORK_IDENTITY_FILTERS(filters, expression.c_str(), expression.size());
    assert(filters.size() == 1);
    assert(filters[0].pre == NILPO_AND);
    assert(filters[0].property == NIP_LORAWAN_VERSION);
*/
    filters.clear();
    expression = "version <= 1 or class <> 'A'";
    r = string2NETWORK_IDENTITY_FILTERS(filters, expression.c_str(), expression.size());
    assert(filters.size() == 2);
    assert(filters[0].pre == NILPO_AND);
    assert(filters[0].property == NIP_LORAWAN_VERSION);

    assert(filters[1].pre == NILPO_OR);
    assert(filters[1].property == NIP_DEVICE_CLASS);

    NETWORK_IDENTITY_FILTERS2string(filters);
    std::cout << NETWORK_IDENTITY_FILTERS2string(filters) << std::endl;
}

int main() {
    // testIdentityProp();
    testFilter();
    return 0;
}
