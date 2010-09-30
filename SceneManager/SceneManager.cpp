// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "SceneManager.h"
#include "Entity.h"
#include "SceneEvents.h"
#include "Framework.h"
#include "ComponentManager.h"
#include "EventManager.h"
#include "IComponent.h"
#include "ForwardDefines.h"
#include "EC_Name.h"

#include <QString>
#include <QDomDocument>
#include <QFile>

#include "MemoryLeakCheck.h"

namespace Scene
{
    uint SceneManager::gid_ = 0;

    SceneManager::SceneManager() : framework_(0)
    {
    }

    SceneManager::SceneManager(const QString &name, Foundation::Framework *framework) :
        name_(name),
        framework_(framework)
    {
    }

    SceneManager::~SceneManager()
    {
        RemoveAllEntities(false);
    }
    
    Scene::EntityPtr SceneManager::CreateEntity(entity_id_t id, const QStringList &components)
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
            entity->AddComponent(framework_->GetComponentManager()->CreateComponent(components[i])); //change the param to a qstringlist or so \todo XXX

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

    Scene::EntityPtr SceneManager::GetEntityByName(const QString& name) const
    {
        EntityMap::const_iterator it = entities_.begin();
        while(it != entities_.end())
        {
            EntityPtr entity = it->second;
            if (entity->HasComponent(EC_Name::TypeNameStatic()))
                if (entity->GetComponent<EC_Name>()->name.Get() == name)
                    return entity;
            ++it;
        }

        return Scene::EntityPtr();
    }

    entity_id_t SceneManager::GetNextFreeId()
    {
        while(entities_.find(gid_) != entities_.end())
            gid_ = (gid_ + 1) % static_cast<uint>(-1);

        return gid_;
    }

    void SceneManager::RemoveEntity(entity_id_t id, AttributeChange::Type change)
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

    void SceneManager::RemoveAllEntities(bool send_events, AttributeChange::Type change)
    {
        event_category_id_t cat_id = framework_->GetEventManager()->QueryEventCategory("Scene");
        EntityMap::iterator it = entities_.begin();
        while (it != entities_.end())
        {
            // If entity somehow manages to live, at least it doesn't belong to the scene anymore
            if (send_events)
            {
                EmitEntityRemoved(it->second.get(), change);
                
                // Send event.
                Events::SceneEventData event_data(it->second->GetId());
                framework_->GetEventManager()->SendEvent(cat_id, Events::EVENT_ENTITY_DELETED, &event_data);
            }
            it->second->SetScene(0);
            ++it;
        }
        entities_.clear();
    }
    
    EntityList SceneManager::GetEntitiesWithComponent(const QString &type_name) const
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
    
    void SceneManager::EmitComponentChanged(IComponent* comp, AttributeChange::Type change)
    {
        emit ComponentChanged(comp, change);
    }
    
    void SceneManager::EmitComponentAdded(Scene::Entity* entity, IComponent* comp, AttributeChange::Type change)
    {
        emit ComponentAdded(entity, comp, change);
    }
    
    void SceneManager::EmitComponentRemoved(Scene::Entity* entity, IComponent* comp, AttributeChange::Type change)
    {
        emit ComponentRemoved(entity, comp, change);
    }

    void SceneManager::EmitAttributeChanged(IComponent* comp, IAttribute* attribute, AttributeChange::Type change)
    {
        emit AttributeChanged(comp, attribute, change);
    }

  /*void SceneManager::EmitComponentInitialized(IComponent* comp)
    {
        emit ComponentInitialized(comp);
        }*/
 
    void SceneManager::EmitEntityCreated(Scene::Entity* entity, AttributeChange::Type change)
    {
        emit EntityCreated(entity, change);
    }

    void SceneManager::EmitEntityCreated(Scene::EntityPtr entity, AttributeChange::Type change)
    {
        emit EntityCreated(entity.get(), change);
    }
    
    void SceneManager::EmitEntityRemoved(Scene::Entity* entity, AttributeChange::Type change)
    {
        emit EntityRemoved(entity, change);
    }

