// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "SyncState.h"

#include "Scene.h"
#include "Entity.h"
#include "IComponent.h"
#include "Profiler.h"

#include "LoggingFunctions.h"

typedef std::map<entity_id_t, ComponentIdList >::const_iterator PendingConstIter;
typedef std::map<entity_id_t, ComponentIdList >::iterator PendingIter;

SceneSyncState::SceneSyncState(int userConnectionID, bool isServer) :
    userConnectionID_(userConnectionID),
    changeRequest_(userConnectionID),
    isServer_(isServer)
{
    Clear();
}

SceneSyncState::~SceneSyncState()
{
}

// Public slots

QVariantList SceneSyncState::PendingEntityIDs() const
{
    QVariantList list;
    PendingConstIter iter = pendingComponents.begin();
    PendingConstIter end = pendingComponents.end();
    while (iter != end)
    {
        entity_id_t id = iter->first;
        if (!list.contains(id))
            list << id;
        ++iter;
    }
    return list;
}

entity_id_t SceneSyncState::NextPendingEntityID() const
{
    if (pendingComponents.empty())
        return 0;
    PendingConstIter front = pendingComponents.begin();
    if (front == pendingComponents.end())
        return 0;
    return front->first;
}

bool SceneSyncState::HasPendingEntities() const
{
    return !pendingComponents.empty();
}

bool SceneSyncState::HasPendingEntity(entity_id_t id) const
{
    PendingConstIter iter = pendingComponents.find(id);
    if (iter == pendingComponents.end())
        return false;
    // We should never have a empty list in the map for any entity!
    if (iter->second.empty())
        LogWarning("SceneSyncState::HasPendingComponents(): Pending map has entity with empty list of pending components!");
    return !iter->second.empty();
}

void SceneSyncState::MarkEntityPending(entity_id_t id)
{
    if (!isServer_)
        return;

    // If user does not have the entity in the first place, do nothing.
    // Its going to be asked to be added to the state via the permission signals later.
    std::map<entity_id_t, EntitySyncState>::iterator i = entities.find(id);
    if (i == entities.end())
        return;

    MarkEntityRemoved(id);      // Remove from current sync state (removes entity from client)
    FillPendingComponents(id);  // Mark all components from the entity as pending
}

void SceneSyncState::MarkPendingEntitiesDirty()
{
    if (!isServer_)
        return;

    QList<entity_id_t> entIds;
    PendingIter iter = pendingComponents.begin();
    PendingIter end = pendingComponents.end();
    while (iter != end)
    {
        entIds << iter->first;
        ++iter;
    }

    foreach(entity_id_t entId, entIds)
        MarkPendingEntityDirty(entId);
}

void SceneSyncState::MarkPendingEntityDirty(entity_id_t id)
{
    if (!isServer_)
        return;

    // If this entity has no components in pending state,
    // we should not proceed as otherwise this 
    // might be called with a local entity id.
    if (!HasPendingEntity(id))
        return;

    // Above ensures iter to be valid and list not being empty.
    PendingConstIter iter = pendingComponents.find(id);
    ComponentIdList compList = iter->second; 

    EntitySyncState& entityState = MarkEntityDirtySilent(id);
    for (ComponentIdList::const_iterator compIter = compList.begin(); compIter != compList.end(); ++compIter)
        entityState.MarkComponentDirty((*compIter));

    RemovePendingEntity(id);
}

// Public

void SceneSyncState::SetParentScene(SceneWeakPtr scene)
{
    scene_ = scene;
}

void SceneSyncState::Clear()
{
    dirtyQueue.clear();
    entities.clear();
    pendingComponents.clear();
    changeRequest_.Reset();
    scene_.reset();
}

