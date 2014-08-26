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
    internalQObjectPropertyUpdateOngoing_(false),
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
    {
         // Make sure that framework pointer will be valid, in case this component was originally created unparented.
        framework = parentEntity->GetFramework();
        emit ParentEntitySet();
    }
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

QVariant IComponent::GetAttributeQVariant(const QString &id) const
{
    IAttribute* attr = AttributeById(id);
    if (!attr)
        attr = AttributeByName(id);
    return attr ? attr->ToQVariant() : QVariant();
}

QStringList IComponent::GetAttributeNames() const
{
    QStringList attribute_list;
    for(AttributeVector::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter)
        if (*iter)
            attribute_list.push_back((*iter)->Name());
    return attribute_list;
}

QStringList IComponent::GetAttributeIds() const
{
    QStringList attribute_list;
    for(AttributeVector::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter)
        if (*iter)
            attribute_list.push_back((*iter)->Id());
    return attribute_list;
}


IAttribute* IComponent::GetAttribute(const QString &name) const
{
    LogWarning("IComponent::GetAttribute(name) is deprecated and will be removed. Use AttributeByName.");
    return AttributeByName(name);
}

IAttribute* IComponent::AttributeById(const QString &id) const
{
    for(size_t i = 0; i < attributes.size(); ++i)
        if(attributes[i] && attributes[i]->Id().compare(id, Qt::CaseInsensitive) == 0)
            return attributes[i];
    return 0;
}

IAttribute* IComponent::AttributeByName(const QString &name) const
{
    for(size_t i = 0; i < attributes.size(); ++i)
        if(attributes[i] && attributes[i]->Name().compare(name, Qt::CaseInsensitive) == 0)
            return attributes[i];
    return 0;
}

int IComponent::NumAttributes() const
{
    int ret = 0;
    for(size_t i = 0; i < attributes.size(); ++i)
        if(attributes[i])
            ++ret;
    return ret;
}

int IComponent::NumStaticAttributes() const
{
    int ret = 0;
    for(size_t i = 0; i < attributes.size(); ++i)
    {
        // Break when the first hole or dynamically allocated attribute is encountered
        if (attributes[i] && !attributes[i]->IsDynamic())
            ++ret;
        else
            break;
    }
    return ret;
}

IAttribute* IComponent::CreateAttribute(u8 index, u32 typeID, const QString& id, AttributeChange::Type change)
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

    IAttribute *attribute = SceneAPI::CreateAttribute(typeID, id);
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

        // Create dynamic QObject property
        CreateDynamicProperty(attr);
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

    // Create dynamic QObject property
    if (attr->IsDynamic())
        CreateDynamicProperty(attr);

    return true;
}

void IComponent::CreateDynamicProperty(IAttribute* attribute)
{
    // Connect to attribute change signal. We will update the QVariant dynamic property there.
    // Adds a bit of overhead vs. a static component with an extra attr->ToQVariant() but will
    // give us a nicer API. Lets try to only connect the signal when the first dynamic attribute
    // is added. Although this can happen multiple times if all attributes are removed at some point,
    // the Qt::UniqueConnection will protect against multiple connections.
    if (dynamicPropertyNames_.isEmpty())
    {
        // Remove it first so that we wont add multiple filters if we end up
        // here multiple times. Its a no-op if not yet added. See above notes.
        removeEventFilter(this);
        installEventFilter(this);

        connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)),
            this, SLOT(UpdateDynamicProperty(IAttribute*, AttributeChange::Type)), Qt::UniqueConnection);
        connect(this, SIGNAL(AttributeAboutToBeRemoved(IAttribute*)),
            this, SLOT(RemoveDynamicProperty(IAttribute*)), Qt::UniqueConnection);
    }

    // Dynamic attributes must be manually handled via QObject::setAttribute so that
    // they are exposed correctly for scripting and QObject::dynamicPropertyNames().
    QString dynamicPropertyName = CamelCase(attribute->Id());
    if (!dynamicPropertyName.isEmpty())
    {
        // Needs to start with alphabet or _. Needs to end with any number of alphanumerics and _.
        // Otherwise skip adding as a dynamic QObject property.
        static QRegExpValidator propNameValidator(QRegExp("^[a-zA-Z_][a-zA-Z0-9_]*$"));
        int errorPos = 0;
        if (propNameValidator.validate(dynamicPropertyName, errorPos) == QValidator::Acceptable)
        {
            QByteArray asciiPropName = dynamicPropertyName.toAscii();
            dynamicPropertyNames_[attribute->Id()] = asciiPropName;
            setProperty(asciiPropName.data(), attribute->ToQVariant());
        }
    }
}

void IComponent::UpdateDynamicProperty(IAttribute* attribute, AttributeChange::Type /*change*/)
{
    if (!dynamicPropertyNames_.contains(attribute->Id()))
        return;

    if (internalQObjectPropertyUpdateOngoing_)
        return;
    internalQObjectPropertyUpdateOngoing_ = true;

    QVariant variantValue = attribute->ToQVariant();
    QByteArray dynamicPropertyName = dynamicPropertyNames_[attribute->Id()];
    setProperty(dynamicPropertyName.data(), variantValue);

    internalQObjectPropertyUpdateOngoing_ = false;
}

