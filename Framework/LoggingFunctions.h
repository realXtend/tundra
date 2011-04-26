/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 */

#ifndef incl_Framework_LoggingFunctions_h
#define incl_Framework_LoggingFunctions_h

#include <QString>

static void LogFatal(const std::string &msg)    { printf(("Fatal: " + msg + "\n").c_str());                 }
static void LogCritical(const std::string &msg) { printf(("Critical: " + msg + "\n").c_str());              }
static void LogError(const std::string &msg)    { printf(("Error: " + msg + "\n").c_str());                 }
static void LogWarning(const std::string &msg)  { printf(("Warning: " + msg + "\n").c_str());               }
static void LogNotice(const std::string &msg)   { printf(("Notice: " + msg + "\n").c_str());                }
static void LogInfo(const std::string &msg)     { printf("%s\n", msg.c_str());                              }
static void LogTrace(const std::string &msg)    { printf(("Trace: " + msg + "\n").c_str());                 }
                                                                                                                            
static void LogFatal(const char *msg)    { printf((std::string("Fatal: ") + msg + "\n").c_str());           }
static void LogCritical(const char *msg) { printf((std::string("Critical: ") + msg + "\n").c_str());        }
static void LogError(const char *msg)    { printf((std::string("Error: ") + msg + "\n").c_str());           }
static void LogWarning(const char *msg)  { printf((std::string("Warning: ") + msg + "\n").c_str());         }
static void LogNotice(const char *msg)   { printf((std::string("Notice: ") + msg + "\n").c_str());          }
static void LogInfo(const char *msg)     { printf("%s\n", msg);                                             }
static void LogTrace(const char *msg)    { printf((std::string("Trace: ") + msg + "\n").c_str());           }

///\todo UTF-8 -enable the following.

static void LogFatal(const QString &msg)    { printf(("Fatal: " + msg + "\n").toStdString().c_str());       }
static void LogCritical(const QString &msg) { printf(("Critical: " + msg + "\n").toStdString().c_str());    }
static void LogError(const QString &msg)    { printf(("Error: " + msg + "\n").toStdString().c_str());       }
static void LogWarning(const QString &msg)  { printf(("Warning: " + msg + "\n").toStdString().c_str());     }
static void LogNotice(const QString &msg)   { printf(("Notice: " + msg + "\n").toStdString().c_str());      }
static void LogInfo(const QString &msg)     { printf("%s\n", msg.toStdString().c_str());                    }
static void LogTrace(const QString &msg)    { printf(("Trace: " + msg + "\n").toStdString().c_str());       }

#ifdef _DEBUG
static void LogDebug(const std::string &msg)    { printf(("Debug: " + msg + "\n").c_str());                 }
static void LogDebug(const char *msg)    { printf((std::string("Debug: ") + msg + "\n").c_str());           }
static void LogDebug(const QString &msg)    { printf(("Debug: " + msg + "\n").toStdString().c_str());       }
#else
static void LogDebug(const std::string &msg) {}
static void LogDebug(const char *msg) {}
static void LogDebug(const QString &msg) {}
#endif

#endif // incl_Framework_LoggingFunctions_h
