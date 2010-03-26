// For conditions of distribution and use, see copyright notice in license.txt

/// Please note: The code of this module can be pretty fugly from time to time.
/// This module is just meant for quick testing and debugging stuff.

#ifndef incl_DebugStats_h
#define incl_DebugStats_h

#include "DebugStatsModuleApi.h"
#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"
#include "EventDataInterface.h"
#include "TimeProfilerWindow.h"
#include "WorldStream.h"

namespace RexLogic
{
    class EC_OpenSimPrim;
}
namespace OgreRenderer
{
    class EC_OgrePlaceable;
}

namespace DebugStats
{

/// This module shows information about internal core data structures in separate windows. Useful for verifying and understanding
/// the internal state of the application.
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

    Console::CommandResult ShowProfilingWindow(const StringVector &params);
    void CloseProfilingWindow();

private:
    /// A history of estimated frame times.
    std::vector<std::pair<boost::uint64_t, double> > frameTimes;

#ifdef _WINDOWS
    LARGE_INTEGER lastCallTime;
#endif
 
    void operator=(const DebugStatsModule &);
    DebugStatsModule(const DebugStatsModule &);

    Console::CommandResult SendRandomNetworkInPacket(const StringVector &params);
    Console::CommandResult SendRandomNetworkOutPacket(const StringVector &params);

//    void SendRandomNetworkInPacket();
//    void SendRandomNetworkOutPacket();


    event_category_id_t frameworkEventCategory_;
    event_category_id_t networkEventCategory_;

    TimeProfilerWindow *profilerWindow_;

    ProtocolUtilities::WorldStreamPtr current_world_stream_;
};

}

#endif
