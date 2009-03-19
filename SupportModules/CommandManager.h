// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ConsoleCommandManager_h
#define incl_ConsoleCommandManager_h

#include "ConsoleCommandServiceInterface.h"


namespace Console
{
    class ConsoleModule;

    //! Native debug console
    class CommandManager : public Console::ConsoleCommandServiceInterface
    {
    public:
        //! default constructor
        CommandManager(Foundation::ModuleInterface *parent, ConsoleServiceInterface *console);
        //! destructor
        virtual ~CommandManager();

        __inline void Update()
        {

        }

        //! Add a command to the debug console
        virtual void RegisterCommand(const Console::Command &command);

        virtual void QueueCommand(const std::string &commandline) {}

        //! Parse and execute command line
        /*! 
            Threadsafe
        */
        virtual Console::CommandResult ExecuteCommand(const std::string &commandline);

        //! Execute command
        /*! It is assumed that name and params are trimmed and need no touching
            Threadsafe

            \param name Name of the command to execute
            \param params Parameters to pass to the command
        */
        virtual Console::CommandResult ExecuteCommand(const std::string &name, const Core::StringVector &params);

        //! Print out available commands to console
        Console::CommandResult Help(const Core::StringVector &params);

        //! Exit application
        Console::CommandResult Exit(const Core::StringVector &params);
    private:
        typedef std::map<std::string, Console::Command> CommandMap;
        typedef std::queue<std::string> StringQueue;

        //! Available commands
        CommandMap commands_;

        //! Queue of command lines
        StringQueue command_lines;

        //! mutex
        Core::Mutex command_mutex_;

        //! parent module;
        ConsoleModule *parent_;

        //! console
        ConsoleServiceInterface *console_;
    };
}

#endif

