// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "ShellInputThread.h"
#include "LoggingFunctions.h"
#include "MemoryLeakCheck.h"

#include <QMutexLocker>
#include <QTimer>
#include <iostream>

ShellInputThread::ShellInputThread() :
    exiting_(false)
{
    // clear std::cin error state in case it has failed previously
    std::cin.clear(); 
}

ShellInputThread::~ShellInputThread()
{
}

void ShellInputThread::Stop()
{
    exiting_ = true;
    std::cin.clear(std::cin.eofbit);
    
    if (isRunning())
    {
        exit();
        /// @todo This will never return cleanly as ReadShellInput blocks the QEventLoop.
        wait(1);
    }
}

void ShellInputThread::run()
{
    QTimer::singleShot(1, this, SLOT(ReadShellInput()));
    exec(); // Starts QThreads QEventLoop and block until exit()
}

void ShellInputThread::ReadShellInput()
{
    /// @todo This will block the QEventLoop of this thread. Find a non blocking replacement to std::getline and poll it.
    for(;;)
    {
        std::string commandLine;
        std::getline(std::cin, commandLine);
        
        if (std::cin.eof()) exiting_ = true;
        if (exiting_) break;
        
        if (std::cin.fail())
        {
            LogError("[ConsoleAPI] Error in std::getline(std::cin, ...)");
            std::cin.clear(); 
            continue;
        }
        if (!commandLine.empty())
        {
            QMutexLocker lock(&inputQueueLock);
            inputQueue.push_back(commandLine);
        }
    }
}

std::string ShellInputThread::GetLine()
{
    std::string input = "";
    QMutexLocker lock(&inputQueueLock);
    if (inputQueue.size() > 0)
    {
        input = inputQueue.front();
        inputQueue.erase(inputQueue.begin());
    }
    return input;
}