    void SceneManager::EmitActionTriggered(Scene::Entity *entity, const QString &action, const QStringList &params, EntityAction::ExecutionType type)
    {
        emit ActionTriggered(entity, action, params, type);
    }

    QVariantList SceneManager::GetEntityIdsWithComponent(const QString &type_name) const
    {
        QVariantList ret;

        EntityList entities = GetEntitiesWithComponent(type_name);
        foreach(EntityPtr e, entities)
            ret.append(QVariant(e->GetId()));

        return ret;
    }

    QList<Scene::Entity*> SceneManager::GetEntitiesWithComponentRaw(const QString &type_name) const
    {
        QList<Scene::Entity*> ret;

        EntityList entities = GetEntitiesWithComponent(type_name);
        foreach(EntityPtr e, entities)
            ret.append(e.get());

        return ret;
    }
    
    bool SceneManager::LoadScene(const std::string& filename, AttributeChange::Type change)
    {
        QDomDocument scene_doc("Scene");
        
        QFile file(filename.c_str());
        if (!file.open(QIODevice::ReadOnly))
            return false;
        
        if (!scene_doc.setContent(&file))
        {
            file.close();
            return false;
        }
        
        // Check for existence of the scene element before we begin
        QDomElement scene_elem = scene_doc.firstChildElement("scene");
        if (scene_elem.isNull())
            return false;
        
        // Purge all old entities. Send events for the removal
        RemoveAllEntities(true, change);
        
        QDomElement ent_elem = scene_elem.firstChildElement("entity");
        while (!ent_elem.isNull())
        {
            QString id_str = ent_elem.attribute("id");
            if (!id_str.isEmpty())
            {
                entity_id_t id = ParseString<entity_id_t>(id_str.toStdString());
                EntityPtr entity = CreateEntity(id, QStringList());
                QDomElement comp_elem = ent_elem.firstChildElement("component");
                while (!comp_elem.isNull())
                {
                    QString type_name = comp_elem.attribute("type");
                    QString name = comp_elem.attribute("name");
                    ComponentPtr new_comp = entity->GetOrCreateComponent(type_name, name);
                    if (new_comp)
                        new_comp->DeserializeFrom(comp_elem, change);

                    comp_elem = comp_elem.nextSiblingElement("component");
                }
                EmitEntityCreated(entity, change);
                // Kind of a "hack", call OnChanged to the components only after all components have been loaded
                // This allows to resolve component references to the same entity (for example to the Placeable) at this point
                const Scene::Entity::ComponentVector &components = entity->GetComponentVector();
                for(uint i = 0; i < components.size(); ++i)
                    components[i]->ComponentChanged(change);
            }
            ent_elem = ent_elem.nextSiblingElement("entity");
        }
        
        return true;
    }
    
    bool SceneManager::SaveScene(const std::string& filename)
    {
        QDomDocument scene_doc("Scene");
        QDomElement scene_elem = scene_doc.createElement("scene");
        EntityMap::iterator it = entities_.begin();
        
        while(it != entities_.end())
        {
            Scene::Entity *entity = it->second.get();
            if (entity)
            {
                QDomElement entity_elem = scene_doc.createElement("entity");
                
                QString id_str;
                id_str.setNum((int)entity->GetId());
                entity_elem.setAttribute("id", id_str);

                const Scene::Entity::ComponentVector &components = entity->GetComponentVector();
                for(uint i = 0; i < components.size(); ++i)
                    if (components[i]->IsSerializable())
                        components[i]->SerializeTo(scene_doc, entity_elem);

                scene_elem.appendChild(entity_elem);
            }
            ++it;
        }
        
        scene_doc.appendChild(scene_elem);
        
        QByteArray bytes = scene_doc.toByteArray();
        QFile scenefile(filename.c_str());
        if (scenefile.open(QFile::WriteOnly))
        {
            scenefile.write(bytes);
            scenefile.close();
            return true;
        }
        else return false;
    }
}

