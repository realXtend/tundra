// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "CoreTypes.h"

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
        id(0)
    {
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
        i->second.removed = true;
        i->second.isNew = false;
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
    
    std::list<ComponentSyncState*> dirtyQueue; ///< Dirty components
    std::map<component_id_t, ComponentSyncState> components; ///< Component syncstates
    entity_id_t id; ///< Entity ID. Duplicated here intentionally to allow recognizing the entity without the parent map.
    bool removed; ///< The entity has been removed since last update
    bool isNew; ///< The client does not have the entity and it must be serialized in full
    bool isInQueue; ///< The entity is already in the scene's dirty queue
};

/// Scene's per-user network sync state
struct SceneSyncState
{
    std::list<EntitySyncState*> dirtyQueue; ///< Dirty entities
    std::map<entity_id_t, EntitySyncState> entities; ///< Entity syncstates
    
    void Clear()
    {
        dirtyQueue.clear();
        entities.clear();
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
        i->second.removed = true;
        i->second.isNew = false;
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
