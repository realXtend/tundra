// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "Native.h"
#include "CommandManager.h"
#include "MemoryLeakCheck.h"

void NativeInput::operator()()
{
#ifndef WINDOWS_APP
    assert(commandManager);
    while(true)
    {
        std::string command_line;
        std::getline(std::cin, command_line);

        boost::this_thread::interruption_point();

        if (std::cin.fail())
        {
            commandManager->QueueCommand("exit");
            break;
        }

        commandManager->QueueCommand(command_line);
    }
#endif
}

// ***********************************************************

NativeConsole::NativeConsole(CommandManager *mgr)
{
    assert(mgr);
    input.commandManager = mgr;
    thread = boost::thread(boost::ref(input));
}

NativeConsole::~NativeConsole()
{
    thread.interrupt();
    //thread.join();
}
