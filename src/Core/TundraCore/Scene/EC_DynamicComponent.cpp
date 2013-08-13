// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "EC_DynamicComponent.h"

#include "SceneAPI.h"

#include "Entity.h"
#include "LoggingFunctions.h"
#include "Scene/Scene.h"

#include <QScriptEngine>
#include <QScriptValueIterator>

#include <kNet.h>

#include <QDomDocument>

#include "MemoryLeakCheck.h"

/** @cond PRIVATE */
struct DeserializeData
{
    DeserializeData(const QString &id = "", const QString &type = "", const QString &value = ""):
        id_(id),
        type_(type),
        value_(value)
    {
    }

    QString id_;
    QString type_;
    QString value_;
};

/// Function that is used by std::sort algorithm to sort attributes by their ID.
bool CmpAttributeById(const IAttribute *a, const IAttribute *b)
{
    return a->Id().compare(b->Id(), Qt::CaseInsensitive);
}

/// Function that is used by std::sort algorithm to sort DeserializeData by their ID.
bool CmpAttributeDataById(const DeserializeData &a, const DeserializeData &b)
{
    return a.id_.compare(b.id_, Qt::CaseInsensitive);
}

/** @endcond */

EC_DynamicComponent::EC_DynamicComponent(Scene* scene):
    IComponent(scene)
{
}

EC_DynamicComponent::~EC_DynamicComponent()
{
}

void EC_DynamicComponent::DeserializeFrom(QDomElement& element, AttributeChange::Type change)
{
    if (!BeginDeserialization(element))
        return;

    std::vector<DeserializeData> deserializedAttributes;
    QDomElement child = element.firstChildElement("attribute");
    while(!child.isNull())
    {
        QString id = child.attribute("id");
        // Fallback if ID is not defined
        if (!id.length())
            id = child.attribute("name");
        QString type = child.attribute("type");
        QString value = child.attribute("value");
        DeserializeData attributeData(id, type, value);
        deserializedAttributes.push_back(attributeData);

        child = child.nextSiblingElement("attribute");
    }

    DeserializeCommon(deserializedAttributes, change);
}

void EC_DynamicComponent::DeserializeCommon(std::vector<DeserializeData>& deserializedAttributes, AttributeChange::Type change)
{
    // Sort both lists in alphabetical order.
    AttributeVector oldAttributes = NonEmptyAttributes();
    std::stable_sort(oldAttributes.begin(), oldAttributes.end(), &CmpAttributeById);
    std::stable_sort(deserializedAttributes.begin(), deserializedAttributes.end(), &CmpAttributeDataById);

    std::vector<DeserializeData> addAttributes;
    std::vector<DeserializeData> remAttributes;
    AttributeVector::iterator iter1 = oldAttributes.begin();
    std::vector<DeserializeData>::iterator iter2 = deserializedAttributes.begin();

    // Check what attributes we need to add or remove from the dynamic component (done by comparing two list differences).
    while(iter1 != oldAttributes.end() || iter2 != deserializedAttributes.end())
    {
        // No point to continue the iteration if other list is empty. We can just push all new attributes into the dynamic component.
        if(iter1 == oldAttributes.end())
        {
            for(;iter2 != deserializedAttributes.end(); ++iter2)
                addAttributes.push_back(*iter2);
            break;
        }
        // Only old attributes are left and they can be removed from the dynamic component.
        else if(iter2 == deserializedAttributes.end())
        {
            for(;iter1 != oldAttributes.end(); ++iter1)
                remAttributes.push_back(DeserializeData((*iter1)->Id()));
            break;
        }

        // Attribute has already created and we only need to update it's value.
        if((*iter1)->Id() == (*iter2).id_)
        {
            //SetAttribute(QString::fromStdString(iter2->name_), QString::fromStdString(iter2->value_), change);
            for(AttributeVector::const_iterator attr_iter = attributes.begin(); attr_iter != attributes.end(); ++attr_iter)
                if((*attr_iter)->Id() == iter2->id_)
                    (*attr_iter)->FromString(iter2->value_.toStdString(), change);

            ++iter2;
            ++iter1;
        }
        // Found a new attribute that need to be created and added to the component.
        else if((*iter1)->Id() > (*iter2).id_)
        {
            addAttributes.push_back(*iter2);
            ++iter2;
        }
        // Couldn't find the attribute in a new list so it need to be removed from the component.
        else
        {
            remAttributes.push_back(DeserializeData((*iter1)->Id()));
            ++iter1;
        }
    }

    while(!addAttributes.empty())
    {
        DeserializeData attributeData = addAttributes.back();
        IAttribute *attribute = CreateAttribute(attributeData.type_, attributeData.id_);
        if (attribute)
            attribute->FromString(attributeData.value_.toStdString(), change);
        addAttributes.pop_back();
    }
    while(!remAttributes.empty())
    {
        DeserializeData attributeData = remAttributes.back();
        RemoveAttribute(attributeData.id_);
        remAttributes.pop_back();
    }
}

