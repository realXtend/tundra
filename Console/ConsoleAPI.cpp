/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   ConsoleAPI.cpp
 *  @brief  Console core API.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ConsoleAPI.h"
#include "CommandManager.h"
#include "UiConsoleManager.h"
#include "ConsoleCommandUtils.h"

#include "Framework.h"
#include "UiAPI.h"
#include "UiGraphicsView.h"
#include "RenderServiceInterface.h"
#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("ConsoleAPI")

#include "MemoryLeakCheck.h"

void ConsoleCommand::Invoke(const QStringList &params)
{
    emit Invoked(params);
}

ConsoleAPI::~ConsoleAPI()
{
    qDeleteAll(commands_);
}

ConsoleCommand *ConsoleAPI::RegisterCommand(const QString &name, const QString &desc)
{
    if(commands_.contains(name))
    {
        LogWarning("Command " + name.toStdString() + " is already registered.");
        return commands_[name];
    }

    ConsoleCommand *command = new ConsoleCommand(name);
    commands_.insert(name, command);

    ConsoleCommandStruct cmd = { name.toStdString(), desc.toStdString(), ConsoleCallbackPtr(), false };
    commandManager->RegisterCommand(cmd);

    // Use UniqueConnection so that we don't have duplicate connections.
    connect(commandManager, SIGNAL(CommandInvoked(const QString &, const QStringList &)),
        SLOT(InvokeCommand(const QString &, const QStringList &)), Qt::UniqueConnection);

    return command;
}

void ConsoleAPI::RegisterCommand(const QString &name, const QString &desc, const QObject *receiver, const char *member)
{
    if (!commands_.contains(name))
    {
        ConsoleCommand *command = new ConsoleCommand(name);
        commands_.insert(name, command);
        connect(command, SIGNAL(Invoked(const QStringList &)), receiver, member);

        ConsoleCommandStruct cmd = { name.toStdString(), desc.toStdString(), ConsoleCallbackPtr(), false };
        commandManager->RegisterCommand(cmd);

        // Use UniqueConnection so that we don't have duplicate connections.
        connect(commandManager, SIGNAL(CommandInvoked(const QString &, const QStringList &)),
            SLOT(InvokeCommand(const QString &, const QStringList &)), Qt::UniqueConnection);
    }
}

void ConsoleAPI::ExecuteCommand(const QString &command)
{
    commandManager->ExecuteCommand(command.toStdString());
}

void ConsoleAPI::Print(const QString &message)
{
    Print_(message.toStdString());
}

void ConsoleAPI::RegisterCommand(const ConsoleCommandStruct &command)
{
    commandManager->RegisterCommand(command);
}

ConsoleAPI::ConsoleAPI(Foundation::Framework *fw) :
    QObject(fw),
    framework_(fw),
    uiConsoleManager(0),
//START FROM CONSOLEMANAGER
    consoleChannel(new ConsoleChannel(this)),
    logListener(new LogListener(this))
//END FROM CONSOLEMANAGER
{
    if (!fw->IsHeadless())
    {
        uiConsoleManager = new UiConsoleManager(framework_, framework_->Ui()->GraphicsView());
        for(unsigned i=0; i<earlyMessages.size();i++)
            Print(earlyMessages.at(i).c_str());
        earlyMessages.clear();
    }

    inputContext = framework_->Input()->RegisterInputContext("Console", 100);
    inputContext->SetTakeKeyboardEventsOverQt(true);
    connect(inputContext.get(), SIGNAL(KeyEventReceived(KeyEvent *)), SLOT(HandleKeyEvent(KeyEvent *)));

//START FROM CONSOLEMANAGER
    commandManager = new CommandManager(this, framework_);
    framework_->AddLogChannel(consoleChannel.get());

    Foundation::RenderServiceInterface *renderer = framework_->GetService<Foundation::RenderServiceInterface>();
    if (renderer)
        renderer->SubscribeLogListener(logListener);
//        else
//            ConsoleModule::LogWarning("ConsoleManager couldn't acquire renderer service: can't subscribe to renderer log listener.");
//END FROM CONSOLEMANAGER
}

//START FROM CONSOLEMANAGER
void ConsoleAPI::Update(f64 frametime)
{
    commandManager->Update();
}

void ConsoleAPI::Print_(const std::string &text)
{
    if (uiConsoleManager)
        uiConsoleManager->QueuePrintRequest(text.c_str());
    else
        earlyMessages.push_back(text);
}

void ConsoleAPI::UnsubscribeLogListener()
{
    Foundation::RenderServiceInterface *renderer = framework_->GetService<Foundation::RenderServiceInterface>();
    if (renderer)
        renderer->UnsubscribeLogListener(logListener);
//        else
//            ConsoleModule::LogWarning("ConsoleManager couldn't acquire renderer service: can't unsubscribe renderer log listener.");

    framework_->RemoveLogChannel(consoleChannel.get());
}

//END FROM CONSOLEMANAGER

void ConsoleAPI::ToggleConsole()
{
    if (uiConsoleManager)
        uiConsoleManager->ToggleConsole();
}

void ConsoleAPI::HandleKeyEvent(KeyEvent *e)
{
    const QKeySequence &toggleConsole = framework_->Input()->KeyBinding("ToggleConsole", QKeySequence(Qt::Key_F1));
    if (e->sequence == toggleConsole)
        ToggleConsole();
}

void ConsoleAPI::Uninitialize()
{
    SAFE_DELETE(uiConsoleManager);
    UnsubscribeLogListener();
}

void ConsoleAPI::InvokeCommand(const QString &name, const QStringList &params) const
{
    QMap<QString, ConsoleCommand *>::const_iterator i = commands_.find(name);
    if (i != commands_.end())
        i.value()->Invoke(params);
}