void SceneSyncState::RemoveFromQueue(entity_id_t id)
{
    std::map<entity_id_t, EntitySyncState>::iterator i = entities.find(id);
    if (i != entities.end())
    {
        if (i->second.isInQueue)
        {
            for (std::list<EntitySyncState*>::iterator j = dirtyQueue.begin(); j != dirtyQueue.end(); ++j)
            {
                if ((*j) == &i->second)
                {
                    dirtyQueue.erase(j);
                    break;
                }
            }
            i->second.isInQueue = false;
            for (std::map<component_id_t, ComponentSyncState>::iterator j = i->second.components.begin(); j != i->second.components.end(); ++j)
                j->second.isInQueue = false;
            i->second.dirtyQueue.clear();
        }
    }
}

void SceneSyncState::MarkEntityProcessed(entity_id_t id)
{
    EntitySyncState& entityState = entities[id];
    if (!entityState.id)
        entityState.id = id;
    entityState.DirtyProcessed();
}

void SceneSyncState::MarkComponentProcessed(entity_id_t id, component_id_t compId)
{
    EntitySyncState& entityState = entities[id];
    if (!entityState.id)
        entityState.id = id;
    ComponentSyncState& compState = entityState.components[compId];
    if (!compState.id)
        compState.id = compId;
    compState.DirtyProcessed();
}

void SceneSyncState::MarkEntityDirty(entity_id_t id)
{
    // Return if the whole entity change request was rejected
    if (isServer_ && !ShouldMarkAsDirty(id))
        return;

    EntitySyncState& entityState = entities[id]; // Creates new if did not exist
    if (!entityState.id)
        entityState.id = id;
    if (!entityState.isInQueue)
    {
        dirtyQueue.push_back(&entityState);
        entityState.isInQueue = true;
    }
}

void SceneSyncState::MarkEntityRemoved(entity_id_t id)
{
    if (isServer_)
        RemovePendingEntity(id);

    // If user did not have the entity in the first place, do nothing
    std::map<entity_id_t, EntitySyncState>::iterator i = entities.find(id);
    if (i == entities.end())
        return;
    // If entity is marked new, it was not sent yet and can be simply removed from the sync state
    if (i->second.isNew)
    {
        RemoveFromQueue(id);
        entities.erase(id);
        return;
    }
    // Else mark as removed and queue the update
    i->second.removed = true;
    if (!i->second.isInQueue)
    {
        dirtyQueue.push_back(&i->second);
        i->second.isInQueue = true;
    }
}

void SceneSyncState::MarkComponentDirty(entity_id_t id, component_id_t compId)
{
    if (isServer_ && !ShouldMarkAsDirty(id))
        return;

    MarkEntityDirty(id);
    EntitySyncState& entityState = entities[id]; // Creates new if did not exist
    if (!entityState.id)
        entityState.id = id;
    entityState.MarkComponentDirty(compId);
}

void SceneSyncState::MarkComponentRemoved(entity_id_t id, component_id_t compId)
{
    if (isServer_)
        RemovePendingComponent(id, compId);

    // If user did not have the entity or component in the first place, do nothing
    std::map<entity_id_t, EntitySyncState>::iterator i = entities.find(id);
    if (i == entities.end())
        return;
    MarkEntityDirty(id);
    i->second.MarkComponentRemoved(compId);
}

void SceneSyncState::MarkAttributeDirty(entity_id_t id, component_id_t compId, u8 attrIndex)
{
    MarkEntityDirty(id);
    EntitySyncState& entityState = entities[id];
    entityState.MarkComponentDirty(compId);
    ComponentSyncState& compState = entityState.components[compId];
    compState.MarkAttributeDirty(attrIndex);
}

void SceneSyncState::MarkAttributeCreated(entity_id_t id, component_id_t compId, u8 attrIndex)
{
    MarkEntityDirty(id);
    EntitySyncState& entityState = entities[id];
    entityState.MarkComponentDirty(compId);
    ComponentSyncState& compState = entityState.components[compId];
    compState.MarkAttributeCreated(attrIndex);
}

