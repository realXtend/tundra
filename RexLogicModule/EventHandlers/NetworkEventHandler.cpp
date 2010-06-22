/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   NetworkStateEventHandler.cpp
 *  @brief  Handles incoming SLUDP network events (messages) in a reX-specific way.
 */

#include "StableHeaders.h"
#include "EventHandlers/NetworkEventHandler.h"
#include "RexLogicModule.h"
#include "Avatar/AvatarControllable.h"
#include "Avatar/Avatar.h"
#include "Environment/Primitive.h"
#include "Communications/ScriptDialogHandler.h"
#include "Communications/ScriptDialogRequest.h"
#include "Communications/InWorldChat/Provider.h"

#include "NetworkMessages/NetInMessage.h"
#include "WorldStream.h"
#include "RealXtend/RexProtocolMsgIDs.h"
#include "ProtocolModuleOpenSim.h"
#include "BitStream.h"
#include "GenericMessageUtils.h"
#include "SceneEvents.h"
#include "Entity.h"
#include "ModuleManager.h"
#include "EventManager.h"
#include "UiModule.h"
#include "Inworld/InworldSceneController.h"
#include "Inworld/ControlPanel/TeleportWidget.h"
#include "Inworld/NotificationManager.h"
#include "Inworld/ControlPanelManager.h"
#include "Inworld/Notifications/QuestionNotification.h"
#include "Ether/EtherLoginNotifier.h"
#include "ServiceManager.h"
#include "SoundServiceInterface.h"
#include "AssetServiceInterface.h"
#include "ScriptServiceInterface.h" // LoadURL webview opening code is not on the py side, experimentally at least