void IComponent::RemoveDynamicProperty(IAttribute* attribute)
{
    if (!dynamicPropertyNames_.contains(attribute->Id()))
        return;

    /// @bug Investigate why after when a attribute is removed from a dynamic component
    /// the UpdateDynamicProperty slot will be invoked with the current values for all
    /// the other attributes in the component! Potential perf issue and a bug.
    internalQObjectPropertyUpdateOngoing_ = true;

    QByteArray dynamicPropertyName = dynamicPropertyNames_[attribute->Id()];
    setProperty(dynamicPropertyName.data(), QVariant());
    dynamicPropertyNames_.remove(attribute->Id());

    internalQObjectPropertyUpdateOngoing_ = false;
}

bool IComponent::eventFilter(QObject *obj, QEvent *e)
{
    if (!obj || !e)
        return false;

    // A dynamic QObject property has been modified, update the IAttribute* to match it.
    // Note that this signal event is fired also when attribute is added, removed or
    // set from a IAttribute update in UpdateDynamicProperty. There is a boolean flag
    // we check that this change was done from outside the IComponent internals.
    if (e->type() == QEvent::DynamicPropertyChange && obj == this)
    {
        // If a internal update is ongoing, don't update the IAttribute. This would
        // result in infinite recursion via UpdateDynamicProperty!
        if (!internalQObjectPropertyUpdateOngoing_)
        {
            QDynamicPropertyChangeEvent *changeEvent = static_cast<QDynamicPropertyChangeEvent*>(e);
            QByteArray propertyName = changeEvent->propertyName();
            QString attributeId = dynamicPropertyNames_.key(propertyName, "");
            if (!attributeId.isEmpty())
            {
                IAttribute *attr = AttributeById(attributeId);
                if (attr)
                {
                    // Mark the flag as true, the QObject property is already up to date.
                    // There is no need to set it in UpdateDynamicProperty that FromQVariant will invoke.
                    internalQObjectPropertyUpdateOngoing_ = true;
                    attr->FromQVariant(property(propertyName.data()), AttributeChange::Default);
                    internalQObjectPropertyUpdateOngoing_ = false;
                }
            }
        }
    }

    return false;
}

QDomElement IComponent::BeginSerialization(QDomDocument& doc, QDomElement& base_element, bool serializeTemporary) const
{
    QDomElement comp_element = doc.createElement("component");
    comp_element.setAttribute("type", EnsureTypeNameWithoutPrefix(TypeName())); /**< @todo 27.09.2013 typeName would be better here */
    comp_element.setAttribute("typeId", QString::number(TypeId()));
    if (!Name().isEmpty())
        comp_element.setAttribute("name", Name());
    comp_element.setAttribute("sync", BoolToString(replicated));
    if (serializeTemporary)
        comp_element.setAttribute("temporary", BoolToString(temporary));

    if (!base_element.isNull())
        base_element.appendChild(comp_element);
    else
        doc.appendChild(comp_element);
    
    return comp_element;
}

void IComponent::WriteAttribute(QDomDocument& doc, QDomElement& comp_element, const QString& name, const QString& id, const QString& value, const QString &type) const
{
    QDomElement attribute_element = doc.createElement("attribute");
    attribute_element.setAttribute("name", name);
    attribute_element.setAttribute("id", id);
    attribute_element.setAttribute("value", value);
    attribute_element.setAttribute("type", type);
    comp_element.appendChild(attribute_element);
}

bool IComponent::BeginDeserialization(QDomElement& compElem)
{
    if (ParseUInt(compElem.attribute("typeId"), 0xffffffff) == TypeId() || // typeId takes precedence over typeName
        EnsureTypeNameWithPrefix(compElem.attribute("type")) == TypeName())
    {
        SetName(compElem.attribute("name"));
        return true;
    }

    return false;
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

void IComponent::EmitAttributeMetadataChanged(IAttribute* attribute)
{
    if (!attribute)
        return;
    if (!attribute->Metadata())
    {
        LogWarning("IComponent::EmitAttributeMetadataChanged: Given attributes metadata is null, signal won't be emitted!");
        return;
    }
    emit AttributeMetadataChanged(attribute, attribute->Metadata());
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
            WriteAttribute(doc, comp_element, attributes[i]->Name(), attributes[i]->Id(), attributes[i]->ToString(), attributes[i]->TypeName());
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
    QDomElement attribute_element = element.firstChildElement("attribute");
    while(!attribute_element.isNull())
    {
        IAttribute* attr = 0;
        QString id = attribute_element.attribute("id");
        // Prefer lookup by ID if it's specified, but fallback to using attribute's human-readable name if ID not defined or erroneous.
        if (!id.isEmpty())
            attr = AttributeById(id);
        if (!attr)
        {
            id = attribute_element.attribute("name");
            attr = AttributeByName(id);
        }
        
        if (!attr)
            LogWarning(TypeName() + "::DeserializeFrom: Could not find attribute \"" + id + "\" specified in the XML element.");
        else
            attr->FromString(attribute_element.attribute("value"), change);
        
        attribute_element = attribute_element.nextSiblingElement("attribute");
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