void SceneSyncState::MarkAttributeRemoved(entity_id_t id, component_id_t compId, u8 attrIndex)
{
    MarkEntityDirty(id);
    EntitySyncState& entityState = entities[id];
    entityState.MarkComponentDirty(compId);
    ComponentSyncState& compState = entityState.components[compId];
    compState.MarkAttributeRemoved(attrIndex);
}

// Private

bool SceneSyncState::ShouldMarkAsDirty(entity_id_t id)
{
    if (!isServer_)
        return true;

    // If already in pending state, do not request again
    if (HasPendingEntity(id))
        return false;

    PROFILE(SceneSyncState_ShouldMarkAsDirty_Entity);

    // Only request if this entity does not have a sync state yet.
    // Otherwise this id will spam the signal handler on every change if
    // the addition to sync state was accepted.
    std::map<entity_id_t, EntitySyncState>::iterator i = entities.find(id);
    if (i == entities.end())
    {
        // Scene or entity null, do not process yet.
        if (!FillRequest(id))
            return false;

        emit AboutToDirtyEntity(&changeRequest_);

        // Rejected, mark all components as pending.
        if (changeRequest_.Rejected())
            FillPendingComponents(id);
        return changeRequest_.Accepted();
    }
    return true;
}

void SceneSyncState::RemovePendingEntity(entity_id_t id)
{
    PendingIter iter = pendingComponents.find(id);
    if (iter != pendingComponents.end())
        pendingComponents.erase(iter);
}

void SceneSyncState::RemovePendingComponent(entity_id_t id, component_id_t compId)
{
    PendingIter iter = pendingComponents.find(id);
    if (iter != pendingComponents.end())
    {
        iter->second.remove(compId);
        if (iter->second.empty())
            pendingComponents.erase(iter);
    }
}

bool SceneSyncState::FillRequest(entity_id_t id)
{
    changeRequest_.Reset(id);

    if (scene_.expired())
        return false;
    ScenePtr scenePtr = scene_.lock();
    if (!scenePtr.get())
    {
        LogError("SceneSyncState::FillRequest(id): Scene is null!");
        return false;
    }

    EntityPtr entityPtr = scenePtr->GetEntity(id);
    if (!entityPtr.get())
        return false;

    // We trust the SyncManager mechanisms to stop local entities from ever getting here.
    // Print anyways if something starts to leak so at least we notice it here.
    if (!entityPtr->IsReplicated())
        LogError("SceneSyncState::FillRequest(id): Entity " + QString::number(id) + " should not be replicated!");
    changeRequest_.SetEntity(entityPtr.get());

    return true;
}

void SceneSyncState::FillPendingComponents(entity_id_t id)
{
    if (!isServer_)
        return;

    if (scene_.expired())
        return;
    ScenePtr scenePtr = scene_.lock();
    if (!scenePtr.get())
    {
        LogError("SceneSyncState::FillPendingComponents(id): Scene is null!");
        return;
    }

    EntityPtr entityPtr = scenePtr->GetEntity(id);
    if (!entityPtr.get())
    {
        LogError("SceneSyncState::FillPendingComponents(id): Entity is null, cannot fill components!");
        return;
    }
    if (entityPtr->Components().empty())
        return;

    ComponentIdList& pendingComps = pendingComponents[id]; // Creates new if did not exist
    pendingComps.clear();

    Entity::ComponentMap::const_iterator iter = entityPtr->Components().begin();
    Entity::ComponentMap::const_iterator end = entityPtr->Components().end();
    while (iter != end)
    {
        component_id_t compId = iter->second->Id();
        pendingComps.push_back(compId);
        ++iter;
    }
}

EntitySyncState& SceneSyncState::MarkEntityDirtySilent(entity_id_t id)
{
    EntitySyncState& entityState = entities[id]; // Creates new if did not exist
    if (!entityState.id)
        entityState.id = id;
    if (!entityState.isInQueue)
    {
        dirtyQueue.push_back(&entityState);
        entityState.isInQueue = true;
    }
    return entityState;
}
