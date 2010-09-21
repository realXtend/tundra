/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   Console.cpp
 *  @brief  Exposes debug console functionality ot scripting languages.
 *  @note   Currently just simple wrapper class but the idea is to refactor
 *          whole console system later on.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "Console.h"
#include "Framework.h"
#include "ConsoleServiceInterface.h"
#include "ConsoleCommandServiceInterface.h"

#include "MemoryLeakCheck.h"

void ScriptConsole::RegisterCommand(const QString &name, const QObject *receiver, const char *member)
{
//    Console::ConsoleCommandServiceInterface *console = framework_->GetService<Console::ConsoleCommandServiceInterface>();
//    
}

void ScriptConsole::ExecuteCommand(const QString &command)
{
    Console::ConsoleServiceInterface *console = framework_->GetService<Console::ConsoleServiceInterface>();
    if (console)
        console->ExecuteCommand(command.toStdString());
}

void ScriptConsole::Print(const QString &message)
{
    Console::ConsoleServiceInterface *console = framework_->GetService<Console::ConsoleServiceInterface>();
    if (console)
        console->Print(message.toStdString());
}

ScriptConsole::ScriptConsole(Foundation::Framework *fw) : QObject(fw), framework_(fw)
{
}
