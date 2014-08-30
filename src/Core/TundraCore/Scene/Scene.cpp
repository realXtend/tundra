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

    // Connect to frame update to handle signaling entities created on this frame
    connect(framework->Frame(), SIGNAL(Updated(float)), this, SLOT(OnUpdated(float)));
}

Scene::~Scene()
{
    EndAllAttributeInterpolations();
    
    // Do not send entity removal or scene cleared events on destruction
    RemoveAllEntities(false);
    
    emit Removed(this);
}

EntityPtr Scene::CreateLocalEntity(const QStringList &components, AttributeChange::Type change, bool componentsReplicated, bool temporary)
{
    return CreateEntity(0, components, change, false, componentsReplicated, temporary);
}

EntityPtr Scene::CreateEntity(entity_id_t id, const QStringList &components, AttributeChange::Type change, bool replicated, bool componentsReplicated, bool temporary)
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

    EntityPtr entity = MAKE_SHARED(Entity, framework_, id, temporary, this);
    for(int i = 0 ;i < components.size(); ++i)
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
    entitiesCreatedThisFrame_.push_back(std::make_pair(entity, change));

    return entity;
}

EntityPtr Scene::CreateTemporaryEntity(const QStringList &components, AttributeChange::Type change, bool componentsReplicated)
{
    return CreateEntity(0, components, change, true, componentsReplicated, true);
}

EntityPtr Scene::CreateLocalTemporaryEntity(const QStringList &components, AttributeChange::Type change)
{
    return CreateEntity(0, components, change, false, false, true);
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

    for(const_iterator it = begin(); it != end(); ++it)
        if (it->second->Name() == name)
            return it->second;

    return EntityPtr();
}

bool Scene::IsUniqueName(const QString& name) const
{
    return !EntityByName(name);
}

void Scene::ChangeEntityId(entity_id_t old_id, entity_id_t new_id)
{
    if (old_id == new_id)
        return;
    
    EntityPtr old_entity = EntityById(old_id);
    if (!old_entity)
        return;
    
    if (EntityById(new_id))
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
        
        /// \bug Emit entity removal first, as there may be scripts which depend on components still being there for their cleanup.
        /// This is necessary as QScriptEngine may not handle later access to removed objects gracefully and may eg exit() the whole program.
        /// Even a malfunctioning script should not be able to bring the Tundra process down in this manner.
        EmitEntityRemoved(del_entity.get(), change);
        
        // Then make the entity remove all of its components, so that their individual removals are signaled properly
        del_entity->RemoveAllComponents(change);
        
        // If the entity is parented, remove from the parent. No signaling is necessary
        if (del_entity->Parent())
            del_entity->SetParent(EntityPtr(), AttributeChange::Disconnected);

        // Remove all child entities. This may be recursive
        del_entity->RemoveAllChildren(change);

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
    ///\todo The following code was done to resolve a mysterious crash bug.
    ///      See https://github.com/Adminotech/tundra/commit/cb051bb270be3ce6e64a822593f1e14675bbf922
    ///      Contact Jonne for more info. -cs
    std::list<entity_id_t> entIds;
    for (EntityMap::iterator it = entities_.begin(); it != entities_.end(); ++it)
    {
        // Only root-level entities need to be removed, the rest clean themselves up automatically
        if (it->second.get() && !it->second->Parent())
            entIds.push_back(it->second->Id());
    }
    while(entIds.size() > 0)
    {
        RemoveEntity(entIds.back(), change);
        entIds.pop_back();
    }
    
    if (entities_.size())
    {
        LogWarning("Scene::RemoveAllEntities: entity map was not clear after removing all entities, clearing manually");
        entities_.clear();
    }
    
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
    return EntitiesWithComponent(framework_->Scene()->ComponentTypeIdForTypeName(typeName), name);
}

EntityList Scene::EntitiesWithComponent(u32 typeId, const QString &name) const
{
    EntityList entities;
    for(const_iterator it = begin(); it != end(); ++it)
        if ((name.isEmpty() && it->second->Component(typeId)) || it->second->Component(typeId, name))
            entities.push_back(it->second);
    return entities;
}

EntityList Scene::EntitiesOfGroup(const QString &groupName) const
{
    EntityList entities;
    if (groupName.isEmpty())
        return entities;

    for (const_iterator it = begin(); it != end(); ++it)
        if (it->second->Group() == groupName)
            entities.push_back(it->second);

    return entities;
}

Entity::ComponentVector Scene::Components(const QString &typeName, const QString &name) const
{
    return Components(framework_->Scene()->ComponentTypeIdForTypeName(typeName), name);
}

Entity::ComponentVector Scene::Components(u32 typeId, const QString &name) const
{
    Entity::ComponentVector ret;
    if (name.isEmpty())
    {
        for(const_iterator it = begin(); it != end(); ++it)
        {
            Entity::ComponentVector components =  it->second->ComponentsOfType(typeId);
            if (!components.empty())
                ret.insert(ret.end(), components.begin(), components.end());
        }
    }
    else
    {
        for(const_iterator it = begin(); it != end(); ++it)
        {
            ComponentPtr component = it->second->Component(typeId, name);
            if (component)
                ret.push_back(component);
        }
    }
    return ret;
}