IAttribute *EC_DynamicComponent::CreateAttribute(const QString &typeName, const QString &id, AttributeChange::Type change)
{
    if (ContainsAttribute(id))
        return IComponent::AttributeById(id);

    IAttribute *attribute = SceneAPI::CreateAttribute(typeName, id);
    if (!attribute)
    {
        LogError("Failed to create new attribute of type \"" + typeName + "\" with ID \"" + id + "\" to dynamic component \"" + Name() + "\".");
        return 0;
    }

    IComponent::AddAttribute(attribute);

    Scene* scene = ParentScene();
    if (scene)
        scene->EmitAttributeAdded(this, attribute, change);

    emit AttributeAdded(attribute);
    EmitAttributeChanged(attribute, change);
    return attribute;
}

void EC_DynamicComponent::RemoveAttribute(const QString &id, AttributeChange::Type change)
{
    for(AttributeVector::iterator iter = attributes.begin(); iter != attributes.end(); iter++)
        if((*iter) && (*iter)->Id().compare(id, Qt::CaseInsensitive) == 0)
        {
            IComponent::RemoveAttribute((*iter)->Index(), change);
            break;
        }
}

void EC_DynamicComponent::RemoveAllAttributes(AttributeChange::Type change)
{
    for(size_t i = attributes.size() - 1; i < attributes.size(); --i)
    {
        if (attributes[i])
        {
            // Trigger scenemanager signal
            Scene* scene = ParentScene();
            if (scene)
                scene->EmitAttributeRemoved(this, attributes[i], change);
            
            // Trigger internal signal(s)
            emit AttributeAboutToBeRemoved(attributes[i]);
            SAFE_DELETE(attributes[i]);
        }
        attributes.clear();
    }
}

int EC_DynamicComponent::GetInternalAttributeIndex(int index) const
{
    if (index >= (int)attributes.size())
        return -1; // Can be sure that is not found.
    int cmp = 0;
    for (unsigned i = 0; i < attributes.size(); ++i)
    {
        if (!attributes[i])
            continue;
        if (cmp == index)
            return i;
        ++cmp;
    }
    return -1;
}

void EC_DynamicComponent::AddQVariantAttribute(const QString &id, AttributeChange::Type change)
{
    LogWarning("EC_DynamicComponent::AddQVariantAttribute is deprecated and will be removed. Use CreateAttribute(\"QVariant\",...) instead.");
    //Check if the attribute has already been created.
    if(!ContainsAttribute(id))
    {
        Attribute<QVariant> *attribute = new Attribute<QVariant>(this, id.toStdString().c_str());
        EmitAttributeChanged(attribute, change);
        emit AttributeAdded(attribute);
    }
    else
        LogWarning("Failed to add a new QVariant with ID " + id + ", because there already is an attribute with that ID.");
}

QVariant EC_DynamicComponent::GetAttribute(int index) const
{
    // Do not count holes.
    int attrIndex = GetInternalAttributeIndex(index);
    if (attrIndex < 0)
        return QVariant();
    return attributes[attrIndex]->ToQVariant();
}

QVariant EC_DynamicComponent::GetAttribute(const QString &id) const
{
    return IComponent::GetAttributeQVariant(id);
}

void EC_DynamicComponent::SetAttribute(int index, const QVariant &value, AttributeChange::Type change)
{
    int attrIndex = GetInternalAttributeIndex(index);
    if (attrIndex < 0)
    {
        LogWarning("Cannot set attribute, index out of bounds");
        return;
    }
    
    attributes[attrIndex]->FromQVariant(value, change);
}

