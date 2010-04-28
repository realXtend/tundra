// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ComponentManager.h"
#include "Framework.h"
#include "ComponentFactoryInterface.h"
#include "ComponentInterface.h"

namespace Foundation
{
    bool ComponentManager::CanCreate(const std::string &type_name)
    {
        return (factories_.find(type_name) != factories_.end());
    }

    ComponentInterfacePtr ComponentManager::CreateComponent(const std::string &type_name)
    {
        ComponentFactoryMap::const_iterator iter = factories_.find(type_name);
        if (iter == factories_.end())
            return ComponentInterfacePtr();

        ComponentInterfacePtr component = (*iter->second.get())();
        return component;
    }

    ComponentPtr ComponentManager::CreateComponent(const std::string &type_name, const std::string &name)
    {
        ComponentFactoryMap::const_iterator iter = factories_.find(type_name);
        if (iter == factories_.end())
            return ComponentInterfacePtr();

        ComponentInterfacePtr component = (*iter->second.get())();
        component->SetName(name);
        return component;
    }

    ComponentInterfacePtr ComponentManager::CloneComponent(const ComponentInterfacePtr &component)
    {
        ComponentFactoryMap::const_iterator iter = factories_.find(component->TypeName());
        if (iter == factories_.end())
            return ComponentInterfacePtr();

        ComponentInterfacePtr newComponent = (*iter->second.get())(component);
        return newComponent;
    }
}
