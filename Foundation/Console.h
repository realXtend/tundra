/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   Console.h
 *  @brief  Exposes debug console functionality to scripting languages.
 *
 *          Allows printing text to console, executing console commands programmatically
 *          and registering new console commands.
 *
 *  @note   Currently just simple wrapper class but the idea is to refactor
 *          whole console system later on.
 */

#ifndef incl_Foundation_Console
#define incl_Foundation_Console

#include "CoreTypes.h"

#include <QObject>
#include <QMap>

namespace Foundation
{
    class Framework;
}

/// Convenience class so that scripting languages can connect their slots/functions
/// easily when registering console commands.
/** Cannot be created direcly, created by ScripConsole.
*/
class Command: public QObject
{
    Q_OBJECT

    friend class ScriptConsole;

public:
    /// Returns name of the command.
    QString Name() const { return name_; }

signals:
    /// Emitted when this command is invoked.
    /** @param params Provided parameters, if applicable.
    */
    void Invoked(QStringList params);

private:
    Q_DISABLE_COPY(Command);

    /// Constructs new command.
    /** @param name Name of the command, works also as a indentifier.
    */
    Command(const QString &name) : name_(name) {}

    /// Name of the command.
    QString name_;

private slots:
    /// Emitted when the console command is executed on the console.
    /** @param params Parameter list, if provided.
    */
    void Invoke(const QStringList &params);
};

/// Exposes debug console functionality to scripting languages.
/** Allows printing text to console, executing console commands programmatically
    and registering new console commands. This object can be created by Framework only.

    @note Currently just simple wrapper class but the idea is to refactor
    the whole console system later on.
*/
class ScriptConsole : public QObject
{
    Q_OBJECT

    friend class Foundation::Framework;

public:
    /// Destructor.
    ~ScriptConsole();

public slots:
    /// Use this from scripting languages.
    /** @param name Name of the command.
        @param desc Description of the command.
        @return Pointer to the command. Connect the Invoked() signal to your script slot/func.,
        or null if the command already existed.
        @note Never store the returned pointer.
    */
    Command *RegisterCommand(const QString &name, const QString &desc);

    /// Registers new console command and connects it's execution signal to receiver object and member slot.
    /// Use this from C++.
    /** @param name Name of the command.
        @param desc Description of the command.
        @param receiver Receiver object.
        @param member Member slot.
    */
    void RegisterCommand(const QString &name, const QString &desc, const QObject *receiver, const char *member);

    /// Executes console command.
    /** @param command Console command, syntax: "command(param1, param2, param3, ...)".
    */
    void ExecuteCommand(const QString &command);

    /// Prints message to console.
    /** @param message Message to print.
    */
    void Print(const QString &message);

private:
    Q_DISABLE_COPY(ScriptConsole);

    /// Constructs the console.
    /** @param fw Framework.
    */
    explicit ScriptConsole(Foundation::Framework *fw);

    /// Framework.
    Foundation::Framework *framework_;

    /// List of registered console commands.
    QMap<QString, Command *> commands_;

private slots:
    /// Checks if we have executed console command object stored. If we have, we invoke it.
    /** @param name Name of the command.
        @param params List of parameters, if provided.
    */
    void CheckForCommand(const QString &name, const QStringList &params) const;
};

#endif
