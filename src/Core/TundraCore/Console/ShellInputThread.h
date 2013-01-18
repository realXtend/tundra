// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include <vector>
#include <string>

#include <QThread>
#include <QMutex>

/// Retrieves input from the native console/shell.
/** @cond PRIVATE */
class ShellInputThread : public QThread
{
    Q_OBJECT

public:
    /// Starts the thread.
    ShellInputThread();
    /// Stops the thread.
    ~ShellInputThread();
    /// Gets a new line of input from the input thread if it exists, or an empty string otherwise.
    std::string GetLine();

private:
    /// QThread override
    void run();

    QMutex inputQueueLock;
    std::vector<std::string> inputQueue;
};
/** @endcond */
