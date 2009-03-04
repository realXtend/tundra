// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __incl_Interfaces_TestServiceInterface_h__
#define __incl_Interfaces_TestServiceInterface_h__

#include "ServiceInterface.h"

namespace Foundation
{
    class TestServiceInterface : public ServiceInterface
    {
    public:
        TestServiceInterface() {}
        virtual ~TestServiceInterface() {}

        virtual bool test() const = 0;
    };
}

#endif // __incl_Interfaces_TestServiceInterface_h__

