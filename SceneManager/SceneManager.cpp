// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "SceneManager.h"
#include "Entity.h"
#include "SceneEvents.h"
#include "SceneDesc.h"
#include "IComponent.h"
#include "IAttribute.h"
#include "EC_Name.h"

#include "Framework.h"
#include "ComponentManager.h"
#include "EventManager.h"
#include "AssetAPI.h"
#include "Profiler.h"
#include "LoggingFunctions.h"

DEFINE_POCO_LOGGING_FUNCTIONS("SceneManager")

#include <QString>
#include <QDomDocument>
#include <QFile>
#include <QDir>
#include <QTextStream>

#include <kNet/DataDeserializer.h>
#include <kNet/DataSerializer.h>

#include "MemoryLeakCheck.h"

using namespace kNet;

namespace Scene
{
    SceneManager::SceneManager() :
        framework_(0),
        gid_(1),
        gid_local_(LocalEntity + 1),
        viewenabled_(true),
        interpolating_(false)
    {
    }
    
    SceneManager::SceneManager(const QString &name, Foundation::Framework *framework, bool viewenabled) :
        name_(name),
        framework_(framework),
        gid_(1),
        gid_local_(LocalEntity + 1),
        interpolating_(false)
    {
        // In headless mode only view disabled-scenes can be created
        if (framework->IsHeadless())
            viewenabled_ = false;
        else
            viewenabled_ = viewenabled;
    }

    SceneManager::~SceneManager()
    {
        EndAllAttributeInterpolations();
        
        // Do not send entity removal or scene cleared events on destruction
        RemoveAllEntities(false);
        
        emit Removed(this);
    }

