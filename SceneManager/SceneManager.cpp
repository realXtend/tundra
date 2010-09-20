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

#include <QDomDocument>
#include <QFile>

#include "clb/Network/DataDeserializer.h"
#include "clb/Network/DataSerializer.h"

#include "MemoryLeakCheck.h"

namespace Scene
{
    SceneManager::SceneManager() :
        gid_(1),
        gid_local_(LocalEntity + 1)
    {
    }
    
    SceneManager::SceneManager(const std::string &name, Foundation::Framework *framework) : 
        name_(name),
        framework_(framework),
        gid_(1),
        gid_local_(LocalEntity + 1)
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
    
    Scene::EntityPtr SceneManager::GetEntity(const QString& name) const
    {
        EntityMap::const_iterator it = entities_.begin();
        while (it != entities_.end())
        {
            if (it->second->GetName() == name)
                return it->second;
            ++it;
        }
        
        return Scene::EntityPtr();
    }

    entity_id_t SceneManager::GetNextFreeId()
    {
        while(entities_.find(gid_) != entities_.end())
        {
            gid_ = (gid_ + 1) & (LocalEntity - 1);
            if (!gid_) ++gid_;
        }
        return gid_;
    }

    entity_id_t SceneManager::GetNextFreeIdLocal()
    {
        while(entities_.find(gid_local_) != entities_.end())
        {
            gid_local_ = (gid_local_ + 1) | LocalEntity;
            if (gid_local_ == LocalEntity) ++gid_local_;
        }
        return gid_local_;
    }
    
