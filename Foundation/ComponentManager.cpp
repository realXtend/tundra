// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ComponentManager.h"
//#include "ModuleManager.h"
#include "Framework.h"
#include "ComponentFactoryInterface.h"
#include "ComponentInterface.h"

namespace Foundation
{
    bool ComponentManager::CanCreate(const std::string &type)
    {
        return (factories_.find(type) != factories_.end());
    }

    ComponentInterfacePtr ComponentManager::CreateComponent(const std::string &type)
    {
        ComponentFactoryMap::const_iterator iter = factories_.find(type);
        if (iter == factories_.end())
            return ComponentInterfacePtr();

        ComponentInterfacePtr component = (*iter->second.get())();

        return component;
    }

    ComponentInterfacePtr ComponentManager::CloneComponent(const ComponentInterfacePtr &component)
    {
        ComponentFactoryMap::const_iterator iter = factories_.find(component->Name());
        if (iter == factories_.end())
            return ComponentInterfacePtr();

        ComponentInterfacePtr newComponent = (*iter->second.get())(component);
        
        return newComponent;
    }
}
