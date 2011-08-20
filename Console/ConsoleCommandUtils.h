// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Console_ConsoleCommandUtils_h
#define incl_Console_ConsoleCommandUtils_h

#include "CoreStringUtils.h"
#include "ConsoleCommand.h"

#include <boost/optional.hpp>

//! Returns a successful ConsoleCommandResult. \ingroup DebugConsole_group
__inline static ConsoleCommandResult ConsoleResultSuccess(const std::string &why = std::string()) { ConsoleCommandResult result = { true, why, false}; return result; }
//! Returns a failure ConsoleCommandResult. \ingroup DebugConsole_group
__inline static ConsoleCommandResult ConsoleResultFailure(const std::string &why = std::string()) { ConsoleCommandResult result = { false, why, false}; return result; }
//! Returns a failure ConsoleCommandResult, with invalid parameters as the reason. \ingroup DebugConsole_group
__inline static ConsoleCommandResult ConsoleResultInvalidParameters() { ConsoleCommandResult result = { false, "Invalid parameters.", false}; return result; }
//! Returns a delayed ConsoleCommandResult. \ingroup DebugConsole_group
__inline static ConsoleCommandResult ConsoleResultDelayed() { ConsoleCommandResult result = { false, std::string(), true }; return result; }

//! typedef for static callback
typedef ConsoleCommandResult (*StaticCallback)(const StringVector&);

//! functor for console command callback, member function callback
template <typename T>
class Callback : public ConsoleCallbackInterface
{
public:
    typedef ConsoleCommandResult (T::*CallbackFunction)(const StringVector&);

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
    virtual ConsoleCommandResult operator()(const StringVector &params)
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
class StaticCallbackFunctor : public ConsoleCallbackInterface
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
    virtual ConsoleCommandResult operator()(const StringVector &params)
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
    \param callback C++ function callback. Use ConsoleBind().
    \param delayed is the handling of the command immediate, or delayed
    \return a command
*/
static ConsoleCommandStruct CreateConsoleCommand(const std::string &name, const std::string &description, const ConsoleCallbackPtr &callback, bool delayed = false)
{
    ConsoleCommandStruct command = { name, description, callback, delayed };
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
static ConsoleCommandStruct CreateConsoleCommand(const std::string &name, const std::string &description, StaticCallback &static_callback, bool delayed = false)
{
    ConsoleCallbackPtr callback(new StaticCallbackFunctor(static_callback));
    ConsoleCommandStruct command = { name, description, callback, delayed };
    return command;
}

//! Bind a member function to a command callback.
/*!
    \ingroup DebugConsole_group
*/
template <typename T>
static ConsoleCallbackPtr ConsoleBind(T *object, typename Callback<T>::CallbackFunction function)
{
    return ConsoleCallbackPtr(new Callback<T>(object, function));
}

#endif