    void SceneManager::ChangeEntityId(entity_id_t old_id, entity_id_t new_id)
    {
        if (old_id == new_id)
            return;
        
        Scene::EntityPtr old_entity = GetEntity(old_id);
        if (!old_entity)
            return;
        
        if (GetEntity(new_id))
        {
            Foundation::RootLogWarning("Warning: purged entity " + ToString(new_id) + " to make room for a ChangeEntityId request");
            RemoveEntity(new_id, AttributeChange::LocalOnly);
        }
        
        old_entity->SetNewId(old_id);
        entities_.erase(old_id);
        entities_[new_id] = old_entity;
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
    
    EntityList SceneManager::GetEntitiesWithComponent(const QString &type_name)
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
    
    void SceneManager::EmitComponentChanged(Foundation::ComponentInterface* comp, AttributeChange::Type change)
    {
        emit ComponentChanged(comp, change);
    }
    
    void SceneManager::EmitComponentAdded(Scene::Entity* entity, Foundation::ComponentInterface* comp, AttributeChange::Type change)
    {
        emit ComponentAdded(entity, comp, change);
    }
    
    void SceneManager::EmitComponentRemoved(Scene::Entity* entity, Foundation::ComponentInterface* comp, AttributeChange::Type change)
    {
        emit ComponentRemoved(entity, comp, change);
    }

    void SceneManager::EmitAttributeChanged(Foundation::ComponentInterface* comp, AttributeInterface* attribute, AttributeChange::Type change)
    {
        emit AttributeChanged(comp, attribute, change);
    }

  /*void SceneManager::EmitComponentInitialized(Foundation::ComponentInterface* comp)
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

    QVariantList SceneManager::GetEntityIdsWithComponent(const QString &type_name)
    {
        EntityList list = GetEntitiesWithComponent(type_name);
        QVariantList ids;
        EntityList::iterator iter;

        for(iter = list.begin(); iter != list.end(); iter++)
        {
            EntityPtr ent = *iter;
            Entity* e = ent.get();
            entity_id_t id = e->GetId();
            QVariant qv(id);
            ids.append(qv);
        }
        return ids;
    }
    
    bool SceneManager::LoadSceneXML(const std::string& filename, AttributeChange::Type change)
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
                EntityPtr entity = CreateEntity(id);
                if (entity)
                {
                    QDomElement comp_elem = ent_elem.firstChildElement("component");
                    while (!comp_elem.isNull())
                    {
                        QString type_name = comp_elem.attribute("type");
                        QString name = comp_elem.attribute("name");
                        Foundation::ComponentPtr new_comp = entity->GetOrCreateComponent(type_name, name);
                        if (new_comp)
                        {
                            new_comp->DeserializeFrom(comp_elem, change);
                        }
                        comp_elem = comp_elem.nextSiblingElement("component");
                    }
                    EmitEntityCreated(entity, change);
                    // Kind of a "hack", call OnChanged to the components only after all components have been loaded
                    // This allows to resolve component references to the same entity (for example to the Placeable) at this point
                    const Scene::Entity::ComponentVector &components = entity->GetComponentVector();
                    for(uint i = 0; i < components.size(); ++i)
                        components[i]->ComponentChanged(change);
                }
            }
            ent_elem = ent_elem.nextSiblingElement("entity");
        }
        
        return true;
    }
    
    bool SceneManager::SaveSceneXML(const std::string& filename)
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
    
    bool SceneManager::LoadSceneBinary(const std::string& filename, AttributeChange::Type change)
    {
        QDomDocument scene_doc("Scene");
        
        QFile file(filename.c_str());
        if (!file.open(QIODevice::ReadOnly))
            return false;
        
        QByteArray bytes = file.readAll();
        file.close();
        
        if (!bytes.size())
            return false;
        
        try
        {
            // Purge all old entities. Send events for the removal
            RemoveAllEntities(true, change);
            
            DataDeserializer source(bytes.data(), bytes.size());
            
            uint num_entities = source.Read<u32>();
            for (uint i = 0; i < num_entities; ++i)
            {
                EntityPtr entity = CreateEntity(source.Read<u32>());
                if (!entity)
                {
                    std::cout << "Failed to create entity, stopping scene load" << std::endl;
                    return false; // If entity creation fails, stream desync is more than likely so stop right here
                }
                
                uint num_components = source.Read<u32>();
                for (uint i = 0; i < num_components; ++i)
                {
                    QString type_name = QString::fromStdString(source.ReadString());
                    QString name = QString::fromStdString(source.ReadString());
                    bool sync = source.Read<u8>() ? true : false;
                    uint data_size = source.Read<u32>();
                    
                    // Read the component data into a separate byte array, then deserialize from there.
                    // This way the whole stream should not desync even if something goes wrong
                    QByteArray comp_bytes;
                    comp_bytes.resize(data_size);
                    source.ReadArray<u8>((u8*)comp_bytes.data(), comp_bytes.size());
                    DataDeserializer comp_source(comp_bytes.data(), comp_bytes.size());
                    
                    try
                    {
                        Foundation::ComponentPtr new_comp = entity->GetOrCreateComponent(type_name, name);
                        if (new_comp)
                        {
                            new_comp->SetNetworkSyncEnabled(sync);
                            new_comp->DeserializeFromBinary(comp_source, change);
                        }
                        else
                            std::cout << "Failed to load component " << type_name.toStdString() << std::endl;
                    }
                    catch (...)
                    {
                        std::cout << "Failed to load component " << type_name.toStdString() << std::endl;
                    }
                }
                EmitEntityCreated(entity, change);
                // Kind of a "hack", call OnChanged to the components only after all components have been loaded
                // This allows to resolve component references to the same entity (for example to the Placeable) at this point
                const Scene::Entity::ComponentVector &components = entity->GetComponentVector();
                for(uint i = 0; i < components.size(); ++i)
                    components[i]->ComponentChanged(change);
            }
        }
        catch (...)
        {
            return false;
        }
        
        return true;
    }
    
    bool SceneManager::SaveSceneBinary(const std::string& filename)
    {
        QByteArray bytes;
        // Assume 4MB max for now
        bytes.resize(4 * 1024 * 1024);
        DataSerializer dest(bytes.data(), bytes.size());
        
        dest.Add<u32>(entities_.size());
        
        EntityMap::iterator it = entities_.begin();
        while(it != entities_.end())
        {
            Scene::Entity *entity = it->second.get();
            if (entity)
            {
                dest.Add<u32>(entity->GetId());
                const Scene::Entity::ComponentVector &components = entity->GetComponentVector();
                uint num_serializable = 0;
                for(uint i = 0; i < components.size(); ++i)
                    if (components[i]->IsSerializable())
                        num_serializable++;
                dest.Add<u32>(num_serializable);
                for(uint i = 0; i < components.size(); ++i)
                {
                    if (components[i]->IsSerializable())
                    {
                        dest.AddString(components[i]->TypeName().toStdString());
                        dest.AddString(components[i]->Name().toStdString());
                        dest.Add<u8>(components[i]->GetNetworkSyncEnabled() ? 1 : 0);
                        
                        // Write each component to a separate buffer, then write out its size first, so we can skip unknown components
                        QByteArray comp_bytes;
                        // Assume 64KB max per component for now
                        comp_bytes.resize(64 * 1024);
                        DataSerializer comp_dest(comp_bytes.data(), comp_bytes.size());
                        components[i]->SerializeToBinary(comp_dest);
                        comp_bytes.resize(comp_dest.BytesFilled());
                        
                        dest.Add<u32>(comp_bytes.size());
                        dest.AddArray<u8>((const u8*)comp_bytes.data(), comp_bytes.size());
                    }
                }
            }
            ++it;
        }
        
        bytes.resize(dest.BytesFilled());
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

