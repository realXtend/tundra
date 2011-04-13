/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   ConsoleAPI.cpp
 *  @brief  Console core API.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ConsoleAPI.h"
#include "ConsoleManager.h"
#include "CommandManager.h"
#include "UiConsoleManager.h"
#include "ConsoleCommandServiceInterface.h"

#include "Framework.h"
#include "UiAPI.h"
#include "UiGraphicsView.h"
#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("ConsoleAPI")

#include "MemoryLeakCheck.h"

using namespace Console;

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

    Console::Command cmd = { name.toStdString(), desc.toStdString(), Console::CallbackPtr(), false };
    consoleManager->GetCommandManager()->RegisterCommand(cmd);

    // Use UniqueConnection so that we don't have duplicate connections.
    connect(consoleManager->GetCommandManager(), SIGNAL(CommandInvoked(const QString &, const QStringList &)),
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

        Console::Command cmd = { name.toStdString(), desc.toStdString(), Console::CallbackPtr(), false };
        consoleManager->GetCommandManager()->RegisterCommand(cmd);

        // Use UniqueConnection so that we don't have duplicate connections.
        connect(consoleManager->GetCommandManager(), SIGNAL(CommandInvoked(const QString &, const QStringList &)),
            SLOT(InvokeCommand(const QString &, const QStringList &)), Qt::UniqueConnection);
    }
}

void ConsoleAPI::ExecuteCommand(const QString &command)
{
    consoleManager->ExecuteCommand(command.toStdString());
}

void ConsoleAPI::Print(const QString &message)
{
    consoleManager->Print(message.toStdString());
}

void ConsoleAPI::RegisterCommand(const Console::Command &command)
{
    consoleManager->GetCommandManager()->RegisterCommand(command);
}

ConsoleAPI::ConsoleAPI(Foundation::Framework *fw) :
    QObject(fw),
    framework_(fw)
{
    consoleManager = new ConsoleManager(fw);

    UiGraphicsView *ui_view = framework_->Ui()->GraphicsView();
    if (ui_view)
        uiConsoleManager = new Console::UiConsoleManager(framework_, ui_view);

    consoleManager->SetUiInitialized(!consoleManager->IsUiInitialized());

    inputContext = framework_->Input()->RegisterInputContext("Console", 100);
    inputContext->SetTakeKeyboardEventsOverQt(true);
    connect(inputContext.get(), SIGNAL(OnKeyEvent(KeyEvent *)), SLOT(HandleKeyEvent(KeyEvent *)));
}

void ConsoleAPI::ToggleConsole()
{
    if (uiConsoleManager)
        uiConsoleManager->ToggleConsole();
}

void ConsoleAPI::HandleKeyEvent(KeyEvent *keyEvent)
{
    if (keyEvent->keyCode == Qt::Key_F1 && keyEvent->eventType == KeyEvent::KeyPressed)
        uiConsoleManager->ToggleConsole();
}

void ConsoleAPI::Uninitialize()
{
    SAFE_DELETE(uiConsoleManager);
    consoleManager->UnsubscribeLogListener();
    SAFE_DELETE(consoleManager);
}

void ConsoleAPI::InvokeCommand(const QString &name, const QStringList &params) const
{
    QMap<QString, ConsoleCommand *>::const_iterator i = commands_.find(name);
    if (i != commands_.end())
        i.value()->Invoke(params);
}

