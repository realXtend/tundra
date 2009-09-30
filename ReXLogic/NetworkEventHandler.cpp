// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include <sstream>

#include "NetworkEventHandler.h"
#include "NetInMessage.h"
#include "RexProtocolMsgIDs.h"
#include "OpenSimProtocolModule.h"
#include "RexLogicModule.h"
#include "Entity.h"
#include "AvatarControllable.h"

// Ogre renderer -specific.
#include <OgreMaterialManager.h>

#include "ConversionUtils.h"
#include "BitStream.h"
#include "TerrainDecoder.h"
#include "Terrain.h"
#include "Avatar.h"
#include "Primitive.h"
#include "Sky.h"
#include "Environment.h"
#include "InventoryModel.h"

namespace
{

/// Clones a new Ogre material that renders using the given ambient color. 
/// This function will be removed or refactored later on, once proper material system is present. -jj.
void DebugCreateAmbientColorMaterial(const std::string &materialName, float r, float g, float b)
{
    Ogre::MaterialManager &mm = Ogre::MaterialManager::getSingleton();
    Ogre::MaterialPtr material = mm.getByName(materialName);
    if (material.get()) // The given material already exists, so no need to create it again.
        return;

    material = mm.getByName("SolidAmbient");
    if (!material.get())
        return;

    Ogre::MaterialPtr newMaterial = material->clone(materialName);
    newMaterial->setAmbient(r, g, b);
}

}

namespace RexLogic
{

NetworkEventHandler::NetworkEventHandler(Foundation::Framework *framework, RexLogicModule *rexlogicmodule)
{
    framework_ = framework;
    rexlogicmodule_ = rexlogicmodule;

    // Get the pointe to the OpenSimModule.
    netInterface_ = framework_->GetModuleManager()->GetModule<OpenSimProtocol::OpenSimProtocolModule>
        (Foundation::Module::MT_OpenSimProtocol);
    
    boost::shared_ptr<OpenSimProtocol::OpenSimProtocolModule> sp = netInterface_.lock();
    if (!sp.get())
    {
        RexLogicModule::LogError("NetworkEventHandler: Could not acquire OpenSimProtocolModule!");
        return;
    }
    
    Foundation::ModuleWeakPtr renderer = framework->GetModuleManager()->GetModule(Foundation::Module::MT_Renderer);
    if (renderer.expired() == false)
    {
        DebugCreateAmbientColorMaterial("AmbientWhite", 1.f, 1.f, 1.f);
        DebugCreateAmbientColorMaterial("AmbientGreen", 0.f, 1.f, 0.f);
        DebugCreateAmbientColorMaterial("AmbientRed", 1.f, 0.f, 0.f);
    }
}

NetworkEventHandler::~NetworkEventHandler()
{
}

bool NetworkEventHandler::HandleOpenSimNetworkEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data)
{
    PROFILE(NetworkEventHandler_HandleOpenSimNetworkEvent);
    OpenSimProtocol::NetworkEventInboundData *netdata = checked_static_cast<OpenSimProtocol::NetworkEventInboundData *>(data);
    switch(event_id)
    {
    case RexNetMsgRegionHandshake:
        return HandleOSNE_RegionHandshake(netdata);

    case RexNetMsgAgentMovementComplete:
        return HandleOSNE_AgentMovementComplete(netdata);

    case RexNetMsgAvatarAnimation:
        return rexlogicmodule_->GetAvatarHandler()->HandleOSNE_AvatarAnimation(netdata);

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
        return rexlogicmodule_->GetPrimitiveHandler()->HandleOSNE_ObjectProperties(netdata);

    case RexNetMsgLayerData:
        return rexlogicmodule_->GetTerrainHandler()->HandleOSNE_LayerData(netdata);

    case RexNetMsgSimulatorViewerTimeMessage:
        return rexlogicmodule_->GetEnvironmentHandler()->HandleOSNE_SimulatorViewerTimeMessage(netdata);

    case RexNetMsgInventoryDescendents:
        return HandleOSNE_InventoryDescendents(netdata);

    default:
        break;
    }
    
    return false;
}

