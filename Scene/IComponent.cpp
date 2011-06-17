/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   IComponent.cpp
 *  @brief  Base class for all components. Inherit from this class when creating new components.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "IComponent.h"

#include "CoreStringUtils.h"
#include "Framework.h"
#include "Entity.h"
#include "Scene.h"

#include <QDomDocument>

#include <kNet.h>

#include "MemoryLeakCheck.h"

IComponent::IComponent(Scene* scene) :
    parentEntity(0),
    framework(scene ? scene->GetFramework() : 0),
    networkSync(true),
    updateMode(AttributeChange::Replicate),
    temporary(false)
{
}

IComponent::IComponent(const IComponent &rhs) :
    framework(rhs.framework),
    parentEntity(rhs.parentEntity),
    networkSync(rhs.networkSync),
    updateMode(rhs.updateMode),
    temporary(false)
{
}

IComponent::~IComponent()
{
}

void IComponent::SetName(const QString& name_)
{
    // no point to send a signal if name have stayed same as before.
    if (name == name_)
        return;

    QString oldName = name;
    name = name_;
    emit ComponentNameChanged(name, oldName);
}

void IComponent::SetUpdateMode(AttributeChange::Type defaultmode)
{
    // Note: we can't allow default mode to be Default, because that would be meaningless
    if (defaultmode != AttributeChange::Default)
        updateMode = defaultmode;
}

void IComponent::SetParentEntity(Entity* entity)
{
    parentEntity = entity;
    if (parentEntity)
        emit ParentEntitySet();
    else
        emit ParentEntityDetached();
}

Entity* IComponent::ParentEntity() const
{
    return parentEntity;
}

Scene* IComponent::ParentScene() const
{
    if (!parentEntity)
        return 0;
    return parentEntity->ParentScene();
}

void IComponent::SetNetworkSyncEnabled(bool enabled)
{
    networkSync = enabled;
}

QVariant IComponent::GetAttributeQVariant(const QString &name) const
{
    for(AttributeVector::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter)
        if ((*iter)->Name() == name)
            return (*iter)->ToQVariant();

    return QVariant();
}

QStringList IComponent::GetAttributeNames() const
{
    QStringList attribute_list;
    for(AttributeVector::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter)
        attribute_list.push_back((*iter)->Name());
    return attribute_list;
}

IAttribute* IComponent::GetAttribute(const QString &name) const
{
    for(unsigned int i = 0; i < attributes.size(); ++i)
        if(attributes[i]->Name() == name)
            return attributes[i];
    return 0;
}

QDomElement IComponent::BeginSerialization(QDomDocument& doc, QDomElement& base_element) const
{
    QDomElement comp_element = doc.createElement("component");
    comp_element.setAttribute("type", TypeName());
    if (!Name().isEmpty())
        comp_element.setAttribute("name", Name());
    // Components with no network sync are never network-serialized. However we might be serializing to a file
    comp_element.setAttribute("sync", QString::fromStdString(ToString<bool>(networkSync)));
    
    if (!base_element.isNull())
        base_element.appendChild(comp_element);
    else
        doc.appendChild(comp_element);
    
    return comp_element;
}

void IComponent::WriteAttribute(QDomDocument& doc, QDomElement& comp_element, const QString& name, const QString& value) const
{
    QDomElement attribute_element = doc.createElement("attribute");
    attribute_element.setAttribute("name", name);
    attribute_element.setAttribute("value", value);
    comp_element.appendChild(attribute_element);
}

void IComponent::WriteAttribute(QDomDocument& doc, QDomElement& comp_element, const QString& name, const QString& value, const QString &type) const
{
    QDomElement attribute_element = doc.createElement("attribute");
    attribute_element.setAttribute("name", name);
    attribute_element.setAttribute("value", value);
    attribute_element.setAttribute("type", type);
    comp_element.appendChild(attribute_element);
}

bool IComponent::BeginDeserialization(QDomElement& comp_element)
{
    QString type = comp_element.attribute("type");
    if (type == TypeName())
    {
        SetName(comp_element.attribute("name"));
        SetNetworkSyncEnabled(ParseString<bool>(comp_element.attribute("sync").toStdString(), true));
        return true;
    }
    return false;
}

