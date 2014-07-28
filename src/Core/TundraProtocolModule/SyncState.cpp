// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "SyncState.h"

#include "Scene/Scene.h"
#include "Entity.h"
#include "IComponent.h"
#include "Profiler.h"

#include "LoggingFunctions.h"

/// @remark Enables a 'pending' logic in SyncManager, with which a script can throttle the sending of entities to clients.
typedef std::vector<entity_id_t> EntityIdList;
typedef EntityIdList::const_iterator PendingConstIter;
typedef EntityIdList::iterator PendingIter;

SceneSyncState::SceneSyncState(u32 userConnectionID, bool isServer) :
    userConnectionID_(userConnectionID),
    changeRequest_(userConnectionID),
    isServer_(isServer),
    placeholderComponentsSent_(false),
    locationInitialized(false),
    clientLocation(float3::nan),
    initialLocation(float3::nan)
{
    Clear();
}

SceneSyncState::~SceneSyncState()
{
}

// Public slots

/// @remark Enables a 'pending' logic in SyncManager, with which a script can throttle the sending of entities to clients.
QVariantList SceneSyncState::PendingEntityIDs() const
{
    QVariantList list;
    for(PendingConstIter iter = pendingEntities_.begin(); iter != pendingEntities_.end(); ++iter)
    {
        entity_id_t id = (*iter);
        if (!list.contains(id))
            list << id;
    }
    return list;
}

/// @remark Enables a 'pending' logic in SyncManager, with which a script can throttle the sending of entities to clients.
entity_id_t SceneSyncState::NextPendingEntityID() const
{
    if (pendingEntities_.empty())
        return 0;
    PendingConstIter front = pendingEntities_.begin();
    if (front == pendingEntities_.end())
        return 0;
    return (*front);
}

/// @remark Enables a 'pending' logic in SyncManager, with which a script can throttle the sending of entities to clients.
bool SceneSyncState::HasPendingEntities() const
{
    return !pendingEntities_.empty();
}

/// @remark Enables a 'pending' logic in SyncManager, with which a script can throttle the sending of entities to clients.
bool SceneSyncState::HasPendingEntity(entity_id_t id) const
{
    for(PendingConstIter iter = pendingEntities_.begin(); iter != pendingEntities_.end(); ++iter)
    {
        if ((*iter) == id)
            return true;
    }
    return false;
}

/// @remark Enables a 'pending' logic in SyncManager, with which a script can throttle the sending of entities to clients.
void SceneSyncState::MarkEntityPending(entity_id_t id)
{
    if (!isServer_)
        return;

    // If user does not have the entity in the first place, do nothing.
    // Its going to be asked to be added to the state via the permission signals later.
    std::map<entity_id_t, EntitySyncState>::const_iterator i = entities.find(id);
    if (i == entities.end())
        return;

    MarkEntityRemoved(id);  // Remove from current sync state (removes entity from client)
    AddPendingEntity(id);   // Mark entity as pending
}

/// @remark Enables a 'pending' logic in SyncManager, with which a script can throttle the sending of entities to clients.
void SceneSyncState::MarkPendingEntitiesDirty()
{
    if (!isServer_)
        return;

    // Get current entity ids to a separate list as MarkPendingEntityDirty modified pendingEntities_.
    QList<entity_id_t> entIds;
    for(PendingConstIter iter = pendingEntities_.begin(); iter != pendingEntities_.end(); ++iter)
    {
        entity_id_t id = (*iter);
        if (!entIds.contains(id))
            entIds << id;
    }

    foreach(entity_id_t entId, entIds)
        MarkPendingEntityDirty(entId);

    if (!pendingEntities_.empty())
        LogWarning("SceneSyncState::MarkPendingEntitiesDirty: Pending list not empty after processing all pending entities!");
}

