// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OpenSimProtocolModule.h"
#include "RexServerConnection.h"
#include "RexLogicModule.h"
#include "RexProtocolMsgIDs.h"
#include "EC_OgrePlaceable.h"
#include "EC_OpenSimPrim.h"
#include "QuatUtils.h"
#include "ConversionUtils.h"
#include "SceneManager.h"
#include "RexNetworkUtils.h"
#include "InventoryFolder.h"

namespace RexLogic
{

RexServerConnection::RexServerConnection(Foundation::Framework *framework) :
    framework_(framework),
    connected_(false),
    state_(OpenSimProtocol::Connection::STATE_DISCONNECTED),
    serverAddress_(""),
    serverPort_(0),
    connection_type_(DirectConnection)
{
    myInfo_.Reset();

    // Get pointer to the network interface.
    netInterface_ = framework_->GetModuleManager()->GetModule<OpenSimProtocol::OpenSimProtocolModule>
        (Foundation::Module::MT_OpenSimProtocol);

    boost::shared_ptr<OpenSimProtocol::OpenSimProtocolModule> sp = netInterface_.lock();
    if (!sp.get())
    {
        RexLogicModule::LogError("Getting network interface did not succeed.");
        return;
    }
}

RexServerConnection::~RexServerConnection()
{
}

bool RexServerConnection::ConnectToServer(
    const std::string& username,
    const std::string& password, 
    const std::string& serveraddress,
    const std::string& auth_server_address,
    const std::string& auth_login)
{
    boost::shared_ptr<OpenSimProtocol::OpenSimProtocolModule> sp = netInterface_.lock();
    if (!sp.get())
    {
        RexLogicModule::LogError("Getting network interface did not succeed.");
        return false;
    }

    if(connected_)
    {
        RexLogicModule::LogError("Already connected.");
        return false;
    }

    size_t pos = username.find(" ");
    if(pos == std::string::npos)
    {
        RexLogicModule::LogError("Invalid username, last name not found: " + username);
        return false;
    }

    std::string first_name = username.substr(0, pos);
    std::string last_name = username.substr(pos + 1);

    // Get server address and port.
    int port = 9000;
    pos = serveraddress.rfind(":");
    std::string serveraddress_noport;
    if(pos == std::string::npos)
    {
        serveraddress_noport = serveraddress;
        RexLogicModule::LogInfo("No port defined for the server, using default port (9000).");
    }
    else
    {
        serveraddress_noport = serveraddress.substr(0, pos);

        try
        {
            port = boost::lexical_cast<int>(serveraddress.substr(pos + 1));
        }catch(std::exception)
        {
            RexLogicModule::LogError("Invalid port number, only numbers are allowed.");
            return false;
        }
    }

    if (auth_server_address != "")
    {
        connection_type_ = AuthenticationConnection;
        sp->LoginUsingRexAuthentication(first_name,
            last_name, password, serveraddress_noport, port,
            auth_server_address, auth_login, &threadState_);
    }
    else
    {
        connection_type_ = DirectConnection;
        sp->LoginToServer(first_name, last_name,password, serveraddress_noport, port, &threadState_);
    }

    // Save the server address and port for later use.
    serverAddress_ = serveraddress_noport;
    serverPort_ = port;

    return true;
}

bool RexServerConnection::ConnectToCableBeachServer(
    const std::string& firstname,
    const std::string& lastname,
    int port,
    const std::string& serveraddress)
{
    boost::shared_ptr<OpenSimProtocol::OpenSimProtocolModule> sp = netInterface_.lock();
    if (!sp.get())
    {
        RexLogicModule::LogError("Getting network interface did not succeed.");
        return false;
    }

    connection_type_ = DirectConnection;
    sp->LoginToCBServer(firstname, lastname, serveraddress, port, &threadState_);

    return true;
}

bool RexServerConnection::CreateUDPConnection()
{
    boost::shared_ptr<OpenSimProtocol::OpenSimProtocolModule> sp = netInterface_.lock();
    if (!sp.get())
    {
        RexLogicModule::LogError("Getting network interface did not succeed.");
        return false;
    }

    // Get the client-spesific information.
    myInfo_ = sp->GetClientParameters();

    //Get the udp server and port from login response
    bool connect_result;
    if (myInfo_.gridUrl.size() != 0)
    {
        int port = 9000;
        size_t pos = myInfo_.gridUrl.rfind(":");
        std::string serveraddress_noport;
        if (pos == std::string::npos)
            serveraddress_noport = myInfo_.gridUrl;
        else
        {
            serveraddress_noport = myInfo_.gridUrl.substr(0, pos);
            try
            {
                port = boost::lexical_cast<int>(myInfo_.gridUrl.substr(pos + 1));
            }
            catch(std::exception)
            {
                RexLogicModule::LogError("Invalid port number, only numbers are allowed.");
                return false;
            }
        }

        connect_result = sp->CreateUDPConnection(serveraddress_noport.c_str(), port);
    }
    else
        connect_result = sp->CreateUDPConnection(serverAddress_.c_str(), serverPort_);

    if(connect_result)
    {
        connected_ = true;

        // Check that the parameters are valid.
        if (myInfo_.agentID.IsNull() || myInfo_.sessionID.IsNull())
        {
            // Client parameters not valid. Disconnect.
            sp->DisconnectFromRexServer();
            RexLogicModule::LogError("Client parameters are not valid! Disconnecting.");
            connected_ = false;
            return false;
        }

        // Send the necessary UDP packets.
        SendUseCircuitCodePacket();
        //! \todo release mode viewer sends the following packets "too fast" for some old rexservers to cope. Wait a while.
        /*! Proper solution would be to wait for ack from the UseCircuitCode packet before continuing to send packets.
            It may also be that the issue is only an issue on a localhost server (ie. with no real network delay)
         */
        boost::this_thread::sleep(boost::posix_time::milliseconds(100));
        SendCompleteAgentMovementPacket();
        SendAgentThrottlePacket();
        SendAgentWearablesRequestPacket();

        RexLogicModule::LogInfo("Connected to server " + serverAddress_ + ".");
    }
    else
        RexLogicModule::LogInfo("Connecting to server " + serverAddress_ + " failed.");

    return connect_result;
}

void RexServerConnection::RequestLogout()
{
    if(!connected_)
        return;

    SendLogoutRequestPacket();
}

void RexServerConnection::ForceServerDisconnect()
{
    if(!connected_)
        return;

    boost::shared_ptr<OpenSimProtocol::OpenSimProtocolModule> sp = netInterface_.lock();
    sp->DisconnectFromRexServer(); 

    connected_ = false;
}

void RexServerConnection::SendUseCircuitCodePacket()
{
    if(!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgUseCircuitCode);
    assert(m);

    m->AddU32(myInfo_.circuitCode);
    m->AddUUID(myInfo_.sessionID);
    m->AddUUID(myInfo_.agentID);
    m->MarkReliable();

    FinishMessageBuilding(m);
}

void RexServerConnection::SendAgentWearablesRequestPacket()
{
    if(!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgAgentWearablesRequest);
    assert(m);
    
    m->AddUUID(myInfo_.agentID);
    m->AddUUID(myInfo_.sessionID);
    m->MarkReliable();

    FinishMessageBuilding(m);
}

void RexServerConnection::SendCompleteAgentMovementPacket()
{
    if(!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgCompleteAgentMovement);
    assert(m);

    m->AddUUID(myInfo_.agentID);
    m->AddUUID(myInfo_.sessionID);
    m->AddU32(myInfo_.circuitCode);
    m->MarkReliable();

    FinishMessageBuilding(m);
}

void RexServerConnection::SendAgentThrottlePacket()
{
    if(!connected_)
        return;

    Core::Real max_bits_per_second = framework_->GetDefaultConfig().DeclareSetting("RexLogicModule", "max_bits_per_second", 1000000.0f);

    int idx = 0;
    static const size_t size = 7 * sizeof(Core::Real);
    Core::u8 throttle_block[size];

    WriteFloatToBytes(max_bits_per_second * 0.1f, throttle_block, idx); // resend
    WriteFloatToBytes(max_bits_per_second * 0.1f, throttle_block, idx); // land
    WriteFloatToBytes(max_bits_per_second * 0.02f, throttle_block, idx); // wind
    WriteFloatToBytes(max_bits_per_second * 0.02f, throttle_block, idx); // cloud
    WriteFloatToBytes(max_bits_per_second * 0.25f, throttle_block, idx); // task
    WriteFloatToBytes(max_bits_per_second * 0.26f, throttle_block, idx); // texture
    WriteFloatToBytes(max_bits_per_second * 0.25f, throttle_block, idx); // asset

    NetOutMessage *m = StartMessageBuilding(RexNetMsgAgentThrottle);
    assert(m);

    m->AddUUID(myInfo_.agentID);
    m->AddUUID(myInfo_.sessionID);
    m->AddU32(myInfo_.circuitCode);
    m->AddU32(0); // Generation counter
    m->AddBuffer(size, throttle_block); // throttles
    m->MarkReliable();

    FinishMessageBuilding(m);
}

void RexServerConnection::SendLogoutRequestPacket()
{
    if(!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgLogoutRequest);
    assert(m);

    m->AddUUID(myInfo_.agentID);
    m->AddUUID(myInfo_.sessionID);

    FinishMessageBuilding(m);

    RexLogicModule::LogInfo("Sent a Logout Request to the server... waiting for reply before quitting.");
}

void RexServerConnection::SendChatFromViewerPacket(const std::string &text)
{
    if(!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgChatFromViewer);
    assert(m);

    m->AddUUID(myInfo_.agentID);
    m->AddUUID(myInfo_.sessionID);
    m->AddBuffer(text.length(), (uint8_t*)text.c_str());
    m->AddU8(1);
    m->AddS32(0);

    FinishMessageBuilding(m);
}

void RexServerConnection::SendAgentUpdatePacket(Core::Quaternion bodyrot, Core::Quaternion headrot, uint8_t state, 
    RexTypes::Vector3 camcenter, RexTypes::Vector3 camataxis, RexTypes::Vector3 camleftaxis, RexTypes::Vector3 camupaxis,
    float fardist, uint32_t controlflags, uint8_t flags)
{
    if(!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgAgentUpdate);
    assert(m);

    m->AddUUID(myInfo_.agentID);
    m->AddUUID(myInfo_.sessionID);
    m->AddQuaternion(Core::OgreToOpenSimQuaternion(bodyrot));
    m->AddQuaternion(Core::OgreToOpenSimQuaternion(headrot));
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

void RexServerConnection::SendObjectSelectPacket(Core::entity_id_t object_id)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgObjectSelect);
    assert(m);

