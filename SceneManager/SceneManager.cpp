// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "SceneManager.h"
#include "Entity.h"
#include "SceneEvents.h"

namespace Scene
{
    Core::uint SceneManager::gid_ = 0;

    Scene::ScenePtr SceneManager::Clone(const std::string &newName) const
    {
        ScenePtr new_scene = framework_->CreateScene(newName);
        if (new_scene)
        {
		*new_scene = *this;
        }
        return new_scene;
    }

    Scene::EntityPtr SceneManager::CreateEntity(Core::entity_id_t id, const Core::StringVector &components)
    {
        // Figure out new entity id
        Core::entity_id_t newentityid = 0;
        if(id == 0)
            newentityid = GetNextFreeId();
        else
        {
            if(entities_.find(id) != entities_.end())
            {
                Foundation::RootLogError("Can't create entity with given id because it's already used: " + Core::ToString(id));
                return Scene::EntityPtr();
            }
            else
                newentityid = id;
        }

        Scene::EntityPtr entity = Scene::EntityPtr(new Scene::Entity(newentityid));
        for (size_t i=0 ; i<components.size() ; ++i)
        {
            entity->AddEntityComponent(framework_->GetComponentManager()->CreateComponent(components[i]));
        }
        
        entities_[entity->GetId()] = entity;
        
        // Send event.
        Events::SceneEventData event_data(entity->GetId());
        Core::event_category_id_t cat_id = framework_->GetEventManager()->QueryEventCategory("Scene");
        framework_->GetEventManager()->SendEvent(cat_id, Events::EVENT_ENTITY_ADDED, &event_data);
        
        return entity;        
    }

    Scene::EntityPtr SceneManager::CloneEntity(const Scene::EntityPtr &entity)
    {
        assert (entity.get());
    
        Scene::EntityPtr new_entity = Scene::EntityPtr(new Scene::Entity(*checked_static_cast<Entity*> (entity.get())));
        checked_static_cast<Scene::Entity*>(new_entity.get())->SetNewId(GetNextFreeId());
        entities_[new_entity->GetId()] = new_entity;

        return new_entity;
    }
    
    Scene::EntityPtr SceneManager::GetEntity(Core::entity_id_t id) const
    {
        EntityMap::const_iterator it = entities_.find(id);
        if (it != entities_.end())
            return it->second;

        return Scene::EntityPtr();
    }

    Core::entity_id_t SceneManager::GetNextFreeId()
    {
        while(entities_.find(gid_) != entities_.end())
            gid_ = (gid_ + 1) % static_cast<Core::uint>(-1);
        
        return gid_;
    }

    void SceneManager::RemoveEntity(Core::entity_id_t id)
    {
        EntityMap::iterator it = entities_.find(id);
        if (it != entities_.end())    
        {
            Scene::EntityPtr del_entity = it->second;    
            
            // Send event.         
            Events::SceneEventData event_data(id);
            Core::event_category_id_t cat_id = framework_->GetEventManager()->QueryEventCategory("Scene");
            framework_->GetEventManager()->SendEvent(cat_id, Events::EVENT_ENTITY_DELETED, &event_data);

            entities_.erase(it); 
            del_entity.reset();
        }
    }
}

