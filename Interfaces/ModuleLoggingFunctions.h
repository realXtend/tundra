// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_ModuleLoggingFunctions_h
#define incl_Interfaces_ModuleLoggingFunctions_h

#include <Poco/Logger.h>

#define MODULE_LOGGING_FUNCTIONS                                                                                            \
    static void LogFatal(const std::string &msg)    { Poco::Logger::get(NameStatic()).fatal("Fatal: " + msg);         }     \
    static void LogCritical(const std::string &msg) { Poco::Logger::get(NameStatic()).critical("Critical: " + msg);   }     \
    static void LogError(const std::string &msg)    { Poco::Logger::get(NameStatic()).error("Error: " + msg);         }     \
    static void LogWarning(const std::string &msg)  { Poco::Logger::get(NameStatic()).warning("Warning: " + msg);     }     \
    static void LogNotice(const std::string &msg)   { Poco::Logger::get(NameStatic()).notice("Notice: " + msg);       }     \
    static void LogInfo(const std::string &msg)     { Poco::Logger::get(NameStatic()).information(msg);               }     \
    static void LogTrace(const std::string &msg)    { Poco::Logger::get(NameStatic()).trace("Trace: " + msg);         }     \
    static void LogDebug(const std::string &msg)    { Poco::Logger::get(NameStatic()).debug("Debug: " + msg);         }

#endif

