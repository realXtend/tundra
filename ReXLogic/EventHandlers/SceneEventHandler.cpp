// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EventHandlers/SceneEventHandler.h"
#include "SceneEvents.h"
#include "RexLogicModule.h"
#include "SceneManager.h"

namespace RexLogic
{
SceneEventHandler::SceneEventHandler(Foundation::Framework *framework, RexLogicModule *rexlogicmodule) :
    framework_(framework), rexlogicmodule_(rexlogicmodule)
{
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
    case Scene::Events::EVENT_ENTITY_GRAB:
        rexlogicmodule_->GetServerConnection()->SendObjectGrabPacket(event_data->localID);
        break;
    case Scene::Events::EVENT_ENTITY_DELETED:
        HandleEntityDeletedEvent(event_data->localID);
        break;
    case Scene::Events::EVENT_ENTITY_CREATE:
    {
        Scene::Events::CreateEntityEventData *pos_data = dynamic_cast<Scene::Events::CreateEntityEventData *>(data);
        if (pos_data)
            rexlogicmodule_->GetServerConnection()->SendObjectAddPacket(pos_data->position);
    }
    default:
        break;
    }

    return false;
}

void SceneEventHandler::HandleEntityDeletedEvent(Core::event_id_t entityid)
{
}

}
