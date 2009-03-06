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
            LOG("System " + Name() + " loaded.");
        }
        virtual void Unload()
        {
            LOG("System " + Name() + " unloaded.");
        }
        virtual void Initialize(Foundation::Framework *framework)
        {
            LOG("System " + Name() + " initialized.");
        }
        virtual void Uninitialize(Foundation::Framework *framework)
        {
            LOG("System " + Name() + " uninitialized.");
        }

        virtual void Update() {}
    };
}
#endif
