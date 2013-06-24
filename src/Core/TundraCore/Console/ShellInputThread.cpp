// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "ShellInputThread.h"
#include "LoggingFunctions.h"
#include "MemoryLeakCheck.h"

#include <iostream>

ShellInputThread::ShellInputThread()
{
    std::cin.clear(); // clear std::cin error state in case it has failed previously
    start();
}

ShellInputThread::~ShellInputThread()
{
}

void ShellInputThread::run()
{
    for(;;)
    {
        std::string commandLine;
        std::getline(std::cin, commandLine);
        if (std::cin.fail())
        {
            /// @todo Ideally we'd like to use LogError for this, but currently we end up here also when doing regular ShellInputThread teardown.
            LogDebug("ShellInputThread::run: cin failed! Aborting input reading.");
            return;
        }

        inputQueueLock.lock();
        inputQueue.push_back(commandLine);
        inputQueueLock.unlock();
    }
}

std::string ShellInputThread::GetLine()
{
    std::string input = "";
    inputQueueLock.lock();
    if (inputQueue.size() > 0)
    {
        input = inputQueue.front();
        inputQueue.erase(inputQueue.begin());
    }
    inputQueueLock.unlock();
    return input;
}
