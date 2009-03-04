// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __incl_Interfaces_ScriptServiceInterface_h__
#define __incl_Interfaces_ScriptServiceInterface_h__

#include "ServiceInterface.h"

namespace Foundation
{
    class ScriptServiceInterface : public ServiceInterface
    {
    public:
        ScriptServiceInterface() {}
        virtual ~ScriptServiceInterface() {}

        virtual void runScript() = 0;
    };
}

#endif

