// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "WorldStream.h"
#include "RealXtend/RexProtocolMsgIDs.h"
#include "NetworkMessages/NetOutMessage.h"

#include "ProtocolModuleOpenSim.h"
#include "ProtocolModuleTaiga.h"
#include "QuatUtils.h"
#include "Framework.h"
#include "ConfigurationManager.h"
#include "ModuleManager.h"
#include "RexTypes.h"
#include "LoggingFunctions.h"
#include "EC_OpenSimPrim.h"

DEFINE_POCO_LOGGING_FUNCTIONS("WorldStream");

#include <QString>
#include <QUrl>
#include <QStringList>

#include "MemoryLeakCheck.h"

namespace ProtocolUtilities
{

WorldStream::WorldStream(Foundation::Framework *framework) :
    framework_(framework),
    connected_(false),
    state_(Connection::STATE_DISCONNECTED),
    simName_(""),
    serverAddress_(""),
    serverPort_(0),
    connection_type_(DirectConnection),
    password_(""),
    username_(""),
    auth_server_address_(""),
    blockSerialNumber_(0)
{
    clientParameters_.Reset();
    SetCurrentProtocolType(NotSet);
    LogInfo("World Stream created and ready.");
}

WorldStream::~WorldStream()
{
}

bool WorldStream::CreateUdpConnection()
{
    bool connected_successfully = false;
    protocolModule_ = GetCurrentProtocolModule();

    if (!protocolModule_.get())
    {
        LogError("Getting current Protocol Module did not succeed");
        return false;
    }

    clientParameters_ = protocolModule_->GetClientParameters();
    QUrl gridUrl(QString("http://%1").arg(clientParameters_.gridUrl.c_str()));

    if (gridUrl.isValid())
    {
        QString host = gridUrl.host();
        int port = gridUrl.port();

        if (!host.isEmpty() && !host.isNull() && port != -1)
        {
            serverAddress_ = gridUrl.toString().toStdString();
            connected_successfully = protocolModule_->CreateUdpConnection(host.toStdString().c_str(), port);
        }
    }
    else
    {
        // What for is this here?? Can be removed? -Jonne
        connected_successfully = protocolModule_->CreateUdpConnection(serverAddress_.c_str(), serverPort_);
    }

    if (!connected_successfully)
    {
        LogInfo("Connecting to server " + serverAddress_ + " failed");
        return false;
    }

    if (clientParameters_.agentID.IsNull() || clientParameters_.sessionID.IsNull())
    {
        protocolModule_->DisconnectFromServer();
        LogError("Client parameters agentID and sessionIDare not valid! Disconnecting...");
        connected_ = false;
        return false;
    }
    else
    {
        connected_ = true;
        SendLoginSuccessfullPackets();
        SendMapBlockPacket();
        LogInfo("Connected to server " + serverAddress_);
        return true;
    }
}

void WorldStream::RequestLogout()
{
    if (!connected_)
        return;

    SendLogoutRequestPacket();
}

void WorldStream::ForceServerDisconnect()
{
    if (!connected_)
        return;

    protocolModule_ = GetCurrentProtocolModule();
    if (!protocolModule_.get())
    {
        LogError("Could not force disconnect, can't get a handle to network interface");
        return;
    }

    protocolModule_->DisconnectFromServer();

    connected_ = false;
    protocolModule_.reset();
}

void WorldStream::SendUseCircuitCodePacket()
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgUseCircuitCode);
    assert(m);

    m->AddU32(clientParameters_.circuitCode);
    m->AddUUID(clientParameters_.sessionID);
    m->AddUUID(clientParameters_.agentID);
    m->MarkReliable();

    FinishMessageBuilding(m);
}

void WorldStream::SendAgentWearablesRequestPacket()
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgAgentWearablesRequest);
    assert(m);

    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);
    m->MarkReliable();

    FinishMessageBuilding(m);
}

void WorldStream::SendCompleteAgentMovementPacket()
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgCompleteAgentMovement);
    assert(m);

    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);
    m->AddU32(clientParameters_.circuitCode);
    m->MarkReliable();

    FinishMessageBuilding(m);
}

void WorldStream::SendAgentThrottlePacket()
{
    if (!connected_)
        return;

    float max_bits_per_second = framework_->GetDefaultConfig().DeclareSetting(
        "RexLogicModule", "max_bits_per_second", 1000000.0f);

    int idx = 0;
    static const size_t size = 7 * sizeof(float);
    u8 throttle_block[size];

    WriteFloatToBytes(max_bits_per_second * 0.1f, throttle_block, idx); // resend
    WriteFloatToBytes(max_bits_per_second * 0.1f, throttle_block, idx); // land
    WriteFloatToBytes(max_bits_per_second * 0.02f, throttle_block, idx); // wind
    WriteFloatToBytes(max_bits_per_second * 0.02f, throttle_block, idx); // cloud
    WriteFloatToBytes(max_bits_per_second * 0.25f, throttle_block, idx); // task
    WriteFloatToBytes(max_bits_per_second * 0.26f, throttle_block, idx); // texture
    WriteFloatToBytes(max_bits_per_second * 0.25f, throttle_block, idx); // asset

    NetOutMessage *m = StartMessageBuilding(RexNetMsgAgentThrottle);
    assert(m);

    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);
    m->AddU32(clientParameters_.circuitCode);
    m->AddU32(0); // Generation counter
    m->AddBuffer(size, throttle_block); // throttles
    m->MarkReliable();

    FinishMessageBuilding(m);
}

void WorldStream::SendRexStartupPacket(const std::string& state)
{
    StringVector strings;

    strings.push_back(clientParameters_.agentID.ToString());
    strings.push_back(state);
    SendGenericMessage("RexStartup", strings);
}

