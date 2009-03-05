// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TestService_h
#define incl_TestService_h

#include "TestServiceInterface.h"

namespace Test
{
    //! Unit test for service
    class TestService : public Foundation::TestServiceInterface
    {
    public:
        TestService() {};
        virtual ~TestService() {};

        virtual bool Test() const { return true; }
    };
}
#endif
