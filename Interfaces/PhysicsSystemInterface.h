
#ifndef __incl_Interfaces_PhysicsSystemInterface_h__
#define __incl_Interfaces_PhysicsSystemInterface_h__

#include "ServiceInterface.h"

namespace Foundation
{
    class PhysicsSystemInterface : ServiceInterface
    {
    public:
        PhysicsSystemInterface() {}
        virtual ~PhysicsSystemInterface() {}

        virtual void testCollision() = 0;
    };
}

#endif // __incl_Interfaces_PhysicsSystemInterface_h__

