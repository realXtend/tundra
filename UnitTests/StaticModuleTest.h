// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TestStaticModuleTest_h
#define incl_TestStaticModuleTest_h

#include "ModuleInterface.h"

namespace Test
{
    class StaticModuleTest : public Foundation::ModuleInterface_Impl
    {
    public:
        StaticModuleTest() : ModuleInterface_Impl("Static_Test") {}
        virtual ~StaticModuleTest() {}

        virtual void Load()
        {
            LogInfo("Module " + Name() + " loaded.");
        }
        virtual void Unload()
        {
            LogInfo("Module " + Name() + " unloaded.");
        }
        virtual void Initialize(Foundation::Framework *framework)
        {
            LogInfo("Module " + Name() + " initialized.");
        }
        virtual void Uninitialize(Foundation::Framework *framework)
        {
            LogInfo("Module " + Name() + " uninitialized.");
        }

        virtual void Update() {}
    };
}
#endif
