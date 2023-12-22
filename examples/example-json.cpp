/**
 * Example shows how to use device in-memory storage stored in JSON file
 * g++ -o example-json -I.. example-json.cpp -L../cmake-build-debug -llorawan
 */
#include <iostream>
#include "lorawan/storage/service/identity-service-json.h"

int main(int argc, char *argv[])
{
    JsonIdentityService c;
    c.init("identity.json", nullptr);
    std::cout << "Entries: " << c.size() << std::endl;
    // c.flush();
}
