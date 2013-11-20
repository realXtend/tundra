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
    ShellInputThread();
    ~ShellInputThread();
    
    /// Stops the thread.
    void Stop();
    
    /// Gets a new line of input from the input thread if it exists, or an empty string otherwise.
    std::string GetLine();

private slots:
    void ReadShellInput();

protected:
    /// QThread override
    void run();

private:
    bool exiting_;

    QMutex inputQueueLock;
    std::vector<std::string> inputQueue;
};
/** @endcond */