EntityList Scene::GetAllEntities() const
{
    LogWarning("Scene::GetAllEntities: this function is deprecated and will be removed. Use Scene::Entities instead");

    std::list<EntityPtr> entities;
    for(const_iterator it = begin(); it != end(); ++it)
        entities.push_back(it->second);

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

void Scene::EmitEntityParentChanged(Entity* entity, Entity* newParent, AttributeChange::Type change)
{
    if (!entity || change == AttributeChange::Disconnected)
        return;
    if (change == AttributeChange::Default)
        change = entity->IsLocal() ? AttributeChange::LocalOnly : AttributeChange::Replicate;
    emit EntityParentChanged(entity, newParent, change);
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
    {
        emit EntityAcked(entity, oldId);

        /** On client feed the acked ids, once tracker receives the last
            id it will process the tracked scene part for broken parenting.
            This is done after above emit so that behavior for all entities
            in the tracker is the same (emit before parenting is fixed). */
        if (!IsAuthority())
            parentTracker_.Ack(this, entity->Id(), oldId);
    }
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

QByteArray Scene::GetSceneXML(bool serializeTemporary, bool serializeLocal) const
{
    LogWarning("Scene::GetSceneXML is deprecated and will be removed. Migrate to using SerializeToXmlString instead.");
    return SerializeToXmlString(serializeTemporary, serializeLocal);
}

QList<Entity *> Scene::LoadSceneXML(const QString& filename, bool clearScene, bool useEntityIDsFromFile, AttributeChange::Type change)
{
    QList<Entity *> ret;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        LogError("Scene::LoadSceneXML: Failed to open file " + filename + ".");
        return ret;
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    QDomDocument scene_doc("Scene");
    QString errorMsg;
    int errorLine, errorColumn;
    if (!scene_doc.setContent(stream.readAll(), &errorMsg, &errorLine, &errorColumn))
    {
        LogError(QString("Scene::LoadSceneXML: Parsing scene XML from %1 failed when loading Scene XML: %2 at line %3 column %4.")
            .arg(filename).arg(errorMsg).arg(errorLine).arg(errorColumn));
        file.close();
        return ret;
    }

    // Purge all old entities. Send events for the removal
    if (clearScene)
        RemoveAllEntities(true, change);

    return CreateContentFromXml(scene_doc, useEntityIDsFromFile, change);
}

QByteArray Scene::SerializeToXmlString(bool serializeTemporary, bool serializeLocal) const
{
    QDomDocument sceneDoc("Scene");
    QDomElement sceneElem = sceneDoc.createElement("scene");

    EntityList rootLevel = RootLevelEntities();

    for(EntityList::const_iterator iter = rootLevel.begin(); iter != rootLevel.end(); ++iter)
    {
        EntityPtr ent = *iter;
        if ((ent->IsLocal() && !serializeLocal) || (ent->IsTemporary() && !serializeTemporary))
            continue;
        ent->SerializeToXML(sceneDoc, sceneElem, serializeTemporary);
    }

    sceneDoc.appendChild(sceneElem);

    return sceneDoc.toByteArray();
}

bool Scene::SaveSceneXML(const QString& filename, bool saveTemporary, bool saveLocal)
{
    QByteArray bytes = SerializeToXmlString(saveTemporary, saveLocal);
    QFile scenefile(filename);
    if (scenefile.open(QFile::WriteOnly))
    {
        scenefile.write(bytes);
        scenefile.close();
        return true;
    }
    else
    {
        LogError("SaveSceneXML: Failed to open file " + filename + " for writing.");
        return false;
    }
}

QList<Entity *> Scene::LoadSceneBinary(const QString& filename, bool clearScene, bool useEntityIDsFromFile, AttributeChange::Type change)
{
    QList<Entity *> ret;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        LogError("Scene::LoadSceneBinary: Failed to open file " + filename + " for reading.");
        return ret;
    }

    ///\todo Use Latin 1?
    QByteArray bytes = file.readAll();
    file.close();

    if (!bytes.size())
    {
        LogError("Scene::LoadSceneBinary: File " + filename + " contained 0 bytes when loading scene binary.");
        return ret;
    }

    if (clearScene)
        RemoveAllEntities(true, change);

    return CreateContentFromBinary(bytes.data(), bytes.size(), useEntityIDsFromFile, change);
}

bool Scene::SaveSceneBinary(const QString& filename, bool getTemporary, bool getLocal) const
{
    QByteArray bytes;
    // Assume 4MB max for now
    bytes.resize(4 * 1024 * 1024);
    DataSerializer dest(bytes.data(), bytes.size());
    
    // Count number of entities we accept
    uint num_entities = 0;
    EntityList rootLevel = RootLevelEntities();

    for(EntityList::const_iterator iter = rootLevel.begin(); iter != rootLevel.end(); ++iter)
    {
        bool serialize = true;
        EntityPtr ent = *iter;
        if (ent->IsLocal() && !getLocal)
            serialize = false;
        if (ent->IsTemporary() && !getTemporary)
            serialize = false;
        if (serialize)
            ++num_entities;
    }
    
    dest.Add<u32>(num_entities);

    for(EntityList::const_iterator iter = rootLevel.begin(); iter != rootLevel.end(); ++iter)
    {
        bool serialize = true;
        EntityPtr ent = *iter;
        if (ent->IsLocal() && !getLocal)
            serialize = false;
        if (ent->IsTemporary() && !getTemporary)
            serialize = false;
        if (serialize)
            ent->SerializeToBinary(dest, getTemporary);
    }
    
    bytes.resize(static_cast<int>(dest.BytesFilled()));
    QFile scenefile(filename);
    if (scenefile.open(QFile::WriteOnly))
    {
        scenefile.write(bytes);
        scenefile.close();
        return true;
    }
    else
    {
        LogError("Scene::SaveSceneBinary: Could not open file " + filename + " for writing when saving scene binary.");
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
        LogError(QString("Scene::CreateContentFromXml: Parsing scene XML from text failed when loading Scene XML: %1 at line %2 column %3.")
            .arg(errorMsg).arg(errorLine).arg(errorColumn));
        return ret;
    }

    return CreateContentFromXml(scene_doc, useEntityIDsFromFile, change);
}

QList<Entity *> Scene::CreateContentFromXml(const QDomDocument &xml, bool useEntityIDsFromFile, AttributeChange::Type change)
{
    /// @todo Use and test parentTracker_ when !IsAuthority() (client side mass imports). See CreateContentFromSceneDesc().

    /// @todo Make server fix any broken parenting when it changes the entity IDs from unacked to replicated!
    if (!IsAuthority() && !useEntityIDsFromFile)
        LogWarning("Scene::CreateContentFromXml: The created entitity IDs need to be verified from the server. This will break EC_Placeable parenting.");

    std::vector<EntityWeakPtr> entities;
    
    // Check for existence of the scene element before we begin
    QDomElement scene_elem = xml.firstChildElement("scene");
    if (scene_elem.isNull())
    {
        LogError("Scene::CreateContentFromXml: Could not find 'scene' element from XML.");
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
        CreateEntityFromXml(EntityPtr(), ent_elem, useEntityIDsFromFile, change, entities, oldToNewIds);
        ent_elem = ent_elem.nextSiblingElement("entity");
    }

    // Fix parent ref of EC_Placeable if new entity IDs were generated.
    // This should be done first so that we wont be firing signals
    // with partially updated state (these ends are already in the scene for querying).
    if (!useEntityIDsFromFile)
        FixPlaceableParentIds(entities, oldToNewIds, AttributeChange::Disconnected);

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
    {
        if (!entities[i].expired())
            ret.append(entities[i].lock().get());
    }
    
    return ret;
}

void Scene::CreateEntityFromXml(EntityPtr parent, const QDomElement& ent_elem, bool useEntityIDsFromFile, AttributeChange::Type change, std::vector<EntityWeakPtr>& entities, QHash<entity_id_t, entity_id_t>& oldToNewIds)
{
    const bool replicated = ParseBool(ent_elem.attribute("sync"), true);
    const bool temporary = ParseBool(ent_elem.attribute("temporary"), false);

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

    EntityPtr entity;
    if (!parent)
        entity = CreateEntity(id);
    else
        entity = parent->CreateChild(id);

    if (entity)
    {
        entity->SetTemporary(temporary);

        QDomElement comp_elem = ent_elem.firstChildElement("component");
        while(!comp_elem.isNull())
        {
            const QString typeName = comp_elem.attribute("type");
            const u32 typeId = ParseUInt(comp_elem.attribute("typeId"), 0xffffffff);
            const QString name = comp_elem.attribute("name");
            const bool compReplicated = ParseBool(comp_elem.attribute("sync"), true);
            const bool temporary = ParseBool(comp_elem.attribute("temporary"), false);

            // If we encounter an unknown component type, now is the time to register a placeholder type for it
            // The XML holds all needed data for it, while binary doesn't
            SceneAPI* sceneAPI = framework_->Scene();
            if (!sceneAPI->IsComponentTypeRegistered(typeName))
                sceneAPI->RegisterPlaceholderComponentType(comp_elem);
            
            ComponentPtr new_comp = (!typeName.isEmpty() ? entity->GetOrCreateComponent(typeName, name, AttributeChange::Default, compReplicated) :
                entity->GetOrCreateComponent(typeId, name, AttributeChange::Default, compReplicated));
            if (new_comp)
            {
                new_comp->SetTemporary(temporary);
                new_comp->DeserializeFrom(comp_elem, AttributeChange::Disconnected);// Trigger no signal yet when scene is in incoherent state
            }

            comp_elem = comp_elem.nextSiblingElement("component");
        }
        entities.push_back(entity);
    }
    else
    {
        LogError("Scene::CreateContentFromXml: Failed to create entity with id " + QString::number(id) + "!");
    }

    // Spawn any child entities
    QDomElement childEnt_elem = ent_elem.firstChildElement("entity");
    while (!childEnt_elem.isNull())
    {
        CreateEntityFromXml(entity, childEnt_elem, useEntityIDsFromFile, change, entities, oldToNewIds);
        childEnt_elem = childEnt_elem.nextSiblingElement("entity");
    }
}


QList<Entity *> Scene::CreateContentFromBinary(const QString &filename, bool useEntityIDsFromFile, AttributeChange::Type change)
{

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        LogError("Scene::CreateContentFromBinary: Failed to open file " + filename + " when loading scene binary.");
        return QList<Entity*>();
    }

    QByteArray bytes = file.readAll();
    file.close();
    
    if (!bytes.size())
    {
        LogError("Scene::CreateContentFromBinary: File " + filename + "contained 0 bytes when loading scene binary.");
        return QList<Entity*>();
    }

    return CreateContentFromBinary(bytes.data(), bytes.size(), useEntityIDsFromFile, change);
}

