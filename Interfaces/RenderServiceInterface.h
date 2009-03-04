// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __incl_Interfaces_RenderServiceInterface_h__
#define __incl_Interfaces_RenderServiceInterface_h__

#include "ServiceInterface.h"

namespace Foundation
{
    class RenderServiceInterface : public ServiceInterface
    {
    public:
        RenderServiceInterface() {}
        virtual ~RenderServiceInterface() {}

        virtual void raycast() = 0;
    };
}

#endif