    // AgentData
    m->AddUUID(myInfo_.agentID);
    m->AddUUID(myInfo_.sessionID);

    // ObjectData
    m->SetVariableBlockCount(1);
    m->AddU32(object_id);

    FinishMessageBuilding(m);
}

void RexServerConnection::SendObjectSelectPacket(std::vector<Core::entity_id_t> object_id_list)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgObjectSelect);
    assert(m);

    // AgentData
    m->AddUUID(myInfo_.agentID);
    m->AddUUID(myInfo_.sessionID);

    // ObjectData
    m->SetVariableBlockCount(object_id_list.size());
    for(size_t i = 0; i < object_id_list.size(); ++i)
        m->AddU32(object_id_list[i]);

    FinishMessageBuilding(m);
}

void RexServerConnection::SendObjectDeselectPacket(Core::entity_id_t object_id)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgObjectDeselect);
    assert(m);

    // AgentData
    m->AddUUID(myInfo_.agentID);
    m->AddUUID(myInfo_.sessionID);

    // ObjectData
    m->SetVariableBlockCount(1);
    m->AddU32(object_id);

    FinishMessageBuilding(m);
}

void RexServerConnection::SendObjectDeselectPacket(std::vector<Core::entity_id_t> object_id_list)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgObjectDeselect);
    assert(m);

    // AgentData
    m->AddUUID(myInfo_.agentID);
    m->AddUUID(myInfo_.sessionID);

    // ObjectData
    m->SetVariableBlockCount(object_id_list.size());
    for(size_t i = 0; i < object_id_list.size(); ++i)
        m->AddU32(object_id_list[i]);

    FinishMessageBuilding(m);
}

