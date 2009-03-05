// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_TestServiceInterface_h
#define incl_Interfaces_TestServiceInterface_h

#include "ServiceInterface.h"

namespace Foundation
{
    class TestServiceInterface : public ServiceInterface
    {
    public:
        TestServiceInterface() {}
        virtual ~TestServiceInterface() {}

        virtual bool Test() const = 0;
    };
}

#endif // incl_Interfaces_TestServiceInterface_h

