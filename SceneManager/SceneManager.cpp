// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "SceneManager.h"
#include "Entity.h"
#include "SceneEvents.h"
#include "Framework.h"
#include "ComponentManager.h"
#include "EventManager.h"
#include "ComponentInterface.h"
#include "ForwardDefines.h"

#include "MemoryLeakCheck.h"

namespace Scene
{
    uint SceneManager::gid_ = 0;

    SceneManager::~SceneManager()
    {
        EntityMap::iterator it = entities_.begin();
        while (it != entities_.end())
        {
            // If entity somehow manages to live, at least it doesn't belong to the scene anymore
            it->second->SetScene(0);
            ++it;
        }
        entities_.clear();
    }
    
    Scene::EntityPtr SceneManager::CreateEntity(entity_id_t id, const StringVector &components, Foundation::ChangeType change)
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

        Scene::EntityPtr entity = Scene::EntityPtr(new Scene::Entity(framework_, newentityid, this));
        for (size_t i=0 ; i<components.size() ; ++i)
            entity->AddComponent(framework_->GetComponentManager()->CreateComponent(components[i]));

        entities_[entity->GetId()] = entity;

        EmitEntityCreated(entity.get(), change);
        
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

    void SceneManager::RemoveEntity(entity_id_t id, Foundation::ChangeType change)
    {
        EntityMap::iterator it = entities_.find(id);
        if (it != entities_.end())
        {
            Scene::EntityPtr del_entity = it->second;

            EmitEntityRemoved(del_entity.get(), change);
        
            // Send event.
            Events::SceneEventData event_data(id);
            event_category_id_t cat_id = framework_->GetEventManager()->QueryEventCategory("Scene");
            framework_->GetEventManager()->SendEvent(cat_id, Events::EVENT_ENTITY_DELETED, &event_data);

            entities_.erase(it);
            // If entity somehow manages to live, at least it doesn't belong to the scene anymore
            del_entity->SetScene(0);
            del_entity.reset();
        }
    }

    EntityList SceneManager::GetEntitiesWithComponent(const std::string &type_name)
    {
        std::list<EntityPtr> entities;
        EntityMap::const_iterator it = entities_.begin();
        while(it != entities_.end())
        {
            EntityPtr entity = it->second;
            if (entity->HasComponent(type_name))
                entities.push_back(entity);
            ++it;
        }

        return entities;
    }
    
    void SceneManager::EmitComponentChanged(Foundation::ComponentInterface* comp, Foundation::ChangeType change)
    {
        emit ComponentChanged(comp, change);
    }
    
    void SceneManager::EmitComponentAdded(Scene::Entity* entity, Foundation::ComponentInterface* comp, Foundation::ChangeType change)
    {
        emit ComponentAdded(entity, comp, change);
    }
    
    void SceneManager::EmitComponentRemoved(Scene::Entity* entity, Foundation::ComponentInterface* comp, Foundation::ChangeType change)
    {
        emit ComponentRemoved(entity, comp, change);
    }
    
    void SceneManager::EmitEntityCreated(Scene::Entity* entity, Foundation::ChangeType change)
    {
        emit EntityCreated(entity, change);
    }
    
    void SceneManager::EmitEntityRemoved(Scene::Entity* entity, Foundation::ChangeType change)
    {
        emit EntityRemoved(entity, change);
    }
}