void RexServerConnection::SendMultipleObjectUpdatePacket(std::vector<Scene::EntityPtr> entity_ptr_list)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgMultipleObjectUpdate);
    assert(m);

    // AgentData
    m->AddUUID(myInfo_.agentID);
    m->AddUUID(myInfo_.sessionID);

    // ObjectData        
    ///\todo Update just the necessary parameters (use update flags) & test with multiple objects.
    size_t offset = 0;
    uint8_t data[2048]; ///\todo What is the max size?

    m->SetVariableBlockCount(entity_ptr_list.size());

    for(size_t i = 0; i < entity_ptr_list.size(); ++i)
    {
        const Foundation::ComponentInterfacePtr &prim_component = entity_ptr_list[i]->GetComponent("EC_OpenSimPrim");
        if (!prim_component) 
        {
            /* the py api allows moving any entity with a placeable component, not just prims.
               without this check an attempt to move the avatar from py crashed here */
            RexLogicModule::LogWarning("Not sending entity position update of a non-prim entity (e.g. avatar), as the protocol doesn't support it.");
            return;
        }

        RexLogic::EC_OpenSimPrim *prim = checked_static_cast<RexLogic::EC_OpenSimPrim *>(prim_component.get());

        const Foundation::ComponentInterfacePtr &ogre_component = entity_ptr_list[i]->GetComponent("EC_OgrePlaceable");
        OgreRenderer::EC_OgrePlaceable *ogre_pos = checked_static_cast<OgreRenderer::EC_OgrePlaceable *>(ogre_component.get());

        m->AddU32(prim->LocalId);
        m->AddU8(13);

        // Position
        memcpy(&data[offset], &Core::OgreToOpenSimCoordinateAxes(ogre_pos->GetPosition()), sizeof(Vector3));
        offset += sizeof(Vector3);

        // Scale
        memcpy(&data[offset], &Core::OgreToOpenSimCoordinateAxes(ogre_pos->GetScale()), sizeof(Vector3));
        offset += sizeof(Vector3);
    }

    // Add the data.
    m->AddBuffer(offset, data);

    ///\todo Make this work!
    /*for(size_t i = 0; i < entity_ptr_list.size(); ++i)
    {
        const Foundation::ComponentInterfacePtr &prim_component = entity_ptr_list[i]->GetComponent("EC_OpenSimPrim");
        RexLogic::EC_OpenSimPrim *prim = checked_static_cast<RexLogic::EC_OpenSimPrim *>(prim_component.get());

        const Foundation::ComponentInterfacePtr &ogre_component = entity_ptr_list[i]->GetComponent("EC_OgrePlaceable");
        OgreRenderer::EC_OgrePlaceable *ogre_pos = checked_static_cast<OgreRenderer::EC_OgrePlaceable *>(ogre_component.get());

        m->AddU32(prim->LocalId);
        m->AddU8(2);

        // Rotation
        Vector3 val = Core::PackQuaternionToFloat3(ogre_pos->GetOrientation());
        memcpy(&data[offset], &val, sizeof(Vector3));
        offset += sizeof(Vector3);
    }

    // Add the data.
    m->AddBuffer(offset, data);*/

    FinishMessageBuilding(m);
}

