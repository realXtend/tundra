// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "SyncState.h"

#include "Scene.h"
#include "Entity.h"
#include "IComponent.h"
#include "Profiler.h"

#include "LoggingFunctions.h"

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
    EntityIdList::const_iterator iter = pendingEntities.begin();
    EntityIdList::const_iterator end = pendingEntities.end();
    while (iter != end)
    {
        entity_id_t id = (*iter);
        if (!list.contains(id))
            list << id;
        ++iter;
    }
    return list;
}

QVariantList SceneSyncState::PendingComponentIDs(entity_id_t id) const
{
    QVariantList list;  
    std::map<entity_id_t, ComponentIdList >::const_iterator iter = pendingComponents.find(id);
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
    if (pendingEntities.empty())
        return 0;

    return pendingEntities.front();
}

component_id_t SceneSyncState::NextPendingComponentID(entity_id_t id) const
{
    std::map<entity_id_t, ComponentIdList >::const_iterator iter = pendingComponents.find(id);
    if (iter == pendingComponents.end())
        return 0;
    if (iter->second.empty())
        return 0;
    return iter->second.front();
}

bool SceneSyncState::HasPendingEntities() const
{ 
    return !pendingEntities.empty();
}

bool SceneSyncState::HasPendingEntity(entity_id_t id) const
{
    for (EntityIdList::const_iterator entIter = pendingEntities.begin(); entIter != pendingEntities.end(); ++entIter)
        if ((*entIter) == id)
            return true;
    return false;
}

bool SceneSyncState::HasPendingComponents(entity_id_t id) const
{
    std::map<entity_id_t, ComponentIdList >::const_iterator iter = pendingComponents.find(id);
    if (iter == pendingComponents.end())
        return false;
    return !iter->second.empty();
}

bool SceneSyncState::HasPendingComponent(entity_id_t id, component_id_t compId) const
{
    std::map<entity_id_t, ComponentIdList >::const_iterator iter = pendingComponents.find(id);
    if (iter == pendingComponents.end())
        return false;
    ComponentIdList compList = iter->second;
    if (compList.empty())
        return false;
    for (ComponentIdList::const_iterator compIter = compList.begin(); compIter != compList.end(); ++compIter)
        if ((*compIter) == compId)
            return true;
    return false;
}

void SceneSyncState::MarkPendingEntitiesDirty()
{
    if (!isServer_)
        return;

    EntityIdList::iterator iter = pendingEntities.begin();
    EntityIdList::iterator end = pendingEntities.end();
    while (iter != end)
    {
        MarkPendingEntityDirty((*iter));
        ++iter;
    }
    pendingEntities.clear();
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

    pendingEntities.remove(id);
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

    std::map<entity_id_t, ComponentIdList >::const_iterator iter = pendingComponents.find(id);
    if (iter == pendingComponents.end())    
        return;

    ComponentIdList compList = iter->second; 
    if (compList.empty())
    {
        pendingComponents.erase(iter);
        return;
    }

    MarkPendingEntityDirty(id);
    EntitySyncState& entityState = entities[id]; // Creates new if did not exist
    if (!entityState.id)
        entityState.id = id;
    
    for (ComponentIdList::const_iterator compIter = compList.begin(); compIter != compList.end(); ++compIter)
        entityState.MarkComponentDirty((*compIter));

    compList.clear();
    pendingComponents.erase(iter);
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

    std::map<entity_id_t, ComponentIdList >::iterator iter = pendingComponents.find(id);
    if (iter == pendingComponents.end())    
        return;
    
    MarkPendingEntityDirty(id);
    EntitySyncState& entityState = entities[id]; // Creates new if did not exist
    if (!entityState.id)
        entityState.id = id;
    entityState.MarkComponentDirty(compId);

    iter->second.remove(compId);
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
    pendingEntities.clear();
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
        FillRequest(id);
        emit AboutToDirtyEntity(&changeRequest_);
        if (!changeRequest_.Accepted())
            pendingEntities.push_back(id);
        return changeRequest_.Accepted();
    }
    return true;
}

bool SceneSyncState::ShouldMarkAsDirty(entity_id_t id, component_id_t compId)
{
    if (!isServer_)
        return true;

    // If already in pending state, do not request again
    if (HasPendingEntity(id) || HasPendingComponent(id, compId))
        return false;

    PROFILE(SceneSyncState_ShouldMarkAsDirty_Component);

    // Only request once per component
    std::map<entity_id_t, EntitySyncState>::iterator iterEnt = entities.find(id);
    if (iterEnt == entities.end())
    {
        MarkEntityDirty(id);
        iterEnt = entities.find(id);
    }
    if (iterEnt == entities.end())
        return true;
    std::map<component_id_t, ComponentSyncState>::iterator iterComp = iterEnt->second.components.find(compId);
    if (iterComp == iterEnt->second.components.end())
    {
        // Make a request to add this component into the sync state
        FillRequest(id, compId);
        emit AboutToDirtyComponent(&changeRequest_);
        if (!changeRequest_.Accepted())
        {
            ComponentIdList& pendingComps = pendingComponents[id];
            pendingComps.push_back(compId);
        }
        return changeRequest_.Accepted();
    }
    return true;
}

void SceneSyncState::RemovePendingEntity(entity_id_t id)
{
    pendingEntities.remove(id);
    std::map<entity_id_t, ComponentIdList >::iterator compIter = pendingComponents.find(id);
    if (compIter != pendingComponents.end())
        pendingComponents.erase(compIter);
}

void SceneSyncState::RemovePendingComponent(entity_id_t id, component_id_t compId)
{
    std::map<entity_id_t, ComponentIdList >::iterator compIter = pendingComponents.find(id);
    if (compIter != pendingComponents.end())
    {
        compIter->second.remove(compId);
        if (compIter->second.empty())
            pendingComponents.erase(compIter);
    }
}

void SceneSyncState::FillRequest(entity_id_t id)
{
    changeRequest_.Reset(id);

    if (scene_.expired())
        return;
    ScenePtr scenePtr = scene_.lock();
    if (!scenePtr.get())
    {
        LogError("SceneSyncState::FillRequest(id): Scene is null!");
        return;
    }

    EntityPtr entityPtr = scenePtr->GetEntity(id);
    if (!entityPtr.get())
    {
        LogError("SceneSyncState::FillRequest(id): Entity " + QString::number(id) + " is null!");
        return;
    }
    changeRequest_.SetEntity(entityPtr.get());
}

void SceneSyncState::FillRequest(entity_id_t id, component_id_t compId)
{
    changeRequest_.Reset(id, compId);

    if (scene_.expired())
        return;
    ScenePtr scenePtr = scene_.lock();
    if (!scenePtr.get())
    {
        LogError("SceneSyncState::FillRequest(id,compId): Scene is null!");
        return;
    }

    EntityPtr entityPtr = scenePtr->GetEntity(id);
    if (!entityPtr.get())
    {
        LogError("SceneSyncState::FillRequest(id,compId): Entity " + QString::number(id) + " is null!");
        return;
    }
    changeRequest_.SetEntity(entityPtr.get());

    ComponentPtr compPtr = entityPtr->GetComponentById(compId);
    if (!compPtr.get())
    {
        LogError("SceneSyncState::FillRequest(id,compId): Entitys " + QString::number(id) + " component " + QString::number(compId) + " is null!");
        return;
    }
    changeRequest_.SetComponent(compPtr.get());
}