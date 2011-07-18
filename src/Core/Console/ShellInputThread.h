// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include <vector>
#include <string>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

class ShellInputThread
{
public:
    /// Starts the thread.
    ShellInputThread();
    /// Stops the thread.
    ~ShellInputThread();

    /// Boost thread entry point.
    void ThreadMain();

    /// Gets a new line of input from the input thread if it exists, or an empty string otherwise.
    std::string GetLine();

private:
    boost::thread inputThread;
    boost::mutex inputQueueLock;
    std::vector<std::string> inputQueue;
};
