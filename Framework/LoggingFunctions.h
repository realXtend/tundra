/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 */

#pragma once

#include <QString>

void PrintLogMessage(const char *str);

static void LogFatal(const std::string &msg)    { PrintLogMessage(("Fatal: " + msg + "\n").c_str());                 }
static void LogCritical(const std::string &msg) { PrintLogMessage(("Critical: " + msg + "\n").c_str());              }
static void LogError(const std::string &msg)    { PrintLogMessage(("Error: " + msg + "\n").c_str());                 }
static void LogWarning(const std::string &msg)  { PrintLogMessage(("Warning: " + msg + "\n").c_str());               }
static void LogNotice(const std::string &msg)   { PrintLogMessage(("Notice: " + msg + "\n").c_str());                }
static void LogInfo(const std::string &msg)     { PrintLogMessage((msg + "\n").c_str());                             }
static void LogTrace(const std::string &msg)    { PrintLogMessage(("Trace: " + msg + "\n").c_str());                 }
                                                                                                                            
static void LogFatal(const char *msg)    { PrintLogMessage((std::string("Fatal: ") + msg + "\n").c_str());           }
static void LogCritical(const char *msg) { PrintLogMessage((std::string("Critical: ") + msg + "\n").c_str());        }
static void LogError(const char *msg)    { PrintLogMessage((std::string("Error: ") + msg + "\n").c_str());           }
static void LogWarning(const char *msg)  { PrintLogMessage((std::string("Warning: ") + msg + "\n").c_str());         }
static void LogNotice(const char *msg)   { PrintLogMessage((std::string("Notice: ") + msg + "\n").c_str());          }
static void LogInfo(const char *msg)     { PrintLogMessage((std::string(msg) + "\n").c_str());                       }
static void LogTrace(const char *msg)    { PrintLogMessage((std::string("Trace: ") + msg + "\n").c_str());           }

///\todo UTF-8 -enable the following.

static void LogFatal(const QString &msg)    { PrintLogMessage(("Fatal: " + msg + "\n").toStdString().c_str());       }
static void LogCritical(const QString &msg) { PrintLogMessage(("Critical: " + msg + "\n").toStdString().c_str());    }
static void LogError(const QString &msg)    { PrintLogMessage(("Error: " + msg + "\n").toStdString().c_str());       }
static void LogWarning(const QString &msg)  { PrintLogMessage(("Warning: " + msg + "\n").toStdString().c_str());     }
static void LogNotice(const QString &msg)   { PrintLogMessage(("Notice: " + msg + "\n").toStdString().c_str());      }
static void LogInfo(const QString &msg)     { PrintLogMessage((msg + "\n").toStdString().c_str());                   }
static void LogTrace(const QString &msg)    { PrintLogMessage(("Trace: " + msg + "\n").toStdString().c_str());       }

#ifdef _DEBUG
static void LogDebug(const std::string &msg)    { PrintLogMessage(("Debug: " + msg + "\n").c_str());                 }
static void LogDebug(const char *msg)    { PrintLogMessage((std::string("Debug: ") + msg + "\n").c_str());           }
static void LogDebug(const QString &msg)    { PrintLogMessage(("Debug: " + msg + "\n").toStdString().c_str());       }
#else
static void LogDebug(const std::string &msg) {}
static void LogDebug(const char *msg) {}
static void LogDebug(const QString &msg) {}
#endif
