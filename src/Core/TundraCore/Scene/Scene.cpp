// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "SceneAPI.h"
#include "Scene/Scene.h"
#include "Entity.h"
#include "SceneDesc.h"
#include "IComponent.h"
#include "IAttribute.h"
#include "EC_Name.h"
#include "AttributeMetadata.h"
#include "ChangeRequest.h"
#include "EntityReference.h"

#include "Framework.h"
#include "Application.h"
#include "AssetAPI.h"
#include "FrameAPI.h"
#include "Profiler.h"
#include "LoggingFunctions.h"

#include <QString>
#include <QRegExp>
#include <QDomDocument>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QHash>

#include <kNet/DataDeserializer.h>
#include <kNet/DataSerializer.h>

#include <utility>
#include "MemoryLeakCheck.h"

using namespace kNet;

Scene::Scene(const QString &name, Framework *framework, bool viewEnabled, bool authority) :
    name_(name),
    framework_(framework),
    interpolating_(false),
    authority_(authority)
{
    // In headless mode only view disabled-scenes can be created
    viewEnabled_ = framework->IsHeadless() ? false : viewEnabled;

    // Connect to frame update to handle signalling entities created on this frame
    connect(framework->Frame(), SIGNAL(Updated(float)), this, SLOT(OnUpdated(float)));
}

Scene::~Scene()
{
    EndAllAttributeInterpolations();
    
    // Do not send entity removal or scene cleared events on destruction
    RemoveAllEntities(false);
    
    emit Removed(this);
}

EntityPtr Scene::CreateLocalEntity(const QStringList &components, AttributeChange::Type change, bool componentsReplicated)
{
    return CreateEntity(0, components, change, false, componentsReplicated);
}

EntityPtr Scene::CreateEntity(entity_id_t id, const QStringList &components, AttributeChange::Type change, bool replicated, bool componentsReplicated)
{
    // Figure out new entity id
    if (id == 0)
    {
        // Loop until a free ID found
        for (;;)
        {
            if (IsAuthority())
                id = replicated ? idGenerator_.AllocateReplicated() : idGenerator_.AllocateLocal();
            else
                id = replicated ? idGenerator_.AllocateUnacked() : idGenerator_.AllocateLocal();
            if (entities_.find(id) == entities_.end())
                break;
        }
    }
    else
    {
        if(entities_.find(id) != entities_.end())
        {
            LogError("Can't create entity with given id because it's already used: " + QString::number(id));
            return EntityPtr();
        }
        else
        {
            // Reset the ID generator to the manually assigned value to avoid unnecessary free ID probing in the future
            if (id < UniqueIdGenerator::FIRST_LOCAL_ID)
                idGenerator_.ResetReplicatedId(std::max(id, idGenerator_.id));
        }
    }

    EntityPtr entity = EntityPtr(new Entity(framework_, id, this));
    for(size_t i=0 ; i<(size_t)components.size() ; ++i)
    {
        ComponentPtr newComp = framework_->Scene()->CreateComponentByName(this, components[i]);
        if (newComp)
        {
            newComp->SetReplicated(componentsReplicated);
            entity->AddComponent(newComp, change); //change the param to a qstringlist or so \todo XXX
        }
    }
    entities_[entity->Id()] = entity;

    // Remember the creation and signal at end of frame if EmitEntityCreated() not called for this entity manually
    entitiesCreatedThisFrame_.push_back(std::make_pair(EntityWeakPtr(entity), change));

    return entity;
}

EntityPtr Scene::EntityById(entity_id_t id) const
{
    EntityMap::const_iterator it = entities_.find(id);
    if (it != entities_.end())
        return it->second;

    return EntityPtr();
}

EntityPtr Scene::EntityByName(const QString &name) const
{
    if (name.isEmpty())
        return EntityPtr();
    EntityMap::const_iterator it = entities_.begin();
    while(it != entities_.end())
    {
        if (it->second->Name() == name)
            return it->second;
        ++it;
    }
    
    return EntityPtr();
}

bool Scene::IsUniqueName(const QString& name) const
{
    if (name.isEmpty())
        return false;
    EntityMap::const_iterator it = entities_.begin();
    while(it != entities_.end())
    {
        if (it->second->Name() == name)
            return false;
        ++it;
    }
    
    return true;
}

void Scene::ChangeEntityId(entity_id_t old_id, entity_id_t new_id)
{
    if (old_id == new_id)
        return;
    
    EntityPtr old_entity = GetEntity(old_id);
    if (!old_entity)
        return;
    
    if (GetEntity(new_id))
    {
        LogWarning("Purged entity " + QString::number(new_id) + " to make room for a ChangeEntityId request. This should not happen");
        RemoveEntity(new_id, AttributeChange::LocalOnly);
    }
    
    old_entity->SetNewId(new_id);
    entities_.erase(old_id);
    entities_[new_id] = old_entity;
}

bool Scene::RemoveEntity(entity_id_t id, AttributeChange::Type change)
{
    EntityMap::iterator it = entities_.find(id);
    if (it != entities_.end())
    {
        EntityPtr del_entity = it->second;
        if (!del_entity.get())
        {
            LogError("Scene::RemoveEntity: Found null EntityPtr from internal state with id " + QString::number(id));
            return false;
        }
        
        EmitEntityRemoved(del_entity.get(), change);
        entities_.erase(it);
        
        // If entity somehow manages to live, at least it doesn't belong to the scene anymore
        del_entity->SetScene(0);
        del_entity.reset();
        return true;
    }
    return false;
}

