// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TestStaticModuleDefinitions_h
#define incl_TestStaticModuleDefinitions_h

#include "Foundation.h"
#include "StaticModuleTest.h"
#include "ModuleManager.h"

namespace Test
{
    struct StaticModuleDefinitions
    {
        StaticModuleDefinitions() {}
        ~StaticModuleDefinitions() {}
      
        void operator()(Foundation::Framework *framework)
        {
            assert (framework);
            Foundation::ModuleInterface *module = new StaticModuleTest;
            framework->GetModuleManager()->DeclareStaticModule(module);
        }
    };
}
#endif
