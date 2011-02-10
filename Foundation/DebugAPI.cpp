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

bool DebugAPI::IsDebugBuild() const
{
#ifdef _DEBUG
    return true;
#else
    return false;
#endif
}

#ifdef _MSC_VER
void DebugAPI::Break()
{
    _CrtDbgBreak();
}
#endif

DebugAPI::DebugAPI(Foundation::Framework *fw) : QObject(fw)
{
}
