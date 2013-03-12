// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraProtocolModuleApi.h"

#include "CoreTypes.h"
#include "SceneFwd.h"

#include "kNet/PolledTimer.h"
#include "kNet/Types.h"
#include "Transform.h"
#include "Math/float3.h"

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

    // Special cases for rigid body streaming:
    // On the server side, remember the last sent rigid body parameters, so that we can perform effective pruning of redundant data.
    Transform transform;
    float3 linearVelocity;
    float3 angularVelocity;
    kNet::tick_t lastNetworkSendTime;
};

struct RigidBodyInterpolationState
{
    // On the client side, remember the state for performing Hermite interpolation (C1, i.e. pos and vel are continuous).
    struct RigidBodyState
    {
        float3 pos;
        float3 vel;
        Quat rot;
        float3 scale;
        float3 angVel; // Angular velocity in Euler ZYX.
    };

    RigidBodyState interpStart;
    RigidBodyState interpEnd;
    float interpTime;

    // If true, we are using linear inter/extrapolation to move the entity.
    // If false, we have handed off this entity for physics to extrapolate.
    bool interpolatorActive;

    /// Remembers the packet id of the most recently received network sync packet. Used to enforce
    /// proper ordering (generate latest-data-guarantee messaging) for the received movement packets.
    kNet::packet_id_t lastReceivedPacketCounter;
};

/// State change request to permit/deny changes.
class TUNDRAPROTOCOL_MODULE_API StateChangeRequest : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool accepted READ Accepted WRITE SetAccepted)
    Q_PROPERTY(u32 connectionID READ ConnectionID)

    Q_PROPERTY(entity_id_t entityId READ EntityId)
    Q_PROPERTY(Entity* entity READ GetEntity)

public:
    StateChangeRequest(u32 connectionID) :
        connectionID_(connectionID)
    { 
        Reset(); 
    }

    void Reset(entity_id_t entityId = 0)
    {
        accepted_ = true;
        entityId_ = entityId;
        entity_ = 0;
    }

public slots:
    /// Set the change request accepted
    void SetAccepted(bool accepted)         
    { 
        accepted_ = accepted; 
    }

    /// Accept the whole change request.
    void Accept()
    {
        accepted_ = true;
    }

    /// Reject the whole change request.
    void Reject()
    {
        accepted_ = false;
    }

public:
    bool Accepted()                         { return accepted_; }
    bool Rejected()                         { return !accepted_; }

    u32 ConnectionID()                      { return connectionID_; }

    entity_id_t EntityId()                  { return entityId_; }
    Entity* GetEntity()                     { return entity_; }
    void SetEntity(Entity* entity)          { entity_ = entity; }

private:
    bool accepted_;
    u32 connectionID_;

    entity_id_t entityId_;
    Entity* entity_;
};

typedef std::list<component_id_t> ComponentIdList;

/// Scene's per-user network sync state
class TUNDRAPROTOCOL_MODULE_API SceneSyncState : public QObject
{
    Q_OBJECT

public:
    SceneSyncState(u32 userConnectionID = 0, bool isServer = false);
    virtual ~SceneSyncState();

    /// Dirty entities pending processing
    std::list<EntitySyncState*> dirtyQueue; 

    /// Entity sync states
    std::map<entity_id_t, EntitySyncState> entities; 

    /// Entity interpolations
    std::map<entity_id_t, RigidBodyInterpolationState> entityInterpolations;

    /// Maps containing the relevance factors and visibility data
    /// @remarks InterestManager functionality
    std::map<entity_id_t, bool> visibleEntities;
    std::map<entity_id_t, float> relevanceFactors;

    /// Couple of maps containing the timestamps of last updates and raycasts
    /// @remarks InterestManager functionality
    std::map<entity_id_t, float> lastUpdatedEntitys_;
    std::map<entity_id_t, float> lastRaycastedEntitys_;

    /// @remarks InterestManager functionality
    Quat clientOrientation;
    Quat initialOrientation;
    float3 clientLocation;  //Clients current pos
    float3 initialLocation; //Clients initial pos
    bool locationInitialized;

signals:
    /// This signal is emitted when a entity is being added to the client sync state.
    /// All needed data for evaluation logic is in the StateChangeRequest parameter object.
    /// If 'request.Accepted()' is true (default) the entity will be added to the sync state,
    /// otherwise it will be added to the pending entities list. 
    /// @note See also HasPendingEntity and MarkPendingEntityDirty.
    /// @param request StateChangeRequest object.
    void AboutToDirtyEntity(StateChangeRequest *request);

public slots:
    /// Removes the entity from the sync state and puts it to the pending list.
    /// @remark Enables a 'pending' logic in SyncManager, with which a script can throttle the sending of entities to clients.
    void MarkEntityPending(entity_id_t id);

    /// Adds all currently pending entities to the clients sync state.
    /// @remark Enables a 'pending' logic in SyncManager, with which a script can throttle the sending of entities to clients.
    void MarkPendingEntitiesDirty();

    /// Adds entity with id to the clients sync state.
    /// @remark Enables a 'pending' logic in SyncManager, with which a script can throttle the sending of entities to clients.
    void MarkPendingEntityDirty(entity_id_t id);

    /// Returns all pending entity IDs.
    /// @remark Enables a 'pending' logic in SyncManager, with which a script can throttle the sending of entities to clients.
    QVariantList PendingEntityIDs() const;

    /// Returns 0 if no pending entities.
    /// @remark Enables a 'pending' logic in SyncManager, with which a script can throttle the sending of entities to clients.
    entity_id_t NextPendingEntityID() const;

    /// Returns if we have any pending entitys.
    /// @remark Enables a 'pending' logic in SyncManager, with which a script can throttle the sending of entities to clients.
    bool HasPendingEntities() const;

    /// Returns if we have pending entity with id.
    /// @remark Enables a 'pending' logic in SyncManager, with which a script can throttle the sending of entities to clients.
    bool HasPendingEntity(entity_id_t id) const;

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

    // Silently does the same as MarkEntityDirty without emitting change request signal.
    EntitySyncState& MarkEntityDirtySilent(entity_id_t id);

    // Removes entity from pending lists.
    void RemovePendingEntity(entity_id_t id);

private:
    // Returns if entity with id should be added to the sync state.
    bool ShouldMarkAsDirty(entity_id_t id);

    // Fills changeRequest_ with entity data, returns if request is valid. 
    bool FillRequest(entity_id_t id);
    
    // Adds the entity id to the pending entity list.
    void AddPendingEntity(entity_id_t id);

    /// @remark Enables a 'pending' logic in SyncManager, with which a script can throttle the sending of entities to clients.
    /// @todo This data structure needs to be removed. This is double book-keeping. Instead, track the dirty and pending entities
    ///       with the same dirty bit in EntitySyncState and ComponentSyncState.
    std::vector<entity_id_t> pendingEntities_;

    StateChangeRequest changeRequest_;
    bool isServer_;
    u32 userConnectionID_;

    SceneWeakPtr scene_;
};