void WorldStream::SendLogoutRequestPacket()
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgLogoutRequest);
    assert(m);

    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);

    FinishMessageBuilding(m);

    LogInfo("Sent a Logout Request to the server... waiting for reply before quitting.");
}

void WorldStream::SendChatFromViewerPacket(const std::string &text, s32 channel)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgChatFromViewer);
    assert(m);

    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);
    m->AddBuffer(text.length(), (uint8_t*)text.c_str()); // Message
    m->AddU8(1); // Type
    m->AddS32(channel); // Channel

    FinishMessageBuilding(m);
}

void WorldStream::SendImprovedInstantMessagePacket(const RexUUID &target, const std::string &text)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgImprovedInstantMessage);
    assert(m);

    unsigned int parent_estate_id = 0; //! @todo Find out proper value
    unsigned int time_stamp = 0; 
    std::string from_name = username_; //! @todo Find out proper value

    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);
    m->AddBool(false); // from group
    m->AddUUID(target);
    m->AddU32(parent_estate_id);
    m->AddUUID(clientParameters_.regionID); //! @todo Find out proper value
    m->AddVector3(RexTypes::Vector3());//! @todo Find out proper value
    m->AddU8(0);//! @todo Find out proper value
    m->AddU8(0); // dialog type
    m->AddUUID(RexUUID());
    m->AddU32(time_stamp); // TODO: Timestamp
    m->AddBuffer( strlen(from_name.c_str()), (uint8_t*)(from_name.c_str()) );
    m->AddBuffer( strlen(text.c_str()), (uint8_t*)(text.c_str()) );
    m->AddBuffer(0, 0); // BinaryBucket

    FinishMessageBuilding(m);
}

void WorldStream::SendObjectAddPacket(float x, float y, float z)
{
    const RexTypes::Vector3 pos(x, y, z);
    SendObjectAddPacket(pos);
}

void WorldStream::SendObjectAddPacket(const RexTypes::Vector3 &position)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgObjectAdd);
    assert(m);

    RexTypes::Vector3 scale(0.5f, 0.5f, 0.5f);
    Quaternion rotation(0, 0, 0, 1);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);
    m->AddUUID(RexUUID());      // GroupID

    // ObjectData
    m->AddU8(0x09);             // PCode: 0x09 - prim
    m->AddU8(3);                // Material
    m->AddU32(0x02);            // AddFlags: 0x01 - use physics, 0x02 - create selected

    m->AddU8(16);               // PathCurve
    m->AddU8(1);                // ProfileCurve
    m->AddU16(0);               // PathBegin
    m->AddU16(0);               // PathEnd
    m->AddU8(100);              // PathScaleX
    m->AddU8(100);              // PathScaleY
    m->AddU8(0);                // PathShearX
    m->AddU8(0);                // PathShearY
    m->AddS8(0);                // PathTwist
    m->AddS8(0);                // PathTwistBegin
    m->AddS8(0);                // PathRadiusOffset
    m->AddS8(0);                // PathTaperX
    m->AddS8(0);                // PathTaperY
    m->AddU8(0);                // PathRevolutions
    m->AddS8(0);                // PathSkew
    m->AddU16(0);               // ProfileBegin
    m->AddU16(0);               // ProfileEnd
    m->AddU16(0);               // ProfileHollow
    m->AddU8(1);                // BypassRaycast
    m->AddVector3(position);    // RayStart     ///\note We use same position for both RayStart and RayEnd.
    m->AddVector3(position);    // RayEnd
    m->AddUUID(RexUUID());      // RayTargetID
    m->AddU8(0);                // RayEndIsIntersection
    m->AddVector3(scale);       // Scale
    m->AddQuaternion(rotation); // Rotation
    m->AddU8(0);                // State

    FinishMessageBuilding(m);
}

void WorldStream::SendObjectDeletePacket(const uint32_t &local_id, const bool force)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgObjectDelete);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);
    m->AddBool(force);

    // ObjectData
    m->SetVariableBlockCount(1);
    m->AddU32(local_id);

    FinishMessageBuilding(m);
}

void WorldStream::SendObjectDeletePacket(const std::vector<uint32_t> &local_id_list, const bool force)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgObjectDelete);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);
    m->AddBool(force);

    // ObjectData
    m->SetVariableBlockCount(local_id_list.size());
    for(size_t i = 0; i < local_id_list.size(); ++i)
        m->AddU32(local_id_list[i]);

    FinishMessageBuilding(m);
}

void WorldStream::SendAgentUpdatePacket(
    Quaternion bodyrot,
    Quaternion headrot,
    uint8_t state,
    RexTypes::Vector3 camcenter,
    RexTypes::Vector3 camataxis,
    RexTypes::Vector3 camleftaxis,
    RexTypes::Vector3 camupaxis,
    float fardist,
    uint32_t controlflags,
    uint8_t flags)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgAgentUpdate);
    assert(m);

    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);
    m->AddQuaternion(bodyrot);
    m->AddQuaternion(headrot);
    m->AddU8(state);
    m->AddVector3(camcenter);
    m->AddVector3(camataxis);
    m->AddVector3(camleftaxis);
    m->AddVector3(camupaxis);
    m->AddF32(fardist);
    m->AddU32(controlflags);
    m->AddU8(flags);

    FinishMessageBuilding(m);
}

void WorldStream::SendObjectSelectPacket(const unsigned int object_id)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgObjectSelect);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);

    // ObjectData
    m->SetVariableBlockCount(1);
    m->AddU32((entity_id_t)object_id);

    FinishMessageBuilding(m);
}

void WorldStream::SendObjectSelectPacket(std::vector<entity_id_t> object_id_list)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgObjectSelect);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);

    // ObjectData
    m->SetVariableBlockCount(object_id_list.size());
    for(size_t i = 0; i < object_id_list.size(); ++i)
        m->AddU32(object_id_list[i]);

    FinishMessageBuilding(m);
}

