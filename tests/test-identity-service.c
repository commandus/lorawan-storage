#include <string.h>

#include "lorawan/storage/service/identity-service-c-wrapper.h"

int main() {
    void *o = createIdentityServiceC(makeIdentityService3());

    c_init(o, "test.sqlite.db", NULL);
    C_DEVADDR devAddr = 0x12345678;
    C_DEVICEID devId;
    strcpy(&devId.name[0], "name");
    c_put(o, &devAddr, &devId);
    c_get(o, &devId, &devAddr);

    C_NETWORKIDENTITY nis[2];
    int c = c_list(o, nis, 0, 2);
    c_rm(o, &devAddr);
    c = c_list(o, nis, 0, 2);

    c_done(o);
    destroyIdentityServiceC(o);
    return 0;
}