void EC_DynamicComponent::SetAttributeQScript(const QString &id, const QScriptValue &value, AttributeChange::Type change)
{
    LogWarning("EC_DynamicComponent::SetAttributeQScript is deprecated and will be removed. Use SetAttribute instead.");
    IAttribute* attr = AttributeById(id);
    if (attr)
        attr->FromScriptValue(value, change);
}

void EC_DynamicComponent::SetAttribute(const QString &id, const QVariant &value, AttributeChange::Type change)
{
    IAttribute* attr = AttributeById(id);
    if (attr)
        attr->FromQVariant(value, change);
}

QString EC_DynamicComponent::GetAttributeName(int index) const
{
    LogWarning("EC_DynamicComponent::GetAttributeName is deprecated and will be removed. For dynamic attributes ID is the same as name. Use GetAttributeId instead.");
    
    // Do not count holes.
    int attrIndex = GetInternalAttributeIndex(index);
    if (attrIndex < 0)
    {
        LogWarning("Cannot get attribute name, index out of bounds");
        return QString();
    }
    return attributes[index]->Name();
}

QString EC_DynamicComponent::GetAttributeId(int index) const
{
    // Do not count holes.
    int attrIndex = GetInternalAttributeIndex(index);
    if (attrIndex < 0)
    {
        LogWarning("Cannot get attribute ID, index out of bounds");
        return QString();
    }
    return attributes[index]->Id();
}

bool EC_DynamicComponent::ContainSameAttributes(const EC_DynamicComponent &comp) const
{
    AttributeVector myAttributeVector = NonEmptyAttributes();
    AttributeVector attributeVector = comp.NonEmptyAttributes();
    if(attributeVector.size() != myAttributeVector.size())
        return false;
    if(attributeVector.empty() && myAttributeVector.empty())
        return true;

    std::sort(myAttributeVector.begin(), myAttributeVector.end(), &CmpAttributeById);
    std::sort(attributeVector.begin(), attributeVector.end(), &CmpAttributeById);

    AttributeVector::const_iterator iter1 = myAttributeVector.begin();
    AttributeVector::const_iterator iter2 = attributeVector.begin();
    while(iter1 != myAttributeVector.end() && iter2 != attributeVector.end())
    {
        // Compare attribute names and type and if they mach continue iteration if not components aren't exactly the same.
        if (((*iter1)->Id().compare((*iter2)->Id(), Qt::CaseInsensitive) == 0) &&
            (*iter1)->TypeName().compare((*iter2)->TypeName(), Qt::CaseInsensitive) == 0)
        {
            if(iter1 != myAttributeVector.end())
                ++iter1;
            if(iter2 != attributeVector.end())
                ++iter2;
        }
        else
        {
            return false;
        }
    }
    return true;

    /*// Get both attributes and check if they are holding exact number of attributes.
    AttributeVector myAttributeVector = Attributes();
    AttributeVector attributeVector = comp.Attributes();
    if(attributeVector.size() != myAttributeVector.size())
        return false;
    
    // Compare that every attribute is same in both components.
    QSet<IAttribute*> myAttributeSet;
    QSet<IAttribute*> attributeSet;
    for(uint i = 0; i < myAttributeSet.size(); i++)
    {
        attributeSet.insert(myAttributeVector[i]);
        myAttributeSet.insert(attributeVector[i]);
    }
    if(attributeSet != myAttributeSet)
        return false;
    return true;*/
}

bool EC_DynamicComponent::ContainsAttribute(const QString &id) const
{
    return AttributeById(id) != 0;
}

void EC_DynamicComponent::SerializeToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<u8>((u8)attributes.size());
    // For now, transmit all values as strings
    AttributeVector::const_iterator iter = attributes.begin();
    while(iter != attributes.end())
    {
        if (*iter)
        {
            dest.AddString((*iter)->Id().toStdString());
            dest.AddString((*iter)->TypeName().toStdString());
            dest.AddString((*iter)->ToString());
        }
        ++iter;
    }
}

void EC_DynamicComponent::DeserializeFromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    u8 num_attributes = source.Read<u8>();
    std::vector<DeserializeData> deserializedAttributes;
    for(uint i = 0; i < num_attributes; ++i)
    {
        std::string id = source.ReadString();
        std::string typeName = source.ReadString();
        std::string value = source.ReadString();
        
        DeserializeData attrData(id.c_str(), typeName.c_str(), value.c_str());
        deserializedAttributes.push_back(attrData);
    }

    DeserializeCommon(deserializedAttributes, change);
}
