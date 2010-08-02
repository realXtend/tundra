#pragma once

#include "Entity.h"

/// Scene that contains entities
class Scene
{
public:
    Scene();
    
    EntityPtr GetEntity(u32 entityID);
    EntityPtr GetEntity(u32 entityID, const RexUUID& entityUUID);
    EntityPtr GetOrCreateEntity(u32 entityID);
    void RemoveEntity(u32 entityID);
    void RemoveEntity(EntityPtr entity);
    void Clear();
    u32 GetNextFreeEntityID();
    
    EntityVector entities;
    u32 nextEntityID;
};
