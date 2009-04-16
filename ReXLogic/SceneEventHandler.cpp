// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "SceneEventHandler.h"
#include "SceneModule.h"
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
        Scene::SceneEventData *event_data = dynamic_cast<Scene::SceneEventData *>(data);        
        
        switch(event_id)
        {
            case Scene::EVENT_ENTITY_SELECT:
                rexlogicmodule_->GetServerConnection()->SendObjectSelectPacket(event_data->localID);
                break;
            case Scene::EVENT_ENTITY_DESELECT:
                rexlogicmodule_->GetServerConnection()->SendObjectDeselectPacket(event_data->localID);
                break;
            case Scene::EVENT_ENTITY_UPDATED:
                rexlogicmodule_->GetServerConnection()->SendMultipleObjectUpdatePacket(event_data->entity_ptr_list);
                break;                
            default:
                break;
        }
        
        return false;
    }
}
