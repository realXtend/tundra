// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "SceneEventHandler.h"
#include "SceneEvents.h"
#include "RexLogicModule.h"

namespace RexLogic
{
    SceneEventHandler::SceneEventHandler(Foundation::Framework *framework, RexLogicModule *rexlogicmodule)
    {
        framework_ = framework;
        rexlogicmodule_ = rexlogicmodule;    
    }

    SceneEventHandler::~SceneEventHandler()
    {
    }
    
    bool SceneEventHandler::HandleSceneEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        Scene::Events::SceneEventData *event_data = dynamic_cast<Scene::Events::SceneEventData *>(data);        
        
        switch(event_id)
        {
            case Scene::Events::EVENT_ENTITY_SELECT:
                rexlogicmodule_->GetServerConnection()->SendObjectSelectPacket(event_data->localID);
                break;
            case Scene::Events::EVENT_ENTITY_DESELECT:
                rexlogicmodule_->GetServerConnection()->SendObjectDeselectPacket(event_data->localID);
                break;
            case Scene::Events::EVENT_ENTITY_UPDATED:
                rexlogicmodule_->GetServerConnection()->SendMultipleObjectUpdatePacket(event_data->entity_ptr_list);
                break;
            case Scene::Events::EVENT_ENTITY_DELETED:
                HandleEntityDeletedEvent(event_data->localID);
                break;
            default:
                break;
        }
 
        return false;
    }
    
    void SceneEventHandler::HandleEntityDeletedEvent(Core::event_id_t entityid)    
    {        
        Scene::ScenePtr scene = framework_->GetScene("World");
        if (!scene)
            return;

        Scene::EntityPtr entity = scene->GetEntity(entityid);
        if(entity && rexlogicmodule_->GetAvatarController()->GetAvatarEntity() && entity->GetId() == rexlogicmodule_->GetAvatarController()->GetAvatarEntity()->GetId())
        {
            Scene::EntityPtr emptyavatar;
            rexlogicmodule_->GetAvatarController()->SetAvatarEntity(emptyavatar);
        }      
    }    
    
}
