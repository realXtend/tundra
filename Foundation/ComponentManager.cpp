// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ComponentManager.h"
//#include "ModuleManager.h"
#include "Framework.h"
#include "ComponentFactoryInterface.h"
#include "ComponentInterface.h"

namespace Foundation
{
    ComponentInterfacePtr ComponentManager::CreateComponent(const std::string &type) const
    {
        ComponentFactoryMap::const_iterator iter = factories_.find(type);
        if (iter == factories_.end())
            return ComponentInterfacePtr();

        ComponentInterfacePtr component = (*iter->second.get())();

        return component;
    }

    ComponentInterfacePtr ComponentManager::CloneComponent(const ComponentInterfacePtr &component) const
    {
        ComponentFactoryMap::const_iterator iter = factories_.find(component->Name());
        if (iter == factories_.end())
            return ComponentInterfacePtr();

        ComponentInterfacePtr newComponent = (*iter->second.get())(component);

        return newComponent;
    }

    ComponentInterfacePtr ComponentManager::GetComponent(Core::entity_id_t id, const std::string &component)
    {
        return ComponentInterfacePtr(); //! \todo FIXME
    }
}
