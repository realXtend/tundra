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
    DeserializeData(const QString &name = "", const QString &type = "", const QString &value = ""):
        name_(name),
        type_(type),
        value_(value)
    {
    }

    QString name_;
    QString type_;
    QString value_;
};

/// Function that is used by std::sort algorithm to sort attributes by their name.
bool CmpAttributeByName(const IAttribute *a, const IAttribute *b)
{
    return a->Name().compare(b->Name(), Qt::CaseInsensitive);
}

/// Function that is used by std::sort algorithm to sort DeserializeData by their name.
bool CmpAttributeDataByName(const DeserializeData &a, const DeserializeData &b)
{
    return a.name_.compare(b.name_, Qt::CaseInsensitive);
}

/** @endcond */

EC_DynamicComponent::EC_DynamicComponent(Scene* scene):
    IComponent(scene)
{
}

EC_DynamicComponent::~EC_DynamicComponent()
{
}

void EC_DynamicComponent::SerializeTo(QDomDocument& doc, QDomElement& base_element, bool serializeTemporary) const
{
    QDomElement comp_element = BeginSerialization(doc, base_element, serializeTemporary);

    AttributeVector::const_iterator iter = attributes.begin();
    while(iter != attributes.end())
    {
        if (*iter)
            WriteAttribute(doc, comp_element, (*iter)->Name(), (*iter)->ToString().c_str(), (*iter)->TypeName());
        ++iter;
    }
}

void EC_DynamicComponent::DeserializeFrom(QDomElement& element, AttributeChange::Type change)
{
    if (!BeginDeserialization(element))
        return;

    std::vector<DeserializeData> deserializedAttributes;
    QDomElement child = element.firstChildElement("attribute");
    while(!child.isNull())
    {
        QString name = child.attribute("name");
        QString type = child.attribute("type");
        QString value = child.attribute("value");
        DeserializeData attributeData(name, type, value);
        deserializedAttributes.push_back(attributeData);

        child = child.nextSiblingElement("attribute");
    }

    DeserializeCommon(deserializedAttributes, change);
}

void EC_DynamicComponent::DeserializeCommon(std::vector<DeserializeData>& deserializedAttributes, AttributeChange::Type change)
{
    // Sort both lists in alphabetical order.
    AttributeVector oldAttributes = NonEmptyAttributes();
    std::stable_sort(oldAttributes.begin(), oldAttributes.end(), &CmpAttributeByName);
    std::stable_sort(deserializedAttributes.begin(), deserializedAttributes.end(), &CmpAttributeDataByName);

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
                remAttributes.push_back(DeserializeData((*iter1)->Name()));
            break;
        }

        // Attribute has already created and we only need to update it's value.
        if((*iter1)->Name() == (*iter2).name_)
        {
            //SetAttribute(QString::fromStdString(iter2->name_), QString::fromStdString(iter2->value_), change);
            for(AttributeVector::const_iterator attr_iter = attributes.begin(); attr_iter != attributes.end(); ++attr_iter)
                if((*attr_iter)->Name() == iter2->name_)
                    (*attr_iter)->FromString(iter2->value_.toStdString(), change);

            ++iter2;
            ++iter1;
        }
        // Found a new attribute that need to be created and added to the component.
        else if((*iter1)->Name() > (*iter2).name_)
        {
            addAttributes.push_back(*iter2);
            ++iter2;
        }
        // Couldn't find the attribute in a new list so it need to be removed from the component.
        else
        {
            remAttributes.push_back(DeserializeData((*iter1)->Name()));
            ++iter1;
        }
    }

    while(!addAttributes.empty())
    {
        DeserializeData attributeData = addAttributes.back();
        IAttribute *attribute = CreateAttribute(attributeData.type_, attributeData.name_);
        if (attribute)
            attribute->FromString(attributeData.value_.toStdString(), change);
        addAttributes.pop_back();
    }
    while(!remAttributes.empty())
    {
        DeserializeData attributeData = remAttributes.back();
        RemoveAttribute(attributeData.name_);
        remAttributes.pop_back();
    }
}

IAttribute *EC_DynamicComponent::CreateAttribute(const QString &typeName, const QString &name, AttributeChange::Type change)
{
    if(ContainsAttribute(name))
        return IComponent::GetAttribute(name);

    IAttribute *attribute = SceneAPI::CreateAttribute(typeName, name);
    if(!attribute)
    {
        LogError("Failed to create new attribute:" + name + " in dynamic component:" + Name());
        return 0;
    }
    
    IComponent::AddAttribute(attribute);
    
    // Trigger scenemanager signal
    Scene* scene = ParentScene();
    if (scene)
        scene->EmitAttributeAdded(this, attribute, change);
    
    // Trigger internal signal
    emit AttributeAdded(attribute);
    EmitAttributeChanged(attribute, change);
    return attribute;
}

