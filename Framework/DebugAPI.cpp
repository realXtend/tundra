/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   DebugAPI.cpp
 *  @brief  Debug core API.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MemoryLeakCheck.h"
#include "DebugAPI.h"
#include "LoggingFunctions.h"

#ifdef _MSC_VER
#include <CRTDBG.h>
#endif

void DebugAPI::Log(const QString &msg)
{
    ::LogInfo(msg.toStdString());
}

void DebugAPI::LogWarning(const QString &msg)
{
    ::LogWarning(msg.toStdString());
}

void DebugAPI::LogError(const QString &msg)
{
    ::LogError(msg.toStdString());
}

void DebugAPI::LogDebug(const QString &msg)
{
    ::LogDebug(msg.toStdString());
}

bool DebugAPI::IsDebugBuild() const
{
#ifdef _DEBUG
    return true;
#else
    return false;
#endif
}

void DebugAPI::Break()
{
#ifdef _MSC_VER
    _CrtDbgBreak();
#else
    ::LogInfo("DebugAPI::Break() works only on Windows with Visual Studio.");
#endif
}

DebugAPI::DebugAPI(Foundation::Framework *fw) : QObject(fw)
{
}
