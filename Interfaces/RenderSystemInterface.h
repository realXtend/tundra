// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __incl_Interfaces_RenderSystemInterface_h__
#define __incl_Interfaces_RenderSystemInterface_h__

#include "ServiceInterface.h"

namespace Foundation
{
    class RenderSystemInterface : public ServiceInterface
    {
    public:
        RenderSystemInterface() {}
        virtual ~RenderSystemInterface() {}

        virtual void raycast() = 0;
    };
}

#endif // __incl_Interfaces_RenderSystemInterface_h__