void EC_DynamicComponent::RemoveAttribute(const QString &name, AttributeChange::Type change)
{
    for(AttributeVector::iterator iter = attributes.begin(); iter != attributes.end(); iter++)
    {
        if((*iter) && (*iter)->Name().compare(name, Qt::CaseInsensitive) == 0)
        {
            // Trigger scenemanager signal
            Scene* scene = ParentScene();
            if (scene)
                scene->EmitAttributeRemoved(this, *iter, change);
            
            // Trigger internal signal(s)
            emit AttributeAboutToBeRemoved(*iter);
            // Leave a hole in the array, which will be filled when new attributes are created
            SAFE_DELETE(*iter);
            break;
        }
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

            QString name(attributes[i]->Name());

            // Trigger internal signal(s)
            emit AttributeAboutToBeRemoved(attributes[i]);
            SAFE_DELETE(attributes[i]);
            attributes[i] = 0;
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

void EC_DynamicComponent::AddQVariantAttribute(const QString &name, AttributeChange::Type change)
{
    LogWarning("EC_DynamicComponent::AddQVariantAttribute is deprecated and will be removed. Use CreateAttribute(\"qvariant\",...) instead.");
    //Check if the attribute has already been created.
    if(!ContainsAttribute(name))
    {
        Attribute<QVariant> *attribute = new Attribute<QVariant>(this, name.toStdString().c_str());
        EmitAttributeChanged(attribute, change);
        emit AttributeAdded(attribute);
    }
    else
        LogWarning("Failed to add a new QVariant in name of " + name + ", cause there already is an attribute in that name.");
}

QVariant EC_DynamicComponent::GetAttribute(int index) const
{
    // Do not count holes.
    int attrIndex = GetInternalAttributeIndex(index);
    if (attrIndex < 0)
        return QVariant();
    return attributes[attrIndex]->ToQVariant();
}

QVariant EC_DynamicComponent::GetAttribute(const QString &name) const
{
    return IComponent::GetAttributeQVariant(name);
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

void EC_DynamicComponent::SetAttributeQScript(const QString &name, const QScriptValue &value, AttributeChange::Type change)
{
    LogWarning("EC_DynamicComponent::SetAttributeQScript is deprecated and will be removed. Use SetAttribute instead.");
    for(AttributeVector::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter)
        if((*iter) && (*iter)->Name().compare(name, Qt::CaseInsensitive) == 0)
        {
            (*iter)->FromScriptValue(value, change);
            break; 
        }
}

void EC_DynamicComponent::SetAttribute(const QString &name, const QVariant &value, AttributeChange::Type change)
{
    for(AttributeVector::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter)
        if((*iter) && (*iter)->Name().compare(name, Qt::CaseInsensitive) == 0)
        {
            (*iter)->FromQVariant(value, change);
            break;
        }
}

QString EC_DynamicComponent::GetAttributeName(int index) const
{
    // Do not count holes.
    int attrIndex = GetInternalAttributeIndex(index);
    if (attrIndex < 0)
    {
        LogWarning("Cannot get attribute name, index out of bounds");
        return QString();
    }
    return attributes[index]->Name();
}

bool EC_DynamicComponent::ContainSameAttributes(const EC_DynamicComponent &comp) const
{
    AttributeVector myAttributeVector = NonEmptyAttributes();
    AttributeVector attributeVector = comp.NonEmptyAttributes();
    if(attributeVector.size() != myAttributeVector.size())
        return false;
    if(attributeVector.empty() && myAttributeVector.empty())
        return true;

    std::sort(myAttributeVector.begin(), myAttributeVector.end(), &CmpAttributeByName);
    std::sort(attributeVector.begin(), attributeVector.end(), &CmpAttributeByName);

    AttributeVector::const_iterator iter1 = myAttributeVector.begin();
    AttributeVector::const_iterator iter2 = attributeVector.begin();
    while(iter1 != myAttributeVector.end() && iter2 != attributeVector.end())
    {
        // Compare attribute names and type and if they mach continue iteration if not components aren't exactly the same.
        if (((*iter1)->Name().compare((*iter2)->Name(), Qt::CaseInsensitive) == 0) &&
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

bool EC_DynamicComponent::ContainsAttribute(const QString &name) const
{
    AttributeVector::const_iterator iter = attributes.begin();
    while(iter != attributes.end())
    {
        if((*iter) && (*iter)->Name().compare(name, Qt::CaseInsensitive) == 0)
            return true;
        ++iter;
    }

    return false;
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
            dest.AddString((*iter)->Name().toStdString());
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
        std::string name = source.ReadString();
        std::string typeName = source.ReadString();
        std::string value = source.ReadString();
        
        DeserializeData attrData(name.c_str(), typeName.c_str(), value.c_str());
        deserializedAttributes.push_back(attrData);
    }

    DeserializeCommon(deserializedAttributes, change);
}
