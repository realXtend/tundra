// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include <sstream>

#include "NetworkEventHandler.h"
#include "NetInMessage.h"
#include "RexProtocolMsgIDs.h"
#include "OpenSimProtocolModule.h"
#include "RexLogicModule.h"
#include "Entity.h"
#include "AvatarController.h"

// Ogre renderer -specific.
#include <OgreMaterialManager.h>

#include "ConversionUtils.h"
#include "BitStream.h"
#include "TerrainDecoder.h"
#include "Terrain.h"
#include "Avatar.h"
#include "Primitive.h"

using namespace Core;

namespace
{
    /// Clones a new Ogre material that renders using the given ambient color. This function will be removed or refactored later on, once proper material system is present. -jj.
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
        
        DebugCreateAmbientColorMaterial("AmbientWhite", 1.f, 1.f, 1.f);
        DebugCreateAmbientColorMaterial("AmbientGreen", 0.f, 1.f, 0.f);
        DebugCreateAmbientColorMaterial("AmbientRed", 1.f, 0.f, 0.f);
    }

    NetworkEventHandler::~NetworkEventHandler()
    {

    }

    bool NetworkEventHandler::HandleOpenSimNetworkEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        OpenSimProtocol::NetworkEventInboundData *netdata = checked_static_cast<OpenSimProtocol::NetworkEventInboundData *>(data);
        switch(event_id)
        {
            case RexNetMsgRegionHandshake:              return HandleOSNE_RegionHandshake(netdata); break;
            case RexNetMsgAgentMovementComplete:        return HandleOSNE_AgentMovementComplete(netdata); break;
            case RexNetMsgGenericMessage:               return HandleOSNE_GenericMessage(netdata); break;
            case RexNetMsgLogoutReply:                  return HandleOSNE_LogoutReply(netdata); break;
            case RexNetMsgImprovedTerseObjectUpdate:    return HandleOSNE_ImprovedTerseObjectUpdate(netdata); break;
            case RexNetMsgKillObject:                   return HandleOSNE_KillObject(netdata); break;               
            case RexNetMsgObjectUpdate:                 return HandleOSNE_ObjectUpdate(netdata); break;
            case RexNetMsgObjectProperties:             return rexlogicmodule_->GetPrimitiveHandler()->HandleOSNE_ObjectProperties(netdata); break;
            case RexNetMsgLayerData:                    return rexlogicmodule_->GetTerrainHandler()->HandleOSNE_LayerData(netdata); break;
            default:                                    return false; break;
        }
        
        return false;
    }

    bool NetworkEventHandler::HandleOSNE_ObjectUpdate(OpenSimProtocol::NetworkEventInboundData* data)
    {
        data->message->ResetReading();
        data->message->SkipToFirstVariableByName("PCode");
        uint8_t pcode = data->message->ReadU8();
        switch(pcode)
        {
            case 0x09: return rexlogicmodule_->GetPrimitiveHandler()->HandleOSNE_ObjectUpdate(data); break;
            case 0x2f: return rexlogicmodule_->GetAvatarHandler()->HandleOSNE_ObjectUpdate(data); break;
        }
        return false;
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
        else
            return false;    
    }
    
    bool NetworkEventHandler::HandleOSNE_RegionHandshake(OpenSimProtocol::NetworkEventInboundData* data)    
    {
        size_t bytesRead = 0;

        data->message->ResetReading();    
        data->message->SkipToNextVariable(); // RegionFlags U32
        data->message->SkipToNextVariable(); // SimAccess U8

        std::string sim_name = data->message->ReadString();
        rexlogicmodule_->GetServerConnection()->simName_ = sim_name;
        
        RexLogicModule::LogInfo("Joined to the sim \"" + sim_name + "\".");
        
        // Create the "World" scene.
        boost::shared_ptr<OpenSimProtocol::OpenSimProtocolModule> sp = netInterface_.lock();
        if (!sp.get())
        {
            RexLogicModule::LogError("NetworkEventHandler: Could not acquire OpenSimProtocolModule!");
            return false;
        }
                
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
        
        if (agentid == rexlogicmodule_->GetServerConnection()->GetInfo().agentID && sessionid == rexlogicmodule_->GetServerConnection()->GetInfo().sessionID)
        {
            Vector3 position = data->message->ReadVector3(); 
            Vector3 lookat = data->message->ReadVector3();
            rexlogicmodule_->GetAvatarController()->HandleServerObjectUpdate(Core::OpenSimToOgreCoordinateAxes(position),Core::OpenSimToOgreQuaternion(lookat));

            /// \todo tucofixme, what to do with regionhandle & timestamp?
            uint64_t regionhandle = data->message->ReadU64();
            uint32_t timestamp = data->message->ReadU32(); 
        }
        return false;
    }

    bool NetworkEventHandler::HandleOSNE_ImprovedTerseObjectUpdate(OpenSimProtocol::NetworkEventInboundData* data)
    {
        data->message->ResetReading();   
    
        uint64_t regionhandle = data->message->ReadU64();    
        data->message->SkipToNextVariable(); // TimeDilation U16 ///\todo Unhandled inbound variable 'TimeDilation'.
  
        NetVariableType nextvartype = data->message->CheckNextVariableType();
        while(nextvartype != NetVarNone)
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
                    if(rexlogicmodule_->GetPrimEntity(localid)) 
                        rexlogicmodule_->GetPrimitiveHandler()->HandleTerseObjectUpdateForPrim_60bytes(bytes); 
                    else if(rexlogicmodule_->GetAvatarEntity(localid))
                        rexlogicmodule_->GetAvatarHandler()->HandleTerseObjectUpdateForAvatar_60bytes(bytes);                            
                    break;
                default:    
                    std::stringstream ss; 
                    ss << "Unhandled ImprovedTerseObjectUpdate block of size ";
                    ss << bytes_read << "!";
                    RexLogicModule::LogInfo(ss.str());
                    break;
            }
            data->message->SkipToNextVariable(); // TextureEntry variable ///\todo Unhandled inbound variable 'TextureEntry'.
            nextvartype = data->message->CheckNextVariableType();
        }
        return false;
    }

    bool NetworkEventHandler::HandleOSNE_KillObject(OpenSimProtocol::NetworkEventInboundData* data)
    {
        data->message->ResetReading();
        uint32_t killedobjectid = data->message->ReadU32();

        if(rexlogicmodule_->GetPrimEntity(killedobjectid))
            return rexlogicmodule_->GetPrimitiveHandler()->HandleOSNE_KillObject(killedobjectid);
        else if(rexlogicmodule_->GetAvatarEntity(killedobjectid))    
            return rexlogicmodule_->GetAvatarHandler()->HandleOSNE_KillObject(killedobjectid);            

        return false;
    }
}
