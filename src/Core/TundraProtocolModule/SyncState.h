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
        deleted(false),
        isInQueue(false)
    {
    }
    
    unsigned char dirtyAttributes[32]; ///< Dirty attributes bitfield. A maximum of 256 attributes are supported.
    std::map<unsigned, bool> newAndDeletedAttributes; ///< Dynamic attributes by index that have been deleted or created since last update. True = create, false = delete
    bool deleted; ///< The component has been deleted since last update
    bool isInQueue; ///< The component is already in the entity's dirty queue
};

/// Entity's per-user network sync state
struct EntitySyncState
{
    EntitySyncState() :
        deleted(false),
        isInQueue(false)
    {
    }
    
    std::list<ComponentSyncState*> dirtyQueue; ///< Dirty components
    std::map<component_id_t, ComponentSyncState> components; ///< Component syncstates
    bool deleted; ///< The entity has been deleted since last update
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
};
