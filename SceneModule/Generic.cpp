// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Generic.h"
#include "Entity.h"
#include "SceneModule.h"
#include "SceneEvents.h"

namespace Scene
{
    Core::uint Generic::gid_ = 0;

    Foundation::ScenePtr Generic::Clone(const std::string &newName) const
    {
        return module_->GetSceneManager()->CloneScene(Name(), newName);
    }

    Foundation::EntityPtr Generic::CreateEntity(Core::entity_id_t id, const Core::StringVector &components)
    {
        Foundation::Framework *framework = module_->GetFramework();

        // Figure out new entity id
        Core::entity_id_t newentityid = 0;
        if(id == 0)
            newentityid = GetNextFreeId();
        else
        {
            if(entities_.find(id) != entities_.end())
            {
                SceneModule::LogError("Can't create entity with given id because it's already used:" + Core::ToString(id));
                Foundation::EntityPtr emptyptr;
                return emptyptr;
            }
            else
                newentityid = id;
        }

        Foundation::EntityPtr entity = Foundation::EntityPtr(new Scene::Entity(newentityid, module_));
        for (size_t i=0 ; i<components.size() ; ++i)
        {
            entity->AddEntityComponent(framework->GetComponentManager()->CreateComponent(components[i]));
        }
        
        entities_[entity->GetId()] = entity;
        
        // Send event.
        Events::SceneEventData event_data(entity->GetId());
        Core::event_category_id_t cat_id = framework->GetEventManager()->QueryEventCategory("Scene");
        framework->GetEventManager()->SendEvent(cat_id, Events::EVENT_ENTITY_ADDED, &event_data);
        
        return entity;        
    }

    Foundation::EntityPtr Generic::CreateEntity(Core::entity_id_t id)
    {
        Core::StringVector empty;
        return CreateEntity(id,empty);
    }

    Foundation::EntityPtr Generic::CloneEntity(const Foundation::EntityPtr &entity)
    {
        assert (entity.get());
    
        Foundation::EntityPtr new_entity = Foundation::EntityPtr(new Scene::Entity(*checked_static_cast<Entity*> (entity.get())));
        checked_static_cast<Scene::Entity*>(new_entity.get())->SetNewId(GetNextFreeId());
        entities_[new_entity->GetId()] = new_entity;

        return new_entity;
    }
    
    Foundation::EntityPtr Generic::GetEntity(Core::entity_id_t id) const
    {
        EntityMap::const_iterator it = entities_.find(id);
        if (it != entities_.end())
            return it->second;

        return Foundation::EntityPtr();
    }

    Core::entity_id_t Generic::GetNextFreeId()
    {
        while(entities_.find(gid_) != entities_.end())
            gid_ = (gid_ + 1) % static_cast<Core::uint>(-1);
        
        return gid_;
    }

    void Generic::DestroyEntity(Core::entity_id_t id)
    {
        EntityMap::const_iterator it = entities_.find(id);
        if (it != entities_.end())    
        {
            Foundation::EntityPtr del_entity = it->second;    
            
            // Send event.
            Foundation::Framework *framework = module_->GetFramework();           
            Events::SceneEventData event_data(id);
            Core::event_category_id_t cat_id = framework->GetEventManager()->QueryEventCategory("Scene");
            framework->GetEventManager()->SendEvent(cat_id, Events::EVENT_ENTITY_DELETED, &event_data);

            entities_.erase(it); 
            del_entity.reset();
        }
    }
}

