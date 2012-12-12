// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"
#include "CoreTypes.h"
#include <set>

/// Generates unique integer ID's.
/** Used for entity and component ID's. Supports both a local range and replicated range, which the high bit determines.
///\todo This class (needlessly) duplicates the reserved ID's set, which also is contained in the scene & entity object maps. */
class TUNDRACORE_API UniqueIdGenerator
{
public:
    // Last replicated ID is guaranteed to fit inside 30 bits since we send them into the network as VLE8/16/32.
    static const entity_id_t LAST_REPLICATED_ID = 0x3fffffff; // Used as a bitmask, so must be of form 2^n-1.
    static const entity_id_t FIRST_UNACKED_ID = 0x40000000;
    static const entity_id_t FIRST_LOCAL_ID = 0x80000000;
    
    /// Construct
    UniqueIdGenerator() :
        id(0),
        unackedId(FIRST_UNACKED_ID),
        localId(FIRST_LOCAL_ID)
    {
    }
    
    /// Allocate and return the next replicated ID. Never returns zero. 
    /** Clients should never allocate replicated ID's on their own. Instead, server sends a reply upon assigning the proper ID. */
    entity_id_t AllocateReplicated()
    {
        ++id;
        if (id > LAST_REPLICATED_ID) id = 1;
        return id;
    }
    
    /// Allocate and return an unacked ID.
    entity_id_t AllocateUnacked()
    {
        ++unackedId;
        if (unackedId == FIRST_LOCAL_ID) unackedId = FIRST_UNACKED_ID + 1;
        return unackedId;
    }
    
    
    /// Allocate and return the next local ID. Never returns zero.
    entity_id_t AllocateLocal()
    {
        ++localId;
        if (localId == 0) localId = FIRST_LOCAL_ID + 1;
        return localId;
    }
    
    /// Manually reset the replicated ID generator to a specific value. The next returned ID will be value + 1.
    void ResetReplicatedId(entity_id_t id_)
    {
        id = id_ & LAST_REPLICATED_ID;
    }
    
    /// Reset all ID generators.
    void Reset()
    {
        id = 0;
        unackedId = FIRST_UNACKED_ID;
        localId = FIRST_LOCAL_ID;
    }
    
    /// Last returned ID
    entity_id_t id;
    /// Last returned unacked ID
    entity_id_t unackedId;
    /// Last returned local ID
    entity_id_t localId;
};
