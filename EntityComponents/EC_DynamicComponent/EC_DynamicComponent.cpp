// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_DynamicComponent.h"
#include "IModule.h"
#include "ModuleManager.h"
#include "Entity.h"
#include "LoggingFunctions.h"

#include <QScriptEngine>
#include <QScriptValueIterator>

#include "kNet.h"

DEFINE_POCO_LOGGING_FUNCTIONS("EC_DynamicComponent")

#include <QDomDocument>

struct DeserializeData
{
    DeserializeData(const std::string name = std::string(""),
                    const std::string type = std::string(""),
                    const std::string value = std::string("")):
        name_(name),
        type_(type),
        value_(value)
    {
    }

    //! Checks if any of data structure's values are null.
    bool isNull() const
    {
        return name_ == "" || type_ == "" || value_ == "";
    }

    std::string name_;
    std::string type_;
    std::string value_;
};

//! Function that is used by std::sort algorithm to sort attributes by their name.
bool CmpAttributeByName(const IAttribute *a, const IAttribute *b)
{
    return a->GetNameString() < b->GetNameString();
}

//! Function that is used by std::sort algorithm to sort DeserializeData by their name.
bool CmpAttributeDataByName(const DeserializeData &a, const DeserializeData &b)
{
    return a.name_ < b.name_;
}

EC_DynamicComponent::EC_DynamicComponent(IModule *module):
    IComponent(module->GetFramework())
{
}

EC_DynamicComponent::~EC_DynamicComponent()
{
    foreach(IAttribute *a, attributes_)
        SAFE_DELETE(a);
}

void EC_DynamicComponent::SerializeTo(QDomDocument& doc, QDomElement& base_element) const
{
    QDomElement comp_element = BeginSerialization(doc, base_element);

    AttributeVector::const_iterator iter = attributes_.begin();
    while(iter != attributes_.end())
    {
        WriteAttribute(doc, comp_element, (*iter)->GetNameString().c_str(), (*iter)->ToString().c_str(), (*iter)->TypenameToString().c_str());
        iter++;
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
        DeserializeData attributeData(name.toStdString(), type.toStdString(), value.toStdString());
        deserializedAttributes.push_back(attributeData);

        child = child.nextSiblingElement("attribute");
    }

    DeserializeCommon(deserializedAttributes, change);
}

void EC_DynamicComponent::DeserializeCommon(std::vector<DeserializeData>& deserializedAttributes, AttributeChange::Type change)
{
    // Sort both lists in alphabetical order.
    AttributeVector oldAttributes = attributes_;
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
            for(;iter2 != deserializedAttributes.end(); iter2++)
            {
                addAttributes.push_back(*iter2);
            }
            break;
        }
        // Only old attributes are left and they can be removed from the dynamic component.
        else if(iter2 == deserializedAttributes.end())
        {
            for(;iter1 != oldAttributes.end(); iter1++)
                remAttributes.push_back(DeserializeData((*iter1)->GetNameString().c_str()));
            break;
        }

        // Attribute has already created and we only need to update it's value.
        if((*iter1)->GetNameString() == (*iter2).name_)
        {
            //SetAttribute(QString::fromStdString(iter2->name_), QString::fromStdString(iter2->value_), change);
            for(AttributeVector::const_iterator attr_iter = attributes_.begin(); attr_iter != attributes_.end(); attr_iter++)
            {
                if((*attr_iter)->GetNameString() == iter2->name_)
                {
                    (*attr_iter)->FromString(iter2->value_, change);
                }
            }
            iter2++;
            iter1++;
        }
        // Found a new attribute that need to be created and added to the component.
        else if((*iter1)->GetNameString() > (*iter2).name_)
        {
            addAttributes.push_back(*iter2);
            iter2++;
        }
        // Couldn't find the attribute in a new list so it need to be removed from the component.
        else
        {
            remAttributes.push_back(DeserializeData((*iter1)->GetNameString().c_str()));
            iter1++;
        }
    }

    while(!addAttributes.empty())
    {
        DeserializeData attributeData = addAttributes.back();
        IAttribute *attribute = CreateAttribute(attributeData.type_.c_str(), attributeData.name_.c_str());
        if (attribute)
            attribute->FromString(attributeData.value_, change);
        addAttributes.pop_back();
    }
    while(!remAttributes.empty())
    {
        DeserializeData attributeData = remAttributes.back();
        RemoveAttribute(QString::fromStdString(attributeData.name_));
        remAttributes.pop_back();
    }
}

IAttribute *EC_DynamicComponent::CreateAttribute(const QString &typeName, const QString &name, AttributeChange::Type change)
{
    IAttribute *attribute = 0;
    if(ContainsAttribute(name))
        return attribute;

    attribute = framework_->GetComponentManager()->CreateAttribute(this, typeName.toStdString(), name.toStdString());
    if(attribute)
        emit AttributeAdded(attribute);

    AttributeChanged(attribute, change);

    return attribute;
}

