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
#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"
#include "RexTypes.h"

#include <QObject>
#include <QPointer>

namespace OgreRenderer
{
    class EC_OgrePlaceable;
}

namespace Foundation
{
    class EventDataInterface;
}

namespace ProtocolUtilities
{
    class WorldStream;
    typedef boost::shared_ptr<WorldStream> WorldStreamPtr;
}

namespace DebugStats
{
    class TimeProfilerWindow;
    class ParticipantWindow;

    class DEBUGSTATS_MODULE_API DebugStatsModule : public QObject, public Foundation::ModuleInterfaceImpl
    {
        Q_OBJECT

    public:
        DebugStatsModule();
        virtual ~DebugStatsModule();

        void PostInitialize();
        void Update(f64 frametime);
        bool HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data);

        MODULE_LOGGING_FUNCTIONS

        /// Returns name of this module. Needed for logging.
        static const std::string &NameStatic();

        /// Name of this module.
        static const std::string ModuleName;

public slots:
        /// Creates and shows the profiling window.
        Console::CommandResult ShowProfilingWindow(const StringVector &params);

        /// Creates and shows the participant window.
        Console::CommandResult ShowParticipantWindow(const StringVector &params);

    private:
         Q_DISABLE_COPY(DebugStatsModule);

        /// Sends random NetInMessage packet
        Console::CommandResult SendRandomNetworkInPacket(const StringVector &params);

        /// Sends random NetOutMessage packet
        Console::CommandResult SendRandomNetworkOutPacket(const StringVector &params);

        /// A history of estimated frame times.
        std::vector<std::pair<uint64_t, double> > frameTimes;

#ifdef _WINDOWS
        /// Last call time of Update() function
        LARGE_INTEGER lastCallTime;
#endif

        /// Framework event category
        event_category_id_t frameworkEventCategory_;

        /// NetworkIn event category
        event_category_id_t networkEventCategory_;

        /// NetworkState event category
        event_category_id_t networkStateEventCategory_;

        /// Profiler window
        QPointer<TimeProfilerWindow> profilerWindow_;

        /// Participant window
        QPointer<ParticipantWindow> participantWindow_;

        /// World stream pointer.
        ProtocolUtilities::WorldStreamPtr current_world_stream_;
    };
}

#endif
