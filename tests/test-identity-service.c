#include <string.h>

#include "lorawan/storage/service/identity-service-c-wrapper.h"

int main() {
    // sqlite
    void *o = createIdentityServiceC(makeIdentityService3());

    c_init(o, "test.sqlite.db", NULL);
    C_DEVADDR devAddr = 0x12345678;
    C_DEVICEID devId;
    memset(&devId, 0, sizeof(devId));
    devId.activation = 0;
    devId.deviceclass = 1;
    devId.version = 1;
    devId.devEUI = 0x12345678;
    devId.appEUI = 0x12345678;
    devId.appSKey[0] = 1;
    devId.nwkSKey[0] = 2;
    devId.appKey[0] = 3;
    devId.nwkKey[0] = 4;
    strcpy(&devId.name[0], "name");
    c_put(o, &devAddr, &devId);
    memset(&devId, 0, sizeof(devId));
    c_get(o, &devId, &devAddr);

    C_NETWORKIDENTITY nis[2];
    int c = c_list(o, nis, 0, 2);
    // c_rm(o, &devAddr);
    c = c_list(o, nis, 0, 2);
    C_NETWORK_IDENTITY_FILTER ff = {C_NILPO_AND, C_NIP_ACTIVATION, C_NICO_EQ, 1, 1};
    C_NETWORK_IDENTITY_FILTER filters[3] = {
        { C_NILPO_AND, C_NIP_ACTIVATION, C_NICO_EQ, 1, 0 },
        { C_NILPO_AND, C_NIP_DEVEUI, C_NICO_EQ, sizeof(C_DEVEUI), { 0x78, 0x56, 0x34, 0x12 }},
        { C_NILPO_AND, C_NIP_DEVICE_CLASS, C_NICO_EQ, 1, 1 }
    };
    c = c_filter(o, nis, filters, 3, 0, 2);


    const char *filterExpression = "activation = 'ABP' and deveui = '12345678' and class = 'B'";
    c = c_filterExpression(o, nis, filterExpression, strlen(filterExpression), 0, 2);

    c_done(o);
    destroyIdentityServiceC(o);
    return 0;
}