void EC_DynamicComponent::RemoveAttribute(const QString &name, AttributeChange::Type change)
{
    for(AttributeVector::iterator iter = attributes_.begin(); iter != attributes_.end(); iter++)
    {
        if((*iter)->GetNameString() == name.toStdString())
        {
            //! /todo Make sure that component removal is replicated to the server if change type is Replicate.
            AttributeChanged(*iter, change);
            emit AttributeAboutToBeRemoved(*iter);
            SAFE_DELETE(*iter);
            attributes_.erase(iter);
            emit AttributeRemoved(name);
            break;
        }
    }
}

void EC_DynamicComponent::AddQVariantAttribute(const QString &name, AttributeChange::Type change)
{
    //Check if the attribute has already been created.
    if(!ContainsAttribute(name))
    {
        Attribute<QVariant> *attribute = new Attribute<QVariant>(this, name.toStdString().c_str());
        AttributeChanged(attribute, change);
        emit AttributeAdded(attribute);
    }
    LogWarning("Failed to add a new QVariant in name of " + name.toStdString() + ", cause there already is an attribute in that name.");
}

QVariant EC_DynamicComponent::GetAttribute(int index) const
{
    if(index < attributes_.size() && index >= 0)
    {
        return attributes_[index]->ToQVariant();
    }
    return QVariant();
}

QVariant EC_DynamicComponent::GetAttribute(const QString &name) const
{
    for(AttributeVector::const_iterator iter = attributes_.begin(); iter != attributes_.end(); ++iter)
        if ((*iter)->GetNameString() == name.toStdString())
            return (*iter)->ToQVariant();

    return QVariant();
}

void EC_DynamicComponent::SetAttribute(int index, const QVariant &value, AttributeChange::Type change)
{
    if(index < attributes_.size() && index >= 0)
    {
        attributes_[index]->FromQVariant(value, change);
    }
    LogWarning("Cannot get attribute name, cause index is out of range.");
}

void EC_DynamicComponent::SetAttributeQScript(const QString &name, const QScriptValue &value, AttributeChange::Type change)
{
    for(AttributeVector::const_iterator iter = attributes_.begin(); iter != attributes_.end(); iter++)
    {
        if((*iter)->GetNameString() == name.toStdString())
        {
            (*iter)->FromScriptValue(value, change);
            break; 
        }
    }
}

void EC_DynamicComponent::SetAttribute(const QString &name, const QVariant &value, AttributeChange::Type change)
{
    for(AttributeVector::const_iterator iter = attributes_.begin(); iter != attributes_.end(); iter++)
    {
        if((*iter)->GetNameString() == name.toStdString())
        {
            (*iter)->FromQVariant(value, change);
            break;
        }
    }
}

QString EC_DynamicComponent::GetAttributeName(int index) const
{
    if(index < attributes_.size() && index >= 0)
    {
        return attributes_[index]->GetName();
    }
    LogWarning("Cannot get attribute name, cause index is out of range.");
    return QString();
}

bool EC_DynamicComponent::ContainSameAttributes(const EC_DynamicComponent &comp) const
{
    AttributeVector myAttributeVector = GetAttributes();
    AttributeVector attributeVector = comp.GetAttributes();
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
        // Compare attribute names and type and if they mach continue iteration if not components aren't exatly the same.
        if((*iter1)->GetNameString() == (*iter2)->GetNameString() &&
           (*iter1)->TypenameToString() == (*iter2)->TypenameToString())
        {
            if(iter1 != myAttributeVector.end())
                iter1++;
            if(iter2 != attributeVector.end())
                iter2++;
        }
        else
        {
            return false;
        }
    }
    return true;

    /*// Get both attributes and check if they are holding exact number of attributes.
    AttributeVector myAttributeVector = GetAttributes();
    AttributeVector attributeVector = comp.GetAttributes();
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
    AttributeVector::const_iterator iter = attributes_.begin();
    while(iter != attributes_.end())
    {
        if((*iter)->GetName() == name.toStdString())
        {
            return true;
        }
        iter++;
    }
    return false;
}

void EC_DynamicComponent::SerializeToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<u8>(attributes_.size());
    // For now, transmit all values as strings
    AttributeVector::const_iterator iter = attributes_.begin();
    while(iter != attributes_.end())
    {
        dest.AddString((*iter)->GetNameString());
        dest.AddString((*iter)->TypenameToString());
        dest.AddString((*iter)->ToString());
        ++iter;
    }
}

void EC_DynamicComponent::DeserializeFromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    u8 num_attributes = source.Read<u8>();
    std::vector<DeserializeData> deserializedAttributes;
    for (uint i = 0; i < num_attributes; ++i)
    {
        std::string name = source.ReadString();
        std::string type = source.ReadString();
        std::string value = source.ReadString();
        DeserializeData attributeData(name, type, value);
        deserializedAttributes.push_back(attributeData);
    }
    
    DeserializeCommon(deserializedAttributes, change);
}