void Scene::RemoveAllEntities(bool signal, AttributeChange::Type change)
{
    // If we don't want to emit signals, make sure the change mode is disconnected.
    if (!signal && change != AttributeChange::Disconnected)
        change = AttributeChange::Disconnected;

    // Gather entity ids to call RemoveEntity, as it modifies 
    // the entities_ map we should not call RemoveEntity while iterating it.
    std::list<entity_id_t> entIds;
    for (EntityMap::iterator it = entities_.begin(); it != entities_.end(); ++it)
    {
        if (it->second.get())
            entIds.push_back(it->second->Id());
    }
    while(entIds.size() > 0)
    {
        RemoveEntity(entIds.back(), change);
        entIds.pop_back();
    }
    entities_.clear();
    
    if (signal)
        emit SceneCleared(this);

    idGenerator_.Reset();
}

entity_id_t Scene::NextFreeId()
{
    if (IsAuthority())
        return idGenerator_.AllocateReplicated();
    else
        return idGenerator_.AllocateUnacked();
}

entity_id_t Scene::NextFreeIdLocal()
{
    return idGenerator_.AllocateLocal();
}

EntityList Scene::EntitiesWithComponent(const QString &typeName, const QString &name) const
{
    std::list<EntityPtr> entities;
    EntityMap::const_iterator it = entities_.begin();
    while(it != entities_.end())
    {
        EntityPtr entity = it->second;
        if ((name.isEmpty() && entity->GetComponent(typeName)) || entity->GetComponent(typeName, name))
            entities.push_back(entity);
        ++it;
    }

    return entities;
}

EntityList Scene::GetAllEntities() const
{
    LogWarning("Scene::GetAllEntities: this function is deprecated and will be removed. Use Scene::Entities instead");
    std::list<EntityPtr> entities;
    EntityMap::const_iterator it = entities_.begin();
    while(it != entities_.end())
    {
        entities.push_back(it->second);
        ++it;
    }

    return entities;
}

void Scene::EmitComponentAdded(Entity* entity, IComponent* comp, AttributeChange::Type change)
{
    if (change == AttributeChange::Disconnected)
        return;
    if (change == AttributeChange::Default)
        change = comp->UpdateMode();
    emit ComponentAdded(entity, comp, change);
}

void Scene::EmitComponentRemoved(Entity* entity, IComponent* comp, AttributeChange::Type change)
{
    if (change == AttributeChange::Disconnected)
        return;
    if (change == AttributeChange::Default)
        change = comp->UpdateMode();
    emit ComponentRemoved(entity, comp, change);
}

void Scene::EmitAttributeChanged(IComponent* comp, IAttribute* attribute, AttributeChange::Type change)
{
    if (!comp || !attribute || change == AttributeChange::Disconnected)
        return;
    if (change == AttributeChange::Default)
        change = comp->UpdateMode();
    emit AttributeChanged(comp, attribute, change);
}

void Scene::EmitAttributeAdded(IComponent* comp, IAttribute* attribute, AttributeChange::Type change)
{
    // "Stealth" addition (disconnected changetype) is not supported. Always signal.
    if (!comp || !attribute)
        return;
    if (change == AttributeChange::Default)
        change = comp->UpdateMode();
    emit AttributeAdded(comp, attribute, change);
}

void Scene::EmitAttributeRemoved(IComponent* comp, IAttribute* attribute, AttributeChange::Type change)
{
    // "Stealth" removal (disconnected changetype) is not supported. Always signal.
    if (!comp || !attribute)
        return;
    if (change == AttributeChange::Default)
        change = comp->UpdateMode();
    emit AttributeRemoved(comp, attribute, change);
}

void Scene::EmitEntityCreated(Entity *entity, AttributeChange::Type change)
{
    // Remove from the create signalling queue
    for (unsigned i = 0; i < entitiesCreatedThisFrame_.size(); ++i)
    {
        if (entitiesCreatedThisFrame_[i].first.lock().get() == entity)
        {
            entitiesCreatedThisFrame_.erase(entitiesCreatedThisFrame_.begin() + i);
            break;
        }
    }
    
    if (change == AttributeChange::Disconnected)
        return;
    if (change == AttributeChange::Default)
        change = AttributeChange::Replicate;
    ///@note This is not enough, it might be that entity is deleted after this call so we have dangling pointer in queue. 
    if (entity)
        emit EntityCreated(entity, change);
}

void Scene::EmitEntityRemoved(Entity* entity, AttributeChange::Type change)
{
    if (change == AttributeChange::Disconnected)
        return;
    if (change == AttributeChange::Default)
        change = AttributeChange::Replicate;
    emit EntityRemoved(entity, change);
    entity->EmitEntityRemoved(change);
}

void Scene::EmitActionTriggered(Entity *entity, const QString &action, const QStringList &params, EntityAction::ExecTypeField type)
{
    emit ActionTriggered(entity, action, params, type);
}

//before-the-fact counterparts for the modification signals above, for permission checks
bool Scene::AllowModifyEntity(UserConnection* user, Entity *entity)
{
    ChangeRequest req;
    emit AboutToModifyEntity(&req, user, entity);
    return req.allowed;
}

void Scene::EmitEntityAcked(Entity* entity, entity_id_t oldId)
{
    if (entity)
        emit EntityAcked(entity, oldId);
}

void Scene::EmitComponentAcked(IComponent* comp, component_id_t oldId)
{
    if (comp)
        emit ComponentAcked(comp, oldId);
}

QVariantList Scene::GetEntityIdsWithComponent(const QString &typeName) const
{
    LogWarning("Scene::GetEntityIdsWithComponent is deprecated and will be removed. Migrate to using EntitiesWithComponent instead.");
    QVariantList ret;
    foreach(const EntityPtr &e, EntitiesWithComponent(typeName))
        ret.append(e->Id());
    return ret;
}

