/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   DebugAPI.cpp
 *  @brief  Debug core API.
 */

#include "StableHeaders.h"
#include "DebugAPI.h"

#ifdef _MSC_VER
#include <CRTDBG.h>
#endif

void DebugAPI::Log(const QString &msg)
{
    RootLogInfo(msg.toStdString());
}

void DebugAPI::LogWarning(const QString &msg)
{
    RootLogWarning(msg.toStdString());
}

void DebugAPI::LogError(const QString &msg)
{
    RootLogError(msg.toStdString());
}

void DebugAPI::LogDebug(const QString &msg)
{
    RootLogDebug(msg.toStdString());
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
    RootLogInfo("DebugAPI::Break() works only on Windows with Visual Studio.");
#endif
}

DebugAPI::DebugAPI(Foundation::Framework *fw) : QObject(fw)
{
}
