// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Console_Native_h
#define incl_Console_Native_h

class Framework;
class CommandManager;

class NativeInput
{
public:
    /// constructor
    NativeInput() : command_service_(0) { }

    /// destructor
    ~NativeInput() {}

    /// (thread) entry point
    void operator()();

    void SetCommandManager(CommandManager *command_service) { command_service_ = command_service; assert (command_service_); }
    void SetFramework(Framework *framework) { framework_ = framework; assert (framework_); }

private:
    NativeInput(const NativeInput &other);

    CommandManager *command_service_;
    Framework *framework_;
};

/// Native debug input console
class NativeConsole
{
    NativeConsole();
public:
    /// constructor
    NativeConsole(CommandManager *command_service, Framework *framework);

    /// destructor
    virtual ~NativeConsole();

private:
    Thread thread_; ///< input thread
    NativeInput input_; ///< Handles input from native console
};

#endif

