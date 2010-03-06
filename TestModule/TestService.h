// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TestService_h
#define incl_TestService_h

#include "TestServiceInterface.h"
#include "CoreTypes.h"

namespace Test
{
    //! Unit test for service
    class TestService : public TestServiceInterface
    {
    public:
        TestService() {};
        virtual ~TestService() {};

        virtual bool DoTest() const;

        static const service_type_t type_;
    };
}
#endif