QList<Entity *> Scene::CreateContentFromBinary(const char *data, int numBytes, bool useEntityIDsFromFile, AttributeChange::Type change)
{
    /// @todo Use and test parentTracker_ when !IsAuthority() (client side mass imports). See CreateContentFromSceneDesc().

    /// @todo Make server fix any broken parenting when it changes the entity IDs from unacked to replicated!
    if (!IsAuthority() && !useEntityIDsFromFile)
        LogWarning("Scene::CreateContentFromBinary: The created entitity IDs need to be verified from the server. This will break EC_Placeable parenting.");

    assert(data);
    assert(numBytes > 0);

    std::vector<EntityWeakPtr> entities;
    QHash<entity_id_t, entity_id_t> oldToNewIds;

    try
    {
        DataDeserializer source(data, numBytes);

        uint num_entities = source.Read<u32>();
        for(uint i = 0; i < num_entities; ++i)
            CreateEntityFromBinary(EntityPtr(), source, useEntityIDsFromFile, change, entities, oldToNewIds);
    }
    catch(...)
    {
        // Note: if exception happens, no change signals are emitted
        return QList<Entity *>();
    }

    // Fix parent ref of EC_Placeable if new entity IDs were generated.
    // This should be done first so that we wont be firing signals
    // with partially updated state (these ends are already in the scene for querying).
    if (!useEntityIDsFromFile)
        FixPlaceableParentIds(entities, oldToNewIds, AttributeChange::Disconnected);

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

void Scene::CreateEntityFromBinary(EntityPtr parent, kNet::DataDeserializer& source, bool useEntityIDsFromFile, AttributeChange::Type change, std::vector<EntityWeakPtr>& entities, QHash<entity_id_t, entity_id_t>& oldToNewIds)
{
    entity_id_t id = source.Read<u32>();
    bool replicated = source.Read<u8>() ? true : false;
    if (!useEntityIDsFromFile || id == 0)
    {
        entity_id_t originalId = id;
        id = replicated ? NextFreeId() : NextFreeIdLocal();
        if (originalId != 0 && !oldToNewIds.contains(originalId))
            oldToNewIds[originalId] = id;
    }
    else if (useEntityIDsFromFile && HasEntity(id))
    {
        entity_id_t newID = replicated ? NextFreeId() : NextFreeIdLocal();
        ChangeEntityId(id, newID);
    }

    if (HasEntity(id)) // If the entity we are about to add conflicts in ID with an existing entity in the scene.
    {
        LogDebug("Scene::CreateContentFromBinary: Destroying previous entity with id " + QString::number(id) + " to avoid conflict with new created entity with the same id.");
        LogError("Warning: Invoking buggy behavior: Object with id " + QString::number(id) + "might not replicate properly!");
        RemoveEntity(id, AttributeChange::Replicate); ///<@todo Consider do we want to always use Replicate
    }

    EntityPtr entity;
    if (!parent)
        entity = CreateEntity(id);
    else
        entity = parent->CreateChild(id);

    if (!entity)
    {
        LogError("Scene::CreateEntityFromBinary: Failed to create entity.");
        return;
    }
    
    uint num_components = source.Read<u32>();
    uint num_childEntities = num_components >> 16;
    num_components &= 0xffff;

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
                LogError("Scene::CreateEntityFromBinary: Failed to load component \"" + framework_->Scene()->GetComponentTypeName(typeId) + "\"!");
        }
        catch(...)
        {
            LogError("Scene::CreateEntityFromBinary: Failed to load component \"" + framework_->Scene()->GetComponentTypeName(typeId) + "\"!");
        }
    }

    entities.push_back(entity);

    for (uint i = 0; i < num_childEntities; ++i)
        CreateEntityFromBinary(entity, source, useEntityIDsFromFile, change, entities, oldToNewIds);
}