void WorldStream::SendObjectDeselectPacket(const unsigned long ent_id)
{
    entity_id_t object_id = ent_id;
    SendObjectDeselectPacket(object_id);
}

void WorldStream::SendObjectDeselectPacket(entity_id_t object_id)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgObjectDeselect);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);

    // ObjectData
    m->SetVariableBlockCount(1);
    m->AddU32(object_id);

    FinishMessageBuilding(m);
}

void WorldStream::SendObjectDeselectPacket(std::vector<entity_id_t> object_id_list)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgObjectDeselect);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);

    // ObjectData
    m->SetVariableBlockCount(object_id_list.size());
    for(size_t i = 0; i < object_id_list.size(); ++i)
        m->AddU32(object_id_list[i]);

    FinishMessageBuilding(m);
}

void WorldStream::SendObjectShapeUpdate(const EC_OpenSimPrim &prim)
{
    if (!connected_)
        return;

    // Shape update
    NetOutMessage *m = StartMessageBuilding(RexNetMsgObjectShape);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);
    
    // Block count
    m->SetVariableBlockCount(1);

    // ObjectData
    m->AddU32(prim.LocalId);
    m->AddU8(prim.PathCurve.Get());
    m->AddU8(prim.ProfileCurve.Get());

    m->AddU16(prim.PathBegin.Get() / 0.00002f);
    m->AddU16(prim.PathEnd.Get() / 0.00002f);

    m->AddU8(prim.PathScaleX.Get() / 0.01f);
    m->AddU8(prim.PathScaleY.Get() / 0.01f);
    m->AddU8((int8_t)(prim.PathShearX.Get() / 0.01f));
    m->AddU8((int8_t)(prim.PathShearY.Get() / 0.01f));

    m->AddS8(prim.PathTwist.Get() / 0.01f);
    m->AddS8(prim.PathTwistBegin.Get() / 0.01f);
    m->AddS8(prim.PathRadiusOffset.Get() / 0.01f);
    m->AddS8(prim.PathTaperX.Get() / 0.01f);
    m->AddS8(prim.PathTaperY.Get() / 0.01f);

    // prim.PathRevolutions - skip this, has to be 0.015 steps, no editing in ui either
    m->AddU8(1 - 0.015f);
    m->AddS8(prim.PathSkew.Get() / 0.01f);

    m->AddU16(prim.ProfileBegin.Get() / 0.00002f);
    m->AddU16(prim.ProfileEnd.Get() / 0.00002f);
    m->AddU16(prim.ProfileHollow.Get() / 0.00002f);

    // Send
    FinishMessageBuilding(m);
}

void WorldStream::SendMultipleObjectUpdatePacket(const std::vector<MultiObjectUpdateInfo>& update_info_list)
{
    if (!connected_)
        return;

    if (!update_info_list.size())
        return;

    // Will actually send two packets: first one for position & scale, then one for rotation.
    // Protocol does not seem to support sending all three

    // 1. Position & scale packet
    NetOutMessage *m = StartMessageBuilding(RexNetMsgMultipleObjectUpdate);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);

    // ObjectData
    ///\todo Update just the necessary parameters (use update flags) & test with multiple objects.
    size_t offset = 0;
    uint8_t data[2048]; ///\todo What is the max size?

    m->SetVariableBlockCount(update_info_list.size());

    for(size_t i = 0; i < update_info_list.size(); ++i)
    {
        m->AddU32(update_info_list[i].local_id_);
        m->AddU8(13);

        // Position
        memcpy(&data[offset], &update_info_list[i].position_, sizeof(RexTypes::Vector3));
        offset += sizeof(RexTypes::Vector3);

        // Scale
        memcpy(&data[offset], &update_info_list[i].scale_, sizeof(RexTypes::Vector3));
        offset += sizeof(RexTypes::Vector3);
    }

    // Add the data.
    m->AddBuffer(offset, data);
    FinishMessageBuilding(m);
    
    // 2. Rotation packet
    m = StartMessageBuilding(RexNetMsgMultipleObjectUpdate);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);

    // ObjectData
    offset = 0;

    m->SetVariableBlockCount(update_info_list.size());

    for(size_t i = 0; i < update_info_list.size(); ++i)
    {
        m->AddU32(update_info_list[i].local_id_);
        m->AddU8(2);
        
        // Rotation
        RexTypes::Vector3 val = PackQuaternionToFloat3(update_info_list[i].orientation_);
        memcpy(&data[offset], &val, sizeof(RexTypes::Vector3));
        offset += sizeof(RexTypes::Vector3);
    }

    // Add the data.
    m->AddBuffer(offset, data);
    FinishMessageBuilding(m);
}

void WorldStream::SendObjectNamePacket(const std::vector<ObjectNameInfo>& name_info_list)
{
    if (!connected_)
        return;

    if (!name_info_list.size())
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgObjectName);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);

    // ObjectData
    m->SetVariableBlockCount(name_info_list.size());
    for(size_t i = 0; i < name_info_list.size(); ++i)
    {
        m->AddU32(name_info_list[i].local_id_);
        m->AddString(name_info_list[i].name_);
    }

    FinishMessageBuilding(m);
}

void WorldStream::SendObjectNamePacket(const ObjectNameInfo& name_info)
{
    std::vector<ObjectNameInfo> vector;
    vector.push_back(name_info);
    SendObjectNamePacket(vector);
}

void WorldStream::SendObjectGrabPacket(entity_id_t object_id)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgObjectGrab);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);

    // ObjectData
    m->AddU32(object_id);
    //! \todo Touch offset is not send / calculated currently since it is not really used by the server anyway. -cm
    m->AddVector3(RexTypes::Vector3::ZERO);

    FinishMessageBuilding(m);
}

