/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   InWorldChatModule.cpp
 *  @brief  Simple OpenSim world chat module. Listens for ChatFromSimulator packets and shows the chat on the UI.
 *          Outgoing chat sent using ChatFromViewer packets. Manages EC_ChatBubbles
 */

#include "StableHeaders.h"
#include "InWorldChatModule.h"

#include "EC_ChatBubble.h"
#include "EC_Billboard.h"

#include "WorldStream.h"
#include "SceneManager.h"
#include "EventManager.h"
#include "ModuleManager.h"
#include "RealXtend/RexProtocolMsgIDs.h"
#include "NetworkMessages/NetInMessage.h"
#include "CoreStringUtils.h"
#include "UiModule.h"

#include "EntityComponent/EC_OpenSimPresence.h"

namespace Naali
{

InWorldChatModule::InWorldChatModule() :
    ModuleInterfaceImpl(NameStatic()),
    networkStateEventCategory_(0),
    networkInEventCategory_(0),
    frameworkEventCategory_(0)
{
}

InWorldChatModule::~InWorldChatModule()
{
}

void InWorldChatModule::Load()
{
    DECLARE_MODULE_EC(EC_Billboard);
    DECLARE_MODULE_EC(EC_ChatBubble);
}

void InWorldChatModule::PostInitialize()
{
    frameworkEventCategory_ = framework_->GetEventManager()->QueryEventCategory("Framework");
    if (frameworkEventCategory_ == 0)
        LogError("Failed to query \"Framework\" event category");

    uiModule_ = framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices);

    RegisterConsoleCommand(Console::CreateCommand("bbtest", 
        "Adds a billboard to each entity in the scene.",
        Console::Bind(this, &InWorldChatModule::TestAddBillboard)));

    RegisterConsoleCommand(Console::CreateCommand("chat", 
        "Sends a chat message.  Usage: \"chat(message)\"",
        Console::Bind(this, &InWorldChatModule::ConsoleChat)));
}

void InWorldChatModule::Update(f64 frametime)
{
}

bool InWorldChatModule::HandleEvent(
    event_category_id_t category_id,
    event_id_t event_id,
    Foundation::EventDataInterface *data)
{
    if (category_id == frameworkEventCategory_)
    {
        if (event_id == Foundation::NETWORKING_REGISTERED)
        {
            ProtocolUtilities::NetworkingRegisteredEvent *event_data = dynamic_cast<ProtocolUtilities::NetworkingRegisteredEvent *>(data);
            if (event_data)
                SubscribeToNetworkEvents(event_data->currentProtocolModule);
            return false;
        }

        if(event_id == Foundation::WORLD_STREAM_READY)
        {
            ProtocolUtilities::WorldStreamReadyEvent *event_data = dynamic_cast<ProtocolUtilities::WorldStreamReadyEvent *>(data);
            if (event_data)
                currentWorldStream_ = event_data->WorldStream;

            //if (chatWindow_)
            //    chatWindow_->SetWorldStreamPtr(current_world_stream_);

            networkInEventCategory_ = framework_->GetEventManager()->QueryEventCategory("NetworkIn");
            if (networkInEventCategory_ == 0)
                LogError("Failed to query \"NetworkIn\" event category");

            return false;
        }
    }

    if (category_id == networkStateEventCategory_)
    {
        // Connected to server. Create chat UI.
        if (event_id == ProtocolUtilities::Events::EVENT_SERVER_CONNECTED)
        {
            if (!uiModule_.expired())
            {
                //uiModule_.lock()->
            }
        }
        // Disconnected from server. Delete chat UI.
        if (event_id == ProtocolUtilities::Events::EVENT_SERVER_DISCONNECTED)
        {
            if (!uiModule_.expired())
            {
                //uiModule_.lock()->
            }
        }
    }

    if (category_id == networkInEventCategory_)
    {
        if(event_id == RexNetMsgChatFromSimulator)
        {
            ProtocolUtilities::NetworkEventInboundData *netdata = checked_static_cast<ProtocolUtilities::NetworkEventInboundData *>(data);
            assert(netdata);
            if (!netdata)
                return false;

//            assert(currentWorldStream_);

            std::stringstream ss;

            ProtocolUtilities::NetInMessage &msg = *netdata->message;
            msg.ResetReading();

            std::string fromName = msg.ReadString();
            RexUUID sourceId = msg.ReadUUID();

//            if (sourceId == currentWorldStream_->GetInfo().agentID)
//                return false;

            msg.SkipToFirstVariableByName("Message");
            std::string message = msg.ReadString();
            if (message.size() < 1)
                return false;

            ss << "[" << GetLocalTimeString() << "] " << fromName << ": " << message << std::endl;


            LogInfo(ss.str());

            Scene::ScenePtr scene = GetFramework()->GetDefaultWorldScene();
            for(Scene::SceneManager::iterator iter = scene->begin(); iter != scene->end(); ++iter)
            {
                Scene::Entity &entity = **iter;
                boost::shared_ptr<RexLogic::EC_OpenSimPresence> ec_presence = entity.GetComponent<RexLogic::EC_OpenSimPresence>();
                if (!ec_presence)
                    continue;

                if (ec_presence->FullId != sourceId)
                    continue;

                Foundation::ComponentInterfacePtr component = entity.GetOrCreateComponent(EC_ChatBubble::NameStatic());
                assert(component.get());
                EC_ChatBubble &chatBubble = *(checked_static_cast<EC_ChatBubble *>(component.get()));
                chatBubble.ShowMessage(message.c_str());
            }

/*
            QPixmap pixmap = GetPixmap("./media/textures/ChatBubble.png", message.c_str());
            if (!pixmap)
                return false;

            QLabel *w = new QLabel;
            w->setPixmap(pixmap);
            w->setAttribute(Qt::WA_DeleteOnClose);
            w->show();
*/
            // Connect chat ui to this modules ChatReceived
            // emit ChatReceived()
            //if (chatWindow_)
            //    chatWindow_->CharReceived(name, msg);

            return false;
        }
    }

    return false;
}

