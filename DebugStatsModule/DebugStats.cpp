/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   DebugStats.cpp
 *  @brief  DebugStatsModule shows information about internal core data structures in separate windows.
 *          Useful for verifying and understanding the internal state of the application.
 *  @note   Depends on RexLogicModule so don't create dependency to this module.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "DebugStats.h"
#include "TimeProfilerWindow.h"
#include "ParticipantWindow.h"

#include "Framework.h"
#include "EventManager.h"
#include "ModuleManager.h"
#include "ConsoleCommandServiceInterface.h"
#include "WorldStream.h"
#include "SceneEvents.h"
#include "SceneManager.h"
#include "NetworkEvents.h"
//#include "EntityComponent/EC_OpenSimPresence.h"
#include "RealXtend/RexProtocolMsgIDs.h"
#include "NetworkMessages/NetInMessage.h"
#include "NetworkMessages/NetMessageManager.h"
#include "UiModule.h"
#include "Inworld/View/UiProxyWidget.h"
#include "Inworld/InworldSceneController.h"

#include "EC_OpenSimPresence.h"

#include <utility>

#include "MemoryLeakCheck.h"

using namespace std;

namespace DebugStats
{

DebugStatsModule::DebugStatsModule() :
    ModuleInterfaceImpl(NameStatic()),
    frameworkEventCategory_(0),
    networkEventCategory_(0),
    networkStateEventCategory_(0),
    profilerWindow_(0),
    participantWindow_(0)
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

    RegisterConsoleCommand(Console::CreateCommand("Participant", 
        "Shows the participant window.",
        Console::Bind(this, &DebugStatsModule::ShowParticipantWindow)));

    frameworkEventCategory_ = framework_->GetEventManager()->QueryEventCategory("Framework");
    if (frameworkEventCategory_ == 0)
        LogError("Failed to query \"Framework\" event category");
}

Console::CommandResult DebugStatsModule::ShowProfilingWindow(const StringVector &params)
{
    UiModulePtr ui_module = framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
    if (!ui_module.get())
        return Console::ResultFailure("Failed to acquire UiModule pointer!");

    // If the window is already created, bring it to front.
    if (profilerWindow_)
    {
        ui_module->GetInworldSceneController()->BringProxyToFront(profilerWindow_);
        return Console::ResultSuccess();
    }

    profilerWindow_ = new TimeProfilerWindow(framework_);
    UiServices::UiProxyWidget *proxy = ui_module->GetInworldSceneController()->AddWidgetToScene(profilerWindow_,
        UiServices::UiWidgetProperties("Profiler", UiServices::SceneWidget));
    ui_module->GetInworldSceneController()->ShowProxyForWidget(profilerWindow_);
    proxy->resize(650, 530);

    QObject::connect(proxy, SIGNAL(Closed()), profilerWindow_, SLOT(deleteLater()));

    if (current_world_stream_)
        profilerWindow_->SetWorldStreamPtr(current_world_stream_);

    profilerWindow_->RefreshProfilingData();

    return Console::ResultSuccess();
}

Console::CommandResult DebugStatsModule::ShowParticipantWindow(const StringVector &params)
{
    UiModulePtr ui_module = framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
    if (!ui_module.get())
        return Console::ResultFailure("Failed to acquire UiModule pointer!");

    if (participantWindow_)
    {
        ui_module->GetInworldSceneController()->BringProxyToFront(participantWindow_);
        return Console::ResultSuccess();
    }

    participantWindow_ = new ParticipantWindow(framework_);

    UiServices::UiProxyWidget *proxy = ui_module->GetInworldSceneController()->AddWidgetToScene(participantWindow_,
        UiServices::UiWidgetProperties(QApplication::translate("ParticipantWindow", "Participants"), UiServices::SceneWidget));
    QObject::connect(proxy, SIGNAL(Closed()), participantWindow_, SLOT(deleteLater()));

    proxy->show();

//    if (current_world_stream_)
//        participantWindow_->SetWorldStreamPtr(current_world_stream_);

    return Console::ResultSuccess();
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

bool DebugStatsModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface *data)
{
    PROFILE(DebugStatsModule_HandleEvent);

    if (category_id == frameworkEventCategory_)
    {
        if (event_id == Foundation::WORLD_STREAM_READY)
        {
            ProtocolUtilities::WorldStreamReadyEvent *event_data = checked_static_cast<ProtocolUtilities::WorldStreamReadyEvent *>(data);
            assert(event_data);
            if (event_data)
                current_world_stream_ = event_data->WorldStream;
            if (profilerWindow_)
                profilerWindow_->SetWorldStreamPtr(current_world_stream_);

            networkEventCategory_ = framework_->GetEventManager()->QueryEventCategory("NetworkIn");
            if (networkEventCategory_ == 0)
                LogError("Failed to query \"NetworkIn\" event category");

            networkStateEventCategory_ = framework_->GetEventManager()->QueryEventCategory("NetworkState");
            if (networkStateEventCategory_ == 0)
                LogError("Failed to query \"NetworkState\" event category");

            return false;
        }
    }

    if (category_id == networkStateEventCategory_)
    {
        switch(event_id)
        {
        case ProtocolUtilities::Events::EVENT_USER_CONNECTED:
        {
            ProtocolUtilities::UserConnectivityEvent *event_data = checked_static_cast<ProtocolUtilities::UserConnectivityEvent *>(data);
            assert(event_data);
            if (!event_data)
                return false;

            Scene::EntityPtr entity = framework_->GetDefaultWorldScene()->GetEntity(event_data->localId);
            if (!entity)
                return false;

            EC_OpenSimPresence *ec_presence = entity->GetComponent<EC_OpenSimPresence>().get();
            if (ec_presence && participantWindow_)
                participantWindow_->AddUserEntry(ec_presence);
            break;
        }
        case ProtocolUtilities::Events::EVENT_USER_DISCONNECTED:
        {
            ProtocolUtilities::UserConnectivityEvent *event_data = checked_static_cast<ProtocolUtilities::UserConnectivityEvent *>(data);
            assert(event_data);
            if (!event_data)
                return false;

            Scene::EntityPtr entity = framework_->GetDefaultWorldScene()->GetEntity(event_data->localId);
            if (!entity)
                return false;

            EC_OpenSimPresence *ec_presence = entity->GetComponent<EC_OpenSimPresence>().get();
            if (ec_presence && participantWindow_)
                participantWindow_->RemoveUserEntry(ec_presence);
            break;
        }
        default:
            break;
        }

        return false;
    }

    if (category_id == networkEventCategory_)
    {
        if (event_id == RexNetMsgSimStats)
        {
            ProtocolUtilities::NetworkEventInboundData *netdata = checked_static_cast<ProtocolUtilities::NetworkEventInboundData *>(data);
            assert(netdata);
            if (!netdata)
                return false;
            if (profilerWindow_)
                profilerWindow_->RefreshSimStatsData(netdata->message);
        }
    }

    return false;
}

const std::string DebugStatsModule::ModuleName = std::string("DebugStatsModule");

const std::string &DebugStatsModule::NameStatic()
{
    return ModuleName;
}

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
