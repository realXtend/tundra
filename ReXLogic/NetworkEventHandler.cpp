// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "NetworkEventHandler.h"
#include "NetInMessage.h"

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
        // TODO tucofixme, get event_id from opensimprotocol module?
        switch(event_id)
        {
            case 0:  return HandleOSNE_ObjectUpdate(data); break;
            case 1:  return HandleOSNE_RexPrimData(data); break;
            default: return false; break;
        }
    }

    Foundation::EntityPtr NetworkEventHandler::GetEntitySafe(Core::entity_id_t entityid)
    {
        // TODO tucofixme, use stored pointers to scenemanager and scene instead of always getting them?
        Foundation::SceneManagerServiceInterface *sceneManager = framework_->GetService<Foundation::SceneManagerServiceInterface>(Foundation::Service::ST_SceneManager);
        Foundation::ScenePtr scene = sceneManager->GetScene("World");

        Foundation::EntityPtr entity;
        if (!scene->HasEntity(entityid))
            return CreateNewEntity(entityid);
        else
            return scene->GetEntity(entityid);
    }

    Foundation::EntityPtr NetworkEventHandler::CreateNewEntity(Core::entity_id_t entityid)
    {
        // TODO tucofixme, use stored pointers to scenemanager and scene instead of always getting them?
        Foundation::SceneManagerServiceInterface *sceneManager = framework_->GetService<Foundation::SceneManagerServiceInterface>(Foundation::Service::ST_SceneManager);
        Foundation::ScenePtr scene = sceneManager->GetScene("World");
        
        Foundation::EntityPtr entity = scene->CreateEntity(entityid);
        if(entity)
        {
            Foundation::ComponentPtr component = framework_->GetComponentManager()->CreateComponent(EC_OpenSimPrim::NameStatic()); 
            entity->AddEntityComponent(component);
        }
        return entity;
    }

    bool NetworkEventHandler::HandleOSNE_ObjectUpdate(Foundation::EventDataInterface* data)
    {    
        /*
        // RexNetworkEventData *rexdata = static_cast<RexLogic::RexNetworkEventData *>(data);
        NetInMessage *msg = NULL; // todo tucofixme,  = rexdata->Message;
 
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
            Foundation::EntityPtr entity = GetEntitySafe(localid);
            if(entity)
            {
                Foundation::ComponentInterfacePtr component = entity->GetComponent("EC_OpenSimPrim");
                static_cast<EC_OpenSimPrim*>(component.get())->HandleObjectUpdate(data);
            }
        }
        // avatar
        else if (pcode == 0x2f)
        {
        
        }
        */ 
        return false;
    }

    bool NetworkEventHandler::HandleOSNE_RexPrimData(Foundation::EventDataInterface* data)
    {
        return false;
    }
}
