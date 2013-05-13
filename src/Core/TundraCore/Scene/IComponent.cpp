/*
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   IComponent.cpp
    @brief  The common interface for all components, which are the building blocks the scene entities are formed of. */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "IComponent.h"
#include "Entity.h"
#include "Scene/Scene.h"
#include "SceneAPI.h"

#include "CoreStringUtils.h"
#include "Framework.h"
#include "LoggingFunctions.h"

#include <QDomDocument>

#include <kNet.h>

#include "MemoryLeakCheck.h"

IComponent::IComponent(Scene* scene) :
    parentEntity(0),
    framework(scene ? scene->GetFramework() : 0),
    updateMode(AttributeChange::Replicate),
    replicated(true),
    temporary(false),
    id(0)
{
}

IComponent::~IComponent()
{
    foreach(IAttribute *a, attributes)
    {
        if (a && a->IsDynamic())
            SAFE_DELETE(a);
    }
}

void IComponent::SetNewId(entity_id_t newId)
{
    id = newId;
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

bool IComponent::IsUnacked() const
{
    return id >= UniqueIdGenerator::FIRST_UNACKED_ID && id < UniqueIdGenerator::FIRST_LOCAL_ID;
}

void IComponent::SetNetworkSyncEnabled(bool /*enable*/)
{
    LogWarning("SetNetworkSyncEnabled called. This function is deprecated and does not do anything.");
}

void IComponent::SetUpdateMode(AttributeChange::Type defaultMode)
{
    // Note: we can't allow default mode to be Default, because that would be meaningless
    if (defaultMode == AttributeChange::Disconnected || defaultMode == AttributeChange::LocalOnly ||
        defaultMode == AttributeChange::Replicate)
    {
        updateMode = defaultMode;
    }
    else
    {
        LogWarning("IComponent::SetUpdateMode: Trying to set default update mode to an invalid value! (" + QString::number((int)defaultMode) + ")");
    }
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

void IComponent::SetReplicated(bool enable)
{
    if (id)
    {
        LogError("Replication mode can not be changed after an ID has been assigned!");
        return;
    }
    
    replicated = enable;
}

AttributeVector IComponent::NonEmptyAttributes() const
{
    AttributeVector ret;
    for (unsigned i = 0; i < attributes.size(); ++i)
        if (attributes[i])
            ret.push_back(attributes[i]);
    return ret;
}

QVariant IComponent::GetAttributeQVariant(const QString &name) const
{
    for(AttributeVector::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter)
        if ((*iter) && (*iter)->Name().compare(name, Qt::CaseInsensitive) == 0)
            return (*iter)->ToQVariant();

    return QVariant();
}

QStringList IComponent::GetAttributeNames() const
{
    QStringList attribute_list;
    for(AttributeVector::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter)
        if (*iter)
            attribute_list.push_back((*iter)->Name());
    return attribute_list;
}

IAttribute* IComponent::GetAttribute(const QString &name) const
{
    for(unsigned int i = 0; i < attributes.size(); ++i)
        if(attributes[i] && attributes[i]->Name().compare(name, Qt::CaseInsensitive) == 0)
            return attributes[i];
    return 0;
}

int IComponent::NumAttributes() const
{
    int ret = 0;
    for (unsigned int i = 0; i < attributes.size(); ++i)
        if(attributes[i])
            ++ret;
    return ret;
}

int IComponent::NumStaticAttributes() const
{
    int ret = 0;
    for (unsigned int i = 0; i < attributes.size(); ++i)
    {
        // Break when the first hole or dynamically allocated attribute is encountered
        if (attributes[i] && !attributes[i]->IsDynamic())
            ++ret;
        else
            break;
    }
    return ret;
}

IAttribute* IComponent::CreateAttribute(u8 index, u32 typeID, const QString& name, AttributeChange::Type change)
{
    if (!SupportsDynamicAttributes())
    {
        LogError("CreateAttribute called on a component that does not support dynamic attributes");
        return 0;
    }
    
    // If this message should be sent with the default attribute change mode specified in the IComponent,
    // take the change mode from this component.
    if (change == AttributeChange::Default)
        change = updateMode;
    assert(change != AttributeChange::Default);

    IAttribute *attribute = SceneAPI::CreateAttribute(typeID, name);
    if(!attribute)
        return 0;
    
    if (!AddAttribute(attribute, index))
    {
        delete attribute;
        return 0;
    }
    
    // Trigger scenemanager signal
    Scene* scene = ParentScene();
    if (scene)
        scene->EmitAttributeAdded(this, attribute, change);
    
    // Trigger internal signal
    emit AttributeAdded(attribute);
    EmitAttributeChanged(attribute, change);
    return attribute;
}

void IComponent::RemoveAttribute(u8 index, AttributeChange::Type change)
{
    if (!SupportsDynamicAttributes())
    {
        LogError("RemoveAttribute called on a component that does not support dynamic attributes");
        return;
    }
    
    // If this message should be sent with the default attribute change mode specified in the IComponent,
    // take the change mode from this component.
    if (change == AttributeChange::Default)
        change = updateMode;
    assert(change != AttributeChange::Default);

    if (index < attributes.size() && attributes[index])
    {
        IAttribute* attr = attributes[index];
        if (!attr->IsDynamic())
        {
            LogError("Can not remove static attribute at index " + QString::number(index));
            return;
        }
        
        // Trigger scenemanager signal
        Scene* scene = ParentScene();
        if (scene)
            scene->EmitAttributeRemoved(this, attr, change);
        
        // Trigger internal signal(s)
        emit AttributeAboutToBeRemoved(attr);
        SAFE_DELETE(attributes[index]);
    }
    else
        LogError("Can not remove nonexisting attribute at index " + QString::number(index));
}

void IComponent::AddAttribute(IAttribute* attr)
{
    if (!attr)
        return;
    // If attribute is static (member variable attributes), we can just push_back it.
    if (!attr->IsDynamic())
    {
        attr->index = (u8)attributes.size();
        attr->owner = this;
        attributes.push_back(attr);
    }
    else
    {
        // For dynamic attributes, need to scan for holes first, and then push_back if no holes
        for (unsigned i = 0; i < attributes.size(); ++i)
        {
            if (!attributes[i])
            {
                attr->index = i;
                attr->owner = this;
                attributes[i] = attr;
                return;
            }
        }
        attr->index = (u8)attributes.size();
        attr->owner = this;
        attributes.push_back(attr);
    }
}

bool IComponent::AddAttribute(IAttribute* attr, u8 index)
{
    if (!attr)
        return false;
    if (index < attributes.size())
    {
        IAttribute* existing = attributes[index];
        if (existing)
        {
            if (!existing->IsDynamic())
            {
                LogError("Can not overwrite static attribute at index " + QString::number(index));
                return false;
            }
            else
            {
                LogWarning("Removing existing attribute at index " + QString::number(index) + " to make room for new attribute");
                delete existing;
                attributes[index] = 0;
            }
        }
    }
    else
    {
        // Make enough holes until we can reach the index
        while (attributes.size() <= index)
            attributes.push_back(0);
    }
    
    attr->index = index;
    attr->owner = this;
    attributes[index] = attr;
    return true;
}

QDomElement IComponent::BeginSerialization(QDomDocument& doc, QDomElement& base_element, bool serializeTemporary) const
{
    QDomElement comp_element = doc.createElement("component");
    comp_element.setAttribute("type", TypeName());
    if (!Name().isEmpty())
        comp_element.setAttribute("name", Name());
    // Components with no network sync are never network-serialized. However we might be serializing to a file
    comp_element.setAttribute("sync", BoolToString(replicated));
    if (serializeTemporary)
        comp_element.setAttribute("temporary", BoolToString(temporary));

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
        return true;
    }
    return false;
}

QString IComponent::ReadAttribute(QDomElement& comp_element, const QString &name) const
{
    QDomElement attribute_element = comp_element.firstChildElement("attribute");
    while(!attribute_element.isNull())
    {
        if (attribute_element.attribute("name").compare(name, Qt::CaseInsensitive) == 0)
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
        if (attribute_element.attribute("name").compare(name, Qt::CaseInsensitive) == 0)
            return attribute_element.attribute("type");
        
        attribute_element = attribute_element.nextSiblingElement("attribute");
    }
    
    return QString();
}

void IComponent::EmitAttributeChanged(IAttribute* attribute, AttributeChange::Type change)
{
    // If this message should be sent with the default attribute change mode specified in the IComponent,
    // take the change mode from this component.
    if (change == AttributeChange::Default)
        change = updateMode;
    assert(change != AttributeChange::Default);

    if (change == AttributeChange::Disconnected)
        return; // No signals
    
    // Trigger scenemanager signal
    Scene* scene = ParentScene();
    if (scene)
        scene->EmitAttributeChanged(this, attribute, change);
    
    // Trigger internal signal
    emit AttributeChanged(attribute, change);

    // Tell the derived class that some attributes have changed.
    AttributesChanged();
    // After having notified the derived class, clear all change bits on all attributes,
    // since the derived class has reacted on them. 
    for(size_t i = 0; i < attributes.size(); ++i)
        if (attributes[i])
            attributes[i]->ClearChangedFlag();
}

void IComponent::EmitAttributeChanged(const QString& attributeName, AttributeChange::Type change)
{
    // If this message should be sent with the default attribute change mode specified in the IComponent,
    // take the change mode from this component.
    if (change == AttributeChange::Default)
        change = updateMode;
    assert(change != AttributeChange::Default);

    if (change == AttributeChange::Disconnected)
        return; // No signals

    // Roll through attributes and check name match
    for(uint i = 0; i < attributes.size(); ++i)
        if (attributes[i] && attributes[i]->Name() == attributeName)
        {
            EmitAttributeChanged(attributes[i], change);
            break;
        }
}

void IComponent::SerializeTo(QDomDocument& doc, QDomElement& base_element, bool serializeTemporary) const
{
    QDomElement comp_element = BeginSerialization(doc, base_element, serializeTemporary);

    for(uint i = 0; i < attributes.size(); ++i)
        if (attributes[i])
            WriteAttribute(doc, comp_element, attributes[i]->Name(), attributes[i]->ToString().c_str());
}

/// Returns true if the given XML element has the given child attribute.
static bool HasAttribute(QDomElement &comp_element, const QString &name)
{
    QDomElement attribute_element = comp_element.firstChildElement("attribute");
    while(!attribute_element.isNull())
    {
        if (attribute_element.attribute("name").compare(name, Qt::CaseInsensitive) == 0)
            return true;
        attribute_element = attribute_element.nextSiblingElement("attribute");
    }

    return false;
}

void IComponent::DeserializeFrom(QDomElement& element, AttributeChange::Type change)
{
    if (!BeginDeserialization(element))
        return;

    // If this message should be sent with the default attribute change mode specified in the IComponent,
    // take the change mode from this component.
    if (change == AttributeChange::Default)
        change = updateMode;
    assert(change != AttributeChange::Default);

    // When we are deserializing the component from XML, only apply those attribute values which are present in that XML element.
    // For all other elements, use the current value in the attribute (if this is a newly allocated component, the current value
    // is the default value for that attribute specified in ctor. If this is an existing component, the DeserializeFrom can be 
    // thought of applying the given "delta modifications" from the XML element).

    for(uint i = 0; i < attributes.size(); ++i)
    {
        if (attributes[i] && HasAttribute(element, attributes[i]->Name()))
        {
            std::string attr_str = ReadAttribute(element, attributes[i]->Name()).toStdString();
            attributes[i]->FromString(attr_str, change);
        }
    }
}

void IComponent::SerializeToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<u8>((u8)attributes.size());
    for(uint i = 0; i < attributes.size(); ++i)
        if (attributes[i])
            attributes[i]->ToBinary(dest);
}

void IComponent::DeserializeFromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    u8 num_attributes = source.Read<u8>();
    if (num_attributes != NumAttributes())
    {
        LogError("Wrong number of attributes in DeserializeFromBinary!");
        return;
    }
    for(uint i = 0; i < attributes.size(); ++i)
        if (attributes[i])
            attributes[i]->FromBinary(source, change);
}

void IComponent::ComponentChanged(AttributeChange::Type change)
{
    // If this message should be sent with the default attribute change mode specified in the IComponent,
    // take the change mode from this component.
    if (change == AttributeChange::Default)
        change = updateMode;

    // We are signalling attribute changes, but the desired change type is saying "don't signal about changes".
    assert(change != AttributeChange::Default && change != AttributeChange::Disconnected);

    for(uint i = 0; i < attributes.size(); ++i)
        if (attributes[i])
            EmitAttributeChanged(attributes[i], change);
}

void IComponent::SetTemporary(bool enable)
{
    temporary = enable;
}

bool IComponent::IsTemporary() const
{
    if (parentEntity && parentEntity->IsTemporary())
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