void WorldStream::SendObjectDescriptionPacket(const ObjectDescriptionInfo& description_info)
{
    std::vector<ObjectDescriptionInfo> vector;
    vector.push_back(description_info);
    SendObjectDescriptionPacket(vector);
}

void WorldStream::SendObjectDescriptionPacket(const std::vector<ObjectDescriptionInfo>& description_info_list)
{
    if (!connected_)
        return;

    if (!description_info_list.size())
        return;
            
    NetOutMessage *m = StartMessageBuilding(RexNetMsgObjectDescription);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);

    // ObjectData
    m->SetVariableBlockCount(description_info_list.size());
    for(size_t i = 0; i < description_info_list.size(); ++i)
    {
        m->AddU32(description_info_list[i].local_id_);
        m->AddString(description_info_list[i].description_);
    }

    FinishMessageBuilding(m);
}

void WorldStream::SendRegionHandshakeReplyPacket(const RexUUID &agent_id, const RexUUID &session_id, uint32_t flags)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgRegionHandshakeReply);

    m->AddUUID(agent_id);
    m->AddUUID(session_id); 
    m->AddU32(flags);

    FinishMessageBuilding(m);
}

void WorldStream::SendAgentSetAppearancePacket()
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgAgentSetAppearance);

    // Agentdata
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID); 
    m->AddU32(1);
    m->AddVector3(RexTypes::Vector3(0.45f, 0.6f, 1.708378f)); // Values for a default avatar height based on server

    // Wearabledata (empty)
    m->SetVariableBlockCount(0);
    m->AdvanceToNextVariable();

    // ObjectData (empty)
    m->AddBuffer(0,0);

    // VisualParam, these determine avatar's height on the server
    uint8_t visualparams[218];
    for(size_t i = 0; i < 218; ++i)
        visualparams[i] = 0;

    // Params based on source of server, which uses 218 params. 
    visualparams[25] = 122; // Body height
    visualparams[77] = 0; // Shoe heel height
    visualparams[78] = 0; // Shoe platform height
    visualparams[120] = 68; // Head size
    visualparams[125] = 178; // Leg length
    visualparams[148] = 117; // Neck length

    m->SetVariableBlockCount(218);
    for(size_t i = 0; i < 218; ++i)
        m->AddU8(visualparams[i]);

    FinishMessageBuilding(m);
}

void WorldStream::SendModifyLandPacket(f32 x, f32 y, u8 brush, u8 action, float seconds, float height)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgModifyLand);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);

    // ModifyBlock
    m->AddU8(action);
    m->AddU8(brush);
    m->AddF32(seconds);
    m->AddF32(height);

    // ParcelData
    m->SetVariableBlockCount(1);
    m->AddS32(-1);//LocalID
    m->AddF32(x); //west
    m->AddF32(y); //south
    m->AddF32(x); //east
    m->AddF32(y); //north

    FinishMessageBuilding(m);
}

void WorldStream::SendTextureDetail(const RexTypes::RexAssetID &new_texture_id, uint texture_index)
{
    if (!connected_)
        return;

    ProtocolUtilities::NetOutMessage *m = StartMessageBuilding(RexNetMsgEstateOwnerMessage);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);
    m->AddUUID(RexUUID::CreateRandom());

    // MethodData
    /*QString method = "textureheights";
    QByteArray bytearray = method.toUtf8();*/
    std::string method = "texturedetail";
    m->AddBuffer(method.size() + 1, (uint8_t*)method.c_str());
    m->AddUUID(RexUUID::CreateRandom());

    // texture detail id
    m->SetVariableBlockCount(1);
    std::string data = QString("%1").arg(texture_index).toStdString() + " " + new_texture_id;
    m->AddBuffer(data.size() + 1, (uint8_t*)data.c_str());

    FinishMessageBuilding(m);
}

void WorldStream::SendTextureHeightsMessage(float start_height, float height_range, uint corner)
{
    if (!connected_)
        return;

    ProtocolUtilities::NetOutMessage *m = StartMessageBuilding(RexNetMsgEstateOwnerMessage);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);
    m->AddUUID(RexUUID::CreateRandom());

    // MethodData
    /*QString method = "textureheights";
    QByteArray bytearray = method.toUtf8();*/
    std::string method = "textureheights";
    m->AddBuffer(method.size() + 1, (uint8_t*)method.c_str());
    m->AddUUID(RexUUID::CreateRandom());

    // HeightData
    m->SetVariableBlockCount(1);
    // Convert all number parameters into one string parameter that will be send into the server.
    std::string data = QString("%1").arg(corner).toStdString() + " " + QString("%1").arg(start_height).toStdString() + " " + QString("%1").arg(height_range).toStdString();
    m->AddBuffer(data.size() + 1, (uint8_t*)data.c_str());

    FinishMessageBuilding(m);
}

void WorldStream::SendTextureCommitMessage()
{
    if (!connected_)
        return;

    ProtocolUtilities::NetOutMessage *m = StartMessageBuilding(RexNetMsgEstateOwnerMessage);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);
    m->AddUUID(RexUUID::CreateRandom());

    // MethodData
    /*QString method = "textureheights";
    QByteArray bytearray = method.toUtf8();*/
    std::string method = "texturecommit";
    m->AddBuffer(method.size() + 1, (uint8_t*)method.c_str());
    m->AddUUID(RexUUID::CreateRandom());

    m->SetVariableBlockCount(0);

    FinishMessageBuilding(m);
}

void WorldStream::SendCreateInventoryFolderPacket(
    const RexUUID &parent_id,
    const RexUUID &folder_id,
    asset_type_t type,
    const std::string &name)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgCreateInventoryFolder);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);

    // FolderData
    m->AddUUID(folder_id);
    m->AddUUID(parent_id);
    m->AddS8(type);
    m->AddString(name);

    FinishMessageBuilding(m);
}