QList<Entity *> Scene::CreateContentFromSceneDesc(const SceneDesc &desc, bool useEntityIDsFromFile, AttributeChange::Type change)
{
    QList<Entity *> ret;
    if (desc.entities.empty())
    {
        LogError("Scene::CreateContentFromSceneDesc: Empty scene description.");
        return ret;
    }
    if (!IsAuthority() && parentTracker_.IsTracking())
    {
        LogError("Scene::CreateContentFromSceneDesc: Still waiting for previous content creation to complete on the server. Try again after it completes.");
        return ret;
    }

    // Sort the entity list so that parents are before children.
    // This is done to combat the runtime detection of "parent entity/placeable created"
    // that slows down the import considerably on large scenes that relies heavily on parenting.
    EntityDescList iterDescEntities = desc.entities;
    EntityDescList sortedDescEntities;

    // Put Entities that have children first (real Entity level parenting).
    for (int ei=0; ei<iterDescEntities.size(); ++ei)
    {
        const EntityDesc &ent = iterDescEntities[ei];
        if (!ent.children.isEmpty())
        {
            // This does not yet mean its a root level Entity.
            // Check for deeper hierarchies: parent -> child with children.
            int insertIndex = -1;
            for (int sorti=0, sortlen=sortedDescEntities.size(); sorti<sortlen; ++sorti)
            {
                const EntityDesc &parentCandidate = sortedDescEntities[sorti];
                if (parentCandidate.IsParentFor(ent))
                {
                    insertIndex = sorti + 1;
                    break;
                }
            }
            if (insertIndex == -1 || insertIndex >= sortedDescEntities.size())
                sortedDescEntities.push_back(ent);
            else
                sortedDescEntities.insert(insertIndex, ent);

            iterDescEntities.removeAt(ei);
            ei--;
        }
    }

    // Find entities that have parent ref set (EC_Placeble::parentRef parenting).
    int childrenStartIndex = sortedDescEntities.size();
    for (int ei=0; ei<iterDescEntities.size(); ++ei)
    {
        int insertIndex = -1;
        const EntityDesc &ent = iterDescEntities[ei];

        entity_id_t parentId = PlaceableParentId(ent);
        if (parentId > 0)
        {
            // Find parent from the list and insert after, othewise add to end.
            QString parentIdStr = QString::number(parentId);
            for (int sorti=childrenStartIndex, sortlen=sortedDescEntities.size(); sorti<sortlen; ++sorti)
            {
                const EntityDesc &parentCandidate = sortedDescEntities[sorti];
                if (parentCandidate.id == parentIdStr)
                {
                    insertIndex = sorti + 1;
                    break;
                }
            }
        }
        if (insertIndex == -1 || insertIndex >= sortedDescEntities.size())
            sortedDescEntities.push_back(ent);
        else
            sortedDescEntities.insert(insertIndex, ent);

        iterDescEntities.removeAt(ei);
        ei--;
    }

    // Double check no information was lost. If these do not match, use the original passed in entity desc list.
    if (!iterDescEntities.isEmpty() || sortedDescEntities.size() != desc.entities.size())
    {
        LogError("Scene::CreateContentFromSceneDesc: Sorting Entity hierarchy resulted in loss of information. Using original unsorted Entity list. " +
            QString("Iteration list size: %1 Sorted Entities: %2 Original Entities: %3")
                .arg(iterDescEntities.size()).arg(sortedDescEntities.size()).arg(desc.entities.size()));
        sortedDescEntities = desc.entities;
    }

    QHash<entity_id_t, entity_id_t> oldToNewIds;
    for (int ei=0, eilen=sortedDescEntities.size(); ei<eilen; ++ei)
        CreateEntityFromDesc(EntityPtr(), sortedDescEntities[ei], useEntityIDsFromFile, change, ret, oldToNewIds);

    // Fix parent ref of EC_Placeable if new entity IDs were generated.
    // This should be done first so that we wont be firing signals
    // with partially updated state (these ends are already in the scene for querying).
    if (!useEntityIDsFromFile)
        FixPlaceableParentIds(ret, oldToNewIds, AttributeChange::Disconnected);

    // All entities & components have been loaded. Trigger change for them now.
    foreach(Entity *entity, ret)
    {
        // On a client start tracking of the server ack messages.
        if (!IsAuthority())
            parentTracker_.Track(entity);

        // Entity
        EmitEntityCreated(entity, change);

        // Components
        const Entity::ComponentMap &components = entity->Components();
        for(Entity::ComponentMap::const_iterator i = components.begin(); i != components.end(); ++i)
            i->second->ComponentChanged(change);
    }

    return ret;
}

