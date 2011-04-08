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

#include <Windows.h>

class EC_Placeable;

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
        bool HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data);

        //MODULE_LOGGING_FUNCTIONS

        /// Returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return moduleName; }

        /// Name of this module.
        static const std::string moduleName;

    public slots:
        /// Creates and shows the profiling window.
        Console::CommandResult ShowProfilingWindow(/*const StringVector &params*/);

    private slots:
        void AddProfilerWidgetToUi();

        /// Starts profiling if the profiler (proxy) widget is visible.
        /// @param bool visible Visibility.
        void StartProfiling(bool visible);

        void HandleKeyPressed(KeyEvent *e);

    private:
        Q_DISABLE_COPY(DebugStatsModule);

        /// Invokes action in entity.
        Console::CommandResult Exec(const StringVector &params);

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
