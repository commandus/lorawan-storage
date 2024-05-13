#include "identity-serialization.h"

IdentitySerialization::IdentitySerialization(
    IdentityService* aSvc,
    int32_t aCode,
    uint64_t aAccessCode
)
    : code(aCode), accessCode(aAccessCode), svc(aSvc)
{

}
