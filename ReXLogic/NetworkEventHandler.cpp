// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "NetworkEventHandler.h"
#include "NetInMessage.h"
#include "RexProtocolMsgIDs.h"
#include "OpenSimProtocolModule.h"
#include "RexLogicModule.h"

#include "EC_ObjIdentity.h"
#include "EC_ObjCollision.h"
#include "EC_ObjFreeData.h"
#include "EC_SelectPriority.h"
#include "EC_ServerScript.h"
#include "EC_SpatialSound.h"
#include "EC_OpenSimPrim.h"

namespace RexLogic
{
    NetworkEventHandler::NetworkEventHandler(Foundation::Framework *framework)
    {
        framework_ = framework;
    }

    NetworkEventHandler::~NetworkEventHandler()
    {

    }

    bool NetworkEventHandler::HandleOpenSimNetworkEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if(event_id == OpenSimProtocol::OpenSimProtocolModule::EVENT_NETWORK_IN)
        {
            OpenSimProtocol::NetworkEventInboundData *netdata = static_cast<OpenSimProtocol::NetworkEventInboundData *>(data);
            switch(netdata->messageID)
            {
                case RexNetMsgObjectUpdate:             return HandleOSNE_ObjectUpdate(netdata); break;
                case RexNetMsgGenericMessage:           return HandleOSNE_GenericMessage(netdata); break;
                case RexNetMsgObjectName:               return HandleOSNE_ObjectName(netdata); break;
                case RexNetMsgObjectDescription:        return HandleOSNE_ObjectDescription(netdata); break;                
                default: return false; break;
            }
        }
        return false;
    }

    Foundation::EntityPtr NetworkEventHandler::GetPrimEntitySafe(Core::entity_id_t entityid)
    {
        Foundation::SceneManagerServiceInterface *sceneManager = framework_->GetService<Foundation::SceneManagerServiceInterface>(Foundation::Service::ST_SceneManager);
        Foundation::ScenePtr scene = sceneManager->GetScene("World");

        // TODO tucofixme, how to make sure this is a prim entity?
        if (!scene->HasEntity(entityid))
            return CreateNewPrimEntity(entityid);
        else
            return scene->GetEntity(entityid);
    }
    
    Foundation::EntityPtr NetworkEventHandler::CreateNewPrimEntity(Core::entity_id_t entityid)
    {
        Foundation::SceneManagerServiceInterface *sceneManager = framework_->GetService<Foundation::SceneManagerServiceInterface>(Foundation::Service::ST_SceneManager);
        Foundation::ScenePtr scene = sceneManager->GetScene("World");
        
        Core::StringVector defaultcomponents;
        defaultcomponents.push_back(EC_OpenSimPrim::NameStatic());
        
        Foundation::EntityPtr entity = scene->CreateEntity(entityid,defaultcomponents); 
        return entity;
    }
    
    Foundation::EntityPtr NetworkEventHandler::GetAvatarEntitySafe(Core::entity_id_t entityid)
    {
        Foundation::SceneManagerServiceInterface *sceneManager = framework_->GetService<Foundation::SceneManagerServiceInterface>(Foundation::Service::ST_SceneManager);
        Foundation::ScenePtr scene = sceneManager->GetScene("World");

        // TODO tucofixme, how to make sure this is a avatar entity?
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
        // TODO tucofixme, add avatar default components
        
        Foundation::EntityPtr entity = scene->CreateEntity(entityid,defaultcomponents); 
        return entity;
    }


    bool NetworkEventHandler::HandleOSNE_ObjectUpdate(OpenSimProtocol::NetworkEventInboundData* data)
    {
        NetInMessage *msg = data->message;
 
        uint64_t regionhandle = msg->ReadU64();
        msg->SkipToNextVariable(); // TimeDilation U16

        // todo tucofixme, handle multiple ObjectData in same message
        uint32_t localid = msg->ReadU32(); 
        msg->SkipToNextVariable();		// State U8
        RexUUID fullid = msg->ReadUUID();
        msg->SkipToNextVariable();		// CRC U32
        uint8_t pcode = msg->ReadU8();
        
        // object
        if (pcode == 0x09)
        {
            Foundation::EntityPtr entity = GetPrimEntitySafe(localid);
            if(entity)
            {
                Foundation::ComponentInterfacePtr component = entity->GetComponent("EC_OpenSimPrim");
                static_cast<EC_OpenSimPrim*>(component.get())->HandleObjectUpdate(data);
            }
        }
        // avatar
        else if (pcode == 0x2f)
        {
            Foundation::EntityPtr entity = GetAvatarEntitySafe(localid);
            // TODO tucofixme, set values to component      
        }
         
        return false;
    }

    bool NetworkEventHandler::HandleOSNE_ObjectName(OpenSimProtocol::NetworkEventInboundData* data)
    {
        NetInMessage *msg = data->message;
    
        msg->ResetReading();
        msg->SkipToFirstVariableByName("LocalID");
        uint32_t localid = msg->ReadU32();
        
        Foundation::EntityPtr entity = GetPrimEntitySafe(localid);
        if(entity)
        {
            Foundation::ComponentInterfacePtr component = entity->GetComponent("EC_OpenSimPrim");
            static_cast<EC_OpenSimPrim*>(component.get())->HandleObjectDescription(data);        
        }
        return false;    
    }

    bool NetworkEventHandler::HandleOSNE_ObjectDescription(OpenSimProtocol::NetworkEventInboundData* data)
    {
        return false;    
    }


    bool NetworkEventHandler::HandleOSNE_GenericMessage(OpenSimProtocol::NetworkEventInboundData* data)
    {
        return false;    
    }

    bool NetworkEventHandler::HandleOSNE_RexPrimData(OpenSimProtocol::NetworkEventInboundData* data)
    {
        return false;
    }
}
