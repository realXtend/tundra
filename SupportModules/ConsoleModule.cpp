// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "ConsoleModule.h"
#include "ConsoleManager.h"
#include "InputEvents.h"
#include "InputServiceInterface.h"
#include "ConsoleEvents.h"
#include "Framework.h"
#include "Profiler.h"
#include "ServiceManager.h"
#include "EventManager.h"

namespace Console
{
    ConsoleModule::ConsoleModule() : ModuleInterfaceImpl(type_static_)
    {
    }

    ConsoleModule::~ConsoleModule()
    {
    }

    // virtual
    void ConsoleModule::PreInitialize()
    {
        manager_ = ConsolePtr(new ConsoleManager(this));
    }

    // virtual
    void ConsoleModule::Initialize()
    {
        framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Console, manager_);
        framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_ConsoleCommand,
            checked_static_cast<ConsoleManager*>(manager_.get())->GetCommandManager());
    }

    void ConsoleModule::PostInitialize()
    {
        consoleEventCategory_ = framework_->GetEventManager()->QueryEventCategory("Console");
        inputEventCategory_ = framework_->GetEventManager()->QueryEventCategory("Input");
    }

    void ConsoleModule::Update(f64 frametime)
    {
        {
            PROFILE(ConsoleModule_Update);
            assert (manager_);
            manager_->Update(frametime);
        }
        RESETPROFILER;
    }

    // virtual
    bool ConsoleModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
    {
        PROFILE(ConsoleModule_HandleEvent);

        if (consoleEventCategory_ == category_id)
        {
            switch(event_id)
            {
            case Console::Events::EVENT_CONSOLE_CONSOLE_VIEW_INITIALIZED:
                manager_->SetUiInitialized(!manager_->IsUiInitialized());
                break;
            case Console::Events::EVENT_CONSOLE_COMMAND_ISSUED:
            {
                Console::ConsoleEventData *console_data = dynamic_cast<Console::ConsoleEventData *>(data);
                manager_->ExecuteCommand(console_data->message);
                break;
            }
            default:
                return false;
            }

            return true;
        }
        else if (inputEventCategory_ == category_id)
        {
            if (event_id == Input::Events::SHOW_DEBUG_CONSOLE)
            {
                manager_->ToggleConsole();
                return true;
            }
        }

        return false;
    }

    // virtual 
    void ConsoleModule::Uninitialize()
    {
        framework_->GetServiceManager()->UnregisterService(manager_);
        framework_->GetServiceManager()->UnregisterService(checked_static_cast< ConsoleManager* >(manager_.get())->GetCommandManager());

        assert (manager_);
        manager_.reset();
    }
}
