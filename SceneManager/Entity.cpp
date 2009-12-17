// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Entity.h"
#include "Framework.h"
#include "ComponentInterface.h"
#include "SceneManager.h"

namespace Scene
{
    Entity::Entity(uint id) : id_(id)
    {
    }

    Entity::~Entity()
    {
    }

    Scene::EntityPtr Entity::Clone(const ScenePtr &scene) const
    {
        assert (scene);

        EntityPtr new_entity = scene->CreateEntity();
        *new_entity.get() = *this;
        return new_entity;
    }

    void Entity::SetNewId(entity_id_t id)
    {
        id_ = id;
    }

    void Entity::AddEntityComponent(const Foundation::ComponentInterfacePtr &component)
    {
        if (component)
        {
            components_.push_back(component);
        
            ///\todo Ali: send event
        }
    }

    void Entity::RemoveEntityComponent(const Foundation::ComponentInterfacePtr &component)
    {
        if (component)
        {
            ComponentVector::iterator iter = std::find(components_.begin(), components_.end(), component);
            if (iter != components_.end())
            {
                components_.erase(iter);
                ///\todo Ali: send event
            } else
            {
                Foundation::RootLogWarning("Failed to remove component: " + component->Name() + " from entity: " + ToString(GetId()));
            }
        }
    }

    Foundation::ComponentInterfacePtr Entity::GetComponent(const std::string &name) const
    {
        for (size_t i=0 ; i<components_.size() ; ++i)
            if (components_[i]->Name() == name)
                return components_[i];

        return Foundation::ComponentInterfacePtr();
    }

    const Entity::ComponentVector &Entity::GetComponentVector() const { return components_; }

}
