// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ConsoleManager.h"
#include "ConsoleAPI.h"
#include "UiConsoleManager.h"

#include "Framework.h"

#include "ServiceManager.h"
#include "RenderServiceInterface.h"
#include "LoggingFunctions.h"

#include "MemoryLeakCheck.h"

ConsoleManager::ConsoleManager(Foundation::Framework *fw) :
    framework_(fw),
    ui_initialized_(false),
    console_channel_(new ConsoleChannel(this)),
    log_listener_(new LogListener(this))
{
    command_manager_ = new CommandManager(this, framework_);
//    framework_->AddLogChannel(console_channel_.get());

    Foundation::RenderServiceInterface *renderer = framework_->GetService<Foundation::RenderServiceInterface>();
    if (renderer)
        renderer->SubscribeLogListener(log_listener_);
    else
        LogWarning("ConsoleManager couldn't acquire renderer service: can't subscribe to renderer log listener.");
}

ConsoleManager::~ConsoleManager()
{
    UnsubscribeLogListener();
}

void ConsoleManager::UnsubscribeLogListener()
{
    Foundation::RenderServiceInterface *renderer = framework_->GetService<Foundation::RenderServiceInterface>();
    if (renderer)
        renderer->UnsubscribeLogListener(log_listener_);
    else
        LogWarning("ConsoleManager couldn't acquire renderer service: can't unsubscribe renderer log listener.");

//    framework_->RemoveLogChannel(console_channel_.get());
}

__inline void ConsoleManager::Update(f64 frametime)
{
    command_manager_->Update();
}

__inline void ConsoleManager::Print(const std::string &text)
{
    if (ui_initialized_ && framework_->Console()->uiConsoleManager)
        framework_->Console()->uiConsoleManager->QueuePrintRequest(text.c_str());
    else
        early_messages_.push_back(text);
}

void ConsoleManager::ExecuteCommand(const std::string &command)
{
    if (command_manager_)
        command_manager_->QueueCommand(command);
}

void ConsoleManager::SetUiInitialized(bool initialized)
{
    ui_initialized_ = initialized;
    if (ui_initialized_)
    {
        for(unsigned i=0; i<early_messages_.size();i++)
            Print(early_messages_.at(i));

        early_messages_.clear();
    }
}