QList<Entity *> Scene::LoadSceneXML(const QString& filename, bool clearScene, bool useEntityIDsFromFile, AttributeChange::Type change)
{
    QList<Entity *> ret;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        LogError("Failed to open file " + filename + " when loading scene xml.");
        return ret;
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    QDomDocument scene_doc("Scene");
    QString errorMsg;
    int errorLine, errorColumn;
    if (!scene_doc.setContent(stream.readAll(), &errorMsg, &errorLine, &errorColumn))
    {
        LogError(QString("Parsing scene XML from %1 failed when loading Scene XML: %2 at line %3 column %4.").arg(filename).arg(errorMsg).arg(errorLine).arg(errorColumn));
        file.close();
        return ret;
    }

    // Purge all old entities. Send events for the removal
    if (clearScene)
        RemoveAllEntities(true, change);

    return CreateContentFromXml(scene_doc, useEntityIDsFromFile, change);
}

QByteArray Scene::GetSceneXML(bool gettemporary, bool getlocal) const
{
    QDomDocument scene_doc("Scene");
    QDomElement scene_elem = scene_doc.createElement("scene");

    for(EntityMap::const_iterator iter = entities_.begin(); iter != entities_.end(); ++iter) 
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
            EntityPtr entity = iter->second;
            QDomElement entity_elem = scene_doc.createElement("entity");

            entity_elem.setAttribute("id", QString::number(entity->Id()));
            entity_elem.setAttribute("sync", BoolToString(entity->IsReplicated()));

            const Entity::ComponentMap &components = entity->Components();
            for (Entity::ComponentMap::const_iterator i = components.begin(); i != components.end(); ++i)
            {
                if ((!i->second->IsTemporary()) || (gettemporary))
                    i->second->SerializeTo(scene_doc, entity_elem);
            }
            
            scene_elem.appendChild(entity_elem);
        }
    }
    scene_doc.appendChild(scene_elem);

    return scene_doc.toByteArray();
}

