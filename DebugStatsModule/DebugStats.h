/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   DebugStats.h
 *  @brief  DebugStatsModule shows information about internal core data structures in separate windows.
 *          Useful for verifying and understanding the internal state of the application.
 */

#ifndef incl_DebugStats_h
#define incl_DebugStats_h

#include "DebugStatsModuleApi.h"
#include "IModule.h"
#include "InputFwd.h"

#include <QObject>
#include <QPointer>

#include <WinSock2.h>
#include <Windows.h>

class EC_Placeable;
struct ConsoleCommandResult;

namespace DebugStats
{
    class TimeProfilerWindow;
    class ParticipantWindow;

    class DEBUGSTATS_MODULE_API DebugStatsModule : public QObject, public IModule
    {
        Q_OBJECT

    public:
        DebugStatsModule();
        virtual ~DebugStatsModule();

        void PostInitialize();
        void Update(f64 frametime);

        /// Creates and shows the profiling window.
        ConsoleCommandResult ShowProfilingWindowCommand();

    public slots:
        /// Creates and shows the profiling window.
        void ShowProfilingWindow();

    private slots:
        void AddProfilerWidgetToUi();

        /// Starts profiling if the profiler (proxy) widget is visible.
        /// @param bool visible Visibility.
        void StartProfiling(bool visible);

        void HandleKeyPressed(KeyEvent *e);

    private:
        Q_DISABLE_COPY(DebugStatsModule);

        /// Invokes action in entity.
        ConsoleCommandResult Exec(const StringVector &params);

        /// A history of estimated frame times.
        std::vector<std::pair<u64, double> > frameTimes;

#ifdef _WINDOWS
        /// Last call time of Update() function
        LARGE_INTEGER lastCallTime;
#endif

        /// Profiler window
        QPointer<TimeProfilerWindow> profilerWindow_;

        /// DebugStatsModules registers an InputContext to be able to do a Shift-P - Profiler window shortcut.
        boost::shared_ptr<InputContext> inputContext;
    };
}

#endif
