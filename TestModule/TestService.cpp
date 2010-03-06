// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "TestService.h"

namespace Test
{
    const service_type_t TestService::type_ = 667;

    bool TestService::DoTest() const
    {
        return true;
    }
}
