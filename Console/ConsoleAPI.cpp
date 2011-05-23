/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   ConsoleAPI.cpp
 *  @brief  Console core API.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ConsoleAPI.h"
#include "UiConsoleManager.h"
#include "Profiler.h"
#include "Framework.h"
#include "InputAPI.h"
#include "UiAPI.h"
#include "UiGraphicsView.h"
#include "LoggingFunctions.h"
#include "FunctionInvoker.h"
#include "ShellInputThread.h"

#include "MemoryLeakCheck.h"

ConsoleAPI::ConsoleAPI(Framework *fw) :
    QObject(fw),
    framework_(fw)
{
    if (!fw->IsHeadless())
        consoleWidget = boost::shared_ptr<ConsoleWidget>(new ConsoleWidget(framework_));

    inputContext = framework_->Input()->RegisterInputContext("Console", 100);
    inputContext->SetTakeKeyboardEventsOverQt(true);
    connect(inputContext.get(), SIGNAL(KeyEventReceived(KeyEvent *)), SLOT(HandleKeyEvent(KeyEvent *)));

    RegisterCommand("help", "Lists all registered commands.",
        this, SLOT(ListCommands()));

    shellInputThread = boost::shared_ptr<ShellInputThread>(new ShellInputThread);
}

QVariant ConsoleCommand::Invoke(const QStringList &params)
{
    QVariant returnValue;

    // If we have a target QObject, invoke it.
    if (target)
    {
        FunctionInvoker fi;
        QString errorMessage;
        fi.Invoke(target, functionName, params, &returnValue, &errorMessage);
        if (!errorMessage.isEmpty())
            LogError("ConsoleCommand::Invoke returned an error: " + errorMessage);
    }

    // Also, there may exist a script-registered handler that implements this console command - invoke it.
    emit Invoked(params);

    return returnValue;
}

ConsoleAPI::~ConsoleAPI()
{
}

ConsoleCommand *ConsoleAPI::RegisterCommand(const QString &name, const QString &desc)
{
    if (commands.find(name) != commands.end())
    {
        LogWarning("ConsoleAPI: Command " + name + " is already registered.");
        return commands[name].get();
    }

    boost::shared_ptr<ConsoleCommand> command = boost::shared_ptr<ConsoleCommand>(new ConsoleCommand(name, desc, 0, ""));
    commands[name] = command;
    return command.get();
}

void ConsoleAPI::RegisterCommand(const QString &name, const QString &desc, QObject *receiver, const char *memberSlot)
{
    if (commands.find(name) != commands.end())
    {
        LogWarning("ConsoleAPI: Command " + name + " is already registered.");
        return;
    }

    boost::shared_ptr<ConsoleCommand> command = boost::shared_ptr<ConsoleCommand>(new ConsoleCommand(name, desc, receiver, memberSlot+1));
    commands[name] = command;
}

/// Splits a string of form "MyFunctionName(param1, param2, param3, ...)" into
/// a commandName = "MyFunctionName" and a list of parameters as a StringList.
void ParseCommand(QString command, QString &commandName, QStringList &parameterList)
{
    command = command.trimmed();
    if (command.isEmpty())
        return;

    int split = command.indexOf("(");
    if (split == -1)
    {
        commandName = command;
        return;
    }

    commandName = command.left(split).trimmed();
    parameterList = command.mid(split+1).split(",");
}

void ConsoleAPI::ExecuteCommand(const QString &command)
{
    PROFILE(ConsoleAPI_ExecuteCommand);

    QString commandName;
    QStringList parameterList;
    ParseCommand(command, commandName, parameterList);
    if (commandName.isEmpty())
        return;

    CommandMap::iterator iter = commands.find(commandName);
    if (iter == commands.end())
    {
        LogError("Cannot find a console command \"" + commandName + "\"!");
        return;
    }

    iter->second->Invoke(parameterList);
}

void ConsoleAPI::Print(const QString &message)
{
    if (consoleWidget)
        consoleWidget->PrintToConsole(message.toStdString().c_str());
}

void ConsoleAPI::ListCommands()
{
    for(CommandMap::iterator iter = commands.begin(); iter != commands.end(); ++iter)
        Print(iter->first + " - " + iter->second->Description());
}

void ConsoleAPI::Update(f64 frametime)
{
    PROFILE(ConsoleAPI_Update);

    std::string input = shellInputThread->GetLine();
    if (input.length() > 0)
        ExecuteCommand(input.c_str());
}

void ConsoleAPI::ToggleConsole()
{
    if (consoleWidget)
        consoleWidget->ToggleConsole();
}

void ConsoleAPI::HandleKeyEvent(KeyEvent *e)
{
    const QKeySequence &toggleConsole = framework_->Input()->KeyBinding("ToggleConsole", QKeySequence(Qt::Key_F1));
    if (e->sequence == toggleConsole)
        ToggleConsole();
}
