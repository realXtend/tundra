#include "Entity.h"

Entity::Entity(u32 newID) :
    entityID(newID)
{
}

Entity::~Entity()
{
    for (ComponentVector::iterator i = components.begin(); i != components.end(); ++i)
        (*i)->parentEntity = 0;
    components.clear();
}

ComponentPtr Entity::GetComponent(const std::string& typeName, const std::string& name)
{
    for (ComponentVector::iterator i = components.begin(); i != components.end(); ++i)
    {
        if (((*i)->typeName == typeName) && ((*i)->name == name))
            return (*i);
    }
    
    return ComponentPtr();
}

ComponentPtr Entity::GetOrCreateComponent(const std::string& typeName, const std::string& name)
{
    ComponentPtr existing = GetComponent(typeName, name);
    if (existing.ptr())
        return existing;
        
    ComponentPtr newComp = new Component(typeName, name);
    newComp->parentEntity = this;
    components.push_back(newComp);

    return newComp;
}

void Entity::RemoveComponent(const std::string& typeName, const std::string& name)
{
    for (ComponentVector::iterator i = components.begin(); i != components.end(); ++i)
    {
        if (((*i)->typeName == typeName) && ((*i)->name == name))
        {
            (*i)->parentEntity = 0;
            components.erase(i);
            return;
        }
    }
    
    std::cout << "Component " << typeName << " " << name << " not found, cannot remove" << std::endl;
}
