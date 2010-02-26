// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ConsoleCommandManager_h
#define incl_ConsoleCommandManager_h

#include "ConsoleCommandServiceInterface.h"
#include "ConsoleServiceInterface.h"
#include "CoreThread.h"

namespace Foundation
{
    class ModuleInterface;
}

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

        virtual void Update();


        virtual void RegisterCommand(const Console::Command &command);
        virtual void UnregisterCommand(const std::string &name);

        //! Queue console command. The command will be called in the console's thread
        /*! 
            \note if a commandline containing the same command gets queued multiple times,
                  the most recent command's parameters take precedence and the command
                  is only queued once.
            
            \param commandline string that contains the command and any parameters
        */
        virtual void QueueCommand(const std::string &commandline);

        virtual boost::optional<CommandResult> Poll(const std::string &command);

        virtual Console::CommandResult ExecuteCommand(const std::string &commandline);

        //! Execute command
        /*! It is assumed that name and params are trimmed and need no touching

            \param name Name of the command to execute
            \param params Parameters to pass to the command
        */
        __inline virtual Console::CommandResult ExecuteCommand(const std::string &name, const StringVector &params)
        {
            return ExecuteCommandAlways(name, params, false);
        }

        //! Print out available commands to console
        Console::CommandResult ConsoleHelp(const StringVector &params);

        //! Exit application
        Console::CommandResult ConsoleExit(const StringVector &params);

        //! Test command
        Console::CommandResult ConsoleTest(const StringVector &params);

        //! Look command
        Console::CommandResult ConsoleLook(const StringVector &params) { return Console::ResultSuccess("It's dark."); }
    private:
        Console::CommandResult ExecuteCommandAlways(const std::string &name, const StringVector &params, bool always);

        typedef std::map<std::string, Console::Command> CommandMap;
        typedef std::queue<std::string> StringQueue;
        typedef std::map< std::string, StringVector> CommandParamMap;

        //! Available commands
        CommandMap commands_;

        //! mutex for handling registered commands
        RecursiveMutex commands_mutex_;

        //! Queue of command lines
        StringQueue commandlines_;

        //! mutex for handling command line queue
        Mutex commandlines_mutex_;

        //! map of commands that have delayed execution
        CommandParamMap delayed_commands_;

        //! parent module;
        ConsoleModule *parent_;

        //! console
        ConsoleServiceInterface *console_;
    };
}

#endif