bool NetworkEventHandler::HandleOSNE_ObjectUpdate(OpenSimProtocol::NetworkEventInboundData* data)
{
    data->message->ResetReading();
    data->message->SkipToNextVariable();
    data->message->SkipToNextVariable();

    if (data->message->GetCurrentBlock() >= data->message->GetBlockCount())
    {
        RexLogicModule::LogDebug("Empty ObjectUpdate packet received, ignoring.");
        return false;
    }
    
    size_t instance_count = data->message->ReadCurrentBlockInstanceCount();
    bool result = false;
    if (instance_count > 0)
    {
        data->message->SkipToFirstVariableByName("PCode");
        uint8_t pcode = data->message->ReadU8();
        switch(pcode)
        {
        case 0x09:
            result = rexlogicmodule_->GetPrimitiveHandler()->HandleOSNE_ObjectUpdate(data);
            break;
        
        case 0x2f:
            result = rexlogicmodule_->GetAvatarHandler()->HandleOSNE_ObjectUpdate(data);
            break;
        }
    }
    
    return result;
}

bool NetworkEventHandler::HandleOSNE_GenericMessage(OpenSimProtocol::NetworkEventInboundData* data)
{        
    data->message->ResetReading();    
    data->message->SkipToNextVariable();      // AgentId
    data->message->SkipToNextVariable();      // SessionId
    data->message->SkipToNextVariable();      // TransactionId
    std::string methodname = data->message->ReadString(); 

    if (methodname == "RexMediaUrl")
        return rexlogicmodule_->GetPrimitiveHandler()->HandleRexGM_RexMediaUrl(data);
    else if (methodname == "RexPrimData")
        return rexlogicmodule_->GetPrimitiveHandler()->HandleRexGM_RexPrimData(data); 
    else if (methodname == "RexAppearance")
        return rexlogicmodule_->GetAvatarHandler()->HandleRexGM_RexAppearance(data);
    else if (methodname == "RexSky")
          return rexlogicmodule_->GetSkyHandler()->HandleRexGM_RexSky(data);
    else
        return false;    
}

bool NetworkEventHandler::HandleOSNE_RegionHandshake(OpenSimProtocol::NetworkEventInboundData* data)
{
    size_t bytesRead = 0;

    data->message->ResetReading();    
    data->message->SkipToNextVariable(); // RegionFlags U32
    data->message->SkipToNextVariable(); // SimAccess U8

    std::string sim_name = data->message->ReadString(); // SimName
    rexlogicmodule_->GetServerConnection()->simName_ = sim_name;
    
    data->message->SkipToNextVariable(); // SimOwner
    data->message->SkipToNextVariable(); // IsEstateManager
    data->message->SkipToNextVariable(); // WaterHeight
    data->message->SkipToNextVariable(); // BillableFactor
    data->message->SkipToNextVariable(); // CacheID
    for(int i = 0; i < 4; ++i)
        data->message->SkipToNextVariable(); // TerrainBase0..3
    RexAssetID terrain[4];
    terrain[0] = data->message->ReadUUID().ToString();
    terrain[1] = data->message->ReadUUID().ToString();
    terrain[2] = data->message->ReadUUID().ToString();
    terrain[3] = data->message->ReadUUID().ToString();        

    RexLogicModule::LogInfo("Joined to the sim \"" + sim_name + "\".");
    
    // Create the "World" scene.
    boost::shared_ptr<OpenSimProtocol::OpenSimProtocolModule> sp = netInterface_.lock();
    if (!sp.get())
    {
        RexLogicModule::LogError("NetworkEventHandler: Could not acquire OpenSimProtocolModule!");
        return false;
    }

    RexLogic::TerrainPtr terrainHandler = rexlogicmodule_->GetTerrainHandler();
    terrainHandler->SetTerrainTextures(terrain);

    const OpenSimProtocol::ClientParameters& client = sp->GetClientParameters();
    rexlogicmodule_->GetServerConnection()->SendRegionHandshakeReplyPacket(client.agentID, client.sessionID, 0);
    return false;
}

bool NetworkEventHandler::HandleOSNE_LogoutReply(OpenSimProtocol::NetworkEventInboundData* data)
{
    data->message->ResetReading();
    RexUUID aID = data->message->ReadUUID();
    RexUUID sID = data->message->ReadUUID();

    if (aID == rexlogicmodule_->GetServerConnection()->GetInfo().agentID &&
        sID == rexlogicmodule_->GetServerConnection()->GetInfo().sessionID)
    {
        RexLogicModule::LogInfo("LogoutReply received with matching IDs. Logging out.");
        rexlogicmodule_->GetServerConnection()->ForceServerDisconnect();
        rexlogicmodule_->DeleteScene("World");
    }
    
    return false;
}