    Scene::EntityPtr SceneManager::CreateEntity(entity_id_t id, const QStringList &components, AttributeChange::Type change, bool defaultNetworkSync)
    {
        // Figure out new entity id
        entity_id_t newentityid = 0;
        if(id == 0)
            newentityid = GetNextFreeId();
        else
        {
            if(entities_.find(id) != entities_.end())
            {
                LogError("Can't create entity with given id because it's already used: " + ToString(id));
                return Scene::EntityPtr();
            }
            else
                newentityid = id;
        }

        Scene::EntityPtr entity = Scene::EntityPtr(new Scene::Entity(framework_, newentityid, this));
        for (size_t i=0 ; i<components.size() ; ++i)
        {
            ComponentPtr newComp = framework_->GetComponentManager()->CreateComponent(components[i]);
            if (newComp)
            {
                if (!defaultNetworkSync)
                    newComp->SetNetworkSyncEnabled(false);
                entity->AddComponent(newComp, change); //change the param to a qstringlist or so \todo XXX
            }
        }
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

    Scene::Entity *SceneManager::GetEntityByNameRaw(const QString &name) const
    {
        return GetEntityByName(name).get();
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
        // Find the largest non-local entity ID in the scene.
        // NOTE: This iteration is of linear complexity. Can optimize here. (But be sure to properly test for correctness!) -jj.
        entity_id_t largestEntityId = 0;
        for(EntityMap::const_iterator iter = entities_.begin(); iter != entities_.end(); ++iter)
            if ((iter->first & LocalEntity) == 0)
                largestEntityId = std::max(largestEntityId, iter->first);

        // Ensure that the entity id we give out is always larger than the largest entity id currently existing in the scene.
        gid_ = std::max(gid_ + 1, largestEntityId+1);

        while(entities_.find(gid_) != entities_.end())
        {
            gid_ = (gid_ + 1) & (LocalEntity - 1);
            if (!gid_) ++gid_;
        }

        assert(!HasEntity(gid_));
        return gid_;
    }

    entity_id_t SceneManager::GetNextFreeIdLocal()
    {
        // Find the largest local entity ID in the scene.
        // NOTE: This iteration is of linear complexity. Can optimize here. (But be sure to properly test for correctness!) -jj.
        entity_id_t largestEntityId = 0;
        for(EntityMap::const_iterator iter = entities_.begin(); iter != entities_.end(); ++iter)
            if ((iter->first & LocalEntity) != 0)
                largestEntityId = std::max(largestEntityId, iter->first);

        // Ensure that the entity id we give out is always larger than the largest entity id currently existing in the scene.
        gid_local_ = std::max((gid_local_ + 1) | LocalEntity, (largestEntityId+1) | LocalEntity);

        while(entities_.find(gid_local_) != entities_.end())
        {
            gid_local_ = (gid_local_ + 1) | LocalEntity;
            if (gid_local_ == LocalEntity) ++gid_local_;
        }

        assert(!HasEntity(gid_local_));
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
            LogWarning("Warning: purged entity " + ToString(new_id) + " to make room for a ChangeEntityId request");
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
        if (send_events)
            emit SceneCleared(this);
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
    
    void SceneManager::EmitComponentAdded(Scene::Entity* entity, IComponent* comp, AttributeChange::Type change)
    {
        if (change == AttributeChange::Disconnected)
            return;
        if (change == AttributeChange::Default)
            change = comp->GetUpdateMode();
        emit ComponentAdded(entity, comp, change);
    }
    
    void SceneManager::EmitComponentRemoved(Scene::Entity* entity, IComponent* comp, AttributeChange::Type change)
    {
        if (change == AttributeChange::Disconnected)
            return;
        if (change == AttributeChange::Default)
            change = comp->GetUpdateMode();
        emit ComponentRemoved(entity, comp, change);
    }

    void SceneManager::EmitAttributeChanged(IComponent* comp, IAttribute* attribute, AttributeChange::Type change)
    {
        if ((!comp) || (!attribute) || (change == AttributeChange::Disconnected))
            return;
        if (change == AttributeChange::Default)
            change = comp->GetUpdateMode();
        emit AttributeChanged(comp, attribute, change);
    }

    void SceneManager::EmitAttributeAdded(IComponent* comp, IAttribute* attribute, AttributeChange::Type change)
    {
        if ((!comp) || (!attribute) || (change == AttributeChange::Disconnected))
            return;
        if (change == AttributeChange::Default)
            change = comp->GetUpdateMode();
        emit AttributeAdded(comp, attribute, change);
    }
    
    void SceneManager::EmitAttributeRemoved(IComponent* comp, IAttribute* attribute, AttributeChange::Type change)
    {
        if ((!comp) || (!attribute) || (change == AttributeChange::Disconnected))
            return;
        if (change == AttributeChange::Default)
            change = comp->GetUpdateMode();
        emit AttributeRemoved(comp, attribute, change);
    }
    
  /*void SceneManager::EmitComponentInitialized(IComponent* comp)
    {
        emit ComponentInitialized(comp);
        }*/
 
    void SceneManager::EmitEntityCreated(Scene::Entity *entity, AttributeChange::Type change)
    {
        if (change == AttributeChange::Disconnected)
            return;
        if (change == AttributeChange::Default)
            change = AttributeChange::Replicate;
        if (entity)
            emit EntityCreated(entity, change);
    }

    void SceneManager::EmitEntityCreated(Scene::EntityPtr entity, AttributeChange::Type change)
    {
        if (change == AttributeChange::Disconnected)
            return;
        if (change == AttributeChange::Default)
            change = AttributeChange::Replicate;
        emit EntityCreated(entity.get(), change);
    }
    
    void SceneManager::EmitEntityRemoved(Scene::Entity* entity, AttributeChange::Type change)
    {
        if (change == AttributeChange::Disconnected)
            return;
        if (change == AttributeChange::Default)
            change = AttributeChange::Replicate;
        emit EntityRemoved(entity, change);
        entity->EmitEntityRemoved(change);
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

    
    QVariantList SceneManager::LoadSceneXMLRaw(const QString &filename, bool clearScene, bool useEntityIDsFromFile, AttributeChange::Type change)
    {
        //copied from LoadSceneXML and edited
        QVariantList ret;
 
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly))
        {
            LogError("Failed to open file " + filename.toStdString() + " when loading scene xml.");
            return ret;
        }

        // Set codec to ISO 8859-1 a.k.a. Latin 1
        QTextStream stream(&file);
        stream.setCodec("ISO 8859-1");
        QDomDocument scene_doc("Scene");
        if (!scene_doc.setContent(stream.readAll()))
        {
            LogError("Parsing scene XML from "+ filename.toStdString() + " failed when loading scene xml.");
            file.close();
            return ret;
        }

        // Purge all old entities. Send events for the removal
        if (clearScene)
            RemoveAllEntities(true, change);

        QList<Scene::Entity *> entities = CreateContentFromXml(scene_doc, useEntityIDsFromFile, change);
        foreach(Entity * e, entities)
            ret.append(QVariant(e->GetId()));

        return ret;
    }

    QList<Entity *> SceneManager::LoadSceneXML(const std::string& filename, bool clearScene, bool useEntityIDsFromFile, AttributeChange::Type change)
    {
        QList<Entity *> ret;

        QFile file(filename.c_str());
        if (!file.open(QIODevice::ReadOnly))
        {
            LogError("Failed to open file " + filename + " when loading scene xml.");
            return ret;
        }

        // Set codec to ISO 8859-1 a.k.a. Latin 1
        QTextStream stream(&file);
        stream.setCodec("ISO 8859-1");
        QDomDocument scene_doc("Scene");
        if (!scene_doc.setContent(stream.readAll()))
        {
            LogError("Parsing scene XML from "+ filename + " failed when loading scene xml.");
            file.close();
            return ret;
        }

        // Purge all old entities. Send events for the removal
        if (clearScene)
            RemoveAllEntities(true, change);

        return CreateContentFromXml(scene_doc, useEntityIDsFromFile, change);
    }

    QByteArray SceneManager::GetSceneXML(bool gettemporary, bool getlocal) const
    {
        QDomDocument scene_doc("Scene");
        QDomElement scene_elem = scene_doc.createElement("scene");

        for(EntityMap::const_iterator iter = entities_.begin(); iter != entities_.end(); ++iter) 
		{
            if (iter->second)
            {
				bool serialize = true;
				if (iter->second->IsLocal() && !getlocal)
						serialize = false;

				if (iter->second->IsTemporary() && !gettemporary)
						serialize = false;

				if (serialize) 
				{
                    /* copied from GetEntityXML so that we can get local and temporary components also.
                    ugly hack! */
                    Scene::EntityPtr entity = iter->second;
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
				scene_doc.appendChild(scene_elem);
			}
		}

        return scene_doc.toByteArray();
	}
    
    bool SceneManager::SaveSceneXML(const std::string& filename)
    {
        QByteArray bytes = GetSceneXML();
        QFile scenefile(filename.c_str());
        if (scenefile.open(QFile::WriteOnly))
        {
            scenefile.write(bytes);
            scenefile.close();
            return true;
        }
        else
        {
            LogError("Failed to open file " + filename + "for writing when saving scene xml.");
            return false;
        }
    }
    
    QList<Entity *> SceneManager::LoadSceneBinary(const std::string& filename, bool clearScene, bool useEntityIDsFromFile, AttributeChange::Type change)
    {
        QList<Entity *> ret;
        QFile file(filename.c_str());
        if (!file.open(QIODevice::ReadOnly))
        {
            LogError("Failed to open file " + filename + " when loading scene binary.");
            return ret;
        }

        ///\todo Use Latin 1?
        QByteArray bytes = file.readAll();
        file.close();

        if (!bytes.size())
        {
            LogError("File " + filename + " contained 0 bytes when loading scene binary.");
            return ret;
        }

        if (clearScene)
            RemoveAllEntities(true, change);

        return CreateContentFromBinary(bytes.data(), bytes.size(), useEntityIDsFromFile, change);
    }

    bool SceneManager::SaveSceneBinary(const std::string& filename)
    {
        QByteArray bytes;
        // Assume 4MB max for now
        bytes.resize(4 * 1024 * 1024);
        DataSerializer dest(bytes.data(), bytes.size());
        
        // Count non-temporary entities
        uint num_entities = 0;
        for(EntityMap::iterator iter = entities_.begin(); iter != entities_.end(); ++iter)
            if ((iter->second) && (!iter->second->IsTemporary()))
                num_entities++;

        dest.Add<u32>(num_entities);

        for(EntityMap::iterator iter = entities_.begin(); iter != entities_.end(); ++iter)
            if ((iter->second) && (!iter->second->IsTemporary()))
                iter->second->SerializeToBinary(dest);

        bytes.resize(dest.BytesFilled());
        QFile scenefile(filename.c_str());
        if (scenefile.open(QFile::WriteOnly))
        {
            scenefile.write(bytes);
            scenefile.close();
            return true;
        }
        else
        {
            LogError("Could not open file " + filename + " for writing when saving scene binary");
            return false;
        }
    }

    QList<Entity *> SceneManager::CreateContentFromXml(const QString &xml,  bool useEntityIDsFromFile, AttributeChange::Type change)
    {
        QList<Entity *> ret;
        QString errorMsg;
        QDomDocument scene_doc("Scene");
        if (!scene_doc.setContent(xml, false, &errorMsg))
        {
            LogError("Parsing scene XML from text failed: " + errorMsg.toStdString());
            return ret;
        }

        return CreateContentFromXml(scene_doc, useEntityIDsFromFile, change);
    }

    QList<Entity *> SceneManager::CreateContentFromXml(const QDomDocument &xml, bool useEntityIDsFromFile, AttributeChange::Type change)
    {
        QList<Entity *> ret;
        // Check for existence of the scene element before we begin
        QDomElement scene_elem = xml.firstChildElement("scene");
        if (scene_elem.isNull())
        {
            LogError("Could not find 'scene' element from XML.");
            return ret;
        }

        QDomElement ent_elem = scene_elem.firstChildElement("entity");
        while (!ent_elem.isNull())
        {
            QString id_str = ent_elem.attribute("id");
            entity_id_t id = !id_str.isEmpty() ? ParseString<entity_id_t>(id_str.toStdString()) : 0;
            if (!useEntityIDsFromFile || id == 0) // If we don't want to use entity IDs from file, or if file doesn't contain one, generate a new one.
                id = ((id & LocalEntity) != 0) ? GetNextFreeIdLocal() : GetNextFreeId();

            if (HasEntity(id)) // If the entity we are about to add conflicts in ID with an existing entity in the scene, delete the old entity.
            {
                LogDebug("SceneManager::CreateContentFromXml: Destroying previous entity with id " + QString::number(id).toStdString() + " to avoid conflict with new created entity with the same id.");
                LogError("Warning: Invoking buggy behavior: Object with id " + QString::number(id).toStdString() + "might not replicate properly!");
                RemoveEntity(id, AttributeChange::Replicate); ///<@todo Consider do we want to always use Replicate
            }

            EntityPtr entity = CreateEntity(id);
            if (entity)
            {
                QDomElement comp_elem = ent_elem.firstChildElement("component");
                while (!comp_elem.isNull())
                {
                    QString type_name = comp_elem.attribute("type");
                    QString name = comp_elem.attribute("name");
                    ComponentPtr new_comp = entity->GetOrCreateComponent(type_name, name);
                    if (new_comp)
                        // Trigger no signal yet when scene is in incoherent state
                        new_comp->DeserializeFrom(comp_elem, AttributeChange::Disconnected);

                    comp_elem = comp_elem.nextSiblingElement("component");
                }
                ret.append(entity.get());
            }
            else
            {
                LogError("SceneManager::CreateContentFromXml: Failed to create entity with id " + QString::number(id).toStdString() + "!");
            }

            ent_elem = ent_elem.nextSiblingElement("entity");
        }

        // Now that we have each entity spawned to the scene, trigger all the signals for EntityCreated/ComponentChanged messages.
        for (int i = 0; i < ret.size(); ++i)
        {
            Entity* entity = ret[i];
            EmitEntityCreated(entity, change);
            // All entities & components have been loaded. Trigger change for them now.
            const Scene::Entity::ComponentVector &components = entity->GetComponentVector();
            for(uint j = 0; j < components.size(); ++j)
                components[j]->ComponentChanged(change);
        }

        return ret;
    }

    QList<Entity *> SceneManager::CreateContentFromBinary(const QString &filename, bool useEntityIDsFromFile, AttributeChange::Type change)
    {
        QList<Entity *> ret;
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly))
        {
            LogError("Failed to open file " + filename.toStdString() + " when loading scene binary.");
            return ret;
        }

        QByteArray bytes = file.readAll();
        file.close();
        
        if (!bytes.size())
        {
            LogError("File " + filename.toStdString() + "contained 0 bytes when loading scene binary.");
            return ret;
        }

        return CreateContentFromBinary(bytes.data(), bytes.size(), useEntityIDsFromFile, change);
    }

