// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Test_TestServiceInterface_h
#define incl_Test_TestServiceInterface_h

#include "ServiceInterface.h"

namespace Test
{
    //! Interface for test service
    class TestServiceInterface : public Foundation::ServiceInterface
    {
    public:
        TestServiceInterface() {}
        virtual ~TestServiceInterface() {}

        virtual bool Test() const = 0;
    };
}

#endif

