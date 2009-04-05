// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_ScriptServiceInterface_h
#define incl_Interfaces_ScriptServiceInterface_h

#include "ServiceInterface.h"

namespace Foundation
{
    class ScriptServiceInterface : public ServiceInterface
    {
    public:
        ScriptServiceInterface() {}
        virtual ~ScriptServiceInterface() {}

        virtual void RunScript(const std::string& scriptname) = 0;
		virtual void RunString(const std::string& codestr) = 0;
    };
}

#endif

