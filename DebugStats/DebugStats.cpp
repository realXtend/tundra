// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugStats.h"
#include "RealXtend/RexProtocolMsgIDs.h"

#include <UiModule.h>
#include <UiProxyWidget.h>

#include <utility>

using namespace Core;
using namespace std;

namespace DebugStats
{
    const std::string DebugStatsModule::ModuleName = std::string("DebugStatsModule");

    const std::string &DebugStatsModule::NameStatic()
    {
        return ModuleName;
    }

DebugStatsModule::DebugStatsModule()
:ModuleInterfaceImpl(NameStatic()), profilerWindow_(0), networkEventCategory_(0)
{
}

DebugStatsModule::~DebugStatsModule()
{
}

void DebugStatsModule::Load()
{
#ifdef PROFILING
    AutoRegisterConsoleCommand(Console::CreateCommand("Prof", 
        "Shows the profiling window.",
        Console::Bind(this, &DebugStatsModule::ShowProfilingWindow)));
#endif

    LogInfo(Name() + " loaded.");
}

void DebugStatsModule::Unload()
{
    LogInfo(Name() + " unloaded.");
}

void DebugStatsModule::Initialize()
{
    LogInfo(Name() + " initialized.");
}

void DebugStatsModule::PostInitialize()
{
#ifdef _WINDOWS
    QueryPerformanceCounter(&lastCallTime);
#endif

    frameworkEventCategory_ = framework_->GetEventManager()->QueryEventCategory("Framework");
    if (frameworkEventCategory_ == 0)
        LogError("Failed to query \"Framework\" event category");
}

Console::CommandResult DebugStatsModule::ShowProfilingWindow(const StringVector &params)
{
    boost::shared_ptr<UiServices::UiModule> ui_module = 
        framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
    if (!ui_module.get())
        return Console::ResultFailure("Failed to acquire UiModule pointer!");

    // If the window is already visible, no need to create another one.
    if (profilerWindow_)
    {
        ///\todo The ideal path would only return here. We would like to hook the close button of the window
        /// to clear the profilerWindow_ member. Here we check if the profilerWindow_ member is 0, and only
        /// create the window if it does not exist.
        //return;

        // Now we need play 'singleton' with the UI subsystem.
        //profilerWindow_->show();
        return Console::ResultSuccess();
//        delete profilerWindow_;
    }

    profilerWindow_ = new TimeProfilerWindow(ui_module.get(), this);
    UiServices::UiProxyWidget *proxy = ui_module->GetSceneManager()->AddWidgetToScene(profilerWindow_,
        UiServices::UiWidgetProperties("Profiler", UiServices::SceneWidget));
    //Desired: profilerWindow_->show();
    // Instead of:
    proxy->show();
    // The following should not be needed if the size was properly set in Designer.
    proxy->resize(650, 530);
    // Assuming size needs to be se in a custom way:
    // profilerWindow_->resize();
    
//    proxy->setWindowTitle("Profiler");

    QObject::connect(proxy, SIGNAL(Closed()), profilerWindow_, SLOT(Closed()));
//  Desired:    QObject::connect(profilerWindow_, SIGNAL(Closed()), profilerWindow_, SLOT(Closed()));
//  This should be in profilerWindow_.

// profilerWindow_->setAttribute(Qt::WA_DeleteOnClose);
//connect(profilerWindow_, SIGNAL(Destroyed(QObject *)), this, CloseProfilingWindow());
    if (current_world_stream_)
        profilerWindow_->SetWorldStreamPtr(current_world_stream_);

    profilerWindow_->RefreshProfilingData();

    return Console::ResultSuccess();
}

void DebugStatsModule::CloseProfilingWindow()
{
    profilerWindow_->deleteLater();
    profilerWindow_ = 0;
}

void DebugStatsModule::Uninitialize()
{
}

void DebugStatsModule::Update(f64 frametime)
{
    RESETPROFILER;

#ifdef _WINDOWS
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    double timeSpent = Foundation::ProfilerBlock::ElapsedTimeSeconds(lastCallTime, now);
    lastCallTime = now;

    frameTimes.push_back(make_pair(*(boost::uint64_t*)&now, timeSpent));
    if (frameTimes.size() > 2048) // Maintain an upper bound in the frame history.
        frameTimes.erase(frameTimes.begin());

    if (profilerWindow_)
        profilerWindow_->RedrawFrameTimeHistoryGraph(frameTimes);
#endif
}

bool DebugStatsModule::HandleEvent(event_category_id_t category_id,
    event_id_t event_id, Foundation::EventDataInterface *data)
{
    PROFILE(DebugStatsModule_HandleEvent);
  
    if (category_id == frameworkEventCategory_)
    {
        if(event_id == Foundation::WORLD_STREAM_READY)
        {
            Foundation::WorldStreamReadyEvent *event_data = dynamic_cast<Foundation::WorldStreamReadyEvent *>(data);
            if (event_data)
                current_world_stream_ = event_data->WorldStream;
            if (profilerWindow_)
                profilerWindow_->SetWorldStreamPtr(current_world_stream_);

            networkEventCategory_ = framework_->GetEventManager()->QueryEventCategory("NetworkIn");
            if (networkEventCategory_ == 0)
                LogError("Failed to query \"NetworkIn\" event category");

            return false;
        }
    }

    if (category_id == networkEventCategory_)
    {
        if(event_id == RexNetMsgSimStats)
        {
            ProtocolUtilities::NetworkEventInboundData *netdata = checked_static_cast<ProtocolUtilities::NetworkEventInboundData *>(data);
            assert(netdata);
            if (!netdata)
                return false;
            if (profilerWindow_)
                profilerWindow_->RefreshSimStatsData(netdata->message);

            return false;
        }
    }


    return false;
}

}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace DebugStats;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
    POCO_EXPORT_CLASS(DebugStatsModule)
POCO_END_MANIFEST 
