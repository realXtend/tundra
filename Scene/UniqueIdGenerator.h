// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "CoreTypes.h"
#include <set>

/// Generates unique integer ID's starting from 1. Used for entity and component ID's.
class UniqueIdGenerator
{
public:
    /// Construct
    UniqueIdGenerator() :
        id(0)
    {
    }
    
    /// Allocate and return the next free ID. Never returns zero.
    entity_id_t Allocate()
    {
        // Find the largest ID among the reserved
        // NOTE: This iteration is of linear complexity. Can optimize here. (But be sure to properly test for correctness!) -jj.
        entity_id_t largestId = 0;
        for (std::set<entity_id_t>::const_iterator iter = reserved.begin(); iter != reserved.end(); ++iter)
            largestId = std::max(largestId, *iter);
        
        // Ensure that the ID we give out is always larger than the largest currently reserved ID.
        id = std::max(id + 1, largestId + 1);
        if (!id) ++id;
        
        while (reserved.find(id) != reserved.end())
        {
            ++id;
            if (!id) ++id;
        }
        
        reserved.insert(id);
        return id;
    }
    
    /// Manually allocate an ID. Returns true if successful (not yet allocated), or false if already allocated
    bool Allocate(entity_id_t id_)
    {
        if (reserved.find(id_) != reserved.end())
            return false;
        reserved.insert(id_);
        return true;
    }
    
    /// Mark an ID free for reuse. Typically called after deleting entities.
    void Deallocate(entity_id_t id_)
    {
        reserved.erase(id_);
    }
    
    /// Mark all IDs free and reset next ID.
    void Reset()
    {
        reserved.clear();
        id = 0;
    }
    
    /// Return whether an ID is free
    bool IsFree(entity_id_t id) const
    {
        return reserved.find(id) == reserved.end();
    }
    
    /// Last returned ID
    entity_id_t id;
    /// Reserved ID's
    std::set<entity_id_t> reserved;
};
