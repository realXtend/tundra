// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_ConsoleCommandServiceInterface_h
#define incl_Interfaces_ConsoleCommandServiceInterface_h

#include "ServiceInterface.h"
#include "CoreStringUtils.h"
#include "ConsoleCommand.h"

#include <boost/optional.hpp>

namespace Console
{
    //! Returns a succesful CommandResult. \ingroup DebugConsole_group
    __inline static CommandResult ResultSuccess(const std::string &why = std::string()) { CommandResult result = { true, why, false}; return result; }
    //! Returns a failure CommandResult. \ingroup DebugConsole_group
    __inline static CommandResult ResultFailure(const std::string &why = std::string()) { CommandResult result = { false, why, false}; return result; }
    //! Returns a failure CommandResult, with invalid parameters as the reason. \ingroup DebugConsole_group
    __inline static CommandResult ResultInvalidParameters() { CommandResult result = { false, "Invalid parameters.", false}; return result; }
    //! Returns a delayed CommandResult. \ingroup DebugConsole_group
    __inline static CommandResult ResultDelayed() { CommandResult result = { false, std::string(), true }; return result; }

    //! typedef for static callback
    typedef CommandResult (*StaticCallback)(const StringVector&);

    //! functor for console command callback, member function callback
    template <typename T>
    class Callback : public CallbackInterface
    {
    public:
        typedef CommandResult (T::*CallbackFunction)(const StringVector&);

        //! constructor taking object and member function pointer
        Callback(T *object, CallbackFunction function) : object_(object), function_(function) { }

        //! destructor
        virtual ~Callback() {}

        //! copy constructor
        Callback(const Callback &rhs)
        {
            this->object_ = rhs.object_;
            this->function_ = rhs.function_;
        }
        Callback &operator=(const Callback &rhs)
        {
            if (this != &rhs)
            {
                object_ = rhs.object_;
                function_ = rhs.function_;
            }
            return *this;
        }

        //! Calls the function
        virtual CommandResult operator()(const StringVector &params)
        {
            return (*object_.*function_)(params);
        }       

    private:
        //! pointer to the object
        T *object_;
        //! pointer to the member function
        CallbackFunction function_;
    };

    //! functor for console command callback, for static function callback
    class StaticCallbackFunctor : public CallbackInterface
    {
    public:
        //! constructor taking static function pointer
        StaticCallbackFunctor(StaticCallback &function) : function_(function) { }

        //! destructor
        virtual ~StaticCallbackFunctor() {}

        //! copy constructor
        StaticCallbackFunctor(const StaticCallbackFunctor &rhs)
        {
            this->function_ = rhs.function_;
        }

        StaticCallbackFunctor &operator=(const StaticCallbackFunctor &rhs)
        {
            if (this != &rhs)
            {
                function_ = rhs.function_;
            }
            return *this;
        }

        //! Calls the function
        virtual CommandResult operator()(const StringVector &params)
        {
            return (*function_)(params);
        }       

    private:
        //! pointer to function
        StaticCallback function_;
    };

    //! Creates a console command with member function callback
    /*!
        \ingroup DebugConsole_group

        \param name name of the command
        \param description short description of the command
        \param callback C++ function callback. Use Console::Bind().
        \param delayed is the handling of the command immediate, or delayed
        \return a command
    */
    static Command CreateCommand(const std::string &name, const std::string &description, const CallbackPtr &callback, bool delayed = false)
    {
        Command command = { name, description, callback, delayed };
        return command;
    }

    //! Creates a console command with static function callback
    /*!
        \ingroup DebugConsole_group

        \param name name of the command
        \param description short description of the command
        \param static_callback C++ function callback, static function.
        \param delayed is the handling of the command immediate, or delayed
    */
    static Command CreateCommand(const std::string &name, const std::string &description, StaticCallback &static_callback, bool delayed = false)
    {
        CallbackPtr callback(new StaticCallbackFunctor(static_callback));
        Command command = { name, description, callback, delayed };
        return command;
    }

    //! Bind a member function to a command callback.
    /*!
        \ingroup DebugConsole_group
    */
    template <typename T>
    static CallbackPtr Bind(T *object, typename Callback<T>::CallbackFunction function)
    {
        return CallbackPtr(new Callback<T>(object, function));
    }

    //! Interface for console command service.
    /*! One can register and execute registered console commands by using this service.
        Commands can be parsed and executed from a commandline string, or executed directly.

        One can register new commands with RegisterCommand() - functions.
        Each command has a name and a short description. Command names are case-insensitive.
        Each command is associated with C++ callback function, the function can be a static
        function or a member function, but it should have the signature:

            CommandResult Foo(const StringVector& parameters)

        where parameters contains parameters supplied with the command.

        For threadsafe execution of the callbacks, use QueueCommand() when supplying
        commandlines from the user (only for Console-type of classes), and register commands
        with delayed execution and use Poll() to execute the commands in the caller's
        thread context.
        F.ex.
            \verbatim
RegisterCommand("MyCommand", "My great command", &MyClass::MyFunction, true); // register command for delayed execution
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

        \ingroup Services_group
        \ingroup DebugConsole_group
    */
    class ConsoleCommandServiceInterface : public Foundation::ServiceInterface
    {
    public:
        //! default constructor
        ConsoleCommandServiceInterface() {}

        //! destructor
        virtual ~ConsoleCommandServiceInterface() {}

        //! Update the service. Should be called in main thread context. For internal use.
        virtual void Update() {}

        //! Register a command to the debug console
        /*!
            Shortcut functions are present to make registering easier, see
                Command CreateCommand(const std::string &name, const std::string &description, const CallbackPtr &callback, bool delayed)
                Command CreateCommand(const std::string &name, const std::string &description, StaticCallback &static_callback, bool delayed)

            When registering commands from a module, use Foundation::ModuleInterface::AutoRegisterConsoleCommand().

            \param command the command to register
        */
        virtual void RegisterCommand(const Command &command) = 0;

        //! Unregister console command
        /*! See RegisterCommand()

            \param name Name of the command to unregister
        */
        virtual void UnregisterCommand(const std::string &name) = 0;

        //! Queue console command. The command will be called in the console's thread.
        /*! Normally this is for internal use only and need not be called.

            \param commandline string that contains the command and any parameters
        */
        virtual void QueueCommand(const std::string &commandline) = 0;

        //! Poll to see if command has been queued and executes it immediately, in the caller's thread context.
        /*! For each possible command, this needs to be called exactly once.
            The command must have been created as 'delayed'.

            \param command name of the command to poll for.
            \return Result of executing the command, 
        */
        virtual boost::optional<CommandResult> Poll(const std::string &command) = 0;

        //! Parse and execute command line. The command is called in the caller's thread. For internal use.
        virtual CommandResult ExecuteCommand(const std::string &commandline) = 0;

        //! Execute command. For internal use.
        /*!
            \param name Name of the command to execute
            \param params Parameters to pass to the command
        */
        virtual CommandResult ExecuteCommand(const std::string &name, const StringVector &params) = 0;
    };

    //! \ingroup DebugConsole_group
    typedef ConsoleCommandServiceInterface CommandService;
    //! Shared pointer for command manager. \ingroup DebugConsole_group
    typedef boost::shared_ptr<CommandService> CommandManagerPtr;
}

#endif

