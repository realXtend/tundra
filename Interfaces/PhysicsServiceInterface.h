// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_PhysicsServiceInterface_h
#define incl_Interfaces_PhysicsServiceInterface_h

#include "ServiceInterface.h"

namespace Foundation
{
    class PhysicsServiceInterface : ServiceInterface
    {
    public:
        PhysicsServiceInterface() {}
        virtual ~PhysicsServiceInterface() {}

        virtual void testCollision() = 0;
    };
}

#endif

