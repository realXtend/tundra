#pragma once

#include "Component.h"
#include "RexUUID.h"

struct EntityPermission
{
    RexUUID userUUID;
    u32 permissionFlags;
};

/// An entity in a scene. Contains entity-components
class Entity : public clb::RefCountable
{
public:
    Entity(u32 newID);
    ~Entity();
    
    ComponentPtr GetComponent(const std::string& typeName, const std::string& name);
    ComponentPtr GetOrCreateComponent(const std::string& typeName, const std::string& name);
    void RemoveComponent(const std::string& typeName, const std::string& name);
    
    u32 entityID;
    RexUUID entityUUID;
    RexUUID creatorUUID;
    ComponentVector components;
    std::vector<EntityPermission> permissions;
};

typedef clb::SharedPtr<Entity> EntityPtr;
typedef std::vector<EntityPtr> EntityVector;
