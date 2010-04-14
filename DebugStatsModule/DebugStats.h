/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   DebugStats.h
 *  @brief  DebugStatsModule shows information about internal core data structures in separate windows.
 *          Useful for verifying and understanding the internal state of the application.
 *  @note   Depends on RexLogicModule so don't create dependency to this module.
 */

#ifndef incl_DebugStats_h
#define incl_DebugStats_h

#include "DebugStatsModuleApi.h"
#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"
#include "RexTypes.h"

#include <QObject>

namespace RexLogic
{
    class EC_OpenSimPrim;
}

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
        /// Shows the profiling window
        Console::CommandResult ShowProfilingWindow(const StringVector &params);

        /// Closes the profiling window
        void CloseProfilingWindow();

        /// Shows the participant window
        Console::CommandResult ShowParticipantWindow(const StringVector &params);

        /// Closes the participant window
        void CloseParticipantWindow();

    private:
         Q_DISABLE_COPY(DebugStatsModule);

        /// A history of estimated frame times.
        std::vector<std::pair<uint64_t, double> > frameTimes;

#ifdef _WINDOWS
        /// Last call time of Update() function
        LARGE_INTEGER lastCallTime;
#endif
        /// Sends random NetInMessage packet
        Console::CommandResult SendRandomNetworkInPacket(const StringVector &params);

        /// Sends random NetOutMessage packet
        Console::CommandResult SendRandomNetworkOutPacket(const StringVector &params);

        /// Framework event category
        event_category_id_t frameworkEventCategory_;

        /// Network event category
        event_category_id_t networkEventCategory_;

        /// Profiler window
        TimeProfilerWindow *profilerWindow_;

        /// Participant window
        ParticipantWindow *participantWindow_;

        ///
        ProtocolUtilities::WorldStreamPtr current_world_stream_;
    };
}

#endif
