// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ConsoleCommandManager_h
#define incl_ConsoleCommandManager_h

#include "ConsoleCommandServiceInterface.h"
#include "CoreThread.h"

#include <QObject>

#include <queue>

namespace Foundation { class Framework; }

namespace Console
{
    class ConsoleManager;
    class Native;

    /// Handles console commands
    /** see ConsoleCommandServiceInterface for more information

        @note nothing is handled very efficiently, but should not be necessary as commands are issued rarely.
    */
    class CommandManager : public QObject //: public ConsoleCommandServiceInterface
    {
        Q_OBJECT

    public:
        CommandManager(ConsoleManager *console, Foundation::Framework *fw);
        virtual ~CommandManager();

        virtual void Update();
        virtual void RegisterCommand(const Console::Command &command);
        virtual void UnregisterCommand(const std::string &name);

        /// Queue console command. The command will be called in the console's thread
        /*! @note if a commandline containing the same command gets queued multiple times,
                  the most recent command's parameters take precedence and the command
                  is only queued once.

            @param commandline string that contains the command and any parameters
        */
        virtual void QueueCommand(const std::string &commandline);

        virtual boost::optional<CommandResult> Poll(const std::string &command);

        virtual Console::CommandResult ExecuteCommand(const std::string &commandline);

        /// Execute command
        /** It is assumed that name and params are trimmed and need no touching

            \param name Name of the command to execute
            \param params Parameters to pass to the command
        */
        __inline virtual Console::CommandResult ExecuteCommand(const std::string &name, const StringVector &params)
        {
            return ExecuteCommandAlways(name, params, false);
        }

        /// Print out available commands to console
        Console::CommandResult ConsoleHelp(const StringVector &params);

        /// Exit application
        Console::CommandResult ConsoleExit(const StringVector &params);

        /// Test command
        Console::CommandResult ConsoleTest(const StringVector &params);

    signals:
        /// Emitted when console command with no callback pointer is invoked.
        /** @param command Command name.
            @param params Parameters (optional).
        */
        void CommandInvoked(const QString &command, const QStringList &params = QStringList());

    private:
        Console::CommandResult ExecuteCommandAlways(const std::string &name, const StringVector &params, bool always);

        typedef std::map<std::string, Console::Command> CommandMap;
        typedef std::queue<std::string> StringQueue;
        typedef std::map< std::string, StringVector> CommandParamMap;

        Foundation::Framework *framework_;

        CommandMap commands_; ///< Available commands
        RecursiveMutex commands_mutex_; ///< Mutex for handling registered commands.
        StringQueue commandlines_; ///< Queue of command lines.
        Mutex commandlines_mutex_; ///< Mutex for handling command line queue.
        CommandParamMap delayed_commands_; ///< Map of commands that have delayed execution.
        ConsoleManager *console_; ///< Console.
        Native* nativeinput_; ///< Native input for headless mode.
    };
}

#endif

