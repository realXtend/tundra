/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 */

#ifndef incl_Interfaces_LoggingFunctions_h
#define incl_Interfaces_LoggingFunctions_h

#include <QString>

static void LogFatal(const std::string &msg)    { printf(("Fatal: " + msg).c_str());                   }
static void LogCritical(const std::string &msg) { printf(("Critical: " + msg).c_str());             }
static void LogError(const std::string &msg)    { printf(("Error: " + msg).c_str());                   }
static void LogWarning(const std::string &msg)  { printf(("Warning: " + msg).c_str());               }
static void LogNotice(const std::string &msg)   { printf(("Notice: " + msg).c_str());                 }
static void LogInfo(const std::string &msg)     { printf(msg.c_str());                         }
static void LogTrace(const std::string &msg)    { printf(("Trace: " + msg).c_str());                   }
static void LogDebug(const std::string &msg)    { printf(("Debug: " + msg).c_str());                   }
                                                                                                                            
static void LogFatal(const char *msg)    { printf((std::string("Fatal: ") + msg).c_str());             }
static void LogCritical(const char *msg) { printf((std::string("Critical: ") + msg).c_str());       }
static void LogError(const char *msg)    { printf((std::string("Error: ") + msg).c_str());             }
static void LogWarning(const char *msg)  { printf((std::string("Warning: ") + msg).c_str());         }
static void LogNotice(const char *msg)   { printf((std::string("Notice: ") + msg).c_str());           }
static void LogInfo(const char *msg)     { printf(msg);                                }
static void LogTrace(const char *msg)    { printf((std::string("Trace: ") + msg).c_str());             }
static void LogDebug(const char *msg)    { printf((std::string("Debug: ") + msg).c_str());             }

///\todo UTF-8 -enable the following.

static void LogFatal(const QString &msg)    { printf(("Fatal: " + msg).toStdString().c_str());       }
static void LogCritical(const QString &msg) { printf(("Critical: " + msg).toStdString().c_str()); }
static void LogError(const QString &msg)    { printf(("Error: " + msg).toStdString().c_str());       }
static void LogWarning(const QString &msg)  { printf(("Warning: " + msg).toStdString().c_str());   }
static void LogNotice(const QString &msg)   { printf(("Notice: " + msg).toStdString().c_str());     }
static void LogInfo(const QString &msg)     { printf(msg.toStdString().c_str());               }
static void LogTrace(const QString &msg)    { printf(("Trace: " + msg).toStdString().c_str());       }
static void LogDebug(const QString &msg)    { printf(("Debug: " + msg).toStdString().c_str());       }

#endif // incl_Interfaces_LoggingFunctions_h