    QList<Entity *> SceneManager::CreateContentFromBinary(const char *data, int numBytes, bool useEntityIDsFromFile, AttributeChange::Type change)
    {
        QList<Entity *> ret;
        assert(data);
        assert(numBytes > 0);
        try
        {
            DataDeserializer source(data, numBytes);
            
            uint num_entities = source.Read<u32>();
            for (uint i = 0; i < num_entities; ++i)
            {
                entity_id_t id = source.Read<u32>();
                if (!useEntityIDsFromFile || id == 0)
                    id = ((id & LocalEntity) != 0) ? GetNextFreeIdLocal() : GetNextFreeId();

                if (HasEntity(id)) // If the entity we are about to add conflicts in ID with an existing entity in the scene.
                {
                    LogDebug("SceneManager::CreateContentFromBinary: Destroying previous entity with id " + QString::number(id).toStdString() + " to avoid conflict with new created entity with the same id.");
                    LogError("Warning: Invoking buggy behavior: Object with id " + QString::number(id).toStdString() + "might not replicate properly!");
                    RemoveEntity(id, AttributeChange::Replicate); ///<@todo Consider do we want to always use Replicate
                }

                EntityPtr entity = CreateEntity(id);
                if (!entity)
                {
                    std::cout << "Failed to create entity, stopping scene load" << std::endl;
                    return ret; // If entity creation fails, stream desync is more than likely so stop right here
                }
                
                uint num_components = source.Read<u32>();
                for (uint i = 0; i < num_components; ++i)
                {
                    uint type_hash = source.Read<u32>();
                    QString name = QString::fromStdString(source.ReadString());
                    bool sync = source.Read<u8>() ? true : false;
                    uint data_size = source.Read<u32>();
                    
                    // Read the component data into a separate byte array, then deserialize from there.
                    // This way the whole stream should not desync even if something goes wrong
                    QByteArray comp_bytes;
                    comp_bytes.resize(data_size);
                    if (data_size)
                        source.ReadArray<u8>((u8*)comp_bytes.data(), comp_bytes.size());
                    
                    try
                    {
                        ComponentPtr new_comp = entity->GetOrCreateComponent(type_hash, name);
                        if (new_comp)
                        {
                            new_comp->SetNetworkSyncEnabled(sync);
                            if (data_size)
                            {
                                DataDeserializer comp_source(comp_bytes.data(), comp_bytes.size());
                                // Trigger no signal yet when scene is in incoherent state
                                new_comp->DeserializeFromBinary(comp_source, AttributeChange::Disconnected);
                            }
                        }
                        else
                            LogError("Failed to load component " + framework_->GetComponentManager()->GetComponentTypeName(type_hash).toStdString());
                    }
                    catch (...)
                    {
                        LogError("Failed to load component " + framework_->GetComponentManager()->GetComponentTypeName(type_hash).toStdString());
                    }
                }

                ret.append(entity.get());
            }
        }
        catch (...)
        {
            // Note: if exception happens, no change signals are emitted
            return QList<Entity *>();
        }

        for (uint i = 0; i < ret.size(); ++i)
        {
            Entity* entity = ret[i];
            EmitEntityCreated(entity, change);
            // All entities & components have been loaded. Trigger change for them now.
            foreach(ComponentPtr comp, entity->GetComponentVector())
                comp->ComponentChanged(change);
        }
        
        return ret;
    }