bool NetworkEventHandler::HandleOSNE_AgentMovementComplete(OpenSimProtocol::NetworkEventInboundData* data)
{
    data->message->ResetReading();

    RexUUID agentid = data->message->ReadUUID();
    RexUUID sessionid = data->message->ReadUUID();
    
    //if (agentid == rexlogicmodule_->GetServerConnection()->GetInfo().agentID && sessionid == rexlogicmodule_->GetServerConnection()->GetInfo().sessionID)
    {
        Vector3 position = data->message->ReadVector3(); 
        Vector3 lookat = data->message->ReadVector3();

        assert (rexlogicmodule_->GetAvatarControllable() && "Handling agent movement complete event before avatar controller is created.");
        rexlogicmodule_->GetAvatarControllable()->HandleAgentMovementComplete(Core::OpenSimToOgreCoordinateAxes(position), Core::OpenSimToOgreCoordinateAxes(lookat));

        /// \todo tucofixme, what to do with regionhandle & timestamp?
        uint64_t regionhandle = data->message->ReadU64();
        uint32_t timestamp = data->message->ReadU32(); 
    }
    
    rexlogicmodule_->GetServerConnection()->SendAgentSetAppearancePacket();
    return false;
}

bool NetworkEventHandler::HandleOSNE_ImprovedTerseObjectUpdate(OpenSimProtocol::NetworkEventInboundData* data)
{
    data->message->ResetReading();   

    uint64_t regionhandle = data->message->ReadU64();    
    data->message->SkipToNextVariable(); // TimeDilation U16 ///\todo Unhandled inbound variable 'TimeDilation'.

    if (data->message->GetCurrentBlock() >= data->message->GetBlockCount())
    {
        RexLogicModule::LogDebug("Empty ImprovedTerseObjectUpdate packet received, ignoring.");
        return false;
    }

    // Variable block
    size_t instance_count = data->message->ReadCurrentBlockInstanceCount();
    for(size_t i = 0; i < instance_count; i++)
    {
        size_t bytes_read = 0;
        const uint8_t *bytes = data->message->ReadBuffer(&bytes_read);

        uint32_t localid = 0;
        switch(bytes_read)
        {
        case 30:
            rexlogicmodule_->GetAvatarHandler()->HandleTerseObjectUpdate_30bytes(bytes); 
            break;
        case 60:
            localid = *reinterpret_cast<uint32_t*>((uint32_t*)&bytes[0]); 
            if (rexlogicmodule_->GetPrimEntity(localid)) 
                rexlogicmodule_->GetPrimitiveHandler()->HandleTerseObjectUpdateForPrim_60bytes(bytes);
            else if (rexlogicmodule_->GetAvatarEntity(localid))
                rexlogicmodule_->GetAvatarHandler()->HandleTerseObjectUpdateForAvatar_60bytes(bytes);
            break;
        default:    
            std::stringstream ss; 
            ss << "Unhandled ImprovedTerseObjectUpdate block of size " << bytes_read << "!";
            RexLogicModule::LogInfo(ss.str());
            break;
        }
        
        data->message->SkipToNextVariable(); ///\todo Unhandled inbound variable 'TextureEntry'.
    }
    return false;
}

bool NetworkEventHandler::HandleOSNE_KillObject(OpenSimProtocol::NetworkEventInboundData* data)
{
    data->message->ResetReading();

    // Variable block
    size_t instance_count = data->message->ReadCurrentBlockInstanceCount();
    for(size_t i = 0; i < instance_count; ++i)
    {
        uint32_t killedobjectid = data->message->ReadU32();

        if (rexlogicmodule_->GetPrimEntity(killedobjectid))
            return rexlogicmodule_->GetPrimitiveHandler()->HandleOSNE_KillObject(killedobjectid);
        if (rexlogicmodule_->GetAvatarEntity(killedobjectid))
            return rexlogicmodule_->GetAvatarHandler()->HandleOSNE_KillObject(killedobjectid);
    }

    return false;
}

