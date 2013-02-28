// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "ShellInputThread.h"
#include "MemoryLeakCheck.h"

#include <iostream>

ShellInputThread::ShellInputThread()
{
    start();
}

ShellInputThread::~ShellInputThread()
{
    terminate();
}

void ShellInputThread::run()
{
    for(;;)
    {
        std::string commandLine;
        std::getline(std::cin, commandLine);
        if (std::cin.fail())
        {
            std::cout << "ShellInputThread cin failed! Killing thread!" << std::endl;
            terminate();
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
