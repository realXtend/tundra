/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   InWorldChatModule.cpp
 *  @brief  Simple OpenSim world chat module. Listens for ChatFromSimulator packets and shows the chat on the UI.
 *          Outgoing chat sent using ChatFromViewer packets. Manages EC_ChatBubbles.
 *  @note   Depends on RexLogicModule so don't create dependency to this module.
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
#include "GenericMessageUtils.h"
#include "UiModule.h"
#include "ConfigurationManager.h"

#include "EntityComponent/EC_OpenSimPresence.h"
#include "EntityComponent/EC_OpenSimPrim.h"

#include <QColor>

namespace Naali
{

InWorldChatModule::InWorldChatModule() :
    ModuleInterfaceImpl(NameStatic()),
    networkStateEventCategory_(0),
    networkInEventCategory_(0),
    frameworkEventCategory_(0),
    showChatBubbles_(true)
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
        "Sends a chat message. Usage: \"chat(message)\"",
        Console::Bind(this, &InWorldChatModule::ConsoleChat)));

    showChatBubbles_ = framework_->GetDefaultConfig().DeclareSetting("InWorldChatModule", "ShowChatBubbles", true);
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
        if (event_id != RexNetMsgGenericMessage && event_id != RexNetMsgChatFromSimulator)
            return false;

        using namespace ProtocolUtilities;
        NetworkEventInboundData *netdata = checked_static_cast<NetworkEventInboundData *>(data);
        assert(netdata);
        if (!netdata)
            return false;
        ProtocolUtilities::NetInMessage &msg = *netdata->message;

        switch(event_id)
        {
        case RexNetMsgGenericMessage:
        {
            std::string method = ParseGenericMessageMethod(msg);
            StringVector params = ParseGenericMessageParameters(msg);

            if (method == "RexEmotionIcon")
                HandleRexEmotionIconMessage(params);

            return false;
        }
        case RexNetMsgChatFromSimulator:
        {
            HandleChatFromSimulatorMessage(msg);
            return false;
        }
        default:
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

void InWorldChatModule::SendChatFromViewer(const QString &msg)
{
    if (currentWorldStream_)
        currentWorldStream_->SendChatFromViewerPacket(std::string(msg.toUtf8()));
}

Console::CommandResult InWorldChatModule::TestAddBillboard(const StringVector &params)
{
    Scene::ScenePtr scene = GetFramework()->GetDefaultWorldScene();
    /// If/when there are multiple scenes at some day, have the SceneManager know the currently active one
    /// instead of RexLogicModule, so no dependency to it is needed.
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

void InWorldChatModule::ApplyDefaultChatBubble(Scene::Entity &entity, const QString &message)
{
    Foundation::ComponentInterfacePtr component = entity.GetOrCreateComponent(EC_ChatBubble::TypeNameStatic());
    assert(component.get());
    EC_ChatBubble &chatBubble = *(checked_static_cast<EC_ChatBubble *>(component.get()));
    chatBubble.ShowMessage(message);
}

void InWorldChatModule::ApplyBillboard(Scene::Entity &entity, const std::string &texture, float timeToShow)
{
    boost::shared_ptr<EC_Billboard> ec_bb = entity.GetComponent<EC_Billboard>();

    // If we didn't have the billboard component yet, create one now.
    if (!ec_bb)
    {
        entity.AddComponent(framework_->GetComponentManager()->CreateComponent("EC_Billboard"));
        ec_bb = entity.GetComponent<EC_Billboard>();
        assert(ec_bb.get());
    }

    ec_bb->Show(Vector3df(0.f, 0.f, 1.5f), timeToShow, texture.c_str());
}

Scene::Entity *InWorldChatModule::GetEntityWithId(const RexUUID &id)
{
    Scene::ScenePtr scene = GetFramework()->GetDefaultWorldScene();
    for(Scene::SceneManager::iterator iter = scene->begin(); iter != scene->end(); ++iter)
    {
        Scene::Entity &entity = **iter;

        boost::shared_ptr<RexLogic::EC_OpenSimPresence> ec_presence = entity.GetComponent<RexLogic::EC_OpenSimPresence>();
        boost::shared_ptr<RexLogic::EC_OpenSimPrim> ec_prim = entity.GetComponent<RexLogic::EC_OpenSimPrim>();

        if (ec_presence)
        {
            if (ec_presence->FullId == id)
                return &entity;
        }
        else if (ec_prim)
        {
            if (ec_prim->FullId == id)
                return &entity;
        }
    }

    return 0;
}

void InWorldChatModule::HandleRexEmotionIconMessage(StringVector &params)
{
    // Param 0: avatar UUID
    // Param 1: texture ID
    // Param 2: timeout (remember to replace any , with . before parsing)
    if (params.size() < 3)
        throw Exception("Failed to parse RexEmotionIcon message!");

    LogInfo("Received RexEmotionIcon: " + params[0] + " " + params[1] + " " + params[2]);

    ReplaceCharInplace(params[2], ',', '.');

    if (!RexUUID::IsValid(params[0]))
        throw Exception("Invalid Entity UUID passed in RexEmotionIcon message!");

    RexUUID entityUUID(params[0]);
    if (entityUUID.IsNull())
        throw Exception("Null Entity UUID passed in RexEmotionIcon message!");

    Scene::Entity *entity = GetEntityWithId(entityUUID);
    if (!entity)
        throw Exception("Received RexEmotionIcon message for a nonexisting entity!");

    // timeToShow: value of [-1, 0[ denotes "infinite".
    //             a positive value between [0, 86400] denotes the number of seconds to show. (max roughly one day)
    float timeToShow = atof(params[2].c_str());
    if (!(timeToShow >= -2.f && timeToShow <= 86401.f)) // Checking through negation due to possible NaNs and infs. (being lax and also allowing off-by-one)
        throw Exception("Invalid time-to-show passed in RexEmotionIcon message!");

    if (RexUUID::IsValid(params[1]))
    {
        RexUUID textureID(params[1]);
        // We've been passed a texture UUID to show on the billboard.
        ///\todo request the asset and show that on the billboard.
    }
    else // We've been passed a string URL or a filename on the local computer.
    {
        ///\todo Request a download from that URL and show the resulting image on the billboard.

        // Now assuming that the textureID points to a local file, just to get a proof-of-concept something showing in the UI.
        ApplyBillboard(*entity, params[1], timeToShow);
    }
}

void InWorldChatModule::HandleChatFromSimulatorMessage(ProtocolUtilities::NetInMessage &msg)
{
    msg.ResetReading();

    std::string fromName = msg.ReadString();
    RexUUID sourceId = msg.ReadUUID();

    msg.SkipToFirstVariableByName("Message");
    std::string message = msg.ReadString();
    if (message.size() < 1)
        return;

/*
    std::stringstream ss;
    ss << "[" << GetLocalTimeString() << "] " << fromName << ": " << message;
    LogDebug(ss.str());
*/

    if (showChatBubbles_)
    {
        Scene::Entity *entity = GetEntityWithId(sourceId);
        if (entity)
            ApplyDefaultChatBubble(*entity, QString::fromUtf8(message.c_str()));
    }

    // Connect chat ui to this modules ChatReceived
    // emit ChatReceived()
    //if (chatWindow_)
    //    chatWindow_->CharReceived(name, msg);
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
