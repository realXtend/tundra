// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Console_Native_h
#define incl_Console_Native_h

class Framework;
class CommandManager;

class NativeInput
{
public:
    /// constructor
    NativeInput() : commandManager(0) { }

    /// destructor
    ~NativeInput() {}

    /// (thread) entry point
    void operator()();

    CommandManager *commandManager;

private:
    Q_DISABLE_COPY(NativeInput)
};

/// Native debug input console
class NativeConsole
{
public:
    //! constructor
    NativeConsole(CommandManager *mgr);

    /// destructor
    virtual ~NativeConsole();

private:
    Q_DISABLE_COPY(NativeConsole)

    Thread thread; ///< Input thread
    NativeInput input; ///< Handles input from native console
};

#endif

