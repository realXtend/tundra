// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_RenderServiceInterface_h
#define incl_Interfaces_RenderServiceInterface_h

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

