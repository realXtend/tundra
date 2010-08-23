// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ComponentManager.h"
#include "Framework.h"
#include "ComponentFactoryInterface.h"
#include "ComponentInterface.h"
#include "AttributeInterface.h"
#include "AssetInterface.h"

#include <QVariant>

namespace Foundation
{
    ComponentManager::ComponentManager(Framework *framework) : framework_(framework)
    {
        attributeTypes_.push_back("string");
        attributeTypes_.push_back("int");
        attributeTypes_.push_back("real");
        attributeTypes_.push_back("color");
        attributeTypes_.push_back("vector3df");
        attributeTypes_.push_back("bool");
        attributeTypes_.push_back("uint");
        attributeTypes_.push_back("quaternion");
        attributeTypes_.push_back("assetreference");
        attributeTypes_.push_back("qvariant");
        attributeTypes_.push_back("qvariantarray");
    }

    bool ComponentManager::CanCreate(const QString &type_name)
    {
        return (factories_.find(type_name) != factories_.end());
    }

    ComponentInterfacePtr ComponentManager::CreateComponent(const QString &type_name)
    {
        ComponentFactoryMap::const_iterator iter = factories_.find(type_name);
        if (iter == factories_.end())
            return ComponentInterfacePtr();

        ComponentInterfacePtr component = (*iter->second.get())();
        return component;
    }

    ComponentPtr ComponentManager::CreateComponent(const QString &type_name, const QString &name)
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

    AttributeInterface *ComponentManager::CreateAttribute(ComponentInterface *owner, const std::string &typeName, const std::string &name)
    {
        AttributeInterface *attribute = 0;
        if (typeName == "string")
            attribute = new Attribute<std::string>(owner, name.c_str());
        else if(typeName == "int")
            attribute = new Attribute<int>(owner, name.c_str());
        else if(typeName == "real")
            attribute = new Attribute<Real>(owner, name.c_str());
        else if(typeName == "color")
            attribute = new Attribute<Color>(owner, name.c_str());
        else if(typeName == "vector3df")
            attribute = new Attribute<Vector3df>(owner, name.c_str());
        else if(typeName == "bool")
            attribute = new Attribute<bool>(owner, name.c_str());
        else if(typeName == "uint")
            attribute = new Attribute<uint>(owner, name.c_str());
        else if(typeName == "quaternion")
            attribute = new Attribute<Quaternion>(owner, name.c_str());
        else if(typeName == "assetreference")
            attribute = new Attribute<AssetReference>(owner, name.c_str());
        else if(typeName == "qvariant")
            attribute = new Attribute<QVariant>(owner, name.c_str());
        else if(typeName == "qvariantarray")
            attribute = new Attribute<std::vector<QVariant> >(owner, name.c_str());
        return attribute;
    }

    StringVector ComponentManager::GetAttributeTypes() const
    {
        return attributeTypes_;
    }
}
