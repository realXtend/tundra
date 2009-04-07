// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "NetworkEventHandler.h"
#include "NetInMessage.h"
#include "RexProtocolMsgIDs.h"
#include "OpenSimProtocolModule.h"
#include "RexLogicModule.h"

#include "EC_Viewable.h"
#include "EC_FreeData.h"
#include "EC_SpatialSound.h"
#include "EC_OpenSimPrim.h"

namespace RexLogic
{
    NetworkEventHandler::NetworkEventHandler(Foundation::Framework *framework, RexLogicModule *rexlogicmodule)
    {
        framework_ = framework;
        rexlogicmodule_ = rexlogicmodule;
    }

    NetworkEventHandler::~NetworkEventHandler()
    {

    }

    bool NetworkEventHandler::HandleOpenSimNetworkEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if(event_id == OpenSimProtocol::EVENT_NETWORK_IN)
        {
            OpenSimProtocol::NetworkEventInboundData *netdata = checked_static_cast<OpenSimProtocol::NetworkEventInboundData *>(data);
            switch(netdata->messageID)
            {
                case RexNetMsgGenericMessage:           return HandleOSNE_GenericMessage(netdata); break;
                case RexNetMsgLogoutReply:              return HandleOSNE_LogoutReply(netdata); break;
                case RexNetMsgObjectDescription:        return HandleOSNE_ObjectDescription(netdata); break; 
                case RexNetMsgObjectName:               return HandleOSNE_ObjectName(netdata); break;
                case RexNetMsgObjectUpdate:             return HandleOSNE_ObjectUpdate(netdata); break;
                case RexNetMsgRegionHandshake:          return HandleOSNE_RegionHandshake(netdata); break;
                default:                                return false; break;
            }
        }
        return false;
    }

    Foundation::EntityPtr NetworkEventHandler::GetOrCreatePrimEntity(Core::entity_id_t entityid)
    {
        Foundation::SceneManagerServiceInterface *sceneManager = framework_->GetService<Foundation::SceneManagerServiceInterface>(Foundation::Service::ST_SceneManager);
        Foundation::ScenePtr scene = sceneManager->GetScene("World");

        Foundation::EntityPtr entity = scene->GetEntity(entityid);
        if (!entity)
            return CreateNewPrimEntity(entityid);

        ///\todo Check that the entity has a prim component, if not, add it to the entity. 
        return entity;
    }
  
    Foundation::EntityPtr NetworkEventHandler::GetOrCreatePrimEntity(Core::entity_id_t entityid, const RexUUID &fullid)
    {
        Foundation::SceneManagerServiceInterface *sceneManager = framework_->GetService<Foundation::SceneManagerServiceInterface>(Foundation::Service::ST_SceneManager);
        Foundation::ScenePtr scene = sceneManager->GetScene("World");

        Foundation::EntityPtr entity = scene->GetEntity(entityid);
        if (!entity)
        {
            UUIDs_[fullid] = entityid;
            return CreateNewPrimEntity(entityid);
        }

        ///\todo Check that the entity has a prim component, if not, add it to the entity.
        return entity;
    }  
   
    Foundation::EntityPtr NetworkEventHandler::GetOrCreatePrimEntity(const RexUUID &entityuuid)
    {
        Foundation::SceneManagerServiceInterface *sceneManager = framework_->GetService<Foundation::SceneManagerServiceInterface>(Foundation::Service::ST_SceneManager);
        Foundation::ScenePtr scene = sceneManager->GetScene("World");

        IDMap::iterator iter = UUIDs_.find(entityuuid);
        if (iter == UUIDs_.end())
            return Foundation::EntityPtr();
        else
            return scene->GetEntity(iter->second);
    }    
    
    
    Foundation::EntityPtr NetworkEventHandler::CreateNewPrimEntity(Core::entity_id_t entityid)
    {
        Foundation::SceneManagerServiceInterface *sceneManager = framework_->GetService<Foundation::SceneManagerServiceInterface>(Foundation::Service::ST_SceneManager);
        Foundation::ScenePtr scene = sceneManager->GetScene("World");
        
        Core::StringVector defaultcomponents;
        defaultcomponents.push_back(EC_OpenSimPrim::NameStatic());
        defaultcomponents.push_back(EC_Viewable::NameStatic());
        
        Foundation::EntityPtr entity = scene->CreateEntity(entityid,defaultcomponents); 
        return entity;
    }
    
    Foundation::EntityPtr NetworkEventHandler::GetAvatarEntitySafe(Core::entity_id_t entityid)
    {
        Foundation::SceneManagerServiceInterface *sceneManager = framework_->GetService<Foundation::SceneManagerServiceInterface>(Foundation::Service::ST_SceneManager);
        Foundation::ScenePtr scene = sceneManager->GetScene("World");

        /// \todo tucofixme, how to make sure this is a avatar entity?
        if (!scene->HasEntity(entityid))
            return CreateNewAvatarEntity(entityid);
        else
            return scene->GetEntity(entityid);
    }    

    Foundation::EntityPtr NetworkEventHandler::CreateNewAvatarEntity(Core::entity_id_t entityid)
    {
        Foundation::SceneManagerServiceInterface *sceneManager = framework_->GetService<Foundation::SceneManagerServiceInterface>(Foundation::Service::ST_SceneManager);
        Foundation::ScenePtr scene = sceneManager->GetScene("World");
        
        Core::StringVector defaultcomponents;
        /// \todo tucofixme, add avatar default components
        
        Foundation::EntityPtr entity = scene->CreateEntity(entityid,defaultcomponents); 
        return entity;
    }


    bool NetworkEventHandler::HandleOSNE_ObjectUpdate(OpenSimProtocol::NetworkEventInboundData* data)
    {
        NetInMessage *msg = data->message;
 
        msg->ResetReading();
        uint64_t regionhandle = msg->ReadU64();
        msg->SkipToNextVariable(); // TimeDilation U16

        uint32_t localid = msg->ReadU32(); 
        msg->SkipToNextVariable();		// State U8
        RexUUID fullid = msg->ReadUUID();
        msg->SkipToNextVariable();		// CRC U32
        uint8_t pcode = msg->ReadU8();
        
        Foundation::EntityPtr entity;
        switch(pcode)
        {
            // Prim
            case 0x09:
                entity = GetOrCreatePrimEntity(localid,fullid);
                if(entity)
                {
                    Foundation::ComponentInterfacePtr component = entity->GetComponent("EC_OpenSimPrim");
                    checked_static_cast<EC_OpenSimPrim*>(component.get())->HandleObjectUpdate(data);
                }
                break;
            // Avatar                
            case 0x2f:
                entity = GetAvatarEntitySafe(localid);
                /// \todo tucofixme, set values to component      
                break;
        }
        return false;
    }

    bool NetworkEventHandler::HandleOSNE_ObjectName(OpenSimProtocol::NetworkEventInboundData* data)
    {
        NetInMessage *msg = data->message;
    
        msg->ResetReading();
        msg->SkipToFirstVariableByName("LocalID");
        uint32_t localid = msg->ReadU32();
        
        Foundation::EntityPtr entity = GetOrCreatePrimEntity(localid);
        if(entity)
        {
            Foundation::ComponentInterfacePtr component = entity->GetComponent("EC_OpenSimPrim");
            checked_static_cast<EC_OpenSimPrim*>(component.get())->HandleObjectName(data);        
        }
        return false;
    }

    bool NetworkEventHandler::HandleOSNE_ObjectDescription(OpenSimProtocol::NetworkEventInboundData* data)
    {
        NetInMessage *msg = data->message;
    
        msg->ResetReading();
        msg->SkipToFirstVariableByName("LocalID");
        uint32_t localid = msg->ReadU32();
        
        Foundation::EntityPtr entity = GetOrCreatePrimEntity(localid);
        if(entity)
        {
            Foundation::ComponentInterfacePtr component = entity->GetComponent("EC_OpenSimPrim");
            checked_static_cast<EC_OpenSimPrim*>(component.get())->HandleObjectDescription(data);        
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

        if(methodname == "RexMediaUrl")
            return HandleRexGM_RexMediaUrl(data);
        else if(methodname == "RexPrimData")
            return HandleRexGM_RexPrimData(data); 
        else
            return false;    
    }

    bool NetworkEventHandler::HandleRexGM_RexMediaUrl(OpenSimProtocol::NetworkEventInboundData* data)
    {
        /// \todo tucofixme
        return false;
    }

    bool NetworkEventHandler::HandleRexGM_RexPrimData(OpenSimProtocol::NetworkEventInboundData* data)
    {
        size_t bytes_read;    
        data->message->ResetReading();
        data->message->SkipToFirstVariableByName("Parameter");
        RexUUID primuuid(data->message->ReadString());
        
        Foundation::EntityPtr entity = GetOrCreatePrimEntity(primuuid);
        if(entity)
        {
            // Calculate full data size
            size_t fulldatasize = 0;        
            NetVariableType nextvartype = data->message->CheckNextVariableType();
            while(nextvartype != NetVarNone)
            {
                data->message->ReadBuffer(&bytes_read);
                fulldatasize += bytes_read;
                nextvartype = data->message->CheckNextVariableType();
            }
           
            size_t bytes_read;
            const uint8_t *readbytedata;
            data->message->ResetReading();
            data->message->SkipToFirstVariableByName("Parameter");
            data->message->SkipToNextVariable();            // Prim UUID
            
            uint8_t *fulldata = new uint8_t[fulldatasize];
            int pos = 0;
            nextvartype = data->message->CheckNextVariableType();
            while(nextvartype != NetVarNone)
            {
                readbytedata = data->message->ReadBuffer(&bytes_read);
                memcpy(fulldata+pos,readbytedata,bytes_read);
                pos += bytes_read;
                nextvartype = data->message->CheckNextVariableType();
            }

            Foundation::ComponentInterfacePtr oscomponent = entity->GetComponent("EC_OpenSimPrim");
            checked_static_cast<EC_OpenSimPrim*>(oscomponent.get())->HandleRexPrimData(fulldata);
            // todo tucofixme, checked_static_cast<EC_OpenSimPrim*>(oscomponent.get())->PrintDebug();

            Foundation::ComponentInterfacePtr viewcomponent = entity->GetComponent("EC_Viewable");
            checked_static_cast<EC_Viewable*>(viewcomponent.get())->HandleRexPrimData(fulldata);
            // todo tucofixme, checked_static_cast<EC_Viewable*>(viewcomponent.get())->PrintDebug();
            
            delete fulldata;
        }
        return false;
    }
    
    bool NetworkEventHandler::HandleOSNE_RegionHandshake(OpenSimProtocol::NetworkEventInboundData* data)    
    {
        size_t bytesRead = 0;

        data->message->ResetReading();    
        data->message->SkipToNextVariable(); // RegionFlags U32
        data->message->SkipToNextVariable(); // SimAccess U8

        std::string simname = data->message->ReadString();
        rexlogicmodule_->GetServerConnection()->simname_ = simname;
        
        RexLogicModule::LogInfo("Joined to the sim \"" + simname + "\".");
        return false;  
    } 

    bool NetworkEventHandler::HandleOSNE_LogoutReply(OpenSimProtocol::NetworkEventInboundData* data)   
    {
        data->message->ResetReading();    
        RexUUID aID = data->message->ReadUUID();
        RexUUID sID = data->message->ReadUUID();

        if (aID == rexlogicmodule_->GetServerConnection()->GetInfo().agentID && sID == rexlogicmodule_->GetServerConnection()->GetInfo().sessionID)
        {
            RexLogicModule::LogInfo("LogoutReply received with matching IDs. Logging out.");
            rexlogicmodule_->GetServerConnection()->CloseServerConnection();
        } 
        return false;   
    } 
}
