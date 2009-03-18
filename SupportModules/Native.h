// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ConsoleNative_h
#define incl_ConsoleNative_h

#include "ConsoleServiceInterface.h"


namespace Foundation
{
    class Framework;
}

namespace Console
{
    class NativeInput
    {
    public:
        //! constructor
        NativeInput() : console_(NULL) { }
        //! destructor
        ~NativeInput() {}
        //! (thread) entry point
        void operator()();

        void SetNative(Foundation::Console::ConsoleServiceInterface *console) { console_ = console; }

    public:
        NativeInput(const NativeInput &other);

        Foundation::Console::ConsoleServiceInterface *console_;
    };

    //! Native debug console
    class Native : public Foundation::Console::ConsoleServiceInterface
    {
    public:
        //! default constructor
        Native();
        //! destructor
        virtual ~Native();

        //! Add a command to the debug console
        virtual void RegisterCommand(const Foundation::Console::Command &command);

        //! Parse and execute command line
        /*! 
            Threadsafe
        */
        virtual Foundation::Console::CommandResult ExecuteCommand(const std::string &commandline);

        //! Execute command
        /*! It is assumed that name and params are trimmed and need no touching
            Threadsafe

            \param name Name of the command to execute
            \param params Parameters to pass to the command
        */
        virtual Foundation::Console::CommandResult ExecuteCommand(const std::string &name, const Core::StringVector &params);

        //! Print out available commands to console
        Foundation::Console::CommandResult Help(const Core::StringVector &params);
    private:
        typedef std::map<std::string, Foundation::Console::Command> CommandMap;

        //! Available commands
        CommandMap commands_;

        //! mutex
        Core::Mutex command_mutex_;

        //! input thread
        Core::Thread thread_;

        //! Handles input from native console
        NativeInput input_;
    };
}

#endif

