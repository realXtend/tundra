// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "SceneManager.h"
#include "Entity.h"
#include "SceneEvents.h"
#include "Framework.h"
#include "ComponentManager.h"
#include "EventManager.h"

namespace Scene
{
    uint SceneManager::gid_ = 0;

    Scene::ScenePtr SceneManager::Clone(const std::string &newName) const
    {
        ScenePtr new_scene = framework_->CreateScene(newName);
        if (new_scene)
            *new_scene = *this;

        return new_scene;
    }

    Scene::EntityPtr SceneManager::CreateEntity(entity_id_t id, const StringVector &components)
    {
        // Figure out new entity id
        entity_id_t newentityid = 0;
        if(id == 0)
            newentityid = GetNextFreeId();
        else
        {
            if(entities_.find(id) != entities_.end())
            {
                Foundation::RootLogError("Can't create entity with given id because it's already used: " + ToString(id));
                return Scene::EntityPtr();
            }
            else
                newentityid = id;
        }

        Scene::EntityPtr entity = Scene::EntityPtr(new Scene::Entity(framework_, newentityid));
        for (size_t i=0 ; i<components.size() ; ++i)
        {
            entity->AddComponent(framework_->GetComponentManager()->CreateComponent(components[i]));
        }
        
        entities_[entity->GetId()] = entity;
        
        // Send event.
        Events::SceneEventData event_data(entity->GetId());
        event_category_id_t cat_id = framework_->GetEventManager()->QueryEventCategory("Scene");
        framework_->GetEventManager()->SendEvent(cat_id, Events::EVENT_ENTITY_ADDED, &event_data);
        
        return entity;
    }

    Scene::EntityPtr SceneManager::GetEntity(entity_id_t id) const
    {
        EntityMap::const_iterator it = entities_.find(id);
        if (it != entities_.end())
            return it->second;

        return Scene::EntityPtr();
    }

    entity_id_t SceneManager::GetNextFreeId()
    {
        while(entities_.find(gid_) != entities_.end())
            gid_ = (gid_ + 1) % static_cast<uint>(-1);
        
        return gid_;
    }

    void SceneManager::RemoveEntity(entity_id_t id)
    {
        EntityMap::iterator it = entities_.find(id);
        if (it != entities_.end())    
        {
            Scene::EntityPtr del_entity = it->second;    
            
            // Send event.         
            Events::SceneEventData event_data(id);
            event_category_id_t cat_id = framework_->GetEventManager()->QueryEventCategory("Scene");
            framework_->GetEventManager()->SendEvent(cat_id, Events::EVENT_ENTITY_DELETED, &event_data);

            entities_.erase(it); 
            del_entity.reset();
        }
    }
}

