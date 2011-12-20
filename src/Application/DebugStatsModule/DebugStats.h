/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   DebugStats.h
 *  @brief  Shows information about internal core data structures in separate windows.
 */

#pragma once

#include "DebugStatsModuleApi.h"
#include "IModule.h"
#include "InputFwd.h"

#include <QObject>
#include <QPointer>

#if defined(_WINDOWS)
#if defined(_WINSOCKAPI_)
#undef _WINSOCKAPI_ 
#endif
#include <WinSock2.h>
#include <Windows.h>
#endif

class TimeProfilerWindow;

/// Shows information about internal core data structures in separate windows.
/** Useful for verifying and understanding the internal state of the application. */
class DEBUGSTATS_MODULE_API DebugStatsModule : public IModule
{
    Q_OBJECT

public:
    DebugStatsModule();
    virtual ~DebugStatsModule();

    void Initialize();
    void Update(f64 frametime);

public slots:
    /// Creates and shows the profiling window.
    void ShowProfilingWindow();
    void DumpInputContexts();

private slots:
    /// Starts profiling if the profiler widget is visible.
    /** @param bool visible Visibility. */
    void StartProfiling(bool visible);

    void HandleKeyPressed(KeyEvent *e);

    /// Invokes action in entity.
    void Exec(const QStringList &params);

private:
    Q_DISABLE_COPY(DebugStatsModule);

    std::vector<std::pair<u64, double> > frameTimes; ///< A history of estimated frame times.
    QPointer<TimeProfilerWindow> profilerWindow_; /// Profiler window
    boost::shared_ptr<InputContext> inputContext; ///< InputContext for Shift-P - Profiler window shortcut.
#ifdef _WINDOWS
    LARGE_INTEGER lastCallTime; ///< Last call time of Update() function
#endif
};