    QList<Entity *> SceneManager::CreateContentFromSceneDescription(const SceneDesc &desc, bool useEntityIDsFromFile, AttributeChange::Type change)
    {
        QList<Entity *> ret;

        if (desc.entities.empty())
        {
            LogError("Empty scene descrition.");
            return ret;
        }

        foreach(EntityDesc e, desc.entities)
        {
            entity_id_t id;
            if (e.id.isEmpty() || !useEntityIDsFromFile)
                id = e.local ? GetNextFreeIdLocal() : GetNextFreeId();
            else
                id = ParseString<entity_id_t>(e.id.toStdString());

            if (HasEntity(id)) // If the entity we are about to add conflicts in ID with an existing entity in the scene.
            {
                LogDebug("SceneManager::CreateContentFromSceneDescription: Destroying previous entity with id " + QString::number(id).toStdString() + " to avoid conflict with new created entity with the same id.");
                LogError("Warning: Invoking buggy behavior: Object with id " + QString::number(id).toStdString() + "might not replicate properly!");
                RemoveEntity(id, AttributeChange::Replicate); ///<@todo Consider do we want to always use Replicate
            }

            EntityPtr entity = CreateEntity(id);
            assert(entity);
            if (entity)
            {
                foreach(ComponentDesc c, e.components)
                {
                    if (c.typeName.isNull())
                        continue;

                    ComponentPtr comp = entity->GetOrCreateComponent(c.typeName, c.name);
                    assert(comp);
                    if (!comp)
                        continue;

                    if (comp->TypeName() == "EC_DynamicComponent")
                    {
                        QDomDocument temp_doc;
                        QDomElement root_elem = temp_doc.createElement("component");
                        root_elem.setAttribute("type", c.typeName);
                        root_elem.setAttribute("name", c.name);
                        root_elem.setAttribute("sync", c.sync);
                        foreach(AttributeDesc a, c.attributes)
                        {
                            QDomElement child_elem = temp_doc.createElement("attribute");
                            child_elem.setAttribute("value", a.value);
                            child_elem.setAttribute("type", a.typeName);
                            child_elem.setAttribute("name", a.name);
                            root_elem.appendChild(child_elem);
                        }
                        comp->DeserializeFrom(root_elem, AttributeChange::Default);
                    }
                    else
                    {
                        foreach(IAttribute *attr, comp->GetAttributes())
                            foreach(AttributeDesc a, c.attributes)
                                if (attr->TypeName().c_str() == a.typeName && attr->GetName() == a.name)
                                    // Trigger no signal yet when scene is in incoherent state
                                    attr->FromString(a.value.toStdString(), AttributeChange::Disconnected);
                    }
                }

                ret.append(entity.get());
            }
        }

        // All entities & components have been loaded. Trigger change for them now.
        foreach(Entity *entity, ret)
        {
            EmitEntityCreated(entity, change);
            foreach(ComponentPtr component, entity->GetComponentVector())
                component->ComponentChanged(change);
        }

        return ret;
    }

