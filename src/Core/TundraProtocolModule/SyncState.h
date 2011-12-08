// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "CoreTypes.h"
#include "SceneFwd.h"

#include "kNet/PolledTimer.h"

#include <QObject>
#include <QVariant>

#include <list>
#include <map>
#include <set>

/// Component's per-user network sync state
struct ComponentSyncState
{
    ComponentSyncState() :
        removed(false),
        isNew(true),
        isInQueue(false),
        id(0)
    {
        for (unsigned i = 0; i < 32; ++i)
            dirtyAttributes[i] = 0;
    }
    
    void MarkAttributeDirty(u8 attrIndex)
    {
        dirtyAttributes[attrIndex >> 3] |= (1 << (attrIndex & 7));
    }
    
    void MarkAttributeCreated(u8 attrIndex)
    {
        newAndRemovedAttributes[attrIndex] = true;
    }
    
    void MarkAttributeRemoved(u8 attrIndex)
    {
        newAndRemovedAttributes[attrIndex] = false;
    }
    
    void DirtyProcessed()
    {
        for (unsigned i = 0; i < 32; ++i)
            dirtyAttributes[i] = 0;
        newAndRemovedAttributes.clear();
        isNew = false;
    }
    
    u8 dirtyAttributes[32]; ///< Dirty attributes bitfield. A maximum of 256 attributes are supported.
    std::map<u8, bool> newAndRemovedAttributes; ///< Dynamic attributes by index that have been removed or created since last update. True = create, false = delete
    component_id_t id; ///< Component ID. Duplicated here intentionally to allow recognizing the component without the parent map.
    bool removed; ///< The component has been removed since last update
    bool isNew; ///< The client does not have the component and it must be serialized in full
    bool isInQueue; ///< The component is already in the entity's dirty queue
};

/// Entity's per-user network sync state
struct EntitySyncState
{
    EntitySyncState() :
        removed(false),
        isNew(true),
        isInQueue(false),
        id(0),
        avgUpdateInterval(0.0f)
    {
    }
    
    void RemoveFromQueue(component_id_t id)
    {
        std::map<component_id_t, ComponentSyncState>::iterator i = components.find(id);
        if (i != components.end())
        {
            if (i->second.isInQueue)
            {
                for (std::list<ComponentSyncState*>::iterator j = dirtyQueue.begin(); j != dirtyQueue.end(); ++j)
                {
                    if ((*j) == &i->second)
                    {
                        dirtyQueue.erase(j);
                        break;
                    }
                }
                i->second.isInQueue = false;
            }
        }
    }
    
    void MarkComponentDirty(component_id_t id)
    {
        ComponentSyncState& compState = components[id]; // Creates new if did not exist
        if (!compState.id)
            compState.id = id;
        if (!compState.isInQueue)
        {
            dirtyQueue.push_back(&compState);
            compState.isInQueue = true;
        }
    }
    
    void MarkComponentRemoved(component_id_t id)
    {
        // If user did not have the component in the first place, do nothing
        std::map<component_id_t, ComponentSyncState>::iterator i = components.find(id);
        if (i == components.end())
            return;
        // If component is marked new, it was not sent yet and can be simply removed from the sync state
        if (i->second.isNew)
        {
            RemoveFromQueue(id);
            components.erase(id);
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
    
    void DirtyProcessed()
    {
        for (std::map<component_id_t, ComponentSyncState>::iterator i = components.begin(); i != components.end(); ++i)
        {
            i->second.DirtyProcessed();
            i->second.isInQueue = false;
        }
        dirtyQueue.clear();
        isNew = false;
    }
    
    void UpdateReceived()
    {
        float time = updateTimer.MSecsElapsed() * 0.001f;
        updateTimer.Start();
        // Maximum update rate should be 100fps. Discard either very frequent or very infrequent updates.
        if (time < 0.005f || time >= 0.5f)
            return;
        // If it's the first measurement, set time directly. Else smooth
        if (avgUpdateInterval == 0.0f)
            avgUpdateInterval = time;
        else
            avgUpdateInterval = 0.5 * time + 0.5 * avgUpdateInterval;
    }
    
    std::list<ComponentSyncState*> dirtyQueue; ///< Dirty components
    std::map<component_id_t, ComponentSyncState> components; ///< Component syncstates
    entity_id_t id; ///< Entity ID. Duplicated here intentionally to allow recognizing the entity without the parent map.
    bool removed; ///< The entity has been removed since last update
    bool isNew; ///< The client does not have the entity and it must be serialized in full
    bool isInQueue; ///< The entity is already in the scene's dirty queue
    
    kNet::PolledTimer updateTimer; ///< Last update received timer
    float avgUpdateInterval; ///< Average network update interval in seconds
};

/// State change request to permit/deny changes.
class StateChangeRequest : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool accepted READ Accepted WRITE SetAccepted)
    Q_PROPERTY(int connectionID READ ConnectionID)

    Q_PROPERTY(entity_id_t entityId READ EntityId)
    Q_PROPERTY(Entity* entity READ GetEntity)

    Q_PROPERTY(component_id_t componentId READ ComponentId)
    Q_PROPERTY(IComponent* component READ GetComponent)

public:
    StateChangeRequest(int connectionID) :
        connectionID_(connectionID)
    { 
        Reset(); 
    }

