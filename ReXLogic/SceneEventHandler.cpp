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
        
        if (event_id == Scene::EVENT_ENTITY_SELECT)
        {
            rexlogicmodule_->GetServerConnection()->SendObjectSelectPacket(event_data->localID);
            return false;
        }
        
        if (event_id == Scene::EVENT_ENTITY_DESELECT)
        {
            rexlogicmodule_->GetServerConnection()->SendObjectDeselectPacket(event_data->localID);
            return false;
        }
        
        return false;
    }
}