bool Scene::SaveSceneXML(const QString& filename, bool saveTemporary, bool saveLocal)
{
    QByteArray bytes = GetSceneXML(saveTemporary, saveLocal);
    QFile scenefile(filename);
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

QList<Entity *> Scene::LoadSceneBinary(const QString& filename, bool clearScene, bool useEntityIDsFromFile, AttributeChange::Type change)
{
    QList<Entity *> ret;
    QFile file(filename);
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

bool Scene::SaveSceneBinary(const QString& filename, bool getTemporary, bool getLocal)
{
    QByteArray bytes;
    // Assume 4MB max for now
    bytes.resize(4 * 1024 * 1024);
    DataSerializer dest(bytes.data(), bytes.size());
    
    // Count number of entities we accept
    uint num_entities = 0;
    for(EntityMap::iterator iter = entities_.begin(); iter != entities_.end(); ++iter)
    {
        bool serialize = true;
        if (iter->second->IsLocal() && !getLocal)
            serialize = false;
        if (iter->second->IsTemporary() && !getTemporary)
            serialize = false;
        if (serialize)
            ++num_entities;
    }
    
    dest.Add<u32>(num_entities);

    for(EntityMap::iterator iter = entities_.begin(); iter != entities_.end(); ++iter)
    {
        bool serialize = true;
        if (iter->second->IsLocal() && !getLocal)
            serialize = false;
        if (iter->second->IsTemporary() && !getTemporary)
            serialize = false;
        if (serialize)
            iter->second->SerializeToBinary(dest);
    }
    
    bytes.resize(dest.BytesFilled());
    QFile scenefile(filename);
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

QList<Entity *> Scene::CreateContentFromXml(const QString &xml,  bool useEntityIDsFromFile, AttributeChange::Type change)
{
    QList<Entity *> ret;
    QString errorMsg;
    QDomDocument scene_doc("Scene");
    int errorLine, errorColumn;
    if (!scene_doc.setContent(xml, false, &errorMsg, &errorLine, &errorColumn))
    {
        LogError(QString("Parsing scene XML from text failed when loading Scene XML: %1 at line %2 column %3.").arg(errorMsg).arg(errorLine).arg(errorColumn));
        return ret;
    }

    return CreateContentFromXml(scene_doc, useEntityIDsFromFile, change);
}

QList<Entity *> Scene::CreateContentFromXml(const QDomDocument &xml, bool useEntityIDsFromFile, AttributeChange::Type change)
{
    std::vector<EntityWeakPtr> entities;
    
    // Check for existence of the scene element before we begin
    QDomElement scene_elem = xml.firstChildElement("scene");
    if (scene_elem.isNull())
    {
        LogError("Could not find 'scene' element from XML.");
        return QList<Entity*>();
    }

    // Create all storages fro the scene file.
    QDomElement storage_elem = scene_elem.firstChildElement("storage");
    while(!storage_elem.isNull())
    {
        framework_->Asset()->DeserializeAssetStorageFromString(Application::ParseWildCardFilename(storage_elem.attribute("specifier")), false);
        storage_elem = storage_elem.nextSiblingElement("storage");
    }
    
    QHash<entity_id_t, entity_id_t> oldToNewIds;

    // Spawn all entities in the scene storage.
    QDomElement ent_elem = scene_elem.firstChildElement("entity");
    while(!ent_elem.isNull())
    {
        QString replicatedStr = ent_elem.attribute("sync");
        bool replicated = true;
        if (!replicatedStr.isEmpty())
            replicated = ParseBool(replicatedStr);

        QString temporaryStr = ent_elem.attribute("temporary");
        bool temporary = false;
        if (!temporaryStr.isEmpty())
            temporary = ParseBool(temporaryStr);

        QString id_str = ent_elem.attribute("id");
        entity_id_t id = !id_str.isEmpty() ? static_cast<entity_id_t>(id_str.toInt()) : 0;
        if (!useEntityIDsFromFile || id == 0) // If we don't want to use entity IDs from file, or if file doesn't contain one, generate a new one.
        {
            entity_id_t originaId = id;
            id = replicated ? NextFreeId() : NextFreeIdLocal();
            if (originaId != 0 && !oldToNewIds.contains(originaId))
                oldToNewIds[originaId] = id;
        }
        else if (useEntityIDsFromFile && HasEntity(id)) // If we use IDs from file and they conflict with some of the existing IDs, change the ID of the old entity
        {
            entity_id_t newID = replicated ? NextFreeId() : NextFreeIdLocal();
            ChangeEntityId(id, newID);
        }

        if (HasEntity(id)) // If the entity we are about to add conflicts in ID with an existing entity in the scene, delete the old entity.
        {
            LogDebug("Scene::CreateContentFromXml: Destroying previous entity with id " + QString::number(id) + " to avoid conflict with new created entity with the same id.");
            LogError("Warning: Invoking buggy behavior: Object with id " + QString::number(id) +" might not replicate properly!");
            RemoveEntity(id, AttributeChange::Replicate); ///<@todo Consider do we want to always use Replicate
        }

        EntityPtr entity = CreateEntity(id);
        if (entity)
        {
            entity->SetTemporary(temporary);

            QDomElement comp_elem = ent_elem.firstChildElement("component");
            while(!comp_elem.isNull())
            {
                /// \todo Read component id's from file
                
                QString type_name = comp_elem.attribute("type");
                QString name = comp_elem.attribute("name");
                QString compReplicatedStr = comp_elem.attribute("sync");
                QString temp = comp_elem.attribute("temporary");

                bool compReplicated = true;
                if (!compReplicatedStr.isEmpty())
                    compReplicated = ParseBool(compReplicatedStr);

                bool temporary = false;
                if (!temp.isEmpty())
                    temporary = ParseBool(temp);
                
                ComponentPtr new_comp = entity->GetOrCreateComponent(type_name, name, AttributeChange::Default, compReplicated);
                if (new_comp)
                {
                    new_comp->SetTemporary(temporary);
                    // Trigger no signal yet when scene is in incoherent state
                    new_comp->DeserializeFrom(comp_elem, AttributeChange::Disconnected);
                }
                
                comp_elem = comp_elem.nextSiblingElement("component");
            }
            entities.push_back(entity);
        }
        else
        {
            LogError("Scene::CreateContentFromXml: Failed to create entity with id " + QString::number(id) + "!");
        }

        ent_elem = ent_elem.nextSiblingElement("entity");
    }

    // Now that we have each entity spawned to the scene, trigger all the signals for EntityCreated/ComponentChanged messages.
    for(unsigned i = 0; i < entities.size(); ++i)
    {
        if (!entities[i].expired())
            EmitEntityCreated(entities[i].lock().get(), change);
        if (!entities[i].expired())
        {
            EntityPtr entityShared = entities[i].lock();
            const Entity::ComponentMap &components = entityShared->Components();
            for (Entity::ComponentMap::const_iterator i = components.begin(); i != components.end(); ++i)
            {
                if (!useEntityIDsFromFile && i->second->TypeName() == "EC_Placeable")
                {
                    // Go and fix parent ref of EC_Placeable if new entity IDs were generated
                    IAttribute *iAttr = i->second->GetAttribute("Parent entity ref");
                    Attribute<EntityReference> *parenRef = iAttr != 0 ? dynamic_cast<Attribute<EntityReference> *>(iAttr) : 0;
                    if (parenRef && !parenRef->Get().IsEmpty())
                    {
                        QString ref = parenRef->Get().ref;
                        
                        // We only need to fix the id parent refs.
                        // Ones with entity names should work as expected.
                        bool isNumber = false;
                        entity_id_t refId = ref.toUInt(&isNumber);
                        if (isNumber && refId > 0 && oldToNewIds.contains(refId))
                            parenRef->Set(EntityReference(oldToNewIds[refId]), change);
                    }
                }
                i->second->ComponentChanged(change);
            }
        }
    }
    
    // The above signals may have caused scripts to remove entities. Return those that still exist.
    QList<Entity *> ret;
    for(unsigned i = 0; i < entities.size(); ++i)
    {
        if (!entities[i].expired())
            ret.append(entities[i].lock().get());
    }
    
    return ret;
}

QList<Entity *> Scene::CreateContentFromBinary(const QString &filename, bool useEntityIDsFromFile, AttributeChange::Type change)
{

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        LogError("Failed to open file " + filename + " when loading scene binary.");
        return QList<Entity*>();
    }

    QByteArray bytes = file.readAll();
    file.close();
    
    if (!bytes.size())
    {
        LogError("File " + filename + "contained 0 bytes when loading scene binary.");
        return QList<Entity*>();
    }

    return CreateContentFromBinary(bytes.data(), bytes.size(), useEntityIDsFromFile, change);
}

QList<Entity *> Scene::CreateContentFromBinary(const char *data, int numBytes, bool useEntityIDsFromFile, AttributeChange::Type change)
{
    std::vector<EntityWeakPtr> entities;
    assert(data);
    assert(numBytes > 0);
    try
    {
        DataDeserializer source(data, numBytes);
        
        uint num_entities = source.Read<u32>();
        for(uint i = 0; i < num_entities; ++i)
        {
            entity_id_t id = source.Read<u32>();
            bool replicated = source.Read<u8>() ? true : false;
            if (!useEntityIDsFromFile || id == 0)
                id = replicated ? NextFreeId() : NextFreeIdLocal();

            if (HasEntity(id)) // If the entity we are about to add conflicts in ID with an existing entity in the scene.
            {
                LogDebug("Scene::CreateContentFromBinary: Destroying previous entity with id " + QString::number(id) + " to avoid conflict with new created entity with the same id.");
                LogError("Warning: Invoking buggy behavior: Object with id " + QString::number(id) + "might not replicate properly!");
                RemoveEntity(id, AttributeChange::Replicate); ///<@todo Consider do we want to always use Replicate
            }

            EntityPtr entity = CreateEntity(id);
            if (!entity)
            {
                LogError("Failed to create entity, stopping scene load!");
                return QList<Entity*>(); // If entity creation fails, stream desync is more than likely so stop right here
            }
            
            uint num_components = source.Read<u32>();
            for(uint i = 0; i < num_components; ++i)
            {
                u32 typeId = source.Read<u32>(); ///\todo VLE this!
                QString name = QString::fromStdString(source.ReadString());
                bool compReplicated = source.Read<u8>() ? true : false;
                uint data_size = source.Read<u32>();
                
                // Read the component data into a separate byte array, then deserialize from there.
                // This way the whole stream should not desync even if something goes wrong
                QByteArray comp_bytes;
                comp_bytes.resize(data_size);
                if (data_size)
                    source.ReadArray<u8>((u8*)comp_bytes.data(), comp_bytes.size());
                
                try
                {
                    ComponentPtr new_comp = entity->GetOrCreateComponent(typeId, name, AttributeChange::Default, compReplicated);
                    if (new_comp)
                    {
                        if (data_size)
                        {
                            DataDeserializer comp_source(comp_bytes.data(), comp_bytes.size());
                            // Trigger no signal yet when scene is in incoherent state
                            new_comp->DeserializeFromBinary(comp_source, AttributeChange::Disconnected);
                        }
                    }
                    else
                        LogError("Failed to load component \"" + framework_->Scene()->GetComponentTypeName(typeId) + "\"!");
                }
                catch(...)
                {
                    LogError("Failed to load component \"" + framework_->Scene()->GetComponentTypeName(typeId) + "\"!");
                }
            }

            entities.push_back(entity);
        }
    }
    catch(...)
    {
        // Note: if exception happens, no change signals are emitted
        return QList<Entity *>();
    }

    // Now that we have each entity spawned to the scene, trigger all the signals for EntityCreated/ComponentChanged messages.
    for(unsigned i = 0; i < entities.size(); ++i)
    {
        if (!entities[i].expired())
            EmitEntityCreated(entities[i].lock().get(), change);
        if (!entities[i].expired())
        {
            EntityPtr entityShared = entities[i].lock();
            const Entity::ComponentMap &components = entityShared->Components();
            for (Entity::ComponentMap::const_iterator i = components.begin(); i != components.end(); ++i)
                i->second->ComponentChanged(change);
        }
    }
    
    // The above signals may have caused scripts to remove entities. Return those that still exist.
    QList<Entity *> ret;
    for(unsigned i = 0; i < entities.size(); ++i)
        if (!entities[i].expired())
            ret.append(entities[i].lock().get());

    return ret;
}

QList<Entity *> Scene::CreateContentFromSceneDesc(const SceneDesc &desc, bool useEntityIDsFromFile, AttributeChange::Type change)
{
    QList<Entity *> ret;

    if (desc.entities.empty())
    {
        LogError("Empty scene description.");
        return ret;
    }

    QHash<entity_id_t, entity_id_t> oldToNewIds;

    foreach(const EntityDesc &e, desc.entities)
    {
        entity_id_t id;
        id =  static_cast<entity_id_t>(e.id.toInt());

        if (e.id.isEmpty() || !useEntityIDsFromFile)
        {
            entity_id_t originaId = id;
            id = e.local ? NextFreeIdLocal() : NextFreeId();
            if (originaId != 0 && !oldToNewIds.contains(originaId))
                oldToNewIds[originaId] = id;
        }

        if (HasEntity(id)) // If the entity we are about to add conflicts in ID with an existing entity in the scene.
        {
            LogDebug("Scene::CreateContentFromSceneDescription: Destroying previous entity with id " + QString::number(id) + " to avoid conflict with new created entity with the same id.");
            LogError("Warning: Invoking buggy behavior: Object with id " + QString::number(id) + " might not replicate properly!");
            RemoveEntity(id, AttributeChange::Replicate); ///<@todo Consider do we want to always use Replicate
        }

        EntityPtr entity = CreateEntity(id);
        assert(entity);
        if (entity)
        {
            foreach(const ComponentDesc &c, e.components)
            {
                if (c.typeName.isNull())
                    continue;
                ComponentPtr comp = entity->GetOrCreateComponent(c.typeName, c.name);
                assert(comp);
                if (!comp)
                {
                    LogError(QString("Scene::CreateContentFromSceneDesc: failed to create component %1 %2 .").arg(c.typeName).arg(c.name));
                    continue;
                }
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
                    foreach(IAttribute *attr, comp->Attributes())
                        if (attr)
                            foreach(const AttributeDesc &a, c.attributes)
                                if (attr->TypeName() == a.typeName && attr->Name() == a.name)
                                    attr->FromString(a.value.toStdString(), AttributeChange::Disconnected); // Trigger no signal yet when scene is in incoherent state
                }
            }

            entity->SetTemporary(e.temporary);
            ret.append(entity.get());
        }
    }

    // All entities & components have been loaded. Trigger change for them now.
    foreach(Entity *entity, ret)
    {
        EmitEntityCreated(entity, change);
        const Entity::ComponentMap &components = entity->Components();
        for(Entity::ComponentMap::const_iterator i = components.begin(); i != components.end(); ++i)
        {
            if (!useEntityIDsFromFile && i->second->TypeName() == "EC_Placeable")
            {
                // Go and fix parent ref of EC_Placeable if new entity IDs were generated
                IAttribute *iAttr = i->second->GetAttribute("Parent entity ref");
                Attribute<EntityReference> *parenRef = iAttr != 0 ? dynamic_cast<Attribute<EntityReference> *>(iAttr) : 0;
                if (parenRef && !parenRef->Get().IsEmpty())
                {
                    QString ref = parenRef->Get().ref;
                    // We only need to fix the id parent refs.
                    // Ones with entity names should work as expected.
                    bool isNumber = false;
                    entity_id_t refId = ref.toUInt(&isNumber);
                    if (isNumber && refId > 0 && oldToNewIds.contains(refId))
                        parenRef->Set(EntityReference(oldToNewIds[refId]), change);
                }
            }
            i->second->ComponentChanged(change);
        }
    }

    return ret;
}

SceneDesc Scene::CreateSceneDescFromXml(const QString &filename) const
{
    SceneDesc sceneDesc;
    if (!filename.endsWith(".txml", Qt::CaseInsensitive))
    {
        if (filename.endsWith(".tbin", Qt::CaseInsensitive))
            LogError("Try using CreateSceneDescFromBinary() instead for " + filename);
        else
            LogError("Unsupported file extension : " + filename + " when trying to create scene description.");
        return sceneDesc;
    }

    sceneDesc.filename = filename;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        LogError("Failed to open file " + filename + " when trying to create scene description.");
        return sceneDesc;
    }

    QByteArray data = file.readAll();
    file.close();

    return CreateSceneDescFromXml(data, sceneDesc);
}

