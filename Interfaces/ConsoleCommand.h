// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_ConsoleCommand_h
#define incl_Interfaces_ConsoleCommand_h

#include <string>

namespace Console
{
    //! A result from executing a console command.
    /*! A callback function for console command should return an instance.
        
        \ingroup DebugConsole_group
    */
    struct CommandResult
    {
        //! Set to true if command was completed succesfully, false otherwise
        bool success_;
        //! Print out reason for failure (or success).
        std::string why_;
        //! True for delayed execution. For internal use, this doesn't need to be set normally
        bool delayed_;
    };

    //! Interface for console command callback
    class CallbackInterface
    {
    public:
        CallbackInterface() {}
        virtual ~CallbackInterface() {}

        //! Calls the function
        virtual CommandResult operator()(const StringVector &params) = 0;
    };
    typedef boost::shared_ptr<CallbackInterface> CallbackPtr;

    //! A console command
    /*!
        \ingroup DebugConsole_group
    */
    struct Command
    {
        //! internal name for the command, case insensitive
        std::string name_;
        //! short description of the command
        std::string description_;
        //! callback for the command
        CallbackPtr callback_;
        //! is the handling of the command immediate, or delayed
        bool delayed_;
    };
}

#endif