void RexServerConnection::SendObjectNamePacket(std::vector<Scene::EntityPtr> entity_ptr_list)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgObjectName);
    assert(m);

    // AgentData
    m->AddUUID(myInfo_.agentID);
    m->AddUUID(myInfo_.sessionID);

    // ObjectData
    m->SetVariableBlockCount(entity_ptr_list.size());
    for(size_t i = 0; i < entity_ptr_list.size(); ++i)
    {
        const Foundation::ComponentInterfacePtr &prim_component = entity_ptr_list[i]->GetComponent("EC_OpenSimPrim");
        RexLogic::EC_OpenSimPrim *prim = checked_static_cast<RexLogic::EC_OpenSimPrim *>(prim_component.get());
        
        m->AddU32(prim->LocalId);
        m->AddBuffer(prim->ObjectName.size(), (uint8_t*)prim->ObjectName.c_str());
    }
}

void RexServerConnection::SendObjectGrabPacket(Core::entity_id_t object_id)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgObjectGrab);
    assert(m);

    // AgentData
    m->AddUUID(myInfo_.agentID);
    m->AddUUID(myInfo_.sessionID);

    // ObjectData
    m->AddU32(object_id);
    //! \todo Touch offset is not send / calculated currently since it is not really used by the server anyway. -cm
    m->AddVector3(Vector3::ZERO);

    FinishMessageBuilding(m);
}