SceneDesc Scene::CreateSceneDescFromXml(QByteArray &data, SceneDesc &sceneDesc) const
{
    QTextStream stream(&data);
    stream.setCodec("UTF-8");
    QDomDocument scene_doc("Scene");
    QString errorMsg;
    int errorLine, errorColumn;
    if (!scene_doc.setContent(stream.readAll(), &errorMsg, &errorLine, &errorColumn))
    {
        LogError(QString("Parsing scene XML from %1 failed when loading Scene XML: %2 at line %3 column %4.").arg(sceneDesc.filename).arg(errorMsg).arg(errorLine).arg(errorColumn));
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
    while(!ent_elem.isNull())
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
                    ComponentPtr comp = framework_->Scene()->CreateComponentByName(const_cast<Scene*>(this), type_name, name);
                    EC_Name *ecName = checked_static_cast<EC_Name*>(comp.get());
                    ecName->DeserializeFrom(comp_elem, AttributeChange::Disconnected);
                    entityDesc.name = ecName->name.Get();
                }

                // Find asset references.
                ComponentPtr comp = framework_->Scene()->CreateComponentByName(const_cast<Scene*>(this), type_name, name);
                if (!comp.get()) // Move to next element if component creation fails.
                {
                    comp_elem = comp_elem.nextSiblingElement("component");
                    continue;
                }

                comp->DeserializeFrom(comp_elem, AttributeChange::Disconnected);
                foreach(IAttribute *a,comp->Attributes())
                {
                    if (!a)
                        continue;
                    
                    QString typeName = a->TypeName();
                    AttributeDesc attrDesc = { typeName, a->Name(), a->ToString().c_str() };
                    compDesc.attributes.append(attrDesc);

                    QString attrValue = QString(a->ToString().c_str()).trimmed();
                    if ((typeName == "assetreference" || typeName == "assetreferencelist" || 
                        (a->Metadata() && a->Metadata()->elementType == "assetreference")) &&
                        !attrValue.isEmpty())
                    {
                        // We might have multiple references, ";" used as a separator.
                        QStringList values = attrValue.split(";");
                        foreach(QString value, values)
                        {
                            AssetDesc ad;
                            ad.typeName = a->Name();
                            ad.dataInMemory = false;

                            // Rewrite source refs for asset descs, if necessary.
                            QString basePath = QFileInfo(sceneDesc.filename).dir().path();
                            framework_->Asset()->ResolveLocalAssetPath(value, basePath, ad.source);
                            ad.destinationName = AssetAPI::ExtractFilenameFromAssetRef(ad.source);

                            sceneDesc.assets[qMakePair(ad.source, ad.subname)] = ad;

                            // If this is a script, look for dependecies
                            if (ad.source.toLower().endsWith(".js"))
                                SearchScriptAssetDependencies(ad.source, sceneDesc);
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

///\todo This function is a redundant duplicate copy of void ScriptAsset::ParseReferences(). Delete this code. -jj.
void Scene::SearchScriptAssetDependencies(const QString &filePath, SceneDesc &sceneDesc) const
{
    if (!filePath.toLower().endsWith(".js"))
        return;

    if (QFile::exists(filePath))
    {
        QFile script(filePath);
        if (script.open(QIODevice::ReadOnly))
        {
            QString scriptData = script.readAll();
            std::string content = scriptData.toStdString();
            QStringList foundRefs;
            sregex_iterator searchEnd;

            regex expression("!ref:\\s*(.*?)\\s*(\\n|$)");
            for(sregex_iterator iter(content.begin(), content.end(), expression); iter != searchEnd; ++iter)
            {
                QString ref = QString::fromStdString((*iter)[1].str());
                if (!foundRefs.contains(ref, Qt::CaseInsensitive))
                    foundRefs << ref;
            }

            expression = regex("engine.IncludeFile\\(\\s*\"\\s*(.*?)\\s*\"\\s*\\)");
            for(sregex_iterator iter(content.begin(), content.end(), expression); iter != searchEnd; ++iter)
            {
                QString ref = QString::fromStdString((*iter)[1].str());
                if (!foundRefs.contains(ref, Qt::CaseInsensitive))
                    foundRefs << ref;
            }

            foreach(QString scriptDependency, foundRefs)
            {
                AssetDesc ad;
                ad.typeName = "Script dependency";
                ad.dataInMemory = false;

                QString basePath = QFileInfo(sceneDesc.filename).dir().path();
                framework_->Asset()->ResolveLocalAssetPath(scriptDependency, basePath, ad.source);
                ad.destinationName = AssetAPI::ExtractFilenameFromAssetRef(ad.source);
                
                // We have to check if the asset is already added. As we do this recursively there is a danger of a infinite loop.
                // This check wont let that happen. Situation when infinite loop would happen: A.js depends on B.js and B.js depends on A.js
                // Other than .js depedency assets cannot cause this.
                SceneDesc::AssetMapKey key = qMakePair(ad.source, ad.subname);
                if (!sceneDesc.assets.contains(key))
                {
                    sceneDesc.assets[key] = ad;

                    // Go deeper if dep file is .js
                    if (ad.source.toLower().endsWith(".js"))
                        SearchScriptAssetDependencies(ad.source, sceneDesc);
                }
            }
        }
    }
}

SceneDesc Scene::CreateSceneDescFromBinary(const QString &filename) const
{
    SceneDesc sceneDesc;

    if (!filename.endsWith(".tbin", Qt::CaseInsensitive))
    {
        if (filename.endsWith(".txml", Qt::CaseInsensitive))
            LogError("Try using CreateSceneDescFromXml() instead for " + filename);
        else
            LogError("Unsupported file extension : " + filename + " when trying to create scene description.");
        return sceneDesc;
    }

    sceneDesc.filename = filename;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        LogError("Failed to open file " + filename + " when trying to create scene description.");
        return sceneDesc;
    }

    QByteArray bytes = file.readAll();
    file.close();

    return CreateSceneDescFromBinary(bytes, sceneDesc);
}

SceneDesc Scene::CreateSceneDescFromBinary(QByteArray &data, SceneDesc &sceneDesc) const
{
    QByteArray bytes = data;
    if (!bytes.size())
    {
        LogError("File " + sceneDesc.filename + " contained 0 bytes when trying to create scene description.");
        return sceneDesc;
    }

    try
    {
        DataDeserializer source(bytes.data(), bytes.size());
        
        uint num_entities = source.Read<u32>();
        for(uint i = 0; i < num_entities; ++i)
        {
            EntityDesc entityDesc;
            entity_id_t id = source.Read<u32>();
            entityDesc.id = QString::number((int)id);

            uint num_components = source.Read<u32>();
            for(uint i = 0; i < num_components; ++i)
            {
                SceneAPI *sceneAPI = framework_->Scene();

                ComponentDesc compDesc;
                u32 typeId = source.Read<u32>(); ///\todo VLE this!
                compDesc.typeName = sceneAPI->GetComponentTypeName(typeId);
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
                    ComponentPtr comp = sceneAPI->CreateComponentById(const_cast<Scene*>(this), typeId, compDesc.name);
                    if (comp)
                    {
                        if (data_size)
                        {
                            DataDeserializer comp_source(comp_bytes.data(), comp_bytes.size());
                            // Trigger no signal yet when scene is in incoherent state
                            comp->DeserializeFromBinary(comp_source, AttributeChange::Disconnected);
                            foreach(IAttribute *a, comp->Attributes())
                            {
                                if (!a)
                                    continue;
                                
                                QString typeName = a->TypeName();
                                AttributeDesc attrDesc = { typeName, a->Name(), a->ToString().c_str() };
                                compDesc.attributes.append(attrDesc);

                                QString attrValue = QString(a->ToString().c_str()).trimmed();
                                if ((typeName == "assetreference" || typeName == "assetreferencelist" || 
                                    (a->Metadata() && a->Metadata()->elementType == "assetreference")) &&
                                    !attrValue.isEmpty())
                                {
                                    // We might have multiple references, ";" used as a separator.
                                    QStringList values = attrValue.split(";");
                                    foreach(QString value, values)
                                    {
                                        AssetDesc ad;
                                        ad.typeName = a->Name();
                                        ad.dataInMemory = false;

                                        // Rewrite source refs for asset descs, if necessary.
                                        QString basePath = QFileInfo(sceneDesc.filename).dir().path();
                                        framework_->Asset()->ResolveLocalAssetPath(value, basePath, ad.source);
                                        ad.destinationName = AssetAPI::ExtractFilenameFromAssetRef(ad.source);

                                        sceneDesc.assets[qMakePair(ad.source, ad.subname)] = ad;
                                    }
                                }
                            }
                        }

                        entityDesc.components.append(compDesc);
                    }
                    else
                        LogError("Failed to load component " + compDesc.typeName);
                }
                catch(...)
                {
                    LogError("Failed to load component " + compDesc.typeName);
                }
            }

            sceneDesc.entities.append(entityDesc);
        }
    }
    catch(...)
    {
        // Note: if exception happens, no change signals are emitted
        return SceneDesc();
    }

    return sceneDesc;
}

QByteArray Scene::GetEntityXml(Entity *entity) const
{
    LogWarning("Scene::GetEntityXml: this function is deprecated and will be removed. Use Entity::SerializeToXMLString instead.");
    QDomDocument scene_doc("Scene");
    QDomElement scene_elem = scene_doc.createElement("scene");
    if (entity)
    {
        QDomElement entity_elem = scene_doc.createElement("entity");
        entity_elem.setAttribute("id", QString::number((int)entity->Id()));
        const Entity::ComponentMap &components = entity->Components();
        for (Entity::ComponentMap::const_iterator i = components.begin(); i != components.end(); ++i)
            i->second->SerializeTo(scene_doc, entity_elem);
        scene_elem.appendChild(entity_elem);
    }

    scene_doc.appendChild(scene_elem);
    return scene_doc.toByteArray();
}

float3 Scene::UpVector() const
{
    return float3::unitY;
}

float3 Scene::RightVector() const
{
    return float3::unitX;
}

float3 Scene::ForwardVector() const
{
    return -float3::unitZ;
}

bool Scene::StartAttributeInterpolation(IAttribute* attr, IAttribute* endvalue, float length)
{
    if (!endvalue)
        return false;
    
    IComponent* comp = attr ? attr->Owner() : 0;
    Entity* entity = comp ? comp->ParentEntity() : 0;
    Scene* scene = entity ? entity->ParentScene() : 0;
    
    if (length <= 0.0f || !attr || !attr->Metadata() || attr->Metadata()->interpolation == AttributeMetadata::None ||
        !comp || !entity || !scene || scene != this)
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
    newInterp.comp = comp->shared_from_this();
    newInterp.dest = attr;
    newInterp.start = attr->Clone();
    newInterp.end = endvalue;
    newInterp.length = length;
    
    interpolations_.push_back(newInterp);
    return true;
}

bool Scene::EndAttributeInterpolation(IAttribute* attr)
{
    for(uint i = 0; i < interpolations_.size(); ++i)
    {
        AttributeInterpolation& interp = interpolations_[i];
        if (interp.dest == attr)
        {
            delete interp.start;
            delete interp.end;
            interpolations_.erase(interpolations_.begin() + i);
            return true;
        }
    }
    return false;
}

void Scene::EndAllAttributeInterpolations()
{
    for(uint i = 0; i < interpolations_.size(); ++i)
    {
        AttributeInterpolation& interp = interpolations_[i];
        delete interp.start;
        delete interp.end;
    }
    
    interpolations_.clear();
}

void Scene::UpdateAttributeInterpolations(float frametime)
{
    PROFILE(Scene_UpdateInterpolation);
    
    interpolating_ = true;
    
    for(uint i = interpolations_.size() - 1; i < interpolations_.size(); --i)
    {
        AttributeInterpolation& interp = interpolations_[i];
        bool finished = false;
        
        // Check that the component still exists ie. it's safe to access the attribute
        if (!interp.comp.expired())
        {
            // Allow the interpolation to persist for 2x time, though we are no longer setting the value
            // This is for the continuous/discontinuous update detection in StartAttributeInterpolation()
            if (interp.time <= interp.length)
            {
                interp.time += frametime;
                float t = interp.time / interp.length;
                if (t > 1.0f)
                    t = 1.0f;
                interp.dest->Interpolate(interp.start, interp.end, t, AttributeChange::LocalOnly);
            }
            else
            {
                interp.time += frametime;
                if (interp.time >= interp.length * 2.0f)
                    finished = true;
            }
        }
        else
            // Component pointer has expired, abort this interpolation
            finished = true;
        
        // Remove interpolation (& delete start/endpoints) when done
        if (finished)
        {
            delete interp.start;
            delete interp.end;
            interpolations_.erase(interpolations_.begin() + i);
        }
    }

    interpolating_ = false;
}

void Scene::OnUpdated(float /*frameTime*/)
{
    // Signal queued entity creations now
    for (unsigned i = 0; i < entitiesCreatedThisFrame_.size(); ++i)
    {
        Entity* entity = entitiesCreatedThisFrame_[i].first.lock().get();
        if (!entity)
            continue;
        
        AttributeChange::Type change = entitiesCreatedThisFrame_[i].second;
        if (change == AttributeChange::Disconnected)
            continue;
        if (change == AttributeChange::Default)
            change = AttributeChange::Replicate;
        
        emit EntityCreated(entity, change);
    }
    
    entitiesCreatedThisFrame_.clear();
}

EntityList Scene::FindEntities(const QString &pattern) const
{
    QRegExp regex = QRegExp(pattern, Qt::CaseSensitive, QRegExp::WildcardUnix);
    return FindEntities(regex);
}

EntityList Scene::FindEntities(const QRegExp &pattern) const
{
    EntityList entities;
    if (pattern.isEmpty() || !pattern.isValid())
        return entities;

    EntityMap::const_iterator it = entities_.begin();

    while(it != entities_.end())
    {
        EntityPtr entity = it->second;

        if (pattern.exactMatch(entity->Name()))
            entities.push_back(entity);

        ++it;
    }

    return entities;
}

EntityList Scene::FindEntitiesContaining(const QString &substring) const
{
    EntityList entities;
    if (substring.isEmpty())
        return entities;

    EntityMap::const_iterator it = entities_.begin();

    while(it != entities_.end())
    {
        EntityPtr entity = it->second;
        if (entity->Name().contains(substring, Qt::CaseSensitive))
            entities.push_back(entity);

        ++it;
    }

    return entities;
}
