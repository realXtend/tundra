

#ifndef __incl_Interfaces_ScriptSystemInterface_h__
#define __incl_Interfaces_ScriptSystemInterface_h__

#include "ServiceInterface.h"

namespace Foundation
{
    class ScriptSystemInterface : public ServiceInterface
    {
    public:
        ScriptSystemInterface() {}
        virtual ~ScriptSystemInterface() {}

        virtual void runScript() = 0;
    };
}

#endif // __incl_Interfaces_ScriptSystemInterface_h__

