#pragma once

#include "clb/Core/Types.h"
#include "clb/Core/SharedPtr.h"
#include "clb/Core/SharedPtr.inl"
#include <string>
#include <vector>

class Entity;

/// An entity-component. So far only raw data storage, the server does not distinguish between component types or inspect their attributes
class Component : public clb::RefCountable
{
public:
    Component(const std::string& newTypeName, const std::string& newName) :
        typeName(newTypeName),
        name(newName),
        parentEntity(0)
    {
    }
    
    std::string typeName;
    std::string name;
    std::vector<u8> data;
    Entity* parentEntity;
};

typedef clb::SharedPtr<Component> ComponentPtr;
typedef std::vector<ComponentPtr> ComponentVector;

