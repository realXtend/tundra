// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MemoryLeakCheck.h"
#include "SceneEvents.h"
#include "EventManager.h"
#include "Entity.h"

namespace Scene
{
    namespace Events
    {
        SceneEventData::SceneEventData(const std::string &scene_name)
        :sceneName(scene_name)
        {
        }

        SceneEventData::SceneEventData(entity_id_t id)
        :localID(id)
        {
        }

        SceneEventData::~SceneEventData()
        {
        }

        RaycastEventData::RaycastEventData(entity_id_t id)
        :SceneEventData(id) 
        {        
        }

        RaycastEventData::~RaycastEventData()
        {
        }

        CreateEntityEventData::CreateEntityEventData(Vector3df pos)
        :position(pos)
        {
        }

        CreateEntityEventData::~CreateEntityEventData() {}

        WaterEventData::WaterEventData() : height(0.0) {}
        WaterEventData::WaterEventData(float h) : height(h) {}
        WaterEventData::~WaterEventData() {}
        
        EntityClickedData::EntityClickedData(Scene::Entity *ent) : 
            SceneEventData(ent->GetId()),
            entity(ent)
        {
        }

        EntityClickedData::~EntityClickedData() {}


        void RegisterSceneEvents(const EventManagerPtr &event_manager)
        {

        }
    }
}
