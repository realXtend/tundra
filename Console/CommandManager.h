// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Console_ConsoleCommandManager_h
#define incl_Console_ConsoleCommandManager_h

#include "ConsoleCommandUtils.h"
#include "CoreThread.h"

#include <QObject>

#include <queue>

namespace Foundation { class Framework; }

class ConsoleManager;
class NativeConsole;

//! Handles console commands
/*! One can register and execute registered console commands by using this service.
    Commands can be parsed and executed from a commandline string, or executed directly.

    One can register new commands with RegisterCommand() - functions.
    Each command has a name and a short description. Command names are case-insensitive.
    Each command is associated with C++ callback function, the function can be a static
    function or a member function, but it should have the signature:

        ConsoleCommandResult Foo(const StringVector& parameters)

    where parameters contains parameters supplied with the command.

    For threadsafe execution of the callbacks, use QueueCommand() when supplying
    commandlines from the user (only for Console-type of classes), and register commands
    with delayed execution and use Poll() to execute the commands in the caller's
    thread context.
    F.ex.
    \verbatim
        // register command for delayed execution
        RegisterCommand("MyCommand", "My great command", &MyClass::MyFunction, true);
    \endverbatim

        then in MyClass' update function, in thread context other than the main thread
    \verbatim
        void MyClass::Update()
        {
            ConsoleCommandService->Poll("MyCommand"); // If MyCommand was queued previously, it now gets executed.
            // ...
        }
    \endverbatim

    \note All functions should be threadsafe.
    @note nothing is handled very efficiently, but should not be necessary as commands are issued rarely.
*/
class CommandManager : public QObject //: public ConsoleCommandServiceInterface
{
    Q_OBJECT

public:
    CommandManager(ConsoleManager *console, Foundation::Framework *fw);
    virtual ~CommandManager();

    //! Updates the service. Should be called in main thread context. For internal use.
    virtual void Update();

    //! Register a command to the debug console
    /*!
        Shortcut functions are present to make registering easier, see
            ConsoleCommandStruct CreateCommand(const std::string &name, const std::string &description, const ConsoleCallbackPtr &callback, bool delayed)
            ConsoleCommandStruct CreateCommand(const std::string &name, const std::string &description, StaticCallback &static_callback, bool delayed)

        \param command the command to register
    */

    virtual void RegisterCommand(const ConsoleCommandStruct &command);

    //! Unregister console command
    /*! See RegisterCommand()

        \param name Name of the command to unregister
    */
    virtual void UnregisterCommand(const std::string &name);

    /// Queue console command. The command will be called in the console's thread
    /*! @note if a commandline containing the same command gets queued multiple times,
              the most recent command's parameters take precedence and the command
              is only queued once.

        @param commandline string that contains the command and any parameters
    */
    virtual void QueueCommand(const std::string &commandline);

    //! Poll to see if command has been queued and executes it immediately, in the caller's thread context.
    /*! For each possible command, this needs to be called exactly once.
        The command must have been created as 'delayed'.

        \param command name of the command to poll for.
        \return Result of executing the command, 
    */

    virtual boost::optional<ConsoleCommandResult> Poll(const std::string &command);


    //! Parse and execute command line. The command is called in the caller's thread. For internal use.
    virtual ConsoleCommandResult ExecuteCommand(const std::string &commandline);

    /// Execute command
    /** It is assumed that name and params are trimmed and need no touching

        \param name Name of the command to execute
        \param params Parameters to pass to the command
    */
    __inline virtual ConsoleCommandResult ExecuteCommand(const std::string &name, const StringVector &params)
    {
        return ExecuteCommandAlways(name, params, false);
    }

    /// Print out available commands to console
    ConsoleCommandResult ConsoleHelp(const StringVector &params);

    /// Exit application
    ConsoleCommandResult ConsoleExit(const StringVector &params);

    /// Test command
    ConsoleCommandResult ConsoleTest(const StringVector &params);

signals:
    /// Emitted when console command with no callback pointer is invoked.
    /** @param command Command name.
        @param params Parameters (optional).
    */
    void CommandInvoked(const QString &command, const QStringList &params = QStringList());

private:
    ConsoleCommandResult ExecuteCommandAlways(const std::string &name, const StringVector &params, bool always);

    typedef std::map<std::string, ConsoleCommandStruct> CommandMap;
    typedef std::queue<std::string> StringQueue;
    typedef std::map< std::string, StringVector> CommandParamMap;

    Foundation::Framework *framework_;

    CommandMap commands_; ///< Available commands
    RecursiveMutex commands_mutex_; ///< Mutex for handling registered commands.
    StringQueue commandlines_; ///< Queue of command lines.
    Mutex commandlines_mutex_; ///< Mutex for handling command line queue.
    CommandParamMap delayed_commands_; ///< Map of commands that have delayed execution.
    ConsoleManager *console_; ///< Console.
    NativeConsole* nativeinput_; ///< Native input for headless mode.
};

#endif