void WorldStream::SendMoveInventoryFolderPacket(
    const RexUUID &folder_id,
    const RexUUID &parent_id,
    bool re_timestamp)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgMoveInventoryFolder);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);
    m->AddBool(re_timestamp);

    // FolderData
    m->SetVariableBlockCount(1);
    m->AddUUID(folder_id);
    m->AddUUID(parent_id);

    FinishMessageBuilding(m);
}

///\todo Test this function.
/*
void WorldStream::SendMoveInventoryFolderPacket(
    std::list<OpenSimProtocol::InventoryFolder *> folders,
    const bool &re_timestamp)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgMoveInventoryFolder);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);
    m->AddBool(re_timestamp);

    // FolderData, variable
    m->SetVariableBlockCount(folders.size());
    std::list<OpenSimProtocol::InventoryFolder *>::iterator it;
    for(it = folders.begin(); it != folders.end(); ++it)
    {
        OpenSimProtocol::InventoryFolder *folder = *it;
        m->AddUUID(folder->GetID());
        m->AddUUID(folder->GetParent()->GetID());
    }

    FinishMessageBuilding(m);
}
*/

void WorldStream::SendRemoveInventoryFolderPacket(
    const RexUUID &folder_id)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgRemoveInventoryFolder);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);

    // FolderData
    m->SetVariableBlockCount(1);
    m->AddUUID(folder_id);

    FinishMessageBuilding(m);
}

///\todo Test this function.
/*
void WorldStream::SendRemoveInventoryFolderPacket(
    std::list<OpenSimProtocol::InventoryFolder *> folders)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgRemoveInventoryFolder);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);

    // FolderData, variable
    m->SetVariableBlockCount(folders.size());
    std::list<OpenSimProtocol::InventoryFolder *>::iterator it;
    for(it = folders.begin(); it != folders.end(); ++it)
    {
        OpenSimProtocol::InventoryFolder *folder = *it;
        m->AddUUID(folder->GetID());
    }

    FinishMessageBuilding(m);
}
*/

void WorldStream::SendMoveInventoryItemPacket(
    const RexUUID &item_id,
    const RexUUID &folder_id,
    const std::string &new_name,
    bool re_timestamp)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgMoveInventoryItem);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);
    m->AddBool(re_timestamp);

    // InventoryData, variable
    m->SetVariableBlockCount(1);
    m->AddUUID(item_id);
    m->AddUUID(folder_id);
    m->AddString(new_name);

    FinishMessageBuilding(m);
}

void WorldStream::SendCopyInventoryItemPacket(
    const RexUUID &old_agent_id,
    const RexUUID &old_item_id,
    const RexUUID &new_folder_id,
    const std::string &new_name)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgCopyInventoryItem);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);

    // InventoryData, variable
    m->SetVariableBlockCount(1);
    m->AddU32(0); // CallbackID, we don't need this.
    m->AddUUID(old_agent_id);
    m->AddUUID(old_item_id);
    m->AddUUID(new_folder_id);
    m->AddString(new_name);

    FinishMessageBuilding(m);
}

void WorldStream::SendRemoveInventoryItemPacket(const RexUUID &item_id)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgRemoveInventoryItem);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);

    // InventoryData, variable
    m->SetVariableBlockCount(1);
    m->AddUUID(item_id);

    FinishMessageBuilding(m);
}

///\todo Test this function.
void WorldStream::SendRemoveInventoryItemPacket(std::list<RexUUID> item_id_list)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgRemoveInventoryItem);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);

    // InventoryData, variable
    m->SetVariableBlockCount(item_id_list.size());
    std::list<RexUUID>::iterator it;
    for(it = item_id_list.begin(); it != item_id_list.end(); ++it)
        m->AddUUID(*it);

    FinishMessageBuilding(m);
}

void WorldStream::SendUpdateInventoryFolderPacket(
    const RexUUID &folder_id,
    const RexUUID &parent_id,
    int8_t type,
    const std::string &name)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgUpdateInventoryFolder);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);

    // FolderData, variable
    m->SetVariableBlockCount(1);
    m->AddUUID(folder_id);
    m->AddUUID(parent_id);
    m->AddS8(type);
    m->AddString(name);

    FinishMessageBuilding(m);
}

void WorldStream::SendUpdateInventoryItemPacket(
    const RexUUID &item_id,
    const RexUUID &folder_id,
    asset_type_t asset_type,
    inventory_type_t inventory_type,
    const std::string &name,
    const std::string &description)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgUpdateInventoryItem);
    assert(m);

    // TransactionID, new items only?
    RexUUID transaction_id;

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);
    m->AddUUID(transaction_id);

    // InventoryData, Variable
    m->SetVariableBlockCount(1);
    m->AddUUID(item_id);
    m->AddUUID(folder_id);
    m->AddU32(0);                       // CallbackID
    m->AddUUID(RexUUID());              // CreatorID
    m->AddUUID(RexUUID());              // OwnerID
    m->AddUUID(RexUUID());              // GroupID

    // Permissions-related:
    m->AddU32(0);                       //BaseMask
    m->AddU32(0);                       // OwnerMask
    m->AddU32(0);                       // GroupMask
    m->AddU32(0);                       // EveryoneMask
    m->AddU32(0);                       // NextOwnerMask
    m->AddBool(false);                  // GroupOwned

    m->AddUUID(transaction_id);
    m->AddS8(asset_type);               // Type
    m->AddS8(inventory_type);           // InvType
    m->AddU32(0);                       // Flags
    m->AddU8(0);                        // SaleType
    m->AddS32(0);                       // SalePrice
    m->AddString(name);                 // Name 
    m->AddString(description);          // Description
    m->AddS32(0);                       // CreationDAta
    m->AddU32(0);                       // CRC

    FinishMessageBuilding(m);
}