bool NetworkEventHandler::HandleOSNE_InventoryDescendents(OpenSimProtocol::NetworkEventInboundData* data)
{
    NetInMessage &msg = *data->message;
    msg.ResetReading();

    // AgentData
    RexUUID agent_id = msg.ReadUUID();
    RexUUID session_id = msg.ReadUUID();

    // Check that this packet is for us.
    if (agent_id != rexlogicmodule_->GetServerConnection()->GetInfo().agentID && 
        session_id != rexlogicmodule_->GetServerConnection()->GetInfo().sessionID)
    {
        RexLogicModule::LogError("Received InventoryDescendents packet with wrong AgentID and/or SessionID.");
        return false;
    }

    //Get Inventory pointer.
    InventoryPtr inventory = rexlogicmodule_->GetInventory();

    msg.ReadUUID(); //OwnerID, owner of the folders creatd.
    msg.ReadS32(); //Version, version of the folder for caching
    int32_t descendents = msg.ReadS32(); //Descendents, count to help with caching
    ///\todo Crashes here sometimes, can't read the instance counts right if they are zero.
    if (descendents == 0)
        RexLogicModule::LogInfo("InventoryDescendents, variable Descendents is null");

    // FolderData, Variable block.
    // Contains sub-folders that the requested folder contains.
    size_t instance_count = msg.ReadCurrentBlockInstanceCount();
//    std::cout << "InventoryDescendents: Folders: " << instance_count << std::endl;
    for(size_t i = 0; i < instance_count; ++i)
    {
        RexUUID folder_id = msg.ReadUUID();
        RexUUID parent_id = msg.ReadUUID();
        int8_t type = msg.ReadS8();
        std::string folder_name = msg.ReadString();

        OpenSimProtocol::InventoryFolder *parent = 0, *folder = 0;
        parent = inventory->GetChildFolderByID(parent_id);
        if (!parent)
        {
            RexLogicModule::LogInfo("InventoryDescendents: could not get parent. Variable ParentID is " + parent_id.ToString());
            continue;
        }
        
        folder = inventory->GetOrCreateNewFolder(folder_id, *parent);
        folder->SetName(folder_name);
        //folder->SetType(type);
        std::cout << "Folder: " << folder_name << std::endl;
        
    }

    // ItemData, Variable block.
    // Contains items that the requested folder contains.
    instance_count = msg.ReadCurrentBlockInstanceCount();
//    std::cout << "InventoryDescendents: Assets, count: " << instance_count << std::endl;
    for(size_t i = 0; i < instance_count; ++i)
    {
    /*
        RexUUID item_id = msg.ReadUUID();
        RexUUID folder_id = msg.ReadUUID();

        ///\todo Decide if we want to use these permission related stuff?
        msg.SkipToNextVariable(); //msg.ReadUUID(); //CreatorID
        msg.SkipToNextVariable(); //msg.ReadUUID(); //OwnerID
        msg.SkipToNextVariable(); //msg.ReadUUID(); //GroupID
        msg.SkipToNextVariable(); //msg.ReadU32(); //BaseMask
        msg.SkipToNextVariable(); //msg.ReadU32(); //OwnerMask
        msg.SkipToNextVariable(); //msg.ReadU32(); //GroupMask
        msg.SkipToNextVariable(); //msg.ReadU32(); //EveryoneMask
        msg.SkipToNextVariable(); //msg.ReadU32(); //NextOwnerMask
        msg.SkipToNextVariable(); //msg.ReadBool(); //GroupOwned

        RexUUID asset_id = msg.ReadUUID(); //AssetID
        asset_type_t at = msg.ReadS8(); //Type (asset type?)
        inventory_type_t it = msg.ReadS8(); //InvType

        msg.SkipToNextVariable(); //msg.ReadU32(); //Flags (what are these?)
        msg.SkipToNextVariable(); // SaleType, not interested.
        msg.SkipToNextVariable(); // SalePrice, not interested.

        std::string item_name = msg.ReadString();
        std::string item_desc = msg.ReadString();
        //msg.ReadS32(); //CreationDate
        //msg.ReadU32(); //CRC

        // Add the new item to the inventory.
        InventoryItem item(item_id, asset_id, at);
        std::cout << "Item: " << item_name << std::endl;
        item.SetName(item_name);
        item.SetDescription(item_desc);
        InventoryFolder *folder = inventory->GetFirstSubFolderByID(folder_id);
        folder->AddItem(item);
        */
    }

    return false;
}

} //namespace RexLogic
