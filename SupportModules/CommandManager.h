// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ConsoleCommandManager_h
#define incl_ConsoleCommandManager_h

#include "ConsoleCommandServiceInterface.h"


namespace Console
{
    class ConsoleModule;

    //! Handles console commands
    /*! see ConsoleCommandServiceInterface for more information

        \note nothing is handled very efficiently, but should not be necessary as commands are issued rarely.
    */
    class CommandManager : public Console::ConsoleCommandServiceInterface
    {
    public:
        //! default constructor
        CommandManager(Foundation::ModuleInterface *parent, ConsoleServiceInterface *console);
        //! destructor
        virtual ~CommandManager();

        //! add time. Should be called in main thread context
        virtual void Update();

        //! Register a command to the debug console
        virtual void RegisterCommand(const Console::Command &command);

        //! Register a command to the debug console
        /*! For binding a member function.
            To bind a member function to the command, use Console::Bind()

            \param name name of the command
            \param description short description of the command
            \param callback function object that gets called when command is executed
            \param delayed If true, handle the command in delayed, threadsafe manner
        */
        virtual void RegisterCommand(const std::string &name, const std::string &description, const CallbackPtr &callback, bool delayed = false)
        {
            Console::Command command = {name, description, callback, delayed};
            RegisterCommand(command);
        }

        //! Register a command to the debug console
        /*! For binding a static function

            \param name name of the command
            \param description short description of the command
            \param callback function object that gets called when command is executed
            \param delayed If true, handle the command in delayed, threadsafe manner
        */
        virtual void RegisterCommand(const std::string &name, const std::string &description, StaticCallback &static_callback, bool delayed = false)
        {
            CallbackPtr callback(new StaticCallbackFunctor(static_callback));
            Console::Command command = {name, description, callback, delayed};
            RegisterCommand(command);
        }

        //! Unregister console command with the specified name
        virtual void UnregisterCommand(const std::string &name);

        //! Queue console command. The command will be called in the console's thread
        /*! 
            \note if a commandline containing the same command gets queued multiple times,
                  the most recent command's parameters take precedence and the command
                  is only queued once.
            
            \param commandline string that contains the command and any parameters
        */
        virtual void QueueCommand(const std::string &commandline);

        //! Poll to see if command has been queued and executes it immediately, in the caller's thread context.
        /*! For each possible command, this needs to be called exactly once.

            \note if the same command has been queued multiple times,
                  the most recent command's parameters take precedence and
                  the command is only executed once.

            \param command name of the command to poll for.
            \return Result of executing the command, 
        */
        virtual boost::optional<CommandResult> Poll(const std::string &command);

        //! Parse and execute command line
        virtual Console::CommandResult ExecuteCommand(const std::string &commandline);

        //! Execute command
        /*! It is assumed that name and params are trimmed and need no touching

            \param name Name of the command to execute
            \param params Parameters to pass to the command
        */
        __inline virtual Console::CommandResult ExecuteCommand(const std::string &name, const Core::StringVector &params)
        {
            return ExecuteCommandAlways(name, params, false);
        }

        //! Print out available commands to console
        Console::CommandResult ConsoleHelp(const Core::StringVector &params);

        //! Exit application
        Console::CommandResult ConsoleExit(const Core::StringVector &params);

        //! Test command
        Console::CommandResult ConsoleTest(const Core::StringVector &params);
    private:
        Console::CommandResult ExecuteCommandAlways(const std::string &name, const Core::StringVector &params, bool always);

        typedef std::map<std::string, Console::Command> CommandMap;
        typedef std::queue<std::string> StringQueue;
        typedef std::map< std::string, Core::StringVector> CommandParamMap;

        //! Available commands
        CommandMap commands_;

        //! mutex for handling registered commands
        Core::RecursiveMutex commands_mutex_;

        //! Queue of command lines
        StringQueue commandlines_;

        //! mutex for handling command line queue
        Core::Mutex commandlines_mutex_;

        //! map of commands that have delayed execution
        CommandParamMap delayed_commands_;

        //! parent module;
        ConsoleModule *parent_;

        //! console
        ConsoleServiceInterface *console_;
    };
}

#endif

