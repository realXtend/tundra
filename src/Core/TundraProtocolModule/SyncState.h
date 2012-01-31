// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "CoreTypes.h"

#include "kNet/PolledTimer.h"
#include "kNet/Types.h"
#include "Transform.h"
#include "Math/float3.h"

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

/// Scene's per-user network sync state
struct SceneSyncState
{
    std::list<EntitySyncState*> dirtyQueue; ///< Dirty entities
    std::map<entity_id_t, EntitySyncState> entities; ///< Entity syncstates
    
    std::map<entity_id_t, RigidBodyInterpolationState> entityInterpolations;

    void Clear()
    {
        dirtyQueue.clear();
        entities.clear();
    }
    
    void RemoveFromQueue(entity_id_t id)
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
    
    void MarkEntityProcessed(entity_id_t id)
    {
        EntitySyncState& entityState = entities[id];
        if (!entityState.id)
            entityState.id = id;
        entityState.DirtyProcessed();
    }
    
    void MarkComponentProcessed(entity_id_t id, component_id_t compId)
    {
        EntitySyncState& entityState = entities[id];
        if (!entityState.id)
            entityState.id = id;
        ComponentSyncState& compState = entityState.components[compId];
        if (!compState.id)
            compState.id = compId;
        compState.DirtyProcessed();
    }
    
    void MarkEntityDirty(entity_id_t id)
    {
        EntitySyncState& entityState = entities[id]; // Creates new if did not exist
        if (!entityState.id)
            entityState.id = id;
        if (!entityState.isInQueue)
        {
            dirtyQueue.push_back(&entityState);
            entityState.isInQueue = true;
        }
    }
    
    void MarkEntityRemoved(entity_id_t id)
    {
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
    
    void MarkComponentDirty(entity_id_t id, component_id_t compId)
    {
        MarkEntityDirty(id);
        EntitySyncState& entityState = entities[id]; // Creates new if did not exist
        if (!entityState.id)
            entityState.id = id;
        entityState.MarkComponentDirty(compId);
    }
    
    void MarkComponentRemoved(entity_id_t id, component_id_t compId)
    {
        // If user did not have the entity or component in the first place, do nothing
        std::map<entity_id_t, EntitySyncState>::iterator i = entities.find(id);
        if (i == entities.end())
            return;
        MarkEntityDirty(id);
        i->second.MarkComponentRemoved(compId);
    }
    
    void MarkAttributeDirty(entity_id_t id, component_id_t compId, u8 attrIndex)
    {
        MarkEntityDirty(id);
        EntitySyncState& entityState = entities[id];
        entityState.MarkComponentDirty(compId);
        ComponentSyncState& compState = entityState.components[compId];
        compState.MarkAttributeDirty(attrIndex);
    }
    
    void MarkAttributeCreated(entity_id_t id, component_id_t compId, u8 attrIndex)
    {
        MarkEntityDirty(id);
        EntitySyncState& entityState = entities[id];
        entityState.MarkComponentDirty(compId);
        ComponentSyncState& compState = entityState.components[compId];
        compState.MarkAttributeCreated(attrIndex);
    }
    
    void MarkAttributeRemoved(entity_id_t id, component_id_t compId, u8 attrIndex)
    {
        MarkEntityDirty(id);
        EntitySyncState& entityState = entities[id];
        entityState.MarkComponentDirty(compId);
        ComponentSyncState& compState = entityState.components[compId];
        compState.MarkAttributeRemoved(attrIndex);
    }
};
