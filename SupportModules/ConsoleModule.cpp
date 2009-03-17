// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ConsoleModule.h"

namespace Console
{
    ConsoleModule::ConsoleModule() : ModuleInterface_Impl(type_static_), framework_(NULL)
    {
    }

    ConsoleModule::~ConsoleModule()
    {
    }

    // virtual
    void ConsoleModule::Load()
    {
        LogInfo("Module " + Name() + " loaded.");
    }

    // virtual
    void ConsoleModule::Unload()
    {
        LogInfo("Module " + Name() + " unloaded.");
    }

    // virtual
    void ConsoleModule::Initialize(Foundation::Framework *framework)
    {
        assert (framework);
        framework_ = framework;

        LogInfo("Module " + Name() + " initialized.");
    }

    void ConsoleModule::Update()
    {
    }

    // virtual 
    void ConsoleModule::Uninitialize(Foundation::Framework *framework)
    {
        framework_ = NULL;

        LogInfo("Module " + Name() + " uninitialized.");
    }
}