namespace RexLogic
{

NetworkEventHandler::NetworkEventHandler(RexLogicModule *owner) :
    owner_(owner),
    ongoing_script_teleport_(false)
{
    // Get the pointe to the current protocol module
    ProtocolUtilities::ProtocolModuleInterface *protocol = owner_->GetServerConnection()->GetCurrentProtocolModuleWeakPointer().lock().get();
    if (protocol)
        RexLogicModule::LogInfo("NetworkEventHandler: Protocol module not set yet. Will fetch when networking occurs.");

    script_dialog_handler_ = ScriptDialogHandlerPtr(new ScriptDialogHandler(owner_));
}

NetworkEventHandler::~NetworkEventHandler()
{
    script_dialog_handler_.reset();
}

bool NetworkEventHandler::HandleOpenSimNetworkEvent(event_id_t event_id, Foundation::EventDataInterface* data)
{
    PROFILE(NetworkEventHandler_HandleOpenSimNetworkEvent);
    ProtocolUtilities::NetworkEventInboundData *netdata = checked_static_cast<ProtocolUtilities::NetworkEventInboundData *>(data);
    switch(event_id)
    {
    case RexNetMsgRegionHandshake:
        return HandleOSNE_RegionHandshake(netdata);

    case RexNetMsgAgentMovementComplete:
        return HandleOSNE_AgentMovementComplete(netdata);

    case RexNetMsgAvatarAnimation:
        return owner_->GetAvatarHandler()->HandleOSNE_AvatarAnimation(netdata);

    case RexNetMsgGenericMessage:
        return HandleOSNE_GenericMessage(netdata);

    case RexNetMsgLogoutReply:
        return HandleOSNE_LogoutReply(netdata);

    case RexNetMsgImprovedTerseObjectUpdate:
        return HandleOSNE_ImprovedTerseObjectUpdate(netdata);

    case RexNetMsgKillObject:
        return HandleOSNE_KillObject(netdata);

    case RexNetMsgObjectUpdate:
        return HandleOSNE_ObjectUpdate(netdata);

    case RexNetMsgObjectProperties:
        return owner_->GetPrimitiveHandler()->HandleOSNE_ObjectProperties(netdata);

    case RexNetMsgAttachedSound:
        return owner_->GetPrimitiveHandler()->HandleOSNE_AttachedSound(netdata);

    case RexNetMsgAttachedSoundGainChange:
        return owner_->GetPrimitiveHandler()->HandleOSNE_AttachedSoundGainChange(netdata);

    case RexNetMsgSoundTrigger:
        return HandleOSNE_SoundTrigger(netdata);

    case RexNetMsgPreloadSound:
        return HandleOSNE_PreloadSound(netdata);

    case RexNetMsgScriptDialog:
        return HandleOSNE_ScriptDialog(netdata);

    case RexNetMsgLoadURL:
        return HandleOSNE_LoadURL(netdata);

    case RexNetMsgMapBlockReply:
        return HandleOSNE_MapBlock(netdata);

    case RexNetMsgScriptTeleportRequest:
        return HandleOSNE_ScriptTeleport(netdata);

    case RexNetMsgChatFromSimulator:
        return HandleOSNE_ChatFromSimulator(netdata);

    case RexNetMsgImprovedInstantMessage:
        return HandleOSNE_ImprovedInstantMessage(netdata);

    case RexNetMsgOnlineNotification:
        return HandleOSNE_OnlineNotification(netdata);

    case RexNetMsgOfflineNotification:
        return HandleOSNE_OfflineNotification(netdata);

    case RexNetMsgTerminateFriendship:
        return HandleOSNE_TerminateFriendship(netdata);

    case RexNetMsgDeclineFriendship:
        return HandleOSNE_DeclineFriendship(netdata);

    case RexNetMsgKickUser:
        return HandleOSNE_KickUser(netdata);

    case RexNetMsgEstateOwnerMessage:
        return HandleOSNE_EstateOwnerMessage(netdata);

    default:
        break;
    }

    return false;
}

bool NetworkEventHandler::HandleOSNE_ObjectUpdate(ProtocolUtilities::NetworkEventInboundData* data)
{
    ProtocolUtilities::NetInMessage &msg = *data->message;
    msg.ResetReading();
    msg.SkipToNextVariable();
    msg.SkipToNextVariable();

    if (msg.GetCurrentBlock() >= msg.GetBlockCount())
    {
        RexLogicModule::LogDebug("Empty ObjectUpdate packet received, ignoring.");
        return false;
    }

    size_t instance_count = msg.ReadCurrentBlockInstanceCount();
    bool result = false;
    if (instance_count > 0)
    {
        msg.SkipToFirstVariableByName("PCode");
        uint8_t pcode = msg.ReadU8();
        switch(pcode)
        {
        case 0x09:
            result = owner_->GetPrimitiveHandler()->HandleOSNE_ObjectUpdate(data);
            break;

        case 0x2f:
            result = owner_->GetAvatarHandler()->HandleOSNE_ObjectUpdate(data);
            break;
        }
    }

    return result;
}

bool NetworkEventHandler::HandleOSNE_GenericMessage(ProtocolUtilities::NetworkEventInboundData* data)
{
    ProtocolUtilities::NetInMessage &msg = *data->message;
    std::string methodname = ProtocolUtilities::ParseGenericMessageMethod(msg);

    if (methodname == "RexMediaUrl")
        return owner_->GetPrimitiveHandler()->HandleRexGM_RexMediaUrl(data);
    else if (methodname == "RexData")
        return owner_->GetPrimitiveHandler()->HandleRexGM_RexFreeData(data); 
    else if (methodname == "RexPrimData")
        return owner_->GetPrimitiveHandler()->HandleRexGM_RexPrimData(data); 
    else if (methodname == "RexPrimAnim")
        return owner_->GetPrimitiveHandler()->HandleRexGM_RexPrimAnim(data); 
    else if (methodname == "RexAppearance")
        return owner_->GetAvatarHandler()->HandleRexGM_RexAppearance(data);
    else if (methodname == "RexAnim")
        return owner_->GetAvatarHandler()->HandleRexGM_RexAnim(data);
    else
        return false;
}

bool NetworkEventHandler::HandleOSNE_RegionHandshake(ProtocolUtilities::NetworkEventInboundData* data)
{
    ///\note This message is mostly handled by EnvironmentModule.
    ProtocolUtilities::NetInMessage &msg = *data->message;
    msg.ResetReading();

    msg.SkipToNextVariable(); // RegionFlags U32
    msg.SkipToNextVariable(); // SimAccess U8

    std::string sim_name = msg.ReadString(); // SimName
    owner_->GetServerConnection()->SetSimName(sim_name);
    RexLogicModule::LogInfo("Joined to sim " + sim_name);

    // Create the "World" scene.
    boost::shared_ptr<ProtocolUtilities::ProtocolModuleInterface> sp = owner_->GetServerConnection()->GetCurrentProtocolModuleWeakPointer().lock();
    if (!sp.get())
    {
        RexLogicModule::LogError("NetworkEventHandler: Could not acquire Protocol Module!");
        return false;
    }

    const ProtocolUtilities::ClientParameters& client = sp->GetClientParameters();
    owner_->GetServerConnection()->SendRegionHandshakeReplyPacket(client.agentID, client.sessionID, 0);

    // Tell teleportWidget current region name
    boost::shared_ptr<UiServices::UiModule> ui_module =
        owner_->GetFramework()->GetModuleManager()->GetModule<UiServices::UiModule>().lock();
    if (ui_module)
        ui_module->GetInworldSceneController()->GetControlPanelManager()->GetTeleportWidget()->SetCurrentRegion(QString(owner_->GetServerConnection()->GetSimName().c_str()));

    return false;
}

bool NetworkEventHandler::HandleOSNE_LogoutReply(ProtocolUtilities::NetworkEventInboundData* data)
{
    ProtocolUtilities::NetInMessage &msg = *data->message;
    msg.ResetReading();

    RexUUID aID = msg.ReadUUID();
    RexUUID sID = msg.ReadUUID();

    if (aID == owner_->GetServerConnection()->GetInfo().agentID &&
        sID == owner_->GetServerConnection()->GetInfo().sessionID)
    {
        RexLogicModule::LogInfo("LogoutReply received with matching IDs. Logging out.");
        owner_->GetServerConnection()->ForceServerDisconnect();
        owner_->DeleteScene("World");
    }

    return false;
}

bool NetworkEventHandler::HandleOSNE_AgentMovementComplete(ProtocolUtilities::NetworkEventInboundData* data)
{
    ProtocolUtilities::NetInMessage &msg = *data->message;
    msg.ResetReading();

    RexUUID agentid = msg.ReadUUID();
    RexUUID sessionid = msg.ReadUUID();

//    if (agentid == owner_->GetServerConnection()->GetInfo().agentID &&
//    sessionid == owner_->GetServerConnection()->GetInfo().sessionID)
    {
        Vector3 position = msg.ReadVector3(); 
        Vector3 lookat = msg.ReadVector3();

        assert(owner_->GetAvatarControllable() && "Handling agent movement complete event before avatar controller is created.");
        owner_->GetAvatarControllable()->HandleAgentMovementComplete(position, lookat);

        /// \todo tucofixme, what to do with regionhandle & timestamp?
        uint64_t regionhandle = msg.ReadU64();
        uint32_t timestamp = msg.ReadU32(); 
    }

    owner_->GetServerConnection()->SendAgentSetAppearancePacket();
    return false;
}

bool NetworkEventHandler::HandleOSNE_ImprovedTerseObjectUpdate(ProtocolUtilities::NetworkEventInboundData* data)
{
    ProtocolUtilities::NetInMessage &msg = *data->message;
    msg.ResetReading();

    uint64_t regionhandle = msg.ReadU64();
    msg.SkipToNextVariable(); // TimeDilation U16 ///\todo Unhandled inbound variable 'TimeDilation'.

    if (msg.GetCurrentBlock() >= msg.GetBlockCount())
    {
        RexLogicModule::LogDebug("Empty ImprovedTerseObjectUpdate packet received, ignoring.");
        return false;
    }

    // Variable block
    size_t instance_count = msg.ReadCurrentBlockInstanceCount();
    for(size_t i = 0; i < instance_count; i++)
    {
        size_t bytes_read = 0;
        const uint8_t *bytes = msg.ReadBuffer(&bytes_read);

        uint32_t localid = 0;
        switch(bytes_read)
        {
        case 30:
            owner_->GetAvatarHandler()->HandleTerseObjectUpdate_30bytes(bytes); 
            break;
        case 44:
            //this size is only for prims
            localid = *reinterpret_cast<uint32_t*>((uint32_t*)&bytes[0]);
            if (owner_->GetPrimEntity(localid))
                owner_->GetPrimitiveHandler()->HandleTerseObjectUpdateForPrim_44bytes(bytes);
            break;
        case 60:
            localid = *reinterpret_cast<uint32_t*>((uint32_t*)&bytes[0]); 
            if (owner_->GetPrimEntity(localid)) 
                owner_->GetPrimitiveHandler()->HandleTerseObjectUpdateForPrim_60bytes(bytes);
            else if (owner_->GetAvatarEntity(localid))
                owner_->GetAvatarHandler()->HandleTerseObjectUpdateForAvatar_60bytes(bytes);
            break;
        default:
            std::stringstream ss; 
            ss << "Unhandled ImprovedTerseObjectUpdate block of size " << bytes_read << "!";
            RexLogicModule::LogInfo(ss.str());
            break;
        }

        msg.SkipToNextVariable(); ///\todo Unhandled inbound variable 'TextureEntry'.
    }
    return false;
}

bool NetworkEventHandler::HandleOSNE_KillObject(ProtocolUtilities::NetworkEventInboundData* data)
{
    ProtocolUtilities::NetInMessage &msg = *data->message;
    msg.ResetReading();

    // Variable block
    size_t instance_count = msg.ReadCurrentBlockInstanceCount();
    for(size_t i = 0; i < instance_count; ++i)
    {
        uint32_t killedobjectid = msg.ReadU32();
        if (owner_->GetPrimEntity(killedobjectid))
            return owner_->GetPrimitiveHandler()->HandleOSNE_KillObject(killedobjectid);
        if (owner_->GetAvatarEntity(killedobjectid))
            return owner_->GetAvatarHandler()->HandleOSNE_KillObject(killedobjectid);
    }

    return false;
}

bool NetworkEventHandler::HandleOSNE_PreloadSound(ProtocolUtilities::NetworkEventInboundData* data)
{
    ProtocolUtilities::NetInMessage &msg = *data->message;
    msg.ResetReading();

    size_t instance_count = data->message->ReadCurrentBlockInstanceCount();
    while(instance_count)
    {
        msg.ReadUUID(); // ObjectID
        msg.ReadUUID(); // OwnerID
        std::string asset_id = msg.ReadUUID().ToString(); // Sound asset ID

        // Preload the sound asset into cache, the sound service will get it from there when actually needed.
        boost::shared_ptr<Foundation::AssetServiceInterface> asset_service =
            owner_->GetFramework()->GetServiceManager()->GetService<Foundation::AssetServiceInterface>(Foundation::Service::ST_Asset).lock();
        if (asset_service)
            asset_service->RequestAsset(asset_id, RexTypes::ASSETTYPENAME_SOUNDVORBIS);

        --instance_count;
    }

    return false;
}

bool NetworkEventHandler::HandleOSNE_SoundTrigger(ProtocolUtilities::NetworkEventInboundData* data)
{
    ProtocolUtilities::NetInMessage &msg = *data->message;
    msg.ResetReading();

    std::string asset_id = msg.ReadUUID().ToString(); // Sound asset ID
    msg.ReadUUID(); // OwnerID
    msg.ReadUUID(); // ObjectID
    msg.ReadUUID(); // ParentID
    msg.ReadU64(); // Regionhandle, todo handle
    Vector3df position = msg.ReadVector3(); // Position
    Real gain = msg.ReadF32(); // Gain

    // Because sound triggers are not supposed to stop the previous sound, like attached sounds do, 
    // it is easy to spam with 100's of sound trigger requests.
    // What we do now is that if we find the same sound playing "too many" times, we stop one of them
    static const uint MAX_SOUND_INSTANCE_COUNT = 4;
    uint same_sound_detected = 0;
    sound_id_t sound_to_stop = 0;
    boost::shared_ptr<Foundation::SoundServiceInterface> soundsystem =
        owner_->GetFramework()->GetServiceManager()->GetService<Foundation::SoundServiceInterface>(Foundation::Service::ST_Sound).lock();
    if (!soundsystem)
        return false;

    std::vector<sound_id_t> playing_sounds = soundsystem->GetActiveSounds();
    for (uint i = 0; i < playing_sounds.size(); ++i)
    {
        if ((soundsystem->GetSoundName(playing_sounds[i]) == asset_id) &&
            (soundsystem->GetSoundType(playing_sounds[i]) == Foundation::SoundServiceInterface::Triggered))
        {
            same_sound_detected++;
            // This should be the oldest instance of the sound, because soundsystem gives channel ids from a map (ordered).
            if (!sound_to_stop)
                sound_to_stop = playing_sounds[i]; 
        }
    }

    if (same_sound_detected >= MAX_SOUND_INSTANCE_COUNT)
        soundsystem->StopSound(sound_to_stop);

    sound_id_t new_sound = soundsystem->PlaySound3D(asset_id, Foundation::SoundServiceInterface::Triggered, false, position);
    soundsystem->SetGain(new_sound, gain);

    return false;
}

bool NetworkEventHandler::HandleOSNE_ScriptDialog(ProtocolUtilities::NetworkEventInboundData* data)
{
    ProtocolUtilities::NetInMessage &msg = *data->message;
    msg.ResetReading();

    std::string object_id = msg.ReadUUID().ToString(); // ObjectID
    std::string first_name = msg.ReadString(); // FirstName
    std::string last_name = msg.ReadString(); // LastName
    std::string object_name = msg.ReadString(); // ObjectName
    std::string message = msg.ReadString(); // Message
    s32 chat_channel = msg.ReadS32(); // ChatChannel
    std::string image_id = msg.ReadUUID().ToString(); // ImageID
    
    size_t instance_count = data->message->ReadCurrentBlockInstanceCount();
    std::vector<std::string> button_labels;
    while(instance_count)
    {
        std::string button_label = msg.ReadString(); // ButtonLabel
        button_labels.push_back(button_label);
        --instance_count;
    }

    std::string owners_name = first_name + " " + last_name;
    ScriptDialogRequest request(object_name, owners_name, message, chat_channel, button_labels);
    if (script_dialog_handler_)
        script_dialog_handler_->Handle(request);

    return false;
}

bool NetworkEventHandler::HandleOSNE_LoadURL(ProtocolUtilities::NetworkEventInboundData *data)
{
    ProtocolUtilities::NetInMessage &msg = *data->message;
    msg.ResetReading();

    std::string object_name = msg.ReadString(); // FirstName
    std::string object_id = msg.ReadUUID().ToString(); // ObjectID
    std::string owner_id = msg.ReadUUID().ToString(); // ObjectID
    bool owner_is_group = msg.ReadBool(); // OwnerIsGroup
    std::string message = msg.ReadString(); // Message
    std::string url = msg.ReadString(); // URL

    boost::shared_ptr<Foundation::ScriptServiceInterface> pyservice =
        owner_->GetFramework()->GetServiceManager()->GetService<Foundation::ScriptServiceInterface>(Foundation::Service::ST_PythonScripting).lock();
    if (pyservice)
        pyservice->RunString("import loadurlhandler; loadurlhandler.loadurl('" + QString::fromStdString(url) + "');");

    return false;
}

bool NetworkEventHandler::HandleOSNE_MapBlock(ProtocolUtilities::NetworkEventInboundData *data)
{
    ProtocolUtilities::NetInMessage &msg = *data->message;
    msg.ResetReading();

    RexUUID agent_id = msg.ReadUUID();
    uint32_t flags_ = msg.ReadU32();
    size_t instance_count = msg.ReadCurrentBlockInstanceCount();

    QList<ProtocolUtilities::MapBlock> mapBlocks;
    for (size_t i = 0; i < instance_count; ++i)
    {
        ProtocolUtilities::MapBlock block;
        block.agentID = agent_id;
        block.flags = flags_;
        block.regionX = msg.ReadU16();
        block.regionY = msg.ReadU16();
        block.regionName = msg.ReadString();
        block.access = msg.ReadU8();
        block.regionFlags = msg.ReadU32();
        block.waterHeight = msg.ReadU8();
        block.agents = msg.ReadU8();
        block.mapImageID = msg.ReadUUID();
        mapBlocks.append(block);
    }

    boost::shared_ptr<UiServices::UiModule> ui_module =
        owner_->GetFramework()->GetModuleManager()->GetModule<UiServices::UiModule>().lock();
    if (ui_module)
        ui_module->GetInworldSceneController()->GetControlPanelManager()->GetTeleportWidget()->SetMapBlocks(mapBlocks);
    return false;
}

bool NetworkEventHandler::HandleOSNE_ScriptTeleport(ProtocolUtilities::NetworkEventInboundData *data)
{
    ProtocolUtilities::NetInMessage &msg = *data->message;
    msg.ResetReading();

    std::string object_name = msg.ReadString(); // Sender name
    std::string region_name = msg.ReadString(); // Sim name
    Vector3df position = msg.ReadVector3(); // Sim position
    Vector3df lookAt = msg.ReadVector3(); // LookAt

    if (region_name.empty())
        return false;

    // Ui module
    boost::shared_ptr<UiServices::UiModule> ui_module =
        owner_->GetFramework()->GetModuleManager()->GetModule<UiServices::UiModule>().lock();
    if (!ui_module)
        return false;
            
    // Notifier qobject ptr
    QObject *object = ui_module->GetEtherLoginNotifier();
    if (!object)
        return false;

    // Cast to actual class from qobject ptr
    Ether::Logic::EtherLoginNotifier* notifier = dynamic_cast<Ether::Logic::EtherLoginNotifier*>(object);
    if (notifier)
    {
        if (!notifier->IsTeleporting())
            ongoing_script_teleport_ = false;
        
        if (!ongoing_script_teleport_)
        {
            // Create question notification
            QString posx = "";
            QString posy = "";
            QString posz = "";
            posx.setNum(position.x);
            posy.setNum(position.y);
            posz.setNum(position.z);

            UiServices::QuestionNotification *question_notification = 
                new UiServices::QuestionNotification(QString("Do you want to teleport to region %1.").arg(region_name.c_str()),
                    "Yes", "No", "", QString(region_name.c_str())+"&"+posx+"&"+posy+"&"+posz, 7000);

            // Connect notifier to recieve the answer signal
            QObject::connect(question_notification, SIGNAL(QuestionAnswered(QString, QString)), notifier, SLOT(ScriptTeleportAnswer(QString, QString)));

            // Send notification
            ui_module->GetNotificationManager()->ShowNotification(question_notification);

            // Set bools that we dont get spam notification if you are standing in the script zone
            ongoing_script_teleport_ = true;
            notifier->SetIsTeleporting(true);
        }
    }
    return false;
}

bool NetworkEventHandler::HandleOSNE_ChatFromSimulator(ProtocolUtilities::NetworkEventInboundData *data)
{
    /// @todo: IMPLEMENT

    enum ChatType { Whisper = 0, Say = 1, Shout = 2, StartTyping = 4, StopTyping = 5, DebugChannel = 6, Region = 7, Owner = 8, Broadcast = 0xFF };
    enum ChatAudibleLevel { Not = -1, Barely = 0, Fully = 1 };
    enum ChatSourceType { SOURCE_TYPE_SYSTEM = 0, SOURCE_TYPE_AGENT = 1, SOURCE_TYPE_OBJECT = 2 };
/*
    enum IMDialogTypes { DT_MessageFromAgent = 0, DT_MessageFromObject = 19, DT_FriendshipOffered = 38, DT_FriendshipAccepted = 39,
        DT_FriendshipDeclined = 40, DT_StartTyping = 41, DT_StopTyping = 42 };
*/

    ProtocolUtilities::NetInMessage &msg = *data->message;
    msg.ResetReading();

    std::string from_name = msg.ReadString();
    RexUUID source = msg.ReadUUID();
    RexUUID object_owner = msg.ReadUUID();
    ChatSourceType source_type = static_cast<ChatSourceType>(msg.ReadU8());
    ChatType chat_type = static_cast<ChatType>(msg.ReadU8());
    ChatAudibleLevel audible = static_cast<ChatAudibleLevel>(msg.ReadU8());
    RexTypes::Vector3 position = msg.ReadVector3();
    std::string message = msg.ReadString();
    if (message.size() > 0)
    {
        switch (source_type)
        {
        case SOURCE_TYPE_SYSTEM:
        case SOURCE_TYPE_AGENT:
        case SOURCE_TYPE_OBJECT:
            owner_->GetInWorldChatProvider()->HandleIncomingChatMessage(source.ToQString(),
                QString::fromUtf8(from_name.c_str()), QString::fromUtf8(message.c_str()));
            break;
        }
    }

    return false;
}

bool NetworkEventHandler::HandleOSNE_ImprovedInstantMessage(ProtocolUtilities::NetworkEventInboundData *data)
{
    /// @todo: IMPLEMENT
    return false;
}

bool NetworkEventHandler::HandleOSNE_OnlineNotification(ProtocolUtilities::NetworkEventInboundData *data)
{
    /// @todo: IMPLEMENT
    return false;
}

bool NetworkEventHandler::HandleOSNE_OfflineNotification(ProtocolUtilities::NetworkEventInboundData *data)
{
    /// @todo: IMPLEMENT
    return false;
}

bool NetworkEventHandler::HandleOSNE_TerminateFriendship(ProtocolUtilities::NetworkEventInboundData *data)
{
    /// @todo: IMPLEMENT
    return false;
}

bool NetworkEventHandler::HandleOSNE_DeclineFriendship(ProtocolUtilities::NetworkEventInboundData *data)
{
    /// @todo: IMPLEMENT
    return false;
}

bool NetworkEventHandler::HandleOSNE_KickUser(ProtocolUtilities::NetworkEventInboundData *data)
{
    ProtocolUtilities::NetInMessage &msg = *data->message;

    msg.ResetReading();
    msg.SkipToFirstVariableByName("AgentID");
    RexUUID agent_id = msg.ReadUUID();
    RexUUID session_id = msg.ReadUUID();
    std::string reason = msg.ReadString();

    if (agent_id == owner_->GetServerConnection()->GetInfo().agentID &&
        session_id == owner_->GetServerConnection()->GetInfo().sessionID)
    {
        Foundation::EventManagerPtr eventmgr = owner_->GetFramework()->GetEventManager();
        eventmgr->SendDelayedEvent(eventmgr->QueryEventCategory("NetworkState"), ProtocolUtilities::Events::EVENT_USER_KICKED_OUT, Foundation::EventDataPtr());
    }

    return false;
}

bool NetworkEventHandler::HandleOSNE_EstateOwnerMessage(ProtocolUtilities::NetworkEventInboundData *data)
{
    ProtocolUtilities::NetInMessage &msg = *data->message;

    msg.ResetReading();

    RexUUID agentid = msg.ReadUUID();
    RexUUID sessionid = msg.ReadUUID();
    RexUUID transactionid = msg.ReadUUID();
    std::string method = msg.ReadString();
    RexUUID invoice = msg.ReadUUID();

    // read parameter list
    QStringList ret;
    // Variable block begins
    size_t instance_count = msg.ReadCurrentBlockInstanceCount();
    while (instance_count--)
    {
        ret.push_back(msg.ReadString().c_str());
    }

    QVariantList l;
    l << agentid.ToQString();
    l << sessionid.ToQString();
    l << transactionid.ToQString();
    l << QString(method.c_str());
    l << invoice.ToQString();
    l << ret;
    owner_->EmitIncomingEstateOwnerMessageEvent(l);
    
    return false;
}



} //namespace RexLogic
