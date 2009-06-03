// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AvatarControllable.h"
//#include "EC_Controllable.h"
#include "RexLogicModule.h"
#include "SceneEvents.h"
#include "Entity.h"

namespace RexLogic
{
    AvatarControllable::AvatarControllable(const Foundation::EventManagerPtr &event_manager) : 
        event_manager_(event_manager)
      , input_event_category_(event_manager->QueryEventCategory("Input"))
      , scene_event_category_(event_manager->QueryEventCategory("Scene"))
      , action_event_category_(event_manager->QueryEventCategory("Actions"))
    {
        if (input_event_category_ == Core::IllegalEventCategory)
            RexLogicModule::LogError("AvatarControllable: failed to acquire input event category, controller disabled.");
        if (scene_event_category_ == Core::IllegalEventCategory)
            RexLogicModule::LogError("AvatarControllable: failed to acquire scene event category, controller disabled.");


    }

        
    bool AvatarControllable::HandleSceneEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if (event_id == Scene::Events::EVENT_CONTROLLABLE_ENTITY)
        {
            entity_ = checked_static_cast<Scene::Events::EntityEventData*>(data)->entity;
            Scene::EntityPtr entity = entity_.lock();

            assert (entity && "Received event EVENT_CONTROLLABLE_ENTITY with NULL entity.");
            component_ = entity->GetComponent(EC_Controllable::NameStatic());
            Foundation::ComponentPtr component = component_.lock();

            assert (component && "Received event EVENT_CONTROLLABLE_ENTITY with NULL controllable component.");
            EC_Controllable *controllable = checked_static_cast<EC_Controllable*>(component.get());

            input_events_ = Actions::AssignCommonActions(controllable);

            return true;
        }

        return false;
    }

    bool AvatarControllable::HandleInputEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        Actions::ActionInputMap::const_iterator it = input_events_.find(event_id);
        if (it != input_events_.end())
        {
            Scene::Events::EntityEventData event_data;
            event_data.entity = entity_.lock();
            if (event_data.entity) // only send the event if we have an existing entity, no point otherwise
                event_manager_->SendEvent(action_event_category_, it->second, &event_data);
        }

        return false;
    }

    bool AvatarControllable::HandleActionEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if (checked_static_cast<Scene::Events::EntityEventData*>(data)->entity.get() == entity_.lock().get())
        {
            int id = event_id;
            // move entity
        }

        return false;
    }
}