    void Reset(entity_id_t entityId = 0, component_id_t componentId = 0)
    {
        accepted_ = true;
        entityId_ = entityId;
        componentId_ = componentId;
        entity_ = 0;
        component_ = 0;
    }

    void SetAccepted(bool accepted)         { accepted_ = accepted; }
    bool Accepted()                         { return accepted_; }
    
    int ConnectionID()                      { return connectionID_; }

    entity_id_t EntityId()                  { return entityId_; }
    Entity* GetEntity()                     { return entity_; }
    void SetEntity(Entity* entity)          { entity_ = entity; }

    component_id_t ComponentId()            { return componentId_; }
    IComponent* GetComponent()              { return component_; }
    void SetComponent(IComponent* comp)     { component_ = comp; }

private:
    bool accepted_;
    int connectionID_;

    entity_id_t entityId_;
    Entity* entity_;

    component_id_t componentId_;
    IComponent* component_;
};

typedef std::list<entity_id_t> EntityIdList;
typedef std::list<component_id_t> ComponentIdList;

/// Scene's per-user network sync state
class SceneSyncState : public QObject
{
    Q_OBJECT

public:
    SceneSyncState(int userConnectionID = 0, bool isServer = false);
    virtual ~SceneSyncState();

    /// Dirty entities pending processing
    std::list<EntitySyncState*> dirtyQueue; 

    /// Entity sync states
    std::map<entity_id_t, EntitySyncState> entities; 

signals:
    /// This signal is emitted when a entity is being added to the client sync state.
    /// All needed data for evaluation logic is in the StateChangeRequest parameter object.
    /// If 'request.Accepted()' is true (default) the entity will be added to the sync state,
    /// otherwise it will be added to the pending entities list. 
    /// @note See also HasPendingEntity and MarkPendingEntityDirty.
    /// @param request StateChangeRequest object.
    void AboutToDirtyEntity(StateChangeRequest *request);

    /// This signal is emitted when a component is being added to the client sync state.
    /// All needed data for evaluation logic is in the StateChangeRequest parameter object.
    /// If 'request.Accepted()' is true (default) the component will be added to the sync state,
    /// otherwise it will be added to the pending components list for the parent entity. 
    /// @note See also HasPendingComponents, HasPendingComponent, MarkPendingComponentsDirty and MarkPendingComponentDirty.
    /// @param request StateChangeRequest object.
    void AboutToDirtyComponent(StateChangeRequest *request);

public slots:
    /// Returns all pending entity IDs.
    QVariantList PendingEntityIDs() const;

    /// Returns all pending component IDs for entity id.
    QVariantList PendingComponentIDs(entity_id_t id) const;

    /// Returns 0 if no pending entities.
    entity_id_t NextPendingEntityID() const;

    /// Returns 0 if no pending component for this entity.
    component_id_t NextPendingComponentID(entity_id_t id) const;

    /// Returns if we have any pending entities.
    bool HasPendingEntities() const;

    /// Returns if we have pending entity with id.
    bool HasPendingEntity(entity_id_t id) const;

    /// Returns if we have pending components for entity id.
    bool HasPendingComponents(entity_id_t id) const;

    /// Returns if we have pending component of compId for entity id.
    bool HasPendingComponent(entity_id_t id, component_id_t compId) const;

    /// Adds all currently pending entities to the clients sync state.
    void MarkPendingEntitiesDirty();

    /// Adds entity with id to the clients sync state.
    void MarkPendingEntityDirty(entity_id_t id);

    /// Adds all components (and the parent entity) to the client sync state.
    void MarkPendingComponentsDirty(entity_id_t id);

    /// Adds component of compId from entity id to client sync state.
    void MarkPendingComponentDirty(entity_id_t id, component_id_t compId);

public:
    void SetParentScene(SceneWeakPtr scene);
    void Clear();
    
    void RemoveFromQueue(entity_id_t id);

    void MarkEntityProcessed(entity_id_t id);
    void MarkComponentProcessed(entity_id_t id, component_id_t compId);

    void MarkEntityDirty(entity_id_t id);
    void MarkEntityRemoved(entity_id_t id);

    void MarkComponentDirty(entity_id_t id, component_id_t compId);
    void MarkComponentRemoved(entity_id_t id, component_id_t compId);

    void MarkAttributeDirty(entity_id_t id, component_id_t compId, u8 attrIndex);
    void MarkAttributeCreated(entity_id_t id, component_id_t compId, u8 attrIndex);
    void MarkAttributeRemoved(entity_id_t id, component_id_t compId, u8 attrIndex);

private:
    bool ShouldMarkAsDirty(entity_id_t id);
    bool ShouldMarkAsDirty(entity_id_t id, component_id_t compId);
    
    void RemovePendingEntity(entity_id_t id);
    void RemovePendingComponent(entity_id_t id, component_id_t compId);

    void FillRequest(entity_id_t id);
    void FillRequest(entity_id_t id, component_id_t compId);
    
    EntityIdList pendingEntities;
    std::map<entity_id_t, ComponentIdList > pendingComponents;

    StateChangeRequest changeRequest_;
    bool isServer_;
    int userConnectionID_;

    SceneWeakPtr scene_;
};
