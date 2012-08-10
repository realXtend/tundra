// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "ShellInputThread.h"
#include "MemoryLeakCheck.h"

#include <iostream>

ShellInputThread::ShellInputThread()
{
    inputThread = boost::thread(boost::bind(&ShellInputThread::ThreadMain, this));
}

ShellInputThread::~ShellInputThread()
{
    inputThread.interrupt();
}

void ShellInputThread::ThreadMain()
{
    while(true)
    {
        std::string commandLine;
        std::getline(std::cin, commandLine);

        boost::this_thread::interruption_point();

        if (std::cin.fail())
        {
            std::cout << "NativeInputThread cin failed! Killing thread!" << std::endl;
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
