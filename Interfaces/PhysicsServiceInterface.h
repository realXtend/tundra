// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __incl_Interfaces_PhysicsServiceInterface_h__
#define __incl_Interfaces_PhysicsServiceInterface_h__

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

