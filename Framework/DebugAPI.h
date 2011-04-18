/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   DebugAPI.h
 *  @brief  Debug core API.
 */

#ifndef incl_Foundation_DebugAPI_h
#define incl_Foundation_DebugAPI_h

#include <QObject>

class Framework;

/// Debug core API.
class DebugAPI : public QObject
{
    Q_OBJECT

public slots:
    /// Prints log message to std::cout, log file and console using the information logging level.
    /** @param msg Log message.
    */
    void Log(const QString &msg);

    /// Prints log message to std::cout, log file and console using the warning logging level.
    /** @param msg Log message.
    */
    void LogWarning(const QString &msg);

    /// Prints log message to std::cout, log file and console using the error logging level.
    /** @param msg Log message.
    */
    void LogError(const QString &msg);

    /// Prints log message to std::cout, log file and console using the debug logging level.
    /** @param msg Log message.
    */
    void LogDebug(const QString &msg);

    /// @return True if current build is the debug build, false otherwise.
    bool IsDebugBuild() const;

    /// Sets a break point on within this function (in debug mode only.). Windows/Visual Studio only
    void Break();

private:
    Q_DISABLE_COPY(DebugAPI)
    friend class Framework;

    /// Constructs the Debug API.
    /** @param fw Framework. Takes ownership of the object.
    */
    DebugAPI(Framework *fw);
};

#endif