void RexServerConnection::SendObjectDescriptionPacket(std::vector<Scene::EntityPtr> entity_ptr_list)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgObjectDescription);
    assert(m);

    // AgentData
    m->AddUUID(myInfo_.agentID);
    m->AddUUID(myInfo_.sessionID);

    // ObjectData
    m->SetVariableBlockCount(entity_ptr_list.size());
    for(size_t i = 0; i < entity_ptr_list.size(); ++i)
    {
        const Foundation::ComponentInterfacePtr &prim_component = entity_ptr_list[i]->GetComponent("EC_OpenSimPrim");
        RexLogic::EC_OpenSimPrim *prim = checked_static_cast<RexLogic::EC_OpenSimPrim *>(prim_component.get());
        
        m->AddU32(prim->LocalId);
        m->AddBuffer(prim->Description.size(), (uint8_t*)prim->Description.c_str());
    }
}

void RexServerConnection::SendRegionHandshakeReplyPacket(RexTypes::RexUUID agent_id, RexTypes::RexUUID session_id, uint32_t flags)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgRegionHandshakeReply);

    m->AddUUID(agent_id);
    m->AddUUID(session_id); 
    m->AddU32(flags);

    FinishMessageBuilding(m);
}

void RexServerConnection::SendAgentSetAppearancePacket()
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgAgentSetAppearance);

    // Agentdata
    m->AddUUID(myInfo_.agentID);
    m->AddUUID(myInfo_.sessionID); 
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

void RexServerConnection::SendCreateInventoryFolderPacket(
    const RexTypes::RexUUID &parent_id,
    const RexTypes::RexUUID &folder_id,
    const RexTypes::asset_type_t &type,
    const std::string &name)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgCreateInventoryFolder);
    assert(m);

    // AgentData
    m->AddUUID(myInfo_.agentID);
    m->AddUUID(myInfo_.sessionID);

    // FolderData
    m->AddUUID(folder_id);
    m->AddUUID(parent_id);
    m->AddS8(type);
    ///\todo if "+1" doesn't exist, last char vanishes from the name, eg. "3D Models" -> "3D Model".
    m->AddBuffer(name.length() + 1, (uint8_t*)name.c_str());

    FinishMessageBuilding(m);
}

void RexServerConnection::SendMoveInventoryFolderPacket(
    const RexTypes::RexUUID &folder_id,
    const RexTypes::RexUUID &parent_id,
    const bool &re_timestamp)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgMoveInventoryFolder);
    assert(m);

    // AgentData
    m->AddUUID(myInfo_.agentID);
    m->AddUUID(myInfo_.sessionID);
    m->AddBool(re_timestamp);

    // FolderData
    m->SetVariableBlockCount(1);
    m->AddUUID(folder_id);
    m->AddUUID(parent_id);

    FinishMessageBuilding(m);
}

///\todo Test this function.
void RexServerConnection::SendMoveInventoryFolderPacket(
    std::list<OpenSimProtocol::InventoryFolder *> folders,
    const bool &re_timestamp)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgMoveInventoryFolder);
    assert(m);

    // AgentData
    m->AddUUID(myInfo_.agentID);
    m->AddUUID(myInfo_.sessionID);
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

void RexServerConnection::SendRemoveInventoryFolderPacket(
    const RexTypes::RexUUID &folder_id)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgRemoveInventoryFolder);
    assert(m);

    // AgentData
    m->AddUUID(myInfo_.agentID);
    m->AddUUID(myInfo_.sessionID);

    // FolderData
    m->SetVariableBlockCount(1);
    m->AddUUID(folder_id);

    FinishMessageBuilding(m);
}

///\todo Test this function.
void RexServerConnection::SendRemoveInventoryFolderPacket(
    std::list<OpenSimProtocol::InventoryFolder *> folders)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgRemoveInventoryFolder);
    assert(m);

    // AgentData
    m->AddUUID(myInfo_.agentID);
    m->AddUUID(myInfo_.sessionID);

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

