/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   LoggingFunctions.h
 *  @brief  Defines PoCo logging functions which can be used to print information
 *          to std::cout, log file and console depending on the used priority level.
 *
 *  Usage:
 *  // In the begging of cpp file.
 *  #include "LoggingFunctions."
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

#define DEFINE_POCO_LOGGING_FUNCTIONS(name)                                                                             \
    static const std::string loggingName(name);                                                                         \
    static void LogFatal(const std::string &msg)    { Poco::Logger::get(loggingName).fatal("Fatal: " + msg);         }  \
    static void LogCritical(const std::string &msg) { Poco::Logger::get(loggingName).critical("Critical: " + msg);   }  \
    static void LogError(const std::string &msg)    { Poco::Logger::get(loggingName).error("Error: " + msg);         }  \
    static void LogWarning(const std::string &msg)  { Poco::Logger::get(loggingName).warning("Warning: " + msg);     }  \
    static void LogNotice(const std::string &msg)   { Poco::Logger::get(loggingName).notice("Notice: " + msg);       }  \
    static void LogInfo(const std::string &msg)     { Poco::Logger::get(loggingName).information(msg);               }  \
    static void LogTrace(const std::string &msg)    { Poco::Logger::get(loggingName).trace("Trace: " + msg);         }  \
    static void LogDebug(const std::string &msg)    { Poco::Logger::get(loggingName).debug("Debug: " + msg);         }

#endif

