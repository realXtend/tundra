/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   Console.h
 *  @brief  Exposes debug console functionality ot scripting languages.
 *  @note   Currently just simple wrapper class but the idea is to refactor
 *          whole console system later on.
 */

#ifndef incl_Foundation_Console
#define incl_Foundation_Console

#include "CoreTypes.h"

#include <QObject>

namespace Foundation
{
    class Framework;
}

/// Exposes debug console functionality ot scripting languages.
/** @note Currently just simple wrapper class but the idea is to refactor
    the whole console system.later on.
*/
class ScriptConsole : public QObject
{
    Q_OBJECT

    friend class Foundation::Framework;

public:
    /// Destructor.
    ~ScriptConsole() {}

public slots:
    /// Registers new console command.
    /** @param name Name of the command.
        @param receiver Receiver object.
        @param member Member slot.
    */
    void RegisterCommand(const QString &name, const QObject *receiver, const char *member);

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
};

#endif
