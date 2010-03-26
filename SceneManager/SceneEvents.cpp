// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "SceneEvents.h"
#include "EventManager.h"

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

        ///\todo Refactor these.
        void RegisterSceneEvents(const Foundation::EventManagerPtr &event_manager)
        {
            event_category_id_t scene_event_category = event_manager->RegisterEventCategory("Scene");
            event_manager->RegisterEvent(scene_event_category, Events::EVENT_SCENE_ADDED, "Scene Added");
            event_manager->RegisterEvent(scene_event_category, Events::EVENT_SCENE_DELETED, "Scene Deleted");
            event_manager->RegisterEvent(scene_event_category, Events::EVENT_SCENE_CLONED, "Scene Cloned"); ///\todo
            event_manager->RegisterEvent(scene_event_category, Events::EVENT_ENTITY_ADDED, "Entity Added");
            event_manager->RegisterEvent(scene_event_category, Events::EVENT_ENTITY_UPDATED, "Entity Updated");
            event_manager->RegisterEvent(scene_event_category, Events::EVENT_ENTITY_DELETED, "Entity Deleted");
            event_manager->RegisterEvent(scene_event_category, Events::EVENT_ENTITY_SELECT, "Entity Select");
            event_manager->RegisterEvent(scene_event_category, Events::EVENT_ENTITY_SELECTED, "Entity Selected");
            event_manager->RegisterEvent(scene_event_category, Events::EVENT_ENTITY_DESELECT, "Entity Deselect");

            event_manager->RegisterEvent(scene_event_category, Events::EVENT_CONTROLLABLE_ENTITY, "Controllable Entity Created");
            event_manager->RegisterEvent(scene_event_category, Events::EVENT_ENTITY_VISUALS_MODIFIED, "Entity Visual Appearance Modified");
            event_manager->RegisterEvent(scene_event_category, Events::EVENT_ENTITY_MEDIAURL_SET, "Mediaurl set");
        }
    }
}
