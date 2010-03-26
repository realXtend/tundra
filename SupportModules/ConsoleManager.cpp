// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ConsoleManager.h"
#include "ConsoleEvents.h"
#include "ModuleInterface.h"
#include "Framework.h"
#include "EventManager.h"
#include "ServiceManager.h"

#include <RenderServiceInterface.h>

namespace Console
{
    ConsoleManager::ConsoleManager(Foundation::ModuleInterface *parent) :
        ui_initialized_(0),
        console_channel_(new ConsoleChannel(this)),
        log_listener_(new LogListener(this))
    {
        parent_ = parent;
        framework_ = parent_->GetFramework();
        command_manager_ = CommandManagerPtr(new CommandManager(parent_, this));
        framework_->AddLogChannel(console_channel_.get());
        console_category_id_ = framework_->GetEventManager()->RegisterEventCategory("Console");

        boost::shared_ptr<Foundation::RenderServiceInterface> renderer = 
        framework_->GetService<Foundation::RenderServiceInterface>(Foundation::Service::ST_Renderer).lock();
        if (renderer)
            renderer->SubscribeLogListener(log_listener_);
    }

    ConsoleManager::~ConsoleManager()
    {
        boost::shared_ptr<Foundation::RenderServiceInterface> renderer = 
            framework_->GetService<Foundation::RenderServiceInterface>(Foundation::Service::ST_Renderer).lock();
        if (renderer)
            renderer->UnsubscribeLogListener(log_listener_);

        framework_->RemoveLogChannel(console_channel_.get());
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
            framework_->GetEventManager()->SendDelayedEvent(console_category_id_, Console::Events::EVENT_CONSOLE_PRINT_LINE, Foundation::EventDataPtr(event_data));
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

    void ConsoleManager::ToggleConsole()
    {
        Console::ConsoleEventData event_data("");
        framework_->GetEventManager()->SendEvent(console_category_id_, Console::Events::EVENT_CONSOLE_TOGGLE, &event_data);
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
