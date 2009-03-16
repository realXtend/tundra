// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Entity.h"
#include "Framework.h"
#include "SceneModule.h"


namespace Scene
{
    Core::uint Entity::gid_ = 0;

    Entity::Entity(SceneModule *module) : EntityInterface(), module_(module), id_(gid_)
    {
        assert (module_);
        gid_ = (gid_ + 1) % static_cast<Core::uint>(-1);
    }

    Entity::~Entity()
    {
    }

    Foundation::EntityPtr Entity::Clone(const std::string &scene) const
    {
        Foundation::SceneManagerPtr manager = module_->GetSceneManager();
        assert (manager->HasScene(scene));

        const Foundation::EntityPtr thisEntity(const_cast<Entity*> (this));
        Foundation::EntityPtr new_entity = manager->GetScene(scene)->CloneEntity(thisEntity);

        return new_entity;
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
            if (components_[i]->_Name() == name)
                return components_[i];

        return Foundation::ComponentInterfacePtr();
    }
}