void InWorldChatModule::SubscribeToNetworkEvents(ProtocolUtilities::ProtocolWeakPtr currentProtocolModule)
{
    networkStateEventCategory_ = framework_->GetEventManager()->QueryEventCategory("NetworkState");
    if (networkStateEventCategory_ == 0)
        LogError("Failed to query \"NetworkState\" event category");

    networkInEventCategory_ = framework_->GetEventManager()->QueryEventCategory("NetworkIn");
    if (networkInEventCategory_ == 0)
        LogError("Failed to query \"NetworkIn\" event category");
}

const std::string InWorldChatModule::moduleName = std::string("InWorldChatModule");

const std::string &InWorldChatModule::NameStatic()
{
    return moduleName;
}

Console::CommandResult InWorldChatModule::TestAddBillboard(const StringVector &params)
{
    Scene::ScenePtr scene = GetFramework()->GetDefaultWorldScene();
    /// If/when there are multiple scenes at some day, have the SceneManager know the currently active one instead of RexLogicModule, so no dependency to it is needed.

    for(Scene::SceneManager::iterator iter = scene->begin(); iter != scene->end(); ++iter)
    {
        Scene::EntityPtr entity = *iter;
        entity->AddComponent(framework_->GetComponentManager()->CreateComponent("EC_Billboard"));
        EC_Billboard *billboard = entity->GetComponent<EC_Billboard>().get();
        assert(billboard);
        billboard->Show(Vector3df(0.f, 0.f, 1.5f), 10.f, "bubble.png");
    }

    return Console::ResultSuccess();
}

Console::CommandResult InWorldChatModule::ConsoleChat(const StringVector &params)
{
    if (params.size() == 0)
        return Console::ResultFailure("Can't send empty chat message!");

    SendChatFromViewer(params[0].c_str());
    return Console::ResultSuccess();
}

void InWorldChatModule::SendChatFromViewer(const QString &msg)
{
    if (currentWorldStream_)
        currentWorldStream_->SendChatFromViewerPacket(msg.toStdString());
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

}

using namespace Naali;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
    POCO_EXPORT_CLASS(InWorldChatModule)
POCO_END_MANIFEST
