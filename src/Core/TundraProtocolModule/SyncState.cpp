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

QVariantList SceneSyncState::PendingComponentIDs(entity_id_t id) const
{
    QVariantList list;
    PendingConstIter iter = pendingComponents.find(id);
    if (iter == pendingComponents.end())
        return list;
    if (iter->second.empty())
        return list;

    ComponentIdList::const_iterator compIter = iter->second.begin();
    ComponentIdList::const_iterator end = iter->second.end();
    while (compIter != end)
    {
        component_id_t id = (*compIter);
        if (!list.contains(id))
            list.append(id);
        ++compIter;
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

component_id_t SceneSyncState::NextPendingComponentID(entity_id_t id) const
{
    PendingConstIter iter = pendingComponents.find(id);
    if (iter == pendingComponents.end())
        return 0;
    if (iter->second.empty())
        return 0;
    component_id_t compId = iter->second.front();
    return compId;
}

bool SceneSyncState::HasPendingEntities() const
{
    return !pendingComponents.empty();
}

bool SceneSyncState::HasPendingEntity(entity_id_t id) const
{
    return HasPendingComponents(id);
}

bool SceneSyncState::HasPendingComponents(entity_id_t id) const
{
    PendingConstIter iter = pendingComponents.find(id);
    if (iter == pendingComponents.end())
        return false;
    // We should never have a empty list in the map for any entity!
    if (iter->second.empty())
        LogWarning("SceneSyncState::HasPendingComponents(): Pending map has entity with empty list of pending components!");
    return !iter->second.empty();
}

bool SceneSyncState::HasPendingComponent(entity_id_t id, component_id_t compId) const
{
    PendingConstIter iter = pendingComponents.find(id);
    if (iter == pendingComponents.end())
        return false;
    ComponentIdList compList = iter->second;
    // We should never have a empty list in the map for any entity!
    if (compList.empty())
    {
        LogWarning("SceneSyncState::HasPendingComponent(): Pending map has entity with empty list of pending components!");
        return false;
    }
    for (ComponentIdList::const_iterator compIter = compList.begin(); compIter != compList.end(); ++compIter)
        if ((*compIter) == compId)
            return true;
    return false;
}

void SceneSyncState::MarkPendingEntitiesDirty()
{
    if (!isServer_)
        return;

    PendingIter iter = pendingComponents.begin();
    PendingIter end = pendingComponents.end();
    while (iter != end)
    {
        entity_id_t id = iter->first;
        EntitySyncState& entityState = entities[id]; // Creates new if did not exist
        if (!entityState.id)
            entityState.id = id;
        if (!entityState.isInQueue)
        {
            dirtyQueue.push_back(&entityState);
            entityState.isInQueue = true;
        }
        ++iter;
    }

    pendingComponents.clear();
}

void SceneSyncState::MarkPendingEntityDirty(entity_id_t id)
{
    if (!isServer_)
        return;

    // If this entity is not in a pending state,
    // we should not proceed as otherwise this 
    // might be called with a local entity id.
    if (!HasPendingEntity(id))
        return;

    EntitySyncState& entityState = entities[id]; // Creates new if did not exist
    if (!entityState.id)
        entityState.id = id;
    if (!entityState.isInQueue)
    {
        dirtyQueue.push_back(&entityState);
        entityState.isInQueue = true;
    }

    RemovePendingEntity(id);
}

void SceneSyncState::MarkPendingComponentsDirty(entity_id_t id)
{
    if (!isServer_)
        return;

    // If this entity has no components in pending state,
    // we should not proceed as otherwise this 
    // might be called with a local entity id.
    if (!HasPendingComponents(id))
        return;

    // Above ensures iter to be valid and list not being empty.
    PendingConstIter iter = pendingComponents.find(id);
    ComponentIdList compList = iter->second; 

    EntitySyncState& entityState = MarkEntityDirtySilent(id);
    for (ComponentIdList::const_iterator compIter = compList.begin(); compIter != compList.end(); ++compIter)
        entityState.MarkComponentDirty((*compIter));

    RemovePendingEntity(id);
}

void SceneSyncState::MarkPendingComponentDirty(entity_id_t id, component_id_t compId)
{
    if (!isServer_)
        return;

    // If this entity has no components in pending state,
    // we should not proceed as otherwise this 
    // might be called with a local entity and/or component id.
    if (!HasPendingComponent(id, compId))
        return;
    
    MarkComponentDirtySilent(id, compId);
    RemovePendingComponent(id, compId);
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
    if (isServer_ && !ShouldMarkAsDirty(id, compId))
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
        // Entity sync accepted but following comps were rejected.
        else if (changeRequest_.HasRejectedComponents())
            FillPendingComponents(id, changeRequest_.RejectedComponents());

        return changeRequest_.Accepted();
    }
    return true;
}

bool SceneSyncState::ShouldMarkAsDirty(entity_id_t id, component_id_t compId)
{
    if (!isServer_)
        return true;

    // If already in pending state, do not request again
    if (HasPendingComponent(id, compId))
        return false;

    PROFILE(SceneSyncState_ShouldMarkAsDirty_Component);

    // Only request once per component
    std::map<entity_id_t, EntitySyncState>::iterator iterEnt = entities.find(id);
    if (iterEnt == entities.end())
    {
        MarkEntityDirtySilent(id);
        iterEnt = entities.find(id);
    }
    if (iterEnt == entities.end())
    {
        LogError("SceneSyncState::ShouldMarkAsDirty(id,compId): Failed to mark entity as dirty before component check!");
        return true;
    }
    std::map<component_id_t, ComponentSyncState>::iterator iterComp = iterEnt->second.components.find(compId);
    if (iterComp == iterEnt->second.components.end())
    {
        // Scene or entity null, do not process yet.
        if (!FillRequest(id, compId))
            return false;

        // Make a request to add this component into the sync state
        emit AboutToDirtyComponent(&changeRequest_);
        bool shouldDirty = (changeRequest_.Rejected() || changeRequest_.IsComponentRejected(compId)) ? false : true;
        if (!shouldDirty)
        {
            ComponentIdList& pendingComps = pendingComponents[id];
            pendingComps.push_back(compId);
        }
        return shouldDirty;
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

bool SceneSyncState::FillRequest(entity_id_t id, component_id_t compId)
{
    changeRequest_.Reset(id, compId);

    if (scene_.expired())
        return false;
    ScenePtr scenePtr = scene_.lock();
    if (!scenePtr.get())
    {
        LogError("SceneSyncState::FillRequest(id,compId): Scene is null!");
        return false;
    }

    EntityPtr entityPtr = scenePtr->GetEntity(id);
    if (!entityPtr.get())
        return false;

    // We trust the SyncManager mechanisms to stop local entities from ever getting here.
    // Print anyways if something starts to leak at least we notice it here.
    if (!entityPtr->IsReplicated())
        LogError("SceneSyncState::FillRequest(id,compId): Entity " + QString::number(id) + " should not be replicated!");
    changeRequest_.SetEntity(entityPtr.get());

    ComponentPtr compPtr = entityPtr->GetComponentById(compId);
    if (!compPtr.get())
    {
        LogError("SceneSyncState::FillRequest(id,compId): Entitys " + QString::number(id) + " component " + QString::number(compId) + " is null!");
        return false;
    }
    // We trust the SyncManager mechanisms to stop local components from ever getting here.
    // Print anyways if something starts to leak so at least we notice it here.
    if (!compPtr->IsReplicated())
        LogError("SceneSyncState::FillRequest(id,compId): Entitys " + QString::number(id) + " component " + QString::number(compId) + " should not be replicated!");
    changeRequest_.SetComponent(compPtr.get());

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

void SceneSyncState::FillPendingComponents(entity_id_t id, QList<component_id_t> compIdList)
{
    if (!isServer_)
        return;

    if (compIdList.isEmpty())
        return;
    QList<component_id_t> compsToAdd;
    foreach(component_id_t compId, compIdList)
        if (!HasPendingComponent(id, compId))
            compsToAdd << compId;

    if (compsToAdd.isEmpty())
        return;
    ComponentIdList& pendingComps = pendingComponents[id]; // Creates new if did not exist
    foreach(component_id_t compId, compsToAdd)
            pendingComps.push_back(compId);
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

void SceneSyncState::MarkComponentDirtySilent(entity_id_t id, component_id_t compId)
{
    EntitySyncState& entityState = MarkEntityDirtySilent(id); // Creates new if did not exist
    if (!entityState.id)
        entityState.id = id;
    entityState.MarkComponentDirty(compId);
}
