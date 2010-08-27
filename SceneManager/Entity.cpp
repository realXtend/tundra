// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ComponentInterface.h"
#include "Entity.h"
#include "Framework.h"
#include "SceneManager.h"
#include "CoreStringUtils.h"
#include "ComponentManager.h"
#include "EC_Name.h"

namespace Scene
{
    Entity::Entity(Foundation::Framework* framework, SceneManager* scene) :
        framework_(framework),
        scene_(scene)
    {
    }
    
    Entity::Entity(Foundation::Framework* framework, uint id, SceneManager* scene) :
        framework_(framework),
        id_(id),
        scene_(scene)
    {
    }

    Entity::~Entity()
    {
        // If components still alive, they become free-floating
        for (size_t i=0 ; i<components_.size() ; ++i)
            components_[i]->SetParentEntity(0);
        
        components_.clear();
    }

    void Entity::AddComponent(const Foundation::ComponentInterfacePtr &component, AttributeChange::Type change)
    {
        // Must exist and be free
        if (component && component->GetParentEntity() == 0)
        {
            component->SetParentEntity(this);
            components_.push_back(component);
        
            if (scene_)
                scene_->EmitComponentAdded(this, component.get(), change);
        }
    }

    void Entity::RemoveComponent(const Foundation::ComponentInterfacePtr &component, AttributeChange::Type change)
    {
        if (component)
        {
            ComponentVector::iterator iter = std::find(components_.begin(), components_.end(), component);
            if (iter != components_.end())
            {
                if (scene_)
                    scene_->EmitComponentRemoved(this, (*iter).get(), change);

                (*iter)->SetParentEntity(0);
                components_.erase(iter);
            }
            else
            {
                Foundation::RootLogWarning("Failed to remove component: " + component->TypeName().toStdString() + " from entity: " + ToString(GetId()));
            }
        }
    }

    Foundation::ComponentInterfacePtr Entity::GetOrCreateComponent(const QString &type_name, AttributeChange::Type change)
    {
        for (size_t i=0 ; i<components_.size() ; ++i)
            if (components_[i]->TypeName() == type_name)
                return components_[i];

        // If component was not found, try to create
        Foundation::ComponentInterfacePtr new_comp = framework_->GetComponentManager()->CreateComponent(type_name);
        if (new_comp)
        {
            AddComponent(new_comp, change);
            return new_comp;
        }

        // Could not be created
        return Foundation::ComponentInterfacePtr();
    }

    Foundation::ComponentInterfacePtr Entity::GetOrCreateComponent(const QString &type_name, const QString &name, AttributeChange::Type change)
    {
        for (size_t i=0 ; i<components_.size() ; ++i)
            if (components_[i]->TypeName() == type_name && components_[i]->Name() == name)
                return components_[i];

        // If component was not found, try to create
        Foundation::ComponentInterfacePtr new_comp = framework_->GetComponentManager()->CreateComponent(type_name, name);
        if (new_comp)
        {
            AddComponent(new_comp, change);
            return new_comp;
        }

        // Could not be created
        return Foundation::ComponentInterfacePtr();
    }
    
    Foundation::ComponentInterfacePtr Entity::GetComponent(const QString &type_name) const
    {
        for (size_t i=0 ; i<components_.size() ; ++i)
            if (components_[i]->TypeName() == type_name)
                return components_[i];

        return Foundation::ComponentInterfacePtr();
    }

    Foundation::ComponentInterfacePtr Entity::GetComponent(const Foundation::ComponentInterface *component) const
    {
        for (size_t i = 0; i < components_.size(); i++)
            if(component->TypeName() == components_[i]->TypeName() &&
               component->Name() == components_[i]->Name())
               return components_[i];
        return Foundation::ComponentInterfacePtr();
    }

    Foundation::ComponentInterfacePtr Entity::GetComponent(const QString &type_name, const QString& name) const
    {
        for (size_t i=0 ; i<components_.size() ; ++i)
            if ((components_[i]->TypeName() == type_name) && (components_[i]->Name() == name))
                return components_[i];

        return Foundation::ComponentInterfacePtr();
    }

    bool Entity::HasComponent(const QString &type_name) const
    {
        for(size_t i=0 ; i<components_.size() ; ++i)
            if (components_[i]->TypeName() == type_name)
                return true;
        return false;
    }

    EntityPtr Entity::GetSharedPtr() const
    {
        EntityPtr ptr;
        if(scene_)
            ptr = scene_->GetEntity(GetId());
        return ptr;
    };

    bool Entity::HasComponent(const QString &type_name, const QString& name) const
    {
        for(size_t i=0 ; i<components_.size() ; ++i)
            if ((components_[i]->TypeName() == type_name) && (components_[i]->Name() == name))
                return true;
        return false;
    }

    std::string Entity::GetName() const
    {
        boost::shared_ptr<EC_Name> name = GetComponent<EC_Name>();
        if (name)
            return name->name.Get();
        else
            return "";
    }

    std::string Entity::GetDescription() const
    {
        boost::shared_ptr<EC_Name> name = GetComponent<EC_Name>();
        if (name)
            return name->description.Get();
        else
            return "";
    }
}