void RexServerConnection::SendMoveInventoryItemPacket(
    const RexTypes::RexUUID &item_id,
    const RexTypes::RexUUID &folder_id,
    const std::string &new_name,
    const bool &re_timestamp)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgMoveInventoryItem);
    assert(m);

    // AgentData
    m->AddUUID(myInfo_.agentID);
    m->AddUUID(myInfo_.sessionID);
    m->AddBool(re_timestamp);

    // InventoryData, variable
    m->SetVariableBlockCount(1);
    m->AddUUID(item_id);
    m->AddUUID(folder_id);
    m->AddBuffer(new_name.length(), (uint8_t*)new_name.c_str());

    FinishMessageBuilding(m);
}

void RexServerConnection::SendRemoveInventoryItemPacket(const RexTypes::RexUUID &item_id)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgRemoveInventoryItem);
    assert(m);

    // AgentData
    m->AddUUID(myInfo_.agentID);
    m->AddUUID(myInfo_.sessionID);

    // InventoryData, variable
    m->SetVariableBlockCount(1);
    m->AddUUID(item_id);

    FinishMessageBuilding(m);
}

///\todo Test this function.
void RexServerConnection::SendRemoveInventoryItemPacket(std::list<RexTypes::RexUUID> item_id_list)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgRemoveInventoryItem);
    assert(m);

    // AgentData
    m->AddUUID(myInfo_.agentID);
    m->AddUUID(myInfo_.sessionID);

    // InventoryData, variable
    m->SetVariableBlockCount(item_id_list.size());
    std::list<RexTypes::RexUUID>::iterator it;
    for(it = item_id_list.begin(); it != item_id_list.end(); ++it)
        m->AddUUID(*it);

    FinishMessageBuilding(m);
}

void RexServerConnection::SendFetchInventoryDescendents(
    const RexTypes::RexUUID &folder_id,
    const RexTypes::RexUUID &owner_id,
    const int32_t &sort_order,
    const bool &fetch_folders,
    const bool &fetch_items)
{
    if (!connected_)
        return;

    NetOutMessage *m = StartMessageBuilding(RexNetMsgFetchInventoryDescendents);
    assert(m);

    // AgentData
    m->AddUUID(myInfo_.agentID);
    m->AddUUID(myInfo_.sessionID);

    // InventoryData
    m->AddUUID(folder_id);
    m->AddUUID(owner_id);
    m->AddS32(sort_order);
    m->AddBool(fetch_folders);
    m->AddBool(fetch_items);

    FinishMessageBuilding(m);
}

std::string RexServerConnection::GetCapability(const std::string &name)
{
    boost::shared_ptr<OpenSimProtocol::OpenSimProtocolModule> sp = netInterface_.lock();
    if (!sp.get())
    {
        RexLogicModule::LogError("Getting network interface did not succeed.");
        return "";
    }

    return sp->GetCapability(name);
}

volatile OpenSimProtocol::Connection::State RexServerConnection::GetConnectionState()
{
    boost::shared_ptr<OpenSimProtocol::OpenSimProtocolModule> sp = netInterface_.lock();
    if (!sp.get())
    {
        RexLogicModule::LogError("Getting network interface did not succeed.");
        return OpenSimProtocol::Connection::STATE_ENUM_COUNT;
    }

    return sp->GetConnectionState();
}

NetOutMessage *RexServerConnection::StartMessageBuilding(const NetMsgID &message_id)
{
    boost::shared_ptr<OpenSimProtocol::OpenSimProtocolModule> sp = netInterface_.lock();
    if (!sp.get())
    {
        RexLogicModule::LogError("Getting network interface did not succeed.");
        return 0;
    }

    return sp->StartMessageBuilding(message_id);
}

void RexServerConnection::FinishMessageBuilding(NetOutMessage *msg)
{
    boost::shared_ptr<OpenSimProtocol::OpenSimProtocolModule> sp = netInterface_.lock();
    if (!sp.get())
    {
        RexLogicModule::LogError("Getting network interface did not succeed.");
        return;
    }

    sp->FinishMessageBuilding(msg);
}

} // namespace RexLogic
