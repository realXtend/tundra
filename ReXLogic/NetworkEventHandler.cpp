// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "NetworkEventHandler.h"
#include "RexLogic.h"

#include "EC_ObjIdentity.h"
#include "EC_Collision.h"
#include "EC_ObjFreeData.h"
#include "EC_ObjGeneralProps.h"
#include "EC_SelectPriority.h"
#include "EC_ServerScript.h"
#include "EC_SpatialSound.h"

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
    
    Foundation::EntityPtr entity = scene->CreateEntity(); 
    Foundation::ComponentPtr component = framework_->GetComponentManager()->CreateComponent(EC_ObjIdentity::Name()); 
    entity->AddEntityComponent(component);
   
    return entity;
}

bool NetworkEventHandler::HandleOSNE_ObjectUpdate(Foundation::EventDataInterface* data)
{
    Foundation::EntityPtr entity = GetEntitySafe(0); // TODO tucofixme, get id from data

    Foundation::ComponentInterfacePtr component = entity->GetComponent("EC_ObjIdentity");
    static_cast<EC_ObjIdentity*>(component.get())->HandleObjectUpdate(data);

    return false;
}

bool NetworkEventHandler::HandleOSNE_RexPrimData(Foundation::EventDataInterface* data)
{
    Foundation::EntityPtr entity = GetEntitySafe(0); // tucofixme, get id from data

    return false;
}


