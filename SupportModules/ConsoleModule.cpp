// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ConsoleModule.h"
#include "ConsoleManager.h"

namespace Console
{
    ConsoleModule::ConsoleModule() : ModuleInterface_Impl(type_static_), framework_(NULL)
    {
        manager_ = ConsolePtr(new ConsoleManager(this));
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

        framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Console, manager_.get());
        framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_ConsoleCommand, static_cast<ConsoleManager*>(manager_.get())->GetCommandManager().get());

        LogInfo("Module " + Name() + " initialized.");
    }

    void ConsoleModule::Update()
    {
       // manager_->Update();
    }

    // virtual 
    void ConsoleModule::Uninitialize(Foundation::Framework *framework)
    {
        framework_->GetServiceManager()->UnregisterService(manager_.get());
        framework_->GetServiceManager()->UnregisterService(static_cast< ConsoleManager* >(manager_.get())->GetCommandManager().get());

        framework_ = NULL;

        LogInfo("Module " + Name() + " uninitialized.");
    }
}