void WorldStream::SendFetchInventoryDescendentsPacket(
    const RexUUID &folder_id,
    const RexUUID &owner_id,
    int32_t sort_order,
    bool fetch_folders,
    bool fetch_items)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgFetchInventoryDescendents);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);

    // InventoryData
    m->AddUUID(folder_id);
    if (owner_id.IsNull())
        m->AddUUID(clientParameters_.agentID);
    else
        m->AddUUID(owner_id);
    m->AddS32(sort_order);
    m->AddBool(fetch_folders);
    m->AddBool(fetch_items);

    FinishMessageBuilding(m);
}

void WorldStream::SendAcceptFriendshipPacket(const RexUUID &transaction_id, const RexUUID &folder_id)
{
    NetOutMessage *m = StartMessageBuilding(RexNetMsgAcceptFriendship);
    assert(m);

    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);
    m->AddUUID(transaction_id);
    m->SetVariableBlockCount(1);
    m->AddUUID(folder_id);

    FinishMessageBuilding(m);
}

void WorldStream::SendDeclineFriendshipPacket(const RexUUID &transaction_id)
{
    NetOutMessage *m = StartMessageBuilding(RexNetMsgDeclineFriendship);
    assert(m);

    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);
    m->AddUUID(transaction_id);

    FinishMessageBuilding(m);
}

void WorldStream::SendFormFriendshipPacket(const RexUUID &dest_id)
{
    NetOutMessage *m = StartMessageBuilding(RexNetMsgFormFriendship);
    assert(m);

    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(dest_id);

    FinishMessageBuilding(m);
}

void WorldStream::SendTerminateFriendshipPacket(const RexUUID &other_id)
{
    NetOutMessage *m = StartMessageBuilding(RexNetMsgTerminateFriendship);
    assert(m);

    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(other_id);

    FinishMessageBuilding(m);
}

void WorldStream::SendGenericMessage(const QString &method, const QStringList& strings)
{
    StringVector stringvec;
    for(QStringList::const_iterator it = strings.begin(); it != strings.end(); ++it)
        stringvec.push_back(it->toStdString());

    SendGenericMessage(method.toStdString(), stringvec);
}

void WorldStream::SendGenericMessage(const std::string& method, const StringVector& strings)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgGenericMessage);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);

    // Transaction ID
    m->AddUUID(RexUUID::CreateRandom());

    // Method
    m->AddString(method);

    // Invoice ID
    m->AddUUID(RexUUID::CreateRandom());

    // Variable count of strings
    m->SetVariableBlockCount(strings.size());

    // Strings
    for(uint i = 0; i < strings.size(); ++i)
        m->AddString(strings[i]);

    m->MarkReliable();

    FinishMessageBuilding(m);
}

void WorldStream::SendGenericMessageBinary(
    const std::string& method,
    const StringVector& strings,
    const std::vector<uint8_t>& binary)
{
    if (!connected_)
        return;

    const size_t max_string_size = 200;
    
    NetOutMessage *m = StartMessageBuilding(RexNetMsgGenericMessage);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);

    // Transaction ID
    m->AddUUID(RexUUID::CreateRandom());

    // Method
    m->AddString(method);

    // Invoice ID
    m->AddUUID(RexUUID::CreateRandom());

    // See how many binary strings 
    size_t binarystrings = 0;
    size_t count = binary.size();
    while (count)
    {
        if (count > max_string_size)
            count -= max_string_size;
        else
            count -= count;
        binarystrings++;
    }
    
    // Variable count of strings
    m->SetVariableBlockCount(strings.size() + binarystrings);

    // Strings
    for(uint i = 0; i < strings.size(); ++i)
        m->AddString(strings[i]);

    // Binary strings
    size_t idx = 0;
    count = binary.size();   
    for(uint i = 0; i < binarystrings; ++i)
    {
        size_t size = count;
        if (size > max_string_size)
            size = max_string_size;
            
        m->AddBuffer(size, (uint8_t*)&binary[idx]);
        idx += size;
        count -= size;
    }

    m->MarkReliable();

    FinishMessageBuilding(m);
}

void WorldStream::SendAgentPausePacket()
{
    NetOutMessage *m = StartMessageBuilding(RexNetMsgAgentPause);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);
    m->AddU32(++blockSerialNumber_);

    FinishMessageBuilding(m);
}

void WorldStream::SendAgentResumePacket()
{
    NetOutMessage *m = StartMessageBuilding(RexNetMsgAgentResume);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);
    m->AddU32(++blockSerialNumber_);

    FinishMessageBuilding(m);
}

void WorldStream::SendObjectDeRezPacket(const unsigned long ent_id, const QString &trash_id)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgDeRezObject);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);
    m->AddUUID(RexUUID()); //group_id

    // ObjectData
    m->AddU8(4); //trash ?
    RexUUID ruuid = RexUUID();
    ruuid.FromString(trash_id.toStdString());
    m->AddUUID(ruuid);
    m->AddUUID(RexUUID::CreateRandom()); //transaction_id
    m->AddU8(1);
    m->AddU8(1);

    m->SetVariableBlockCount(1);
    m->AddU32(ent_id);

    FinishMessageBuilding(m);
}

void WorldStream::SendObjectUndoPacket(const QString &ent_id)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgUndo);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);
    m->AddUUID(RexUUID());      // GroupID

    // ObjectData
    m->SetVariableBlockCount(1);
    RexUUID ruuid = RexUUID();
    ruuid.FromString(ent_id.toStdString());
    m->AddUUID(ruuid);

    FinishMessageBuilding(m);
}

void WorldStream::SendObjectRedoPacket(const QString &ent_id)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgRedo);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);
    m->AddUUID(RexUUID());      // GroupID

    // ObjectData
    m->SetVariableBlockCount(1);
    RexUUID ruuid = RexUUID();
    ruuid.FromString(ent_id.toStdString());
    m->AddUUID(ruuid);

    FinishMessageBuilding(m);
}