void Scene::CreateEntityFromDesc(EntityPtr parent, const EntityDesc& e, bool useEntityIDsFromFile, AttributeChange::Type change, QList<Entity *>& entities, QHash<entity_id_t, entity_id_t>& oldToNewIds)
{
    entity_id_t id = static_cast<entity_id_t>(e.id.toInt());

    if (e.id.isEmpty() || !useEntityIDsFromFile)
    {
        entity_id_t originaId = id;
        id = e.local ? NextFreeIdLocal() : NextFreeId();
        if (originaId != 0 && !oldToNewIds.contains(originaId))
            oldToNewIds[originaId] = id;
    }

    if (HasEntity(id)) // If the entity we are about to add conflicts in ID with an existing entity in the scene.
    {
        LogDebug("Scene::CreateEntityFromDesc: Destroying previous entity with id " + QString::number(id) + " to avoid conflict with new created entity with the same id.");
        LogError("Warning: Invoking buggy behavior: Object with id " + QString::number(id) + " might not replicate properly!");
        RemoveEntity(id, AttributeChange::Replicate); ///<@todo Consider do we want to always use Replicate
    }

    EntityPtr entity;
    if (!parent)
        entity = CreateEntity(id);
    else
        entity = parent->CreateChild(id);

    assert(entity);
    if (entity)
    {
        foreach(const ComponentDesc &c, e.components)
        {
            if (c.typeName.isEmpty())
                continue;

            // If we encounter an unknown component type, now is the time to register a placeholder type for it
            // The componentdesc holds all needed data for it
            SceneAPI* sceneAPI = framework_->Scene();
            if (!sceneAPI->IsComponentTypeRegistered(c.typeName))
                sceneAPI->RegisterPlaceholderComponentType(c);

            ComponentPtr comp = entity->GetOrCreateComponent(c.typeName, c.name);
            assert(comp);
            if (!comp)
            {
                LogError(QString("Scene::CreateEntityFromDesc: failed to create component %1 %2 .").arg(c.typeName).arg(c.name));
                continue;
            }
            if (comp->TypeId() == 25 /*EC_DynamicComponent*/)
            {
                QDomDocument temp_doc;
                QDomElement root_elem = temp_doc.createElement("component");
                root_elem.setAttribute("typeId", QString::number(c.typeId)); // Ambiguous on VC9 as u32
                root_elem.setAttribute("type", c.typeName);
                root_elem.setAttribute("name", c.name);
                root_elem.setAttribute("sync", c.sync);
                foreach(const AttributeDesc &a, c.attributes)
                {
                    QDomElement child_elem = temp_doc.createElement("attribute");
                    child_elem.setAttribute("id", a.id);
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
                {
                    if (!attr)
                        continue;
                    foreach(const AttributeDesc &a, c.attributes)
                    {
                        if (attr->TypeName().compare(a.typeName, Qt::CaseInsensitive) == 0 &&
                            (attr->Id().compare(a.id, Qt::CaseInsensitive) == 0 ||
                             attr->Name().compare(a.name, Qt::CaseInsensitive) == 0))
                        {
                           attr->FromString(a.value, AttributeChange::Disconnected); // Trigger no signal yet when scene is in incoherent state
                        }
                    }
                }                   
            }
        }

        entity->SetTemporary(e.temporary);
        entities.append(entity.get());

        // Create child entities recursively
        foreach(const EntityDesc &ce, e.children)
            CreateEntityFromDesc(entity, ce, useEntityIDsFromFile, change, entities, oldToNewIds);
    }
}

SceneDesc Scene::CreateSceneDescFromXml(const QString &filename) const
{
    SceneDesc sceneDesc;
    if (!filename.endsWith(".txml", Qt::CaseInsensitive))
    {
        if (filename.endsWith(".tbin", Qt::CaseInsensitive))
            LogError("Scene::CreateSceneDescFromXml: Try using CreateSceneDescFromBinary() instead for " + filename);
        else
            LogError("Scene::CreateSceneDescFromXml: Unsupported file extension " + filename + " when trying to create scene description.");
        return sceneDesc;
    }

    sceneDesc.filename = filename;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        LogError("Scene::CreateSceneDescFromXml: Failed to open file " + filename + " when trying to create scene description.");
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
        LogError(QString("Scene::CreateSceneDescFromXml: Parsing scene XML from %1 failed when loading Scene XML: %2 at line %3 column %4.")
            .arg(sceneDesc.filename).arg(errorMsg).arg(errorLine).arg(errorColumn));
        return sceneDesc;
    }

    // Check for existence of the scene element before we begin
    QDomElement scene_elem = scene_doc.firstChildElement("scene");
    if (scene_elem.isNull())
    {
        LogError("Scene::CreateSceneDescFromXml: Could not find 'scene' element from XML.");
        return sceneDesc;
    }

    QDomElement ent_elem = scene_elem.firstChildElement("entity");
    while(!ent_elem.isNull())
    {
        CreateEntityDescFromXml(sceneDesc, sceneDesc.entities, ent_elem);
        // Process siblings.
        ent_elem = ent_elem.nextSiblingElement("entity");
    }

    return sceneDesc;
}