    SceneDesc SceneManager::GetSceneDescFromXml(const QString &filename) const
    {
        SceneDesc sceneDesc;
        if (!filename.endsWith(".txml", Qt::CaseInsensitive))
        {
            if (filename.endsWith(".tbin", Qt::CaseInsensitive))
                LogError("Try using GetSceneDescFromBinary() instead for " + filename.toStdString());
            else
                LogError("Unsupported file extension : " + filename.toStdString() + " when trying to create scene description.");
            return sceneDesc;
        }

        sceneDesc.type = SceneDesc::Naali;
        sceneDesc.filename = filename;

        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly))
        {
            LogError("Failed to open file " + filename.toStdString() + " when trying to create scene description.");
            return sceneDesc;
        }

        QByteArray data = file.readAll();
        file.close();

        return GetSceneDescFromXml(data, sceneDesc);
    }

    SceneDesc SceneManager::GetSceneDescFromXml(QByteArray &data, SceneDesc &sceneDesc) const
    {
        // Set codec to ISO 8859-1 a.k.a. Latin 1
        QTextStream stream(&data);
        stream.setCodec("ISO 8859-1");
        QDomDocument scene_doc("Scene");
        if (!scene_doc.setContent(stream.readAll()))
        {
            LogError("Parsing scene XML from " + sceneDesc.filename.toStdString() + " failed when loading scene xml.");
            return sceneDesc;
        }

        // Check for existence of the scene element before we begin
        QDomElement scene_elem = scene_doc.firstChildElement("scene");
        if (scene_elem.isNull())
        {
            LogError("Could not find 'scene' element from XML.");
            return sceneDesc;
        }

        QDomElement ent_elem = scene_elem.firstChildElement("entity");
        while (!ent_elem.isNull())
        {
            QString id_str = ent_elem.attribute("id");
            if (!id_str.isEmpty())
            {
                EntityDesc entityDesc;
                entityDesc.id = id_str;

                QDomElement comp_elem = ent_elem.firstChildElement("component");
                while(!comp_elem.isNull())
                {
                    QString type_name = comp_elem.attribute("type");
                    QString name = comp_elem.attribute("name");
                    QString sync = comp_elem.attribute("sync");
                    ComponentDesc compDesc;
                    compDesc.typeName = type_name;
                    compDesc.name = name;
                    compDesc.sync = sync;

                    // A bit of a hack to get the name from EC_Name.
                    if (entityDesc.name.isEmpty() && type_name == EC_Name::TypeNameStatic())
                    {
                        ComponentPtr comp = framework_->GetComponentManager()->CreateComponent(type_name, name);
                        EC_Name *ecName = checked_static_cast<EC_Name*>(comp.get());
                        ecName->DeserializeFrom(comp_elem, AttributeChange::Disconnected);
                        entityDesc.name = ecName->name.Get();
                    }

                    // Find asset references.
                    ComponentPtr comp = framework_->GetComponentManager()->CreateComponent(type_name, name);
                    comp->DeserializeFrom(comp_elem, AttributeChange::Disconnected);
                    foreach(IAttribute *a,comp->GetAttributes())
                    {
                        QString typeName(a->TypeName().c_str());
                        AttributeDesc attrDesc = { typeName, a->GetNameString().c_str(), a->ToString().c_str() };
                        compDesc.attributes.append(attrDesc);

                        QString attrValue = QString(a->ToString().c_str()).trimmed();
                        if ((typeName == "assetreference" || typeName == "assetreferencelist" || 
                            (a->HasMetadata() && a->GetMetadata()->elementType == "assetreference")) &&
                            !attrValue.isEmpty())
                        {
                            // We might have multiple references, ";" used as a separator.
                            QStringList values = attrValue.split(";");
                            foreach(QString value, values)
                            {
                                AssetDesc ad;
                                ad.typeName = a->GetNameString().c_str();
                                ad.dataInMemory = false;

                                // Rewrite source refs for asset descs, if necessary.
                                QString basePath(boost::filesystem::path(sceneDesc.filename.toStdString()).branch_path().string().c_str());
                                framework_->Asset()->QueryFileLocation(value, basePath, ad.source);
                                ad.destinationName = AssetAPI::ExtractFilenameFromAssetRef(ad.source);

                                sceneDesc.assets[qMakePair(ad.source, ad.subname)] = ad;
                            }
                        }
                    }

                    entityDesc.components.append(compDesc);

                    comp_elem = comp_elem.nextSiblingElement("component");
                }

                sceneDesc.entities.append(entityDesc);
            }

            // Process siblings.
            ent_elem = ent_elem.nextSiblingElement("entity");
        }

        return sceneDesc;
    }

    SceneDesc SceneManager::GetSceneDescFromBinary(const QString &filename) const
    {
        SceneDesc sceneDesc;

        if (!filename.endsWith(".tbin", Qt::CaseInsensitive))
        {
            if (filename.endsWith(".txml", Qt::CaseInsensitive))
                LogError("Try using GetSceneDescFromXml() instead for " + filename.toStdString());
            else
                LogError("Unsupported file extension : " + filename.toStdString() + " when trying to create scene description.");
            return sceneDesc;
        }

        sceneDesc.type = SceneDesc::Naali;
        sceneDesc.filename = filename;

        ///\todo Use Latin 1 encoding?
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly))
        {
            LogError("Failed to open file " + filename.toStdString() + " when trying to create scene description.");
            return sceneDesc;
        }

        QByteArray bytes = file.readAll();
        file.close();

        return GetSceneDescFromBinary(bytes, sceneDesc);
    }

    SceneDesc SceneManager::GetSceneDescFromBinary(QByteArray &data, SceneDesc &sceneDesc) const
    {
        QByteArray bytes = data;
        if (!bytes.size())
        {
            LogError("File " + sceneDesc.filename.toStdString() + " contained 0 bytes when trying to create scene description.");
            return sceneDesc;
        }

        try
        {
            DataDeserializer source(bytes.data(), bytes.size());
            
            uint num_entities = source.Read<u32>();
            for (uint i = 0; i < num_entities; ++i)
            {
                EntityDesc entityDesc;
                entity_id_t id = source.Read<u32>();
                entityDesc.id = QString::number((int)id);

                uint num_components = source.Read<u32>();
                for (uint i = 0; i < num_components; ++i)
                {
                    ComponentManagerPtr compMgr = framework_->GetComponentManager();

                    ComponentDesc compDesc;
                    uint type_hash = source.Read<u32>();
                    compDesc.typeName = compMgr->GetComponentTypeName(type_hash);
                    compDesc.name = QString::fromStdString(source.ReadString());
                    compDesc.sync = source.Read<u8>() ? true : false;
                    uint data_size = source.Read<u32>();

                    // Read the component data into a separate byte array, then deserialize from there.
                    // This way the whole stream should not desync even if something goes wrong
                    QByteArray comp_bytes;
                    comp_bytes.resize(data_size);
                    if (data_size)
                        source.ReadArray<u8>((u8*)comp_bytes.data(), comp_bytes.size());

                    try
                    {
                        ComponentPtr comp = compMgr->CreateComponent(type_hash, compDesc.name);
                        if (comp)
                        {
                            if (data_size)
                            {
                                DataDeserializer comp_source(comp_bytes.data(), comp_bytes.size());
                                // Trigger no signal yet when scene is in incoherent state
                                comp->DeserializeFromBinary(comp_source, AttributeChange::Disconnected);
                                foreach(IAttribute *a, comp->GetAttributes())
                                {
                                    QString typeName = a->TypeName().c_str();
                                    AttributeDesc attrDesc = { typeName, a->GetNameString().c_str(), a->ToString().c_str() };
                                    compDesc.attributes.append(attrDesc);

                                    QString attrValue = QString(a->ToString().c_str()).trimmed();
                                    if ((typeName == "assetreference" || typeName == "assetreferencelist" || 
                                        (a->HasMetadata() && a->GetMetadata()->elementType == "assetreference")) &&
                                        !attrValue.isEmpty())
                                    {
                                        // We might have multiple references, ";" used as a separator.
                                        QStringList values = attrValue.split(";");
                                        foreach(QString value, values)
                                        {
                                            AssetDesc ad;
                                            ad.typeName = a->GetNameString().c_str();
                                            ad.dataInMemory = false;

                                            // Rewrite source refs for asset descs, if necessary.
                                            QString basePath(boost::filesystem::path(sceneDesc.filename.toStdString()).branch_path().string().c_str());
                                            framework_->Asset()->QueryFileLocation(value, basePath, ad.source);
                                            ad.destinationName = AssetAPI::ExtractFilenameFromAssetRef(ad.source);

                                            sceneDesc.assets[qMakePair(ad.source, ad.subname)] = ad;
                                        }
                                    }
                                }
                            }

                            entityDesc.components.append(compDesc);
                        }
                        else
                            LogError("Failed to load component " + compDesc.typeName.toStdString());
                    }
                    catch (...)
                    {
                        LogError("Failed to load component " + compDesc.typeName.toStdString());
                    }
                }

                sceneDesc.entities.append(entityDesc);
            }
        }
        catch (...)
        {
            // Note: if exception happens, no change signals are emitted
            return SceneDesc();
        }

        return sceneDesc;
    }

    QByteArray SceneManager::GetEntityXml(Scene::Entity *entity)
    {
        QDomDocument scene_doc("Scene");
        QDomElement scene_elem = scene_doc.createElement("scene");
        EntityMap::iterator it = entities_.begin();
        
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
        scene_doc.appendChild(scene_elem);
        
        return scene_doc.toByteArray();
    }
    
    bool SceneManager::StartAttributeInterpolation(IAttribute* attr, IAttribute* endvalue, float length)
    {
        if (!endvalue)
            return false;
        
        IComponent* comp = attr ? attr->GetOwner() : 0;
        Entity* entity = comp ? comp->GetParentEntity() : 0;
        SceneManager* scene = entity ? entity->GetScene() : 0;
        
        if ((length <= 0.0f) || (!attr) || (!attr->HasMetadata()) || (attr->GetMetadata()->interpolation == AttributeMetadata::None) ||
            (!comp) || (comp->HasDynamicStructure()) || (!entity) || (!scene) || (scene != this))
        {
            delete endvalue;
            return false;
        }
        
        // End previous interpolation if existed
        bool previous = EndAttributeInterpolation(attr);
        
        // If previous interpolation does not exist, perform a direct snapping to the end value
        // but still start an interpolation period, so that on the next update we detect that an interpolation is going on,
        // and will interpolate normally
        if (!previous)
            attr->CopyValue(endvalue, AttributeChange::LocalOnly);
        
        AttributeInterpolation newInterp;
        newInterp.entityid_ = entity->GetId();
        newInterp.comp_name_ = comp->Name();
        newInterp.comp_typenamehash_ = comp->TypeNameHash();
        newInterp.dest_ = attr;
        newInterp.start_ = attr->Clone();
        newInterp.end_ = endvalue;
        newInterp.length_ = length;
        
        interpolations_.push_back(newInterp);
        return true;
    }
    
    bool SceneManager::EndAttributeInterpolation(IAttribute* attr)
    {
        for (uint i = 0; i < interpolations_.size(); ++i)
        {
            AttributeInterpolation& interp = interpolations_[i];
            if (interp.dest_ == attr)
            {
                delete interp.start_;
                delete interp.end_;
                interpolations_.erase(interpolations_.begin() + i);
                return true;
            }
        }
        return false;
    }
    
    void SceneManager::EndAllAttributeInterpolations()
    {
        for (uint i = 0; i < interpolations_.size(); ++i)
        {
            AttributeInterpolation& interp = interpolations_[i];
            delete interp.start_;
            delete interp.end_;
        }
        
        interpolations_.clear();
    }
    
    void SceneManager::UpdateAttributeInterpolations(float frametime)
    {
        PROFILE(Scene_UpdateInterpolation);
        
        interpolating_ = true;
        
        for (uint i = interpolations_.size() - 1; i < interpolations_.size(); --i)
        {
            AttributeInterpolation& interp = interpolations_[i];
            bool finished = false;
            
            // Check that the entity & component exist ie. it's safe to access the attribute
            Entity* entity = GetEntity(interp.entityid_).get();
            IComponent* comp = 0;
            if (entity)
                comp = entity->GetComponent(interp.comp_typenamehash_, interp.comp_name_).get();
            
            if (comp)
            {
                // Allow the interpolation to persist for 2x time, though we are no longer setting the value
                // This is for the continuous/discontinuous update detection in StartAttributeInterpolation()
                if (interp.time_ <= interp.length_)
                {
                    interp.time_ += frametime;
                    float t = interp.time_ / interp.length_;
                    if (t > 1.0f)
                        t = 1.0f;
                    interp.dest_->Interpolate(interp.start_, interp.end_, t, AttributeChange::LocalOnly);
                }
                else
                {
                    interp.time_ += frametime;
                    if (interp.time_ >= interp.length_ * 2.0f)
                        finished = true;
                }
            }
            else
                // Component could not be found, abort this interpolation
                finished = true;
            
            // Remove interpolation (& delete start/endpoints) when done
            if (finished)
            {
                delete interp.start_;
                delete interp.end_;
                interpolations_.erase(interpolations_.begin() + i);
            }
        }
        
        interpolating_ = false;
    }
}

