/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   ConsoleAPI.h
    @brief  Console core API. */

#pragma once

#include "TundraCoreApi.h"
#include "CoreTypes.h"
#include "CoreStringUtils.h"
#include "InputFwd.h"

#include <QPointer>
#include <QObject>
#include <QMap>

class QFile;
class QTextStream;
class Framework;

class ConsoleWidget;
class ShellInputThread;
class ConsoleCommand;

/// Console core API.
/** Allows printing text to console, executing console commands programmatically and registering new console commands.
    @note Console commands are case-insensitive. */
class TUNDRACORE_API ConsoleAPI : public QObject
{
    Q_OBJECT

public:
    explicit ConsoleAPI(Framework *fw);
    ~ConsoleAPI();

    /// Called by Framework, do not call from elsewhere.
    void Update(f64 frametime);

    typedef std::map<QString, shared_ptr<ConsoleCommand>, QStringLessThanNoCase> CommandMap;

    /// Returns all command for introspection purposes.
    const CommandMap &Commands() const { return commands; }

    /// Erases all registered console commands and stops the native input thread.
    void Reset();

public slots:
    /// Registers a new console command which invokes a slot on the specified QObject.
    /** @param name The function name to use for this command.
        @param desc A help description of this command.
        @param receiver The QObject instance that will be invoked when this command is executed.
        @param memberSlot A slot identifier on the receiver QObject that is to be called when this command is executed.
            Use the SLOT() macro to specify this parameter, exactly like when using the QObject connect() function.
        @param memberSlotDefaultArgs Additional identifier for a slot that has default arguments.
            This is invoked in case the number of parameters provided for console command is less than the required amount.
            Use the SLOT() macro to specify this parameter, exactly like when using the QObject connect() function. This argument is optional.
        @see UnregisterCommand */
    void RegisterCommand(const QString &name, const QString &desc, QObject *receiver, const char *memberSlot, const char *memberSlotDefaultArgs = 0);

    /// Registers a new console command which triggers a signal when executed.
    /** Use this function from QtScript to implement custom console commands from a script.
        @param name The function name to use for this command.
        @param desc A help description of this command.
        @return This function returns a pointer to the newly created ConsoleCommand data structure.
                Connect the Invoked() signal of this structure to your script slot/function.
        @note Never store the returned ConsoleCommand pointer to an internal variable. The ownership of this object is retained by ConsoleAPI.
        @see UnregisterCommand */
    ConsoleCommand *RegisterCommand(const QString &name, const QString &desc);

    /// Unregister console command.
    /** @param name Name of the command. */
    void UnregisterCommand(const QString &name);

    /// Executes a console command.
    /** @param command Console command, syntax: "command(param1, param2, param3, ...)". */
    void ExecuteCommand(const QString &command);

    /// Prints a message to the console widget's log and stdout.
    /** @param message The text message to print. */
    void Print(const QString &message);

    /// Lists all console commands and their descriptions to the log.
    /** This command is invoked by typing 'help' to the console. */
    void ListCommands();

    /// Clears the console log.
    /** This command is invoked by typing 'clear' to the console. */
    void ClearLog();

    /// Sets the current log level.
    /// @param level One of "error, warning, info, debug".
    /// @note This function calls SetEnabledLogChannels with one of the above four predefined combinations. It is possible to further customize the set of 
    /// active log channels by directly calling the SetEnabledLogChannels function with an appropriate bitset.
    void SetLogLevel(const QString &level);

    /// Starts logging to the given file.
    /// By default at startup, logging to file is not enabled.
    /// @param filename The file to log the output to. Passing an empty string will stop logging altogether.
    ///    The filename string accepts some special symbols:
    ///    $(CWD) is expanded to the current working directory.
    ///    $(INSTDIR) is expanded to the Tundra installation directory (Application::InstallationDirectory)
    ///    $(USERDATA) is expanded to Application::UserDataDirectory.
    ///    $(USERDOCS) is expanded to Application::UserDocumentsDirectory.
    ///    $(DATE:format) is expanded to show the current time, in this format http://doc.qt.nokia.com/latest/qdatetime.html#toString .
    ///    E.g. $(DATE:yyyyMMdd) gives something like "20110905".
    void SetLogFile(const QString &filename);

    /// Log printing funtionality for scripts.
    void LogInfo(const QString &message);
    void LogWarning(const QString &message);
    void LogError(const QString &message);
    void LogDebug(const QString &message);
    void Log(u32 logChannel, const QString &message);

    /// Sets the new currently enabled log channels. Messages at the given channels will be printed, and others channels will be disabled.
    void SetEnabledLogChannels(u32 newChannels);
    /// Returns true if the given log channel is enabled.
    bool IsLogChannelEnabled(u32 logChannel) const;
    /// Returns the bitset of currently enabled log channels.
    u32 EnabledLogChannels() const;

	void ToggleConsole();

private:
    Framework *framework;
    CommandMap commands; ///< Stores all the registered console commands.
    InputContextPtr inputContext;
    QPointer<ConsoleWidget> consoleWidget;
    shared_ptr<ShellInputThread> shellInputThread;
    u32 enabledLogChannels; ///< Stores the set of currently active log channels.
    QFile *logFile; ///< Points to the currently open text file for logging.
    QTextStream *logFileText;

private slots:
    void HandleKeyEvent(KeyEvent *e);
};

/// Represents a registered console command.
class TUNDRACORE_API ConsoleCommand : public QObject
{
    Q_OBJECT

public:
    /// Creates new console command. For internal usage of ConsoleAPI.
    ConsoleCommand(const QString &commandName, const QString &desc, QObject *targetObj, const QString &funcName, const QString &funcNameDefaultArgs) :
        name(commandName),
        description(desc),
        target(targetObj),
        functionName(funcName),
        functionNameDefaultArgs(funcNameDefaultArgs)
    {
    }

    /// Returns the name of this command.
    QString Name() const { return name; }

    /// Returns the description of this command.
    QString Description() const { return description; }

    /// Returns the QObject instance which gets invoked when this command is executed.
    /// This may be null if this ConsoleCommand is registered from a script.
    QObject *Target() const { return target; }

    /// Returns the name of the function on the Target QObject that is invoked when this command is executed.
    /// This may be "" if this ConsoleCommand is registered from a script.
    QString FunctionName() const { return functionName; }

    /// Executes this command.
    /// @return The return value of the console command is returned.
    QVariant Invoke(const QStringList &params);

signals:
    /// Emitted when this command is invoked.
    /** Hook onto this signal in QtScript to implement custom QtScript-based console commands.
        @param params The list of parameters passed as an input to this command. May be an empty list. */
    void Invoked(const QStringList &params);

private:
    QString name;
    QString description;
    QPointer<QObject> target;
    QString functionName;
    QString functionNameDefaultArgs;
};