void WorldStream::SendObjectDuplicatePacket(
    const unsigned long ent_id,
    const unsigned long flags,
    const Vector3df &offset)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgObjectDuplicate);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);
    m->AddUUID(RexUUID());      // GroupID

    // SharedData
    m->AddVector3(offset); //umm, prolly needs the location... offset
    m->AddU32(flags);
    m->SetVariableBlockCount(1);
    m->AddU32(ent_id);

    FinishMessageBuilding(m);
}

void WorldStream::SendObjectDuplicatePacket(
    const unsigned long ent_id,
    const unsigned long flags,
    const float offset_x,
    const float offset_y,
    const float offset_z)
{
    SendObjectDuplicatePacket(ent_id, flags, Vector3df(offset_x, offset_y, offset_z));
}

void WorldStream::SendObjectDuplicatePacket(const unsigned long ent_id, const unsigned long flags)
{
    SendObjectDuplicatePacket(ent_id, flags, Vector3df(0,0,0));
}

void WorldStream::SendUUIDNameRequestPacket(const RexUUID &user_id)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgUUIDNameRequest);
    assert(m);

    m->SetVariableBlockCount(1);
    m->AddUUID(user_id);

    FinishMessageBuilding(m);
}

void WorldStream::SendUUIDNameRequestPacket(const std::vector<RexUUID> &user_ids)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgUUIDNameRequest);
    assert(m);

    m->SetVariableBlockCount(user_ids.size());
    for(int i = 0; i < user_ids.size(); ++i)
        m->AddUUID(user_ids[i]);

    FinishMessageBuilding(m);
}

void WorldStream::SendUUIDGroupNameRequestPacket(const RexUUID &group_id)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgUUIDGroupNameRequest);
    assert(m);

    m->SetVariableBlockCount(1);
    m->AddUUID(group_id);

    FinishMessageBuilding(m);
}

void WorldStream::SendUUIDGroupNameRequestPacket(const std::vector<RexUUID> &group_ids)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgUUIDGroupNameRequest);
    assert(m);

    m->SetVariableBlockCount(group_ids.size());
    for(int i = 0; i < group_ids.size(); ++i)
        m->AddUUID(group_ids[i]);

    FinishMessageBuilding(m);
}

void WorldStream::SendObjectLinkPacket(const std::vector<entity_id_t> &local_ids)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgObjectLink);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);

    // ObjectData
    m->SetVariableBlockCount(local_ids.size());
    for(int i = 0; i < local_ids.size(); ++i)
        m->AddU32(local_ids[i]);

    FinishMessageBuilding(m);
}

void WorldStream::SendObjectLinkPacket(const QStringList& strings)
{
    std::vector<entity_id_t> vec;
    for(QStringList::const_iterator it = strings.constBegin(); it != strings.constEnd(); ++it)
        vec.push_back((*it).toUInt());

    SendObjectLinkPacket(vec);
}

void WorldStream::SendObjectDelinkPacket(const std::vector<entity_id_t> &local_ids)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgObjectDelink);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);

    // ObjectData
    m->SetVariableBlockCount(local_ids.size());
    for(int i = 0; i < local_ids.size(); ++i)
        m->AddU32(local_ids[i]);

    FinishMessageBuilding(m);
}

void WorldStream::SendObjectDelinkPacket(const QStringList& strings)
{
    std::vector<entity_id_t> vec;
    for(QStringList::const_iterator it = strings.constBegin(); it != strings.constEnd(); ++it)
        vec.push_back((*it).toUInt());

    SendObjectDelinkPacket(vec);
}

void WorldStream::SendMapBlockRequest()
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgMapBlockRequest);
    assert(m);
    
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);
    //flags, estateID, godLike. Sending default values. Replace when needed
    m->AddU32(0);
    m->AddU32(0);
    m->AddBool(false);    
    m->AddU16(clientParameters_.regionX - 1); //minX
    m->AddU16(clientParameters_.regionX + 1); //maxX
    m->AddU16(clientParameters_.regionY - 1); //minY
    m->AddU16(clientParameters_.regionY + 1); //maxY

    FinishMessageBuilding(m);
}

void WorldStream::SendRequestGodlikePowersPacket(const bool godlike)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgRequestGodlikePowers);
    assert(m);

    // AgentData
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);

    // RequestBlock
    m->AddBool(godlike);
    m->AddUUID(RexUUID());

    FinishMessageBuilding(m);
}

void WorldStream::SendGodKickUserPacket(const RexUUID &user_id, const std::string &reason)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgGodKickUser);
    assert(m);

    // UserInfo
    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);
    m->AddUUID(user_id);
    m->AddU32(0); // seems that KickFlags aren't used for anything for real
    m->AddString(reason);

    FinishMessageBuilding(m);
}

void WorldStream::SendGodKickUserPacket(const QString &user_id, const QString &reason)
{
    RexUUID rex_uuid(user_id);
    std::string std_reason = user_id.toStdString();
    this->SendGodKickUserPacket(rex_uuid, std_reason);
}

void WorldStream::SendEstateOwnerMessage(const QString &method, const QStringList& paramlist)
{
    if (!connected_)
        return;
    
    ProtocolUtilities::NetOutMessage *m = StartMessageBuilding(RexNetMsgEstateOwnerMessage);
    assert(m);

    m->AddUUID(clientParameters_.agentID);
    m->AddUUID(clientParameters_.sessionID);

    // Transaction ID
    m->AddUUID(RexUUID::CreateRandom());
    //m->AddUUID(RexUUID::FromString(transactionid));
    //m->AddBuffer(method.size() + 1, (uint8_t*)method.c_str());
    m->AddBuffer(method.size() + 1, (uint8_t*)method.toStdString().c_str());
    
    //m->AddUUID(RexUUID::FromString(invoice));
    m->AddUUID(RexUUID::CreateRandom());
    
    m->SetVariableBlockCount(paramlist.length());
    
    QList<QString>::const_iterator i;
    for (i = paramlist.begin(); i != paramlist.end(); ++i)
    {
        QString param = *i;
        m->AddString(param.toStdString());
        //m->AddString(*i);
    }
    
    FinishMessageBuilding(m);
}

