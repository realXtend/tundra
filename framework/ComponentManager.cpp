// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ComponentManager.h"
#include "ModuleManager.h"
#include "Framework.h"
#include "ComponentFactoryInterface.h"
#include "ComponentInterface.h"

namespace Foundation
{
    ComponentInterfacePtr ComponentManager::createComponent(const std::string &componentName) const
    {
        ComponentFactoryMap::const_iterator iter = mFactories.find(componentName);
        if (iter == mFactories.end())
            return ComponentInterfacePtr();

        ComponentInterfacePtr component = (*iter->second.get())();

        return component;
    }

    ComponentInterfacePtr ComponentManager::cloneComponent(const ComponentInterfacePtr &component) const
    {
        ComponentFactoryMap::const_iterator iter = mFactories.find(component->_name());
        if (iter == mFactories.end())
            return ComponentInterfacePtr();

        ComponentInterfacePtr newComponent = (*iter->second.get())(component);

        return newComponent;
    }

    ComponentInterfacePtr ComponentManager::getComponent(Core::entity_id_t id, const std::string &component)
    {
        return ComponentInterfacePtr(); //! \todo FIXME
    }
}
