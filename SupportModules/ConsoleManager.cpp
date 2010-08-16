// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ConsoleManager.h"
#include "ConsoleModule.h"
#include "ConsoleEvents.h"
#include "ModuleInterface.h"
#include "Framework.h"
#include "EventManager.h"
#include "ServiceManager.h"
#include "RenderServiceInterface.h"

#include "MemoryLeakCheck.h"

namespace Console
{
    ConsoleManager::ConsoleManager(Foundation::ModuleInterface *parent) :
        parent_(parent),
        ui_initialized_(false),
        console_channel_(new ConsoleChannel(this)),
        log_listener_(new LogListener(this))
    {
        command_manager_ = CommandManagerPtr(new CommandManager(parent_, this));
        parent_->GetFramework()->AddLogChannel(console_channel_.get());
        console_category_id_ = parent_->GetFramework()->GetEventManager()->RegisterEventCategory("Console");

        Foundation::RenderServiceInterface *renderer = parent_->GetFramework()->GetService<Foundation::RenderServiceInterface>();
        if (renderer)
            renderer->SubscribeLogListener(log_listener_);
        else
            ConsoleModule::LogWarning("ConsoleManager couldn't acquite renderer service: can't subscribe to renderer log listener.");
    }

    ConsoleManager::~ConsoleManager()
    {
        Foundation::RenderServiceInterface *renderer = parent_->GetFramework()->GetService<Foundation::RenderServiceInterface>();
        if (renderer)
            renderer->UnsubscribeLogListener(log_listener_);
        else
            ConsoleModule::LogWarning("ConsoleManager couldn't acquite renderer service: can't unsubscribe renderer log listener.");

        parent_->GetFramework()->RemoveLogChannel(console_channel_.get());
    }

    __inline void ConsoleManager::Update(f64 frametime)
    {
        command_manager_->Update();
    }

    __inline void ConsoleManager::Print(const std::string &text)
    {
        if(ui_initialized_)
        {
            Console::ConsoleEventData* event_data = new Console::ConsoleEventData(text);
            parent_->GetFramework()->GetEventManager()->SendDelayedEvent(console_category_id_,
                Console::Events::EVENT_CONSOLE_PRINT_LINE, Foundation::EventDataPtr(event_data));
        }
        else
        {
            early_messages_.push_back(text);
        }
    }

    void ConsoleManager::ExecuteCommand(const std::string &command)
    {
        if (command_manager_)
            command_manager_->QueueCommand(command);
    }

    void ConsoleManager::SetUiInitialized(bool initialized)
    {
        this->ui_initialized_ = initialized;
        if (ui_initialized_)
        {
            for(int i=0; i<early_messages_.size();i++)
                Print(early_messages_.at(i));

            early_messages_.clear();
        }
    }
}
