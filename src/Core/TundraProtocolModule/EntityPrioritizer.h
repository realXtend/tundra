// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraProtocolModuleFwd.h"
#include "TundraProtocolModuleApi.h"
#include "SceneFwd.h"

/// Subclass and provide the implementation to SyncManager to perform application-specific entity prioritizing.
/** The base class implementations of functions are no-ops.
    @remark Interest management */
class TUNDRAPROTOCOL_MODULE_API EntityPrioritizer
{
public:
    /// Computes priorities provided entity sync states.
    /// @todo In addition, or instead, could provide a function with begin and end iterators as input. This could useful
    /// when dealing when very large container and only a fixed number of priority calculation would be wanted per frame.
    virtual void ComputeSyncPriorities(EntitySyncStateMap & UNUSED_PARAM(entities), const float3 & UNUSED_PARAM(observerPos),const float3 & UNUSED_PARAM(observerRot))
    {
    }

     /// @overload
    virtual void ComputeSyncPriorities(EntitySyncState & UNUSED_PARAM(entityState), const float3 & UNUSED_PARAM(observerPos), const float3 & UNUSED_PARAM(observerRot))
    {
    }

    /// @todo Provide virtual Sort() function? Prioritizer could sort then dirty queue using custom predicates.
};

/// Subclass to perform application-specific entity prioritizing.
class TUNDRAPROTOCOL_MODULE_API DefaultEntityPrioritizer : public EntityPrioritizer
{
public:
    explicit DefaultEntityPrioritizer(const SceneWeakPtr &syncedScene) : scene(syncedScene) {}
    /// EntityPrioritizer override
    void ComputeSyncPriorities(EntitySyncStateMap &entities, const float3 &observerPos,const float3 &observerRot);
    /// EntityPrioritizer override
    void ComputeSyncPriorities(EntitySyncState &entityState, const float3 &observerPos, const float3 &observerRot);

    SceneWeakPtr scene;
};
