// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Entity.h"
#include "SceneManager.h"
#include "EC_Name.h"

#include "Framework.h"
#include "IComponent.h"
#include "CoreStringUtils.h"
#include "ComponentManager.h"
#include "LoggingFunctions.h"

DEFINE_POCO_LOGGING_FUNCTIONS("Entity")

#include "MemoryLeakCheck.h"

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
        qDeleteAll(actions_);
    }

    void Entity::AddComponent(const ComponentPtr &component, AttributeChange::Type change)
    {
        // Must exist and be free
        if (component && component->GetParentEntity() == 0)
        {
            QString componentTypeName = component->TypeName();
            componentTypeName.replace(0, 3, "");
            componentTypeName = componentTypeName.toLower();
            if(!property(componentTypeName.toStdString().c_str()).isValid())
            {
                QVariant var = QVariant::fromValue<QObject*>(component.get());
                setProperty(componentTypeName.toStdString().c_str(), var);
            }

            component->SetParentEntity(this);
            components_.push_back(component);

            if (scene_)
                scene_->EmitComponentAdded(this, component.get(), change);
        }
    }

    void Entity::RemoveComponent(const ComponentPtr &component, AttributeChange::Type change)
    {
        if (component)
        {
            ComponentVector::iterator iter = std::find(components_.begin(), components_.end(), component);
            if (iter != components_.end())
            {
                QString componentTypeName = component->TypeName();
                componentTypeName.replace(0, 3, "");
                componentTypeName = componentTypeName.toLower();
                if(property(componentTypeName.toStdString().c_str()).isValid())
                {
                    QObject *obj = property(componentTypeName.toStdString().c_str()).value<QObject*>();
                    //Make sure that QObject is inherited by the IComponent.
                    if (obj && dynamic_cast<IComponent*>(obj))
                    {
                        //Make sure that name is matching incase there are many of same type of components in entity.
                        if (dynamic_cast<IComponent*>(obj)->Name() == component->Name())
                            setProperty(componentTypeName.toStdString().c_str(), QVariant());
                    }
                }

                if (scene_)
                    scene_->EmitComponentRemoved(this, (*iter).get(), change);

                (*iter)->SetParentEntity(0);
                components_.erase(iter);
            }
            else
            {
                LogWarning("Failed to remove component: " + component->TypeName().toStdString() + " from entity: " + ToString(GetId()));
            }
        }
    }

    void Entity::RemoveComponentRaw(IComponent* comp)
    {
        ComponentPtr ptr = GetComponent(comp->TypeName(), comp->Name()); //the shared_ptr to this component
        RemoveComponent(ptr);
    }

    ComponentPtr Entity::GetOrCreateComponent(const QString &type_name, AttributeChange::Type change)
    {
        for (size_t i=0 ; i<components_.size() ; ++i)
            if (components_[i]->TypeName() == type_name)
                return components_[i];

        // If component was not found, try to create
        ComponentPtr new_comp = framework_->GetComponentManager()->CreateComponent(type_name);
        if (new_comp)
        {
            AddComponent(new_comp, change);
            return new_comp;
        }

        // Could not be created
        return ComponentPtr();
    }

    ComponentPtr Entity::GetOrCreateComponent(const QString &type_name, const QString &name, AttributeChange::Type change)
    {
        for (size_t i=0 ; i<components_.size() ; ++i)
            if (components_[i]->TypeName() == type_name && components_[i]->Name() == name)
                return components_[i];

        // If component was not found, try to create
        ComponentPtr new_comp = framework_->GetComponentManager()->CreateComponent(type_name, name);
        if (new_comp)
        {
            AddComponent(new_comp, change);
            return new_comp;
        }

        // Could not be created
        return ComponentPtr();
    }

    ComponentPtr Entity::GetOrCreateComponent(uint type_hash, AttributeChange::Type change)
    {
        for (size_t i=0 ; i<components_.size() ; ++i)
            if (components_[i]->TypeNameHash() == type_hash)
                return components_[i];

        // If component was not found, try to create
        ComponentPtr new_comp = framework_->GetComponentManager()->CreateComponent(type_hash);
        if (new_comp)
        {
            AddComponent(new_comp, change);
            return new_comp;
        }

        // Could not be created
        return ComponentPtr();
    }

    ComponentPtr Entity::GetOrCreateComponent(uint type_hash, const QString &name, AttributeChange::Type change)
    {
        for (size_t i=0 ; i<components_.size() ; ++i)
            if (components_[i]->TypeNameHash() == type_hash && components_[i]->Name() == name)
                return components_[i];

        // If component was not found, try to create
        ComponentPtr new_comp = framework_->GetComponentManager()->CreateComponent(type_hash, name);
        if (new_comp)
        {
            AddComponent(new_comp, change);
            return new_comp;
        }

        // Could not be created
        return ComponentPtr();
    }
    
    
    ComponentPtr Entity::GetComponent(const QString &type_name) const
    {
        for (size_t i=0 ; i<components_.size() ; ++i)
            if (components_[i]->TypeName() == type_name)
                return components_[i];

        return ComponentPtr();
    }

    ComponentPtr Entity::GetComponent(uint type_hash) const
    {
        for (size_t i=0 ; i<components_.size() ; ++i)
            if (components_[i]->TypeNameHash() == type_hash)
                return components_[i];

        return ComponentPtr();
    }

    ComponentPtr Entity::GetComponent(const IComponent *component) const
    {
        for (size_t i = 0; i < components_.size(); i++)
            if(component->TypeName() == components_[i]->TypeName() &&
               component->Name() == components_[i]->Name())
               return components_[i];
        return ComponentPtr();
    }

    ComponentPtr Entity::GetComponent(const QString &type_name, const QString& name) const
    {
        for (size_t i=0 ; i<components_.size() ; ++i)
            if ((components_[i]->TypeName() == type_name) && (components_[i]->Name() == name))
                return components_[i];

        return ComponentPtr();
    }

    ComponentPtr Entity::GetComponent(uint type_hash, const QString& name) const
    {
        for (size_t i=0 ; i<components_.size() ; ++i)
            if ((components_[i]->TypeNameHash() == type_hash) && (components_[i]->Name() == name))
                return components_[i];

        return ComponentPtr();
    }
    
    bool Entity::HasComponent(const QString &type_name) const
    {
        for(size_t i=0 ; i<components_.size() ; ++i)
            if (components_[i]->TypeName() == type_name)
                return true;
        return false;
    }

    bool Entity::HasComponent(uint type_hash) const
    {
        for(size_t i=0 ; i<components_.size() ; ++i)
            if (components_[i]->TypeNameHash() == type_hash)
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

    bool Entity::HasComponent(uint type_hash, const QString& name) const
    {
        for(size_t i=0 ; i<components_.size() ; ++i)
            if ((components_[i]->TypeNameHash() == type_hash) && (components_[i]->Name() == name))
                return true;
        return false;
    }
    
    IAttribute *Entity::GetAttributeInterface(const std::string &name) const
    {
        for(size_t i = 0; i < components_.size() ; ++i)
        {
            IAttribute *attr = components_[i]->GetAttribute(QString(name.c_str()));
            if (attr)
                return attr;
        }
        return 0;
    }

    AttributeVector Entity::GetAttributes(const std::string &name) const
    {
        std::vector<IAttribute *> ret;
        for(size_t i = 0; i < components_.size() ; ++i)
        {
            IAttribute *attr = components_[i]->GetAttribute(QString(name.c_str()));
            if (attr)
                ret.push_back(attr);
        }
        return ret;
    }

    QString Entity::GetName() const
    {
        boost::shared_ptr<EC_Name> name = GetComponent<EC_Name>();
        if (name)
            return name->name.Get();
        else
            return "";
    }

    QString Entity::GetDescription() const
    {
        boost::shared_ptr<EC_Name> name = GetComponent<EC_Name>();
        if (name)
            return name->description.Get();
        else
            return "";
    }

    EntityAction *Entity::Action(const QString &name)
    {
        if (actions_.contains(name))
            return actions_[name];

        EntityAction *action = new EntityAction(name);
        actions_.insert(name, action);
        return action;
    }

    void Entity::ConnectAction(const QString &name, const QObject *receiver, const char *member)
    {
        EntityAction *action = Action(name);
        assert(action);
        connect(action, SIGNAL(Triggered(QString, QString, QString, QStringList)), receiver, member);
    }

    void Entity::Exec(int /*EntityAction::ExecutionType*/ type, const QString &action, const QString &p1, const QString &p2, const QString &p3)
    {
        Exec(type, action, QStringList(QStringList() << p1 << p2 << p3));
    }

    void Entity::Exec(int /*EntityAction::ExecutionType*/ type, const QString &action, const QStringList &params)
    {
        EntityAction *act = Action(action);
        if (!HasReceivers(act))
            return;

        EntityAction::ExecutionType t = (EntityAction::ExecutionType)type;
        if ((t & EntityAction::Local) != 0)
        {
            if (params.size() == 0)
                act->Trigger();
            else if (params.size() == 1)
                act->Trigger(params[0]);
            else if (params.size() == 2)
                act->Trigger(params[0], params[1]);
            else if (params.size() == 3)
                act->Trigger(params[0], params[1], params[2]);
            else if (params.size() >= 4)
                act->Trigger(params[0], params[1], params[2], params.mid(3));
        }

        GetScene()->EmitActionTriggered(this, action, params, t);
    }

    bool Entity::HasReceivers(EntityAction *action)
    {
        int receivers = action->receivers(SIGNAL(Triggered(QString, QString, QString, QStringList)));
        if (receivers == 0)
        {
//            LogInfo("No receivers found for action \"" + action->Name().toStdString() + "\" removing the action.");
            actions_.remove(action->Name());
            SAFE_DELETE(action);
            return false;
        }

        return true;
    }
}
