// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "ConsoleModule.h"
#include "ConsoleManager.h"
#include "ConsoleEvents.h"
#include "UiConsoleManager.h"

#include "InputEvents.h"
#include "InputServiceInterface.h"
#include "Framework.h"
#include "Profiler.h"
#include "ServiceManager.h"
#include "EventManager.h"
#include "ModuleManager.h"

namespace Console
{
    std::string ConsoleModule::type_name_static_ = "Console";

    ConsoleModule::ConsoleModule() : ModuleInterface(type_name_static_), ui_console_manager_(0)
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

        QGraphicsView *ui_view = GetFramework()->GetUIView();
        if (ui_view)
            ui_console_manager_ = new UiConsoleManager(GetFramework(), ui_view);
    }

    void ConsoleModule::PostInitialize()
    {
        consoleEventCategory_ = framework_->GetEventManager()->QueryEventCategory("Console");
        inputEventCategory_ = framework_->GetEventManager()->QueryEventCategory("Input");

        ui_console_manager_->SendInitializationReadyEvent();
//        KeyEventSignal &keySignal = inputModule->TopLevelInputContext().RegisterKeyEvent(Qt::Key_F1);
    }

    // virtual 
    void ConsoleModule::Uninitialize()
    {
        framework_->GetServiceManager()->UnregisterService(manager_);
        framework_->GetServiceManager()->UnregisterService(checked_static_cast< ConsoleManager* >(manager_.get())->GetCommandManager());
        SAFE_DELETE(ui_console_manager_);
        assert (manager_);
        manager_.reset();
    }

    void ConsoleModule::Update(f64 frametime)
    {
        {
            PROFILE(ConsoleModule_Update);
            assert(manager_);
            manager_->Update(frametime);

            // Read from the global top-level input context for console dropdown event.
            if (framework_->Input().IsKeyPressed(Qt::Key_F1))
                manager_->ToggleConsole();
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
            case Console::Events::EVENT_CONSOLE_TOGGLE:
                ui_console_manager_->ToggleConsole();
                break;
            case Console::Events::EVENT_CONSOLE_PRINT_LINE:
            {
                ConsoleEventData *console_data = dynamic_cast<Console::ConsoleEventData*>(data);
                ui_console_manager_->QueuePrintRequest(QString(console_data->message.c_str()));
                break;
            }
            default:
                return false;
            }

            return true;
        }
        return false;
    }
}
