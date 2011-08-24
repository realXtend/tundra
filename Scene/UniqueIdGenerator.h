// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "CoreTypes.h"
#include <set>

/// Generates unique integer ID's. Used for entity and component ID's. Supports both a local range and replicated range, which the high bit determines.
///\todo This class (needlessly) duplicates the reserved ID's set, which also is contained in the scene & entity object maps.
class UniqueIdGenerator
{
public:
    // Last replicated ID is guaranteed to fit inside 30 bits since we send them into the network as VLE8/16/32.
    static const entity_id_t LAST_REPLICATED_ID = 0x3fffffff;
    static const entity_id_t FIRST_LOCAL_ID = 0x80000000;
    
    /// Construct
    UniqueIdGenerator() :
        id(0),
        localId(FIRST_LOCAL_ID)
    {
    }
    
    /// Allocate and return the next replicated ID. Never returns zero.
    entity_id_t AllocateReplicated()
    {
        // Find the largest ID among the reserved
        // NOTE: This iteration is of linear complexity. Can optimize here. (But be sure to properly test for correctness!) -jj.
        entity_id_t largestId = 0;
        for (std::set<entity_id_t>::const_iterator iter = reserved.begin(); iter != reserved.end(); ++iter)
            largestId = std::max(largestId, *iter);
        
        // Ensure that the ID we give out is always larger than the largest currently reserved ID.
        id = std::max(id + 1, largestId + 1);
        if (id > LAST_REPLICATED_ID) id = 1;
        
        while (reserved.find(id) != reserved.end())
        {
            ++id;
            if (id > LAST_REPLICATED_ID) id = 1;
        }
        
        reserved.insert(id);
        return id;
    }
    
    /// Allocate and return the next local ID. Never returns zero.
    entity_id_t AllocateLocal()
    {
        // Find the largest ID among the reserved
        // NOTE: This iteration is of linear complexity. Can optimize here. (But be sure to properly test for correctness!) -jj.
        entity_id_t largestId = FIRST_LOCAL_ID;
        for (std::set<entity_id_t>::const_iterator iter = reservedLocal.begin(); iter != reservedLocal.end(); ++iter)
            largestId = std::max(largestId, *iter);
        
        // Ensure that the ID we give out is always larger than the largest currently reserved ID.
        localId = std::max(localId + 1, largestId + 1);
        if (localId == 0) localId = FIRST_LOCAL_ID + 1;
        
        while (reservedLocal.find(localId) != reservedLocal.end())
        {
            ++localId;
            if (localId == 0) localId = FIRST_LOCAL_ID + 1;
        }
        
        reservedLocal.insert(localId);
        return localId;
    }
    
    /// Manually allocate an ID. Returns true if successful (not yet allocated), or false if already allocated
    bool Allocate(entity_id_t id_)
    {
        if (id_ < FIRST_LOCAL_ID)
        {
            if (reserved.find(id_) != reserved.end())
                return false;
            reserved.insert(id_);
            return true;
        }
        else
        {
            if (reservedLocal.find(id_) != reservedLocal.end())
                return false;
            reservedLocal.insert(id_);
            return true;
        }
    }
    
    /// Mark an ID free for reuse. Typically called after deleting entities.
    void Deallocate(entity_id_t id_)
    {
        if (id_ < FIRST_LOCAL_ID)
            reserved.erase(id_);
        else
            reservedLocal.erase(id_);
    }
    
    /// Mark all IDs free and reset next ID.
    void Reset()
    {
        reserved.clear();
        reservedLocal.clear();
        id = 0;
        localId = FIRST_LOCAL_ID;
    }
    
    /// Return whether an ID is free
    bool IsFree(entity_id_t id_) const
    {
        if (id_ < FIRST_LOCAL_ID)
            return reserved.find(id_) == reserved.end();
        else
            return reservedLocal.find(id_) == reservedLocal.end();
    }
    
    /// Last returned ID
    entity_id_t id;
    /// Last returned local ID
    entity_id_t localId;
    /// Reserved ID's
    std::set<entity_id_t> reserved;
    /// Reserved local ID's
    std::set<entity_id_t> reservedLocal;
};
