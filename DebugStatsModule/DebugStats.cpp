// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "DebugStats.h"
#include "RealXtend/RexProtocolMsgIDs.h"
#include "EventManager.h"
#include "ModuleManager.h"

#include "ConsoleCommand.h"
#include "ConsoleCommandServiceInterface.h"
#include "Framework.h"
#include "NetworkMessages/NetInMessage.h"
#include "NetworkMessages/NetMessageManager.h"

#include <UiModule.h>
#include "Inworld/View/UiProxyWidget.h"
#include "Inworld/InworldSceneController.h"

#include <utility>

#include "MemoryLeakCheck.h"

using namespace std;

namespace DebugStats
{

DebugStatsModule::DebugStatsModule()
:ModuleInterfaceImpl(NameStatic()), profilerWindow_(0), networkEventCategory_(0)
{
}

DebugStatsModule::~DebugStatsModule()
{
}

void DebugStatsModule::PostInitialize()
{
#ifdef _WINDOWS
    QueryPerformanceCounter(&lastCallTime);
#endif

#ifdef PROFILING
    RegisterConsoleCommand(Console::CreateCommand("Prof", 
        "Shows the profiling window.",
        Console::Bind(this, &DebugStatsModule::ShowProfilingWindow)));

    RegisterConsoleCommand(Console::CreateCommand("rin", 
        "Sends a random network message in.",
        Console::Bind(this, &DebugStatsModule::SendRandomNetworkInPacket)));

    RegisterConsoleCommand(Console::CreateCommand("rout", 
        "Sends a random network message out.",
        Console::Bind(this, &DebugStatsModule::SendRandomNetworkOutPacket)));
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
    UiServices::UiProxyWidget *proxy = ui_module->GetInworldSceneController()->AddWidgetToScene(profilerWindow_,
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
            ProtocolUtilities::WorldStreamReadyEvent *event_data = dynamic_cast<ProtocolUtilities::WorldStreamReadyEvent *>(data);
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

const std::string DebugStatsModule::ModuleName = std::string("DebugStatsModule");

const std::string &DebugStatsModule::NameStatic()
{
    return ModuleName;
}

//void DebugStatsModule::SendRandomNetworkInPacket()
Console::CommandResult DebugStatsModule::SendRandomNetworkInPacket(const StringVector &params)
{
    if (params.size() == 0)
        return Console::ResultSuccess();

    int numMessages = atoi(params[0].c_str());
    for(int i = 0; i < numMessages; ++i)
    {
        std::vector<char> data;
        int dataLen = rand() % 1600 + 1;
        for(int i = 0; i < dataLen; ++i)
            data.push_back(rand() % 256);

        int msgID = rand();

        ProtocolUtilities::NetMessageManager *messageManager = current_world_stream_->GetCurrentProtocolModule()->GetNetworkMessageManager();
        if (!messageManager)
            return Console::ResultSuccess();

        try
        {
            ProtocolUtilities::NetInMessage msg(msgID, (boost::uint8_t *)&data[0], dataLen, false);
#ifdef _DEBUG
            msg.SetMessageID(msgID);
#endif
            ProtocolUtilities::NetMsgID id = msgID;

            ProtocolUtilities::NetworkEventInboundData inData(id , &msg);
            const ProtocolUtilities::NetMessageInfo *messageInfo = messageManager->GetMessageInfoByID(msgID);
            if (!messageInfo)
                continue;

            msg.SetMessageInfo(messageInfo);
            framework_->GetEventManager()->SendEvent(networkEventCategory_, id, &inData);
        }
        catch(const Exception &e)
        {
            LogInfo(std::string("Exception thrown: ") + e.what());
        }
        catch(const std::exception &e)
        {
            LogInfo(std::string("std::exception thrown: ") + e.what());
        }
        catch(...)
        {
            LogInfo("Unknown exception thrown.");
        }
    }
    return Console::ResultSuccess();
}

Console::CommandResult DebugStatsModule::SendRandomNetworkOutPacket(const StringVector &params)
{
    return Console::ResultSuccess();
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