QString WorldStream::GetCapability(const QString &name) const
{
    if (!GetCurrentProtocolModule())
    {
        LogError("Getting network interface did not succeed.");
        return "";
    }

    return GetCurrentProtocolModule()->GetCapability(name.toStdString()).c_str();
}

Connection::State WorldStream::GetConnectionState()
{
    protocolModule_ = GetCurrentProtocolModule();
    if (!protocolModule_.get())
        return Connection::STATE_ENUM_COUNT;

    return protocolModule_->GetConnectionState();
}

void WorldStream::SetConnectionState(Connection::State newstate)
{
    protocolModule_ = GetCurrentProtocolModule();
    if (!protocolModule_.get())
        return;

    protocolModule_->SetConnectionState(newstate);
}

std::string WorldStream::GetConnectionErrorMessage()
{
    protocolModule_ = GetCurrentProtocolModule();
    if (!protocolModule_.get())
        return std::string("Could not acquire current protocol module");

    return protocolModule_->GetConnectionErrorMessage();
}

void WorldStream::SetCurrentProtocolType(ProtocolType newType)
{
    currentProtocolType_ = newType;
    switch(currentProtocolType_)
    {
    case OpenSim:
    {
        netInterfaceTaiga_ = boost::shared_ptr<TaigaProtocol::ProtocolModuleTaiga>();
        netInterfaceOpenSim_ = framework_->GetModuleManager()->GetModule<OpenSimProtocol::ProtocolModuleOpenSim>();
        boost::shared_ptr<OpenSimProtocol::ProtocolModuleOpenSim> sp = netInterfaceOpenSim_.lock();
        if (!sp.get())
            LogError("Getting ProtocolModuleOpenSim network interface did not succeed");
        break;
    }
    case Taiga:
    {
        netInterfaceOpenSim_ = boost::shared_ptr<OpenSimProtocol::ProtocolModuleOpenSim>();
        netInterfaceTaiga_ = framework_->GetModuleManager()->GetModule<TaigaProtocol::ProtocolModuleTaiga>();
        boost::shared_ptr<TaigaProtocol::ProtocolModuleTaiga> sp = netInterfaceTaiga_.lock();
        if (!sp.get())
            LogError("Getting ProtocolModuleTaiga network interface did not succeed");
        break;
    }
    case NotSet:
    default:
        protocolModule_.reset();
        break;
    }
}

boost::shared_ptr<ProtocolModuleInterface> WorldStream::GetCurrentProtocolModule() const
{
    switch(currentProtocolType_)
    {
    case OpenSim:
        return netInterfaceOpenSim_.lock();
    case Taiga:
        return netInterfaceTaiga_.lock();
    case NotSet:
    default:
        return boost::shared_ptr<ProtocolModuleInterface>();
    }
}

boost::weak_ptr<ProtocolModuleInterface> WorldStream::GetCurrentProtocolModuleWeakPointer() const
{
    switch(currentProtocolType_)
    {
    case OpenSim:
        return netInterfaceOpenSim_;
    case Taiga:
        return netInterfaceTaiga_;
    case NotSet:
    default:
        return boost::weak_ptr<ProtocolModuleInterface>();
    }
}

bool WorldStream::PrepareCurrentProtocolModule()
{
    protocolModule_ = GetCurrentProtocolModule();
    if (!protocolModule_.get())
    {
        LogWarning("Could not prepare current Protocol Modules for Registering Network Events");
        return false;
    }

    protocolModule_->RegisterNetworkEvents();
    return true;
}

void WorldStream::UnregisterCurrentProtocolModule()
{
    protocolModule_ = GetCurrentProtocolModule();
    if (protocolModule_.get())
        protocolModule_->UnregisterNetworkEvents();
    protocolModule_.reset();
}

void WorldStream::SendLoginSuccessfullPackets()
{
    // Send the necessary UDP packets.
    SendUseCircuitCodePacket();

    ///\todo IS THE BELOW STATEMENT STILL RELEVANT?!
    ///
    //! \todo release mode viewer sends the following packets "too fast" for some old rexservers to cope. Wait a while.
    /*! Proper solution would be to wait for ack from the UseCircuitCode packet before continuing to send packets.
        It may also be that the issue is only an issue on a localhost server (ie. with no real network delay)
     */
    boost::this_thread::sleep(boost::posix_time::milliseconds(100));
    SendCompleteAgentMovementPacket();
    SendAgentThrottlePacket();
    SendAgentWearablesRequestPacket();
    SendRexStartupPacket("started"); 
}

void WorldStream::SendMapBlockPacket()
{
    SendMapBlockRequest();
}

NetOutMessage *WorldStream::StartMessageBuilding(const NetMsgID &message_id)
{
    protocolModule_ = GetCurrentProtocolModule();
    if (!protocolModule_.get())
    {
        LogError("Getting network interface did not succeed.");
        return 0;
    }

    return protocolModule_->StartMessageBuilding(message_id);
}

void WorldStream::FinishMessageBuilding(NetOutMessage *msg)
{
    protocolModule_ = GetCurrentProtocolModule();
    if (!protocolModule_.get())
    {
        LogError("Getting network interface did not succeed.");
        return;
    }

    protocolModule_->FinishMessageBuilding(msg);
}

void WorldStream::WriteFloatToBytes(float value, uint8_t* bytes, int& idx)
{
    *(float*)(&bytes[idx]) = value;
    idx += sizeof(float);
}

} // namespace ProtocolUtilities
