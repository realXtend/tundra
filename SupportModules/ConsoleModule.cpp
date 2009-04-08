// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ConsoleModule.h"
#include "ConsoleManager.h"
#include "InputEvents.h"

namespace Console
{
    ConsoleModule::ConsoleModule() : ModuleInterfaceImpl(type_static_)
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
    void ConsoleModule::PreInitialize()
    {
        manager_ = ConsolePtr(new ConsoleManager(this));
    }

    // virtual
    void ConsoleModule::Initialize()
    {
        checked_static_cast<ConsoleManager*>(manager_.get())->CreateDelayed();
        framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Console, manager_.get());
        framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_ConsoleCommand, checked_static_cast<ConsoleManager*>(manager_.get())->GetCommandManager().get());

        LogInfo("Module " + Name() + " initialized.");
    }

    void ConsoleModule::Update(Core::f64 frametime)
    {
        assert (manager_);
        manager_->Update();
    }

    // virtual
    bool ConsoleModule::HandleEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if (framework_->GetEventManager()->QueryEventCategory("Input") == category_id)
        {
            if (event_id == Input::Events::SCROLL)
            {
                int rel = checked_static_cast<Input::Events::MouseWheel*>(data)->rel_;
                manager_->Scroll(rel);
                return true;
            }
        }

        return false;
    }

    // virtual 
    void ConsoleModule::Uninitialize()
    {
        framework_->GetServiceManager()->UnregisterService(manager_.get());
        framework_->GetServiceManager()->UnregisterService(checked_static_cast< ConsoleManager* >(manager_.get())->GetCommandManager().get());

        assert (manager_);
        manager_.reset();

        LogInfo("Module " + Name() + " uninitialized.");
    }
}

