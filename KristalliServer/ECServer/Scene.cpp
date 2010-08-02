#include "Scene.h"

#include <iostream>

Scene::Scene() :
    /// \todo check how badly this clashes with OpenSim local ID's. (may also be different in different servers)
    nextEntityID(0x10000000)
{
}

EntityPtr Scene::GetEntity(u32 entityID)
{
    for (EntityVector::iterator i = entities.begin(); i != entities.end(); ++i)
    {
        if ((*i)->entityID == entityID)
            return (*i);
    }
    
    return EntityPtr();
}

EntityPtr Scene::GetEntity(u32 entityID, const RexUUID& entityUUID)
{
    // If UUID is null, look up only based by ID
    if (entityUUID.IsNull())
        return GetEntity(entityID);
    else
    {
        // If UUID is non-null, look up only based by UUID
        // Note: this means that the client & server can have ID mismatch, but be still referring to the same entity
        for (EntityVector::iterator i = entities.begin(); i != entities.end(); ++i)
        {
            if ((*i)->entityUUID == entityUUID)
                return (*i);
        }
    }
    return EntityPtr();
}

EntityPtr Scene::GetOrCreateEntity(u32 entityID)
{
    EntityPtr existing = GetEntity(entityID);
    if (existing)
        return existing;
    
    EntityPtr newEntity(new Entity(entityID));
    entities.push_back(newEntity);
    return newEntity;
}

void Scene::RemoveEntity(u32 entityID)
{
    for (EntityVector::iterator i = entities.begin(); i != entities.end(); ++i)
    {
        if ((*i)->entityID == entityID)
        {
            entities.erase(i);
            return;
        }
    }
}

void Scene::RemoveEntity(EntityPtr entity)
{
    for (EntityVector::iterator i = entities.begin(); i != entities.end(); ++i)
    {
        if ((*i) == entity)
        {
            entities.erase(i);
            return;
        }
    }
}

void Scene::Clear()
{
    entities.clear();
    nextEntityID = 0x10000000;
}

u32 Scene::GetNextFreeEntityID()
{
    for (;;)
    {
        if (!GetEntity(nextEntityID).ptr())
            return nextEntityID;
        nextEntityID++;
    }
}