void Scene::CreateEntityDescFromXml(SceneDesc& sceneDesc, QList<EntityDesc>& dest, const QDomElement& ent_elem) const
{
    QString id_str = ent_elem.attribute("id");
    if (!id_str.isEmpty())
    {
        EntityDesc entityDesc;
        entityDesc.id = id_str;
        entityDesc.local = !ParseBool(ent_elem.attribute("sync"), true); /**< @todo if no "sync"* attr, deduct from the ID. */
        entityDesc.temporary = ParseBool(ent_elem.attribute("temporary"), false);

        QDomElement comp_elem = ent_elem.firstChildElement("component");
        while(!comp_elem.isNull())
        {
            ComponentDesc compDesc;
            compDesc.typeName = comp_elem.attribute("type");
            compDesc.typeId = ParseUInt(comp_elem.attribute("typeId"), 0xffffffff);
            /// @todo 27.09.2013 assert that typeName and typeId match
            /// @todo 27.09.2013 If mismatch, show warning, and use SceneAPI's
            /// ComponentTypeNameForTypeId and ComponentTypeIdForTypeName to resolve one or the other?
            compDesc.name = comp_elem.attribute("name");
            compDesc.sync = ParseBool(comp_elem.attribute("sync"));
            const bool hasTypeId = compDesc.typeId != 0xffffffff;

            // A bit of a hack to get the name from EC_Name.
            if (entityDesc.name.isEmpty() && (compDesc.typeId == EC_Name::ComponentTypeId ||
                IComponent::EnsureTypeNameWithPrefix(compDesc.typeName) == EC_Name::TypeNameStatic()))
            {
                ComponentPtr comp = (hasTypeId ? framework_->Scene()->CreateComponentById(0, compDesc.typeId, compDesc.name) :
                    framework_->Scene()->CreateComponentByName(0, compDesc.typeName, compDesc.name));
                EC_Name *ecName = checked_static_cast<EC_Name*>(comp.get());
                ecName->DeserializeFrom(comp_elem, AttributeChange::Disconnected);
                entityDesc.name = ecName->name.Get();
                entityDesc.group = ecName->group.Get();
            }

            ComponentPtr comp = (hasTypeId ? framework_->Scene()->CreateComponentById(0, compDesc.typeId, compDesc.name) :
                framework_->Scene()->CreateComponentByName(0, compDesc.typeName, compDesc.name));
            if (!comp) // Move to next element if component creation fails.
            {
                comp_elem = comp_elem.nextSiblingElement("component");
                continue;
            }

            // Find asset references.
            comp->DeserializeFrom(comp_elem, AttributeChange::Disconnected);
            foreach(IAttribute *a,comp->Attributes())
            {
                if (!a)
                    continue;
                    
                const QString typeName = a->TypeName();
                AttributeDesc attrDesc = { typeName, a->Name(), a->ToString(), a->Id() };
                compDesc.attributes.append(attrDesc);

                QString attrValue = a->ToString();
                if ((typeName.compare("AssetReference", Qt::CaseInsensitive) == 0 || typeName.compare("AssetReferenceList", Qt::CaseInsensitive) == 0|| 
                    (a->Metadata() && a->Metadata()->elementType.compare("AssetReference", Qt::CaseInsensitive) == 0)) &&
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

        // Process child entities
        QDomElement childEnt_elem = ent_elem.firstChildElement("entity");
        while (!childEnt_elem.isNull())
        {
            CreateEntityDescFromXml(sceneDesc, entityDesc.children, childEnt_elem);
            childEnt_elem = childEnt_elem.nextSiblingElement("entity");
        }

        dest.append(entityDesc);
    }
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

            regex expression("!ref:\\s*(.*?)\\s*(\\n|\\r|$)");
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
            LogError("Scene::CreateSceneDescFromBinary: Try using CreateSceneDescFromXml() instead for " + filename);
        else
            LogError("Scene::CreateSceneDescFromBinary: Unsupported file extension : " + filename + " when trying to create scene description.");
        return sceneDesc;
    }

    sceneDesc.filename = filename;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        LogError("Scene::CreateSceneDescFromBinary: Failed to open file " + filename + " when trying to create scene description.");
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
        LogError("Scene::CreateSceneDescFromBinary: File " + sceneDesc.filename + " contained 0 bytes when trying to create scene description.");
        return sceneDesc;
    }

    try
    {
        DataDeserializer source(bytes.data(), bytes.size());
        
        const uint num_entities = source.Read<u32>();
        for(uint i = 0; i < num_entities; ++i)
        {
            EntityDesc entityDesc;
            entity_id_t id = source.Read<u32>();
            entityDesc.id = QString::number(static_cast<uint>(id));

            const uint num_components = source.Read<u32>();
            for(uint i = 0; i < num_components; ++i)
            {
                SceneAPI *sceneAPI = framework_->Scene();

                ComponentDesc compDesc;
                compDesc.typeId = source.Read<u32>(); /**< @todo VLE this! */
                compDesc.typeName = sceneAPI->ComponentTypeNameForTypeId(compDesc.typeId);
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
                    ComponentPtr comp = sceneAPI->CreateComponentById(0, compDesc.typeId, compDesc.name);
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
                                AttributeDesc attrDesc = { typeName, a->Name(), a->ToString(), a->Id() };
                                compDesc.attributes.append(attrDesc);

                                QString attrValue = a->ToString();
                                if ((typeName.compare("AssetReference", Qt::CaseInsensitive) == 0 || typeName.compare("AssetReferenceList", Qt::CaseInsensitive) == 0 || 
                                    (a->Metadata() && a->Metadata()->elementType.compare("AssetReference", Qt::CaseInsensitive) == 0)) &&
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
                    {
                        LogError(QString("Scene::CreateSceneDescFromBinary: Failed to load component %1 %2!").
                            arg(compDesc.typeName).arg(!compDesc.name.isEmpty() ? "\"" + compDesc.name + "\"" : ""));
                    }
                }
                catch(...)
                {
                    LogError(QString("Scene::CreateSceneDescFromBinary: Exception while trying to load component %1 %2!").
                        arg(compDesc.typeName).arg(!compDesc.name.isEmpty() ? "\"" + compDesc.name + "\"" : ""));
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
        entity_elem.setAttribute("id", QString::number(static_cast<uint>(entity->Id())));
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
    newInterp.dest = AttributeWeakPtr(comp->shared_from_this(), attr);
    newInterp.start = AttributeWeakPtr(comp->shared_from_this(), attr->Clone());
    newInterp.end = AttributeWeakPtr(comp->shared_from_this(), endvalue);
    newInterp.length = length;
    
    interpolations_.push_back(newInterp);
    return true;
}

bool Scene::EndAttributeInterpolation(IAttribute* attr)
{
    for(uint i = 0; i < interpolations_.size(); ++i)
    {
        AttributeInterpolation& interp = interpolations_[i];
        if (interp.dest.Get() == attr)
        {
            delete interp.start.Get();
            delete interp.end.Get();
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
        delete interp.start.Get();
        delete interp.end.Get();
    }
    
    interpolations_.clear();
}

void Scene::UpdateAttributeInterpolations(float frametime)
{
    PROFILE(Scene_UpdateInterpolation);
    
    interpolating_ = true;
    
    for(size_t i = interpolations_.size() - 1; i < interpolations_.size(); --i)
    {
        AttributeInterpolation& interp = interpolations_[i];
        bool finished = false;
        
        // Check that the component still exists i.e. it's safe to access the attribute
        if (!interp.start.owner.expired())
        {
            // Allow the interpolation to persist for 2x time, though we are no longer setting the value
            // This is for the continuous/discontinuous update detection in StartAttributeInterpolation()
            if (interp.time <= interp.length)
            {
                interp.time += frametime;
                float t = interp.time / interp.length;
                if (t > 1.0f)
                    t = 1.0f;
                interp.dest.Get()->Interpolate(interp.start.Get(), interp.end.Get(), t, AttributeChange::LocalOnly);
            }
            else
            {
                interp.time += frametime;
                if (interp.time >= interp.length * 2.0f)
                    finished = true;
            }
        }
        else // Component pointer has expired, abort this interpolation
            finished = true;
        
        // Remove interpolation (& delete start/endpoints) when done
        if (finished)
        {
            delete interp.start.Get();
            delete interp.end.Get();
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

    for(const_iterator it = begin(); it != end(); ++it)
    {
        EntityPtr entity = it->second;
        if (pattern.exactMatch(entity->Name()))
            entities.push_back(entity);
    }

    return entities;
}

EntityList Scene::FindEntitiesContaining(const QString &substring, Qt::CaseSensitivity sensitivity) const
{
    EntityList entities;
    if (substring.isEmpty())
        return entities;

    for(const_iterator it = begin(); it != end(); ++it)
    {
        EntityPtr entity = it->second;
        if (entity->Name().contains(substring, sensitivity))
            entities.push_back(entity);
    }

    return entities;
}

EntityList Scene::FindEntitiesByName(const QString &name, Qt::CaseSensitivity sensitivity) const
{
    // Don't check if name is empty, we want to allow querying for all entities without a name too.
    EntityList entities;
    for(const_iterator it = begin(); it != end(); ++it)
    {
        EntityPtr entity = it->second;
        if (entity->Name().compare(name, sensitivity) == 0)
            entities.push_back(entity);
    }

    return entities;
}

EntityList Scene::RootLevelEntities() const
{
    EntityList entities;
    for(const_iterator it = begin(); it != end(); ++it)
    {
        EntityPtr entity = it->second;
        if (!entity->Parent())
            entities.push_back(entity);
    }

    return entities;
}

entity_id_t Scene::PlaceableParentId(const EntityDesc &ent)
{
    // Find placeable typeId 20
    for (int ci=0, cilen=ent.components.size(); ci<cilen; ++ci)
    {
        const ComponentDesc &comp = ent.components[ci];
        if (comp.typeId == 20)
        {
            entity_id_t parentId = 0;
            const QString attrId = "parentRef";

            // Find attribute "parentRef"
            for (int ai=0, ailen=comp.attributes.size(); ai<ailen; ++ai)
            {
                const AttributeDesc &attr = comp.attributes[ai];
                if (attr.id.compare(attrId, Qt::CaseSensitive) == 0)
                {
                    bool isNumber = false;
                    parentId = attr.value.toUInt(&isNumber);
                    if (!isNumber)
                        parentId = 0;
                    break;
                }
            }
            return parentId;
        }
    }
    return 0;
}

void Scene::FixPlaceableParentIds(const std::vector<EntityWeakPtr> entities, const QHash<entity_id_t, entity_id_t> &oldToNewIds, AttributeChange::Type change)
{
    QList<Entity*> rawEntities;
    for (size_t i=0, len=entities.size(); i<len; ++i)
    {
        if (!entities[i].expired())
            rawEntities << entities[i].lock().get();
    }
    FixPlaceableParentIds(rawEntities, oldToNewIds, change);
}

void Scene::FixPlaceableParentIds(const QList<Entity*> entities, const QHash<entity_id_t, entity_id_t> &oldToNewIds, AttributeChange::Type change)
{
    foreach(Entity *entity, entities)
    {
        if (!entity)
            continue;

        ComponentPtr placeable = entity->Component(20); // EC_Placeable
        Attribute<EntityReference> *parentRef = (placeable.get() ? dynamic_cast<Attribute<EntityReference> *>(placeable->AttributeById("parentRef")) : 0);
        if (parentRef && !parentRef->Get().IsEmpty())
        {
            // We only need to fix the id parent refs.
            // Ones with entity names should work as expected.
            bool isNumber = false;
            entity_id_t refId = parentRef->Get().ref.toUInt(&isNumber);
            if (isNumber && refId > 0 && oldToNewIds.contains(refId))
            {
                // Below ComponentChanged will trigger the right signals.
                // This cannot be allowed to emit signals as EmitEntityCreated is not yet fired.
                parentRef->Set(EntityReference(oldToNewIds[refId]), change);
            }
        }
    }
}