/// @remark Enables a 'pending' logic in SyncManager, with which a script can throttle the sending of entities to clients.
void SceneSyncState::MarkPendingEntityDirty(entity_id_t id)
{
    if (!isServer_)
        return;

    // If this entity has no components in pending state,
    // we should not proceed as otherwise this 
    // might be called with a local entity id.
    if (!HasPendingEntity(id))
        return;

    MarkEntityDirtySilent(id);
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
    pendingEntities_.clear();
    changeRequest_.Reset();
    scene_.reset();
    placeholderComponentsSent_ = false;
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

bool SceneSyncState::MarkEntityDirty(entity_id_t id, bool hasPropertyChanges, bool hasParentChange)
{
    /** @todo This logic should be removed. If a script rejects a change, it results in the change *never* being sent to the client.
        E.g. if the script decides to reject a change due to the target entity being too far, and then the entity comes closer,
        the change will not be replicated again, since rejecting here caused SyncState to lose tracking the change. */
    /** @remark to above comment: Yes, this is the whole intention of this pending logic. If applications (C++ or Script) decide
        to take over controlling what gets sent, to which client and when, it must also take care of sending them when it sees appropriate.
        There are actual uses cases where a set of Entities will never be sent to a particular client, or only if the client requests them
        or by some kind of distance etc. metric mentioned above. Once a pending Entity is released to the state it will be sent fully
        with its current state on the server, no information is lost. The client just sees it as a normal new Entity. */

    // Return if the whole entity change request was rejected
    if (isServer_ && !ShouldMarkAsDirty(id))
        return false;

    EntitySyncState& entityState = entities[id]; // Creates new if did not exist
    if (!entityState.id)
        entityState.id = id;
    if (!entityState.isInQueue)
    {
        dirtyQueue.push_back(&entityState);
        entityState.isInQueue = true;
    }
    if (hasPropertyChanges)
        entityState.hasPropertyChanges = true;
    if (hasParentChange)
        entityState.hasParentChange = true;
    return true;
}

void SceneSyncState::MarkEntityRemoved(entity_id_t id)
{
    /// @remark Enables a 'pending' logic in SyncManager, with which a script can throttle the sending of entities to clients.
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
    if (MarkEntityDirty(id))
    {
        EntitySyncState& entityState = entities[id]; // Creates new if did not exist
        if (!entityState.id)
            entityState.id = id;
        entityState.MarkComponentDirty(compId);
    }
}

void SceneSyncState::MarkComponentRemoved(entity_id_t id, component_id_t compId)
{
    // If user did not have the entity or component in the first place, do nothing
    std::map<entity_id_t, EntitySyncState>::iterator i = entities.find(id);
    if (i != entities.end())
    {
        MarkEntityDirty(id);
        i->second.MarkComponentRemoved(compId);
    }
}

void SceneSyncState::MarkAttributeDirty(entity_id_t id, component_id_t compId, u8 attrIndex)
{
    if (MarkEntityDirty(id))
    {
        EntitySyncState& entityState = entities[id];
        entityState.MarkComponentDirty(compId);
        ComponentSyncState& compState = entityState.components[compId];
        compState.MarkAttributeDirty(attrIndex);
    }
}

void SceneSyncState::MarkAttributeCreated(entity_id_t id, component_id_t compId, u8 attrIndex)
{
    if (MarkEntityDirty(id))
    {
        EntitySyncState& entityState = entities[id];
        entityState.MarkComponentDirty(compId);
        ComponentSyncState& compState = entityState.components[compId];
        compState.MarkAttributeCreated(attrIndex);
    }
}

void SceneSyncState::MarkAttributeRemoved(entity_id_t id, component_id_t compId, u8 attrIndex)
{
    if (MarkEntityDirty(id))
    {
        EntitySyncState& entityState = entities[id];
        entityState.MarkComponentDirty(compId);
        ComponentSyncState& compState = entityState.components[compId];
        compState.MarkAttributeRemoved(attrIndex);
    }
}

// Private

bool SceneSyncState::ShouldMarkAsDirty(entity_id_t id)
{
    if (!isServer_)
        return true;

    // If already in pending state, do not request again
    if (HasPendingEntity(id))
        return false;

    // Only request if this entity does not have a sync state yet.
    // Otherwise this id will spam the signal handler on every change if
    // the addition to sync state was accepted.
    std::map<entity_id_t, EntitySyncState>::iterator i = entities.find(id);
    if (i == entities.end())
    {
        PROFILE(SyncState_Emit_AboutToDirtyEntity);
        
        // Scene or entity null, do not process yet.
        if (!FillRequest(id))
            return false;

        emit AboutToDirtyEntity(&changeRequest_);

        // Rejected, mark entity as pending.
        if (changeRequest_.Rejected())
            AddPendingEntity(id);
        return changeRequest_.Accepted();
    }
    return true;
}

void SceneSyncState::RemovePendingEntity(entity_id_t id)
{
    // This assumes that the id has not been added multiple times to our vector.
    for(PendingIter iter = pendingEntities_.begin(); iter != pendingEntities_.end(); ++iter)
    {
        if ((*iter) == id)
        {
            pendingEntities_.erase(iter);
            break;
        }
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

void SceneSyncState::AddPendingEntity(entity_id_t id)
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
        
    if (!HasPendingEntity(id))
        pendingEntities_.push_back(id);
}

EntitySyncState& SceneSyncState::MarkEntityDirtySilent(entity_id_t id)
{
    // Verify that this entity is not known to this client state.
    // If it is we need to remove the ptr from any queues and remove the entity state.
    // This ensures the below creates a new EntitySyncState with isNew == true.
    std::map<entity_id_t, EntitySyncState>::iterator existingIter = entities.find(id);
    if (existingIter != entities.end())
    {
        LogWarning(QString("SceneSyncState::MarkEntityDirtySilent: State for Entity %1 already exist, removing for full recreation.").arg(id));
        RemoveFromQueue(id);
        entities.erase(existingIter);
    }

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
