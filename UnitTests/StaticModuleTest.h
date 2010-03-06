// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TestStaticModuleTest_h
#define incl_TestStaticModuleTest_h

#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"

namespace Test
{
    class StaticModuleTest : public Foundation::ModuleInterfaceImpl
    {
    public:
        StaticModuleTest() : ModuleInterfaceImpl(NameStatic()) {}
        virtual ~StaticModuleTest() {}

        virtual void Load()
        {
            LogInfo("Module " + Name() + " loaded.");
        }
        virtual void Unload()
        {
            LogInfo("Module " + Name() + " unloaded.");
        }
        virtual void Initialize()
        {
            LogInfo("Module " + Name() + " initialized.");
        }
        virtual void Uninitialize()
        {
            LogInfo("Module " + Name() + " uninitialized.");
        }

        virtual void Update(f64 frametime) {}

        MODULE_LOGGING_FUNCTIONS;

        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic()
        { 
            static const std::string name("StaticModuleTest");
            return name;
        }
    };
}
#endif
