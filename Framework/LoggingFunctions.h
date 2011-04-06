/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   LoggingFunctions.h
 *  @brief  Defines PoCo logging functions which can be used to print information
 *          to std::cout, log file and console depending on the used priority level.
 *
 *  Usage:
 *  // In the begging of cpp file.
 *  #include "LoggingFunctions.h"
 *  DEFINE_POCO_LOGGING_FUNCTIONS("MyClass")
 *  // Later on...
 *  LogInfo("Hello!");
 *
 *  Result: prints "[MyClass] Hello!" to std::cout, log file and console.
 *
 *  @note Never include this file from headers!
 */

#ifndef incl_Interfaces_LoggingFunctions_h
#define incl_Interfaces_LoggingFunctions_h

#include <Poco/Logger.h>
#include <QString>

// You cant have #ifdef inside #define so lets break it down
// This is done to prevent LoggingFunctions.h to leak LogDebugs in release mode!
#define DEFINE_POCO_LOGGING_FUNCTIONS(name)                                                                                      \
    static const std::string loggingName(name);                                                                                  \
                                                                                                                                 \
    static void LogFatal(const std::string &msg)    { Poco::Logger::get(loggingName).fatal("Fatal: " + msg);                   } \
    static void LogCritical(const std::string &msg) { Poco::Logger::get(loggingName).critical("Critical: " + msg);             } \
    static void LogError(const std::string &msg)    { Poco::Logger::get(loggingName).error("Error: " + msg);                   } \
    static void LogWarning(const std::string &msg)  { Poco::Logger::get(loggingName).warning("Warning: " + msg);               } \
    static void LogNotice(const std::string &msg)   { Poco::Logger::get(loggingName).notice("Notice: " + msg);                 } \
    static void LogInfo(const std::string &msg)     { Poco::Logger::get(loggingName).information(msg);                         } \
    static void LogTrace(const std::string &msg)    { Poco::Logger::get(loggingName).trace("Trace: " + msg);                   } \
    static void LogDebug(const std::string &msg)    { Poco::Logger::get(loggingName).debug("Debug: " + msg);                   } \
                                                                                                                                 \
    static void LogFatal(const char *msg)    { Poco::Logger::get(loggingName).fatal(std::string("Fatal: ") + msg);             } \
    static void LogCritical(const char *msg) { Poco::Logger::get(loggingName).critical(std::string("Critical: ") + msg);       } \
    static void LogError(const char *msg)    { Poco::Logger::get(loggingName).error(std::string("Error: ") + msg);             } \
    static void LogWarning(const char *msg)  { Poco::Logger::get(loggingName).warning(std::string("Warning: ") + msg);         } \
    static void LogNotice(const char *msg)   { Poco::Logger::get(loggingName).notice(std::string("Notice: ") + msg);           } \
    static void LogInfo(const char *msg)     { Poco::Logger::get(loggingName).information(msg);                                } \
    static void LogTrace(const char *msg)    { Poco::Logger::get(loggingName).trace(std::string("Trace: ") + msg);             } \
    static void LogDebug(const char *msg)    { Poco::Logger::get(loggingName).debug(std::string("Debug: ") + msg);             } \
                                                                                                                                 \
    static void LogFatal(const QString &msg)    { Poco::Logger::get(loggingName).fatal(("Fatal: " + msg).toStdString());       } \
    static void LogCritical(const QString &msg) { Poco::Logger::get(loggingName).critical(("Critical: " + msg).toStdString()); } \
    static void LogError(const QString &msg)    { Poco::Logger::get(loggingName).error(("Error: " + msg).toStdString());       } \
    static void LogWarning(const QString &msg)  { Poco::Logger::get(loggingName).warning(("Warning: " + msg).toStdString());   } \
    static void LogNotice(const QString &msg)   { Poco::Logger::get(loggingName).notice(("Notice: " + msg).toStdString());     } \
    static void LogInfo(const QString &msg)     { Poco::Logger::get(loggingName).information(msg.toStdString());               } \
    static void LogTrace(const QString &msg)    { Poco::Logger::get(loggingName).trace(("Trace: " + msg).toStdString());       } \
    static void LogDebug(const QString &msg)    { Poco::Logger::get(loggingName).debug(("Debug: " + msg).toStdString());       }

#endif // incl_Interfaces_LoggingFunctions_h
