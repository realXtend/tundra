// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Console_ConsoleCommandManager_h
#define incl_Console_ConsoleCommandManager_h

#include "ConsoleCommandUtils.h"
#include "CoreThread.h"

#include <QObject>

#include <queue>

namespace Foundation { class Framework; }

class NativeConsole;
class ConsoleAPI;

class CommandManager : public QObject
{
    Q_OBJECT

public:
    CommandManager(ConsoleAPI *console, Foundation::Framework *fw);
    virtual ~CommandManager();

    virtual void Update();

    virtual void RegisterCommand(const ConsoleCommandStruct &command);

    virtual void UnregisterCommand(const std::string &name);

    /// Queue console command. The command will be called in the console's thread
    /*! @note if a commandline containing the same command gets queued multiple times,
              the most recent command's parameters take precedence and the command
              is only queued once.

        @param commandline string that contains the command and any parameters
    */
    virtual void QueueCommand(const std::string &commandline);

    //! Parse and execute command line. The command is called in the caller's thread. For internal use.
    virtual ConsoleCommandResult ExecuteCommand(const std::string &commandline);
    __inline virtual ConsoleCommandResult ExecuteCommand(const std::string &name, const StringVector &params)
    {
        return ExecuteCommandAlways(name, params, false);
    }

    /// Print out available commands to console
    ConsoleCommandResult ConsoleHelp(const StringVector &params);

    /// Exit application
    ConsoleCommandResult ConsoleExit(const StringVector &params);

    typedef std::map<std::string, ConsoleCommandStruct> CommandMap;

    /// Returns all registered console commands for introspection purposes.
    const CommandMap &Commands() const { return commands_; }

signals:
    /// Emitted when console command with no callback pointer is invoked.
    /** @param command Command name.
        @param params Parameters (optional).
    */
    void CommandInvoked(const QString &command, const QStringList &params = QStringList());

private:
    ConsoleCommandResult ExecuteCommandAlways(const std::string &name, const StringVector &params, bool always);

    typedef std::queue<std::string> StringQueue;
    typedef std::map< std::string, StringVector> CommandParamMap;

    Foundation::Framework *framework_;
    CommandMap commands_; ///< Available commands
    RecursiveMutex commands_mutex_; ///< Mutex for handling registered commands.
    StringQueue commandlines_; ///< Queue of command lines.
    Mutex commandlines_mutex_; ///< Mutex for handling command line queue.
    CommandParamMap delayed_commands_; ///< Map of commands that have delayed execution.
    ConsoleAPI *console_; ///< Console.
    NativeConsole* nativeinput_; ///< Native input for headless mode.
};

#endif
