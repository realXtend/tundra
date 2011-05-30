// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "EC_DynamicComponent.h"

#include "SceneAPI.h"
#include "IModule.h"

#include "Entity.h"
#include "LoggingFunctions.h"
#include "Scene.h"

#include <QScriptEngine>
#include <QScriptValueIterator>

#include <kNet.h>

#include <QDomDocument>

#include "MemoryLeakCheck.h"

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

    /// Checks if any of data structure's values are null.
    bool isNull() const
    {
        return name_ == "" || type_ == "" || value_ == "";
    }

    std::string name_;
    std::string type_;
    std::string value_;
};

/// Function that is used by std::sort algorithm to sort attributes by their name.
bool CmpAttributeByName(const IAttribute *a, const IAttribute *b)
{
    return a->Name() < b->Name();
}

/// Function that is used by std::sort algorithm to sort DeserializeData by their name.
bool CmpAttributeDataByName(const DeserializeData &a, const DeserializeData &b)
{
    return a.name_ < b.name_;
}

EC_DynamicComponent::EC_DynamicComponent(Scene* scene):
    IComponent(scene)
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
        WriteAttribute(doc, comp_element, (*iter)->Name(), (*iter)->ToString().c_str(), (*iter)->TypeName());
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
                addAttributes.push_back(*iter2);
            break;
        }
        // Only old attributes are left and they can be removed from the dynamic component.
        else if(iter2 == deserializedAttributes.end())
        {
            for(;iter1 != oldAttributes.end(); iter1++)
                remAttributes.push_back(DeserializeData((*iter1)->Name()));
            break;
        }

        // Attribute has already created and we only need to update it's value.
        if((*iter1)->Name() == (*iter2).name_)
        {
            //SetAttribute(QString::fromStdString(iter2->name_), QString::fromStdString(iter2->value_), change);
            for(AttributeVector::const_iterator attr_iter = attributes_.begin(); attr_iter != attributes_.end(); attr_iter++)
                if((*attr_iter)->Name() == iter2->name_)
                    (*attr_iter)->FromString(iter2->value_, change);

            iter2++;
            iter1++;
        }
        // Found a new attribute that need to be created and added to the component.
        else if((*iter1)->Name() > (*iter2).name_)
        {
            addAttributes.push_back(*iter2);
            iter2++;
        }
        // Couldn't find the attribute in a new list so it need to be removed from the component.
        else
        {
            remAttributes.push_back(DeserializeData((*iter1)->Name()));
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
    if(ContainsAttribute(name))
        return IComponent::GetAttribute(name);

    IAttribute *attribute = framework_->Scene()->CreateAttribute(this, typeName, name);
    if(!attribute)
    {
        LogError("Failed to create new attribute:" + name + " in dynamic component:" + Name());
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

void EC_DynamicComponent::RemoveAttribute(const QString &name, AttributeChange::Type change)
{
    for(AttributeVector::iterator iter = attributes_.begin(); iter != attributes_.end(); iter++)
    {
        if((*iter)->Name() == name)
        {
            // Trigger scenemanager signal
            Scene* scene = ParentScene();
            if (scene)
                scene->EmitAttributeRemoved(this, *iter, change);
            
            // Trigger internal signal(s)
            emit AttributeAboutToBeRemoved(*iter);
            SAFE_DELETE(*iter);
            attributes_.erase(iter);
            emit AttributeRemoved(name);
            break;
        }
    }
}

void EC_DynamicComponent::RemoveAllAttributes(AttributeChange::Type change)
{
    for(unsigned i = attributes_.size() - 1; i < attributes_.size(); --i)
    {
        // Trigger scenemanager signal
        Scene* scene = ParentScene();
        if (scene)
            scene->EmitAttributeRemoved(this, attributes_[i], change);

        QString name(attributes_[i]->Name());

        // Trigger internal signal(s)
        emit AttributeAboutToBeRemoved(attributes_[i]);
        SAFE_DELETE(attributes_[i]);
        attributes_.erase(attributes_.begin() + i);
        emit AttributeRemoved(name);
    }
}

void EC_DynamicComponent::AddQVariantAttribute(const QString &name, AttributeChange::Type change)
{
    //Check if the attribute has already been created.
    if(!ContainsAttribute(name))
    {
        Attribute<QVariant> *attribute = new Attribute<QVariant>(this, name.toStdString().c_str());
        EmitAttributeChanged(attribute, change);
        emit AttributeAdded(attribute);
    }
    else
        LogWarning("Failed to add a new QVariant in name of " + name.toStdString() + ", cause there already is an attribute in that name.");
}

QVariant EC_DynamicComponent::GetAttribute(int index) const
{
    if (index < (int)attributes_.size() && index >= 0)
        return attributes_[index]->ToQVariant();
    return QVariant();
}

QVariant EC_DynamicComponent::GetAttribute(const QString &name) const
{
    return IComponent::GetAttributeQVariant(name);
}

void EC_DynamicComponent::SetAttribute(int index, const QVariant &value, AttributeChange::Type change)
{
    if (index < (int)attributes_.size() && index >= 0)
        attributes_[index]->FromQVariant(value, change);
    else
        LogWarning("Cannot get attribute name, cause index is out of range.");
}

void EC_DynamicComponent::SetAttributeQScript(const QString &name, const QScriptValue &value, AttributeChange::Type change)
{
    for(AttributeVector::const_iterator iter = attributes_.begin(); iter != attributes_.end(); iter++)
        if((*iter)->Name() == name)
        {
            (*iter)->FromScriptValue(value, change);
            break; 
        }
}

void EC_DynamicComponent::SetAttribute(const QString &name, const QVariant &value, AttributeChange::Type change)
{
    for(AttributeVector::const_iterator iter = attributes_.begin(); iter != attributes_.end(); iter++)
        if((*iter)->Name() == name)
        {
            (*iter)->FromQVariant(value, change);
            break;
        }
}

QString EC_DynamicComponent::GetAttributeName(int index) const
{
    if(index < (int)attributes_.size() && index >= 0)
        return attributes_[index]->Name();

    LogWarning("Cannot get attribute name, cause index is out of range.");
    return QString();
}

bool EC_DynamicComponent::ContainSameAttributes(const EC_DynamicComponent &comp) const
{
    AttributeVector myAttributeVector = Attributes();
    AttributeVector attributeVector = comp.Attributes();
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
        if ((*iter1)->Name() == (*iter2)->Name() && (*iter1)->TypeName() == (*iter2)->TypeName())
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
    AttributeVector::const_iterator iter = attributes_.begin();
    while(iter != attributes_.end())
    {
        if((*iter)->Name() == name.toStdString())
            return true;
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
        dest.AddString((*iter)->Name());
        dest.AddString((*iter)->TypeName().toStdString());
        dest.AddString((*iter)->ToString());
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
        std::string type = source.ReadString();
        std::string value = source.ReadString();
        DeserializeData attributeData(name, type, value);
        deserializedAttributes.push_back(attributeData);
    }

    DeserializeCommon(deserializedAttributes, change);
}
