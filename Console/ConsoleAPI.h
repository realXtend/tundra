/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   ConsoleAPI.h
 *  @brief  Console core API.
 */

#ifndef incl_Console_ConsoleAPI
#define incl_Console_ConsoleAPI

#include "CoreTypes.h"
#include "InputFwd.h"
#include "ConsoleApiExports.h"

#include <QObject>
#include <QMap>

class Framework;

class LogListener;
class CommandManager;
class ConsoleManager;
class UiConsoleManager;
struct ConsoleCommandStruct;

/// Convenience class for scripting languages
/** Cannot be created directly, created by ConsoleAPI.
*/
class ConsoleCommand: public QObject
{
    Q_OBJECT

    friend class ConsoleAPI;

public:
    /// Returns name of the command.
    QString Name() const { return name_; }

signals:
    /// Emitted when this command is invoked.
    /** @param params Provided parameters, if applicable.
    */
    void Invoked(const QStringList &params);

private:
    Q_DISABLE_COPY(ConsoleCommand);

    /// Constructs new command.
    /** @param name Name of the command, works also as an identifier.
    */
    ConsoleCommand(const QString &name) : name_(name) {}

    /// Name of the command.
    QString name_;

private slots:
    /// Emitted when the console command is executed on the console.
    /** @param params Parameter list, if provided.
    */
    void Invoke(const QStringList &params);
};

/// Console core API.
/** Allows printing text to console, executing console commands programmatically
    and registering new console commands. This object can be created by Framework only.
*/
class ConsoleAPI : public QObject
{
    Q_OBJECT

    friend class Framework;

public:
    /// Destructor.
    ~ConsoleAPI();

    ///\todo Temporary function due to ongoing refactoring. Remove.
    void Uninitialize();

public slots:
    /// Registers new console command and connects its execution signal to receiver object and member slot.
    /** @param name Name of the command.
        @param desc Description of the command.
        @param receiver Receiver object.
        @param member Member slot.
    */
    void RegisterCommand(const QString &name, const QString &desc, const QObject *receiver, const char *member);

    /// This is an overloaded function.
    /** Use this from scripting languages.
        @param name Name of the command.
        @param desc Description of the command.
        @return Pointer to the command. Connect the Invoked() signal to your script slot/func.
        @note Never store the returned pointer.
    */
    ConsoleCommand *RegisterCommand(const QString &name, const QString &desc);

    /// This is an overloaded function.
    /** @note Uses the old ConsoleCommandStruct. This function will be removed at some point and should not be used anymore.
        @param command Console command.
    */
    void RegisterCommand(const ConsoleCommandStruct &command);

    /// Executes console command.
    /** @param command Console command, syntax: "command(param1, param2, param3, ...)".
    */
    void ExecuteCommand(const QString &command);

    /// Prints message to console.
    /** @param message Message to print.
    */
    void Print(const QString &message);

private:
    Q_DISABLE_COPY(ConsoleAPI);

    /// Constructs the console API.
    /** @param fw Framework.
    */
    explicit ConsoleAPI(Framework *fw);

    Framework *framework_; ///< Framework.
    QMap<QString, ConsoleCommand *> commands_; ///< List of registered console commands.
    InputContextPtr inputContext;
    UiConsoleManager *uiConsoleManager;

// START FROM CONSOLEMANAGER
public:
    void Update(f64 frametime);
    void Print_(const std::string &text);
private:
    void UnsubscribeLogListener();
    CommandManager *commandManager; ///< Command manager.
    boost::shared_ptr<LogListener> logListener; ///< Listener to get logs from renderer 
    std::vector<std::string> earlyMessages; ///< This is a buffer for messages generated before actual console UI
// END FROM CONSOLEMANAGER

private slots:
    /// Checks if we have executed console command object stored. If we have, we invoke it.
    /** @param name Name of the command.
        @param params List of parameters, if provided.
    */
    void InvokeCommand(const QString &name, const QStringList &params) const;

    void HandleKeyEvent(KeyEvent *e);
    void ToggleConsole();
};

// START FROM CONSOLEMANAGER
/// loglistener is used to listen log messages from renderer
class LogListener
{
    LogListener();

public:
    explicit LogListener(ConsoleAPI *c): console(c) {}
    virtual ~LogListener() {}

    virtual void LogMessage(const std::string &message){ console->Print(message.c_str()); }
    ConsoleAPI *console;
};

// END FROM CONSOLEMANAGER
#endif
