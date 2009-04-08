// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Entity.h"
#include "Framework.h"
#include "SceneModule.h"
#include "ComponentInterface.h"

namespace Scene
{
    Entity::Entity(Core::uint id, SceneModule *module) : EntityInterface(), module_(module), id_(id)
    {
        assert (module_);
    }

    Entity::~Entity()
    {
    }

    Foundation::EntityPtr Entity::Clone(const std::string &scene_name) const
    {
        Foundation::SceneManagerPtr manager = module_->GetSceneManager();
        assert (manager->HasScene(scene_name));

        Foundation::ScenePtr scene = manager->GetScene(scene_name);
        Foundation::EntityPtr entity = scene->GetEntity(GetId());
        Foundation::EntityPtr new_entity = scene->CloneEntity(entity);

        return new_entity;
    }

    void Entity::SetNewId(Core::entity_id_t id)
    {
        id_ = id;
    }

    void Entity::AddEntityComponent(const Foundation::ComponentInterfacePtr &component)
    {
        components_.push_back(component);
    }

    void Entity::RemoveEntityComponent(const Foundation::ComponentInterfacePtr &component)
    {
        ComponentVector::iterator iter = std::find(components_.begin(), components_.end(), component);
        components_.erase(iter);
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