QString IComponent::ReadAttribute(QDomElement& comp_element, const QString &name) const
{
    QDomElement attribute_element = comp_element.firstChildElement("attribute");
    while(!attribute_element.isNull())
    {
        if (attribute_element.attribute("name") == name)
            return attribute_element.attribute("value");
        
        attribute_element = attribute_element.nextSiblingElement("attribute");
    }
    
    return QString();
}

QString IComponent::ReadAttributeType(QDomElement& comp_element, const QString &name) const
{
    QDomElement attribute_element = comp_element.firstChildElement("attribute");
    while(!attribute_element.isNull())
    {
        if (attribute_element.attribute("name") == name)
            return attribute_element.attribute("type");
        
        attribute_element = attribute_element.nextSiblingElement("attribute");
    }
    
    return QString();
}

void IComponent::EmitAttributeChanged(IAttribute* attribute, AttributeChange::Type change)
{
    if (change == AttributeChange::Default)
        change = updateMode;
    if (change == AttributeChange::Disconnected)
        return; // No signals
    
    // Trigger scenemanager signal
    Scene* scene = ParentScene();
    if (scene)
        scene->EmitAttributeChanged(this, attribute, change);
    
    // Trigger internal signal
    emit AttributeChanged(attribute, change);
}

void IComponent::EmitAttributeChanged(const QString& attributeName, AttributeChange::Type change)
{
    if (change == AttributeChange::Disconnected)
        return; // No signals

    // Roll through attributes and check name match
    for(uint i = 0; i < attributes.size(); ++i)
        if (attributes[i]->Name() == attributeName)
        {
            EmitAttributeChanged(attributes[i], change);
            break;
        }
}

void IComponent::SerializeTo(QDomDocument& doc, QDomElement& base_element) const
{
    QDomElement comp_element = BeginSerialization(doc, base_element);

    for(uint i = 0; i < attributes.size(); ++i)
        WriteAttribute(doc, comp_element, attributes[i]->Name(), attributes[i]->ToString().c_str());
}

/// Returns true if the given XML element has the given child attribute.
bool HasAttribute(QDomElement &comp_element, const QString &name)
{
    QDomElement attribute_element = comp_element.firstChildElement("attribute");
    while(!attribute_element.isNull())
    {
        if (attribute_element.attribute("name") == name)
            return true;
        attribute_element = attribute_element.nextSiblingElement("attribute");
    }

    return false;
}

void IComponent::DeserializeFrom(QDomElement& element, AttributeChange::Type change)
{
    if (!BeginDeserialization(element))
        return;

    // When we are deserializing the component from XML, only apply those attribute values which are present in that XML element.
    // For all other elements, use the current value in the attribute (if this is a newly allocated component, the current value
    // is the default value for that attribute specified in ctor. If this is an existing component, the DeserializeFrom can be 
    // thought of applying the given "delta modifications" from the XML element).

    for(uint i = 0; i < attributes.size(); ++i)
    {
        if (HasAttribute(element, attributes[i]->Name()))
        {
            std::string attr_str = ReadAttribute(element, attributes[i]->Name()).toStdString();
            attributes[i]->FromString(attr_str, change);
        }
    }
}

void IComponent::SerializeToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<u8>(attributes.size());
    for(uint i = 0; i < attributes.size(); ++i)
        attributes[i]->ToBinary(dest);
}

void IComponent::DeserializeFromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    u8 num_attributes = source.Read<u8>();
    if (num_attributes != attributes.size())
    {
        std::cout << "Wrong number of attributes in DeserializeFromBinary!" << std::endl;
        return;
    }
    for(uint i = 0; i < attributes.size(); ++i)
        attributes[i]->FromBinary(source, change);
}

void IComponent::ComponentChanged(AttributeChange::Type change)
{
    for(uint i = 0; i < attributes.size(); ++i)
        EmitAttributeChanged(attributes[i], change);
}

void IComponent::SetTemporary(bool enable)
{
    temporary = enable;
}

bool IComponent::IsTemporary() const
{
    if ((parentEntity) && (parentEntity->IsTemporary()))
        return true;
    return temporary;
}

bool IComponent::ViewEnabled() const
{
    if (!parentEntity)
        return true;
    Scene* scene = parentEntity->ParentScene();
    if (scene)
        return scene->ViewEnabled();
    else
        return true;
}
