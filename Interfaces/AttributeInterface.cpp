/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   AttributeInterface.cpp
 *  @brief  Abstract base class and template class for entity-component attributes.
 */

#include "StableHeaders.h"

#include "AttributeInterface.h"
#include "ComponentInterface.h"
#include "AssetInterface.h"
#include "Core.h"
#include "CoreStdIncludes.h"
#include "Transform.h"

#include <QVariant>
#include <QStringList>

// Implementation code for some common attributes

AttributeInterface::AttributeInterface(Foundation::ComponentInterface* owner, const char* name) :
    owner_(owner),
    name_(name),
    change_(AttributeChange::None),
    null_(false),
    metadata_(0)
{
    if (owner)
        owner->AddAttribute(this);
}

void AttributeInterface::Changed(AttributeChange::Type change)
{
    if (owner_)
        owner_->AttributeChanged(this, change);
}

    // TOSTRING TEMPLATE IMPLEMENTATIONS.

template<> std::string Attribute<QString>::ToString() const
{
    ///\todo decode/encode XML-risky characters
    return Get().toStdString();
}

template<> std::string Attribute<bool>::ToString() const
{
    if (Get())
        return "true";
    else
        return "false";
}

template<> std::string Attribute<int>::ToString() const
{
    return ::ToString<int>(Get());
}

template<> std::string Attribute<uint>::ToString() const
{
    return ::ToString<uint>(Get());
}

template<> std::string Attribute<float>::ToString() const
{
    return ::ToString<float>(Get());
}

template<> std::string Attribute<Vector3df>::ToString() const
{
    Vector3df value = Get();
    
    return ::ToString<float>(value.x) + " " +
        ::ToString<float>(value.y) + " " +
        ::ToString<float>(value.z);
}
    
template<> std::string Attribute<Quaternion>::ToString() const
{
    Quaternion value = Get();
    
    return ::ToString<float>(value.w) + " " +
        ::ToString<float>(value.x) + " " +
        ::ToString<float>(value.y) + " " +
        ::ToString<float>(value.z);
}

template<> std::string Attribute<Color>::ToString() const
{
    Color value = Get();
    
    return ::ToString<float>(value.r) + " " +
        ::ToString<float>(value.g) + " " +
        ::ToString<float>(value.b) + " " +
        ::ToString<float>(value.a);
}

template<> std::string Attribute<Foundation::AssetReference>::ToString() const
{
    Foundation::AssetReference value = Get();
    return value.type_ + "," + value.id_;
}

template<> std::string Attribute<QVariant>::ToString() const
{
    QVariant value = Get();
    
    return value.toString().toStdString();
}

template<> std::string Attribute<QVariantList >::ToString() const
{
    QVariantList values = Get();

    std::string stringValue = "";
    for(uint i = 0; i < values.size(); i++)
    {
        stringValue += values[i].toString().toStdString();
        if(i < values.size() - 1)
            stringValue += ";";
    }
    return stringValue;
}

template<> std::string Attribute<Transform>::ToString() const
{
    QString value("");
    Transform transform = Get();
    Vector3D<float> editValues[3];
    editValues[0] = transform.position;
    editValues[1] = transform.rotation;
    editValues[2] = transform.scale;

    for(uint i = 0; i < 3; i++)
    {
        value += QString::number(editValues[i].x);
        value += ",";
        value += QString::number(editValues[i].y);
        value += ",";
        value += QString::number(editValues[i].z);
        if(i < 2)
            value += ",";
    }
    return value.toStdString();
}

// TYPENAMETOSTRING TEMPLATE IMPLEMENTATIONS.

template<> std::string Attribute<int>::TypenameToString() const
{
    return "int";
}

template<> std::string Attribute<uint>::TypenameToString() const
{
    return "uint";
}

template<> std::string Attribute<float>::TypenameToString() const
{
    return "real";
}

template<> std::string Attribute<QString>::TypenameToString() const
{
    return "string";
}

template<> std::string Attribute<bool>::TypenameToString() const
{
    return "bool";
}

template<> std::string Attribute<Vector3df>::TypenameToString() const
{
    return "vector3df";
}

template<> std::string Attribute<Quaternion>::TypenameToString() const
{
    return "quaternion";
}

template<> std::string Attribute<Color>::TypenameToString() const
{
    return "color";
}

template<> std::string Attribute<Foundation::AssetReference>::TypenameToString() const
{
    return "assetreference";
}

template<> std::string Attribute<QVariant>::TypenameToString() const
{
    return "qvariant";
}

template<> std::string Attribute<QVariantList >::TypenameToString() const
{
    return "qvariantlist";
}

template<> std::string Attribute<Transform>::TypenameToString() const
{
    return "transform";
}

    // FROMSTRING TEMPLATE IMPLEMENTATIONS.

template<> void Attribute<QString>::FromString(const std::string& str, AttributeChange::Type change)
{
    ///\todo decode/encode XML-risky characters
    Set(QString::fromStdString(str), change);
}

template<> void Attribute<bool>::FromString(const std::string& str, AttributeChange::Type change)
{
    std::string str_lower = str;
    boost::algorithm::to_lower(str_lower);
    bool value = (boost::algorithm::starts_with(str_lower, "true") || boost::algorithm::starts_with(str_lower, "1"));
    
    Set(value, change);
}

template<> void Attribute<int>::FromString(const std::string& str, AttributeChange::Type change)
{
    try
    {
        int value = ParseString<int>(str);
        Set(value, change);
    }
    catch (...) {}
}

template<> void Attribute<uint>::FromString(const std::string& str, AttributeChange::Type change)
{
    try
    {
        uint value = ParseString<uint>(str);
        Set(value, change);
    }
    catch (...) {}
}

template<> void Attribute<float>::FromString(const std::string& str, AttributeChange::Type change)
{
    try
    {
        float value = ParseString<float>(str);
        Set(value, change);
    }
    catch (...) {}
}

template<> void Attribute<Vector3df>::FromString(const std::string& str, AttributeChange::Type change)
{
    StringVector components = SplitString(str, ' ');
    if (components.size() == 3)
    {
        try
        {
            Vector3df value;
            value.x = ParseString<float>(components[0]);
            value.y = ParseString<float>(components[1]);
            value.z = ParseString<float>(components[2]);
            Set(value, change);
        }
        catch (...) {}
    }
}

template<> void Attribute<Color>::FromString(const std::string& str, AttributeChange::Type change)
{
    Color value;
    StringVector components = SplitString(str, ' ');
    if (components.size() == 3)
    {
        try
        {
            value.r = ParseString<float>(components[0]);
            value.g = ParseString<float>(components[1]);
            value.b = ParseString<float>(components[2]);
            Set(value, change);
        }
        catch (...) {}
    }
    if (components.size() == 4)
    {
        try
        {
            value.r = ParseString<float>(components[0]);
            value.g = ParseString<float>(components[1]);
            value.b = ParseString<float>(components[2]);
            value.a = ParseString<float>(components[3]);
            Set(value, change);
        }
        catch (...) {}
    }
}

template<> void Attribute<Quaternion>::FromString(const std::string& str, AttributeChange::Type change)
{
    StringVector components = SplitString(str, ' ');
    if (components.size() == 4)
    {
        try
        {
            Quaternion value;
            value.w = ParseString<float>(components[0]);
            value.x = ParseString<float>(components[1]);
            value.y = ParseString<float>(components[2]);
            value.z = ParseString<float>(components[3]);
            Set(value, change);
        }
        catch (...) {}
    }
}

template<> void Attribute<Foundation::AssetReference>::FromString(const std::string& str, AttributeChange::Type change)
{
    // We store type first, then ",", then asset id
    std::string::size_type pos = str.find(',');
    if (pos == std::string::npos)
        return;
    std::string type = str.substr(0, pos);
    std::string id = str.substr(pos + 1);
    
    Foundation::AssetReference value(id, type);
    Set(value, change);
}

template<> void Attribute<QVariant>::FromString(const std::string& str, AttributeChange::Type change)
{
    QVariant value(QString(str.c_str()));
    Set(value, change);
}

template<> void Attribute<QVariantList >::FromString(const std::string& str, AttributeChange::Type change)
{
    QVariantList value;
    QString strValue = QString::fromStdString(str);
    QStringList components = strValue.split(';');

    for(uint i = 0; i < components.size(); i++)
        value.push_back(QVariant(components[i]));
    if(value.size() == 1)
        if(value[0] == "")
            value.pop_back();
    Set(value, change);
}

template<> void Attribute<Transform>::FromString(const std::string& str, AttributeChange::Type change)
{
    QString value = QString::fromStdString(str);
    QStringList matrixElements = value.split(',');
    Transform result;
    if(matrixElements.size() == 9) //Ensure that we have right amount of elements.
    {
        float values[9];
        for(uint i = 0; i < 3; i++)
        {
            uint startIndex = 3 * i;
            for(uint j = 0; j < 3; j++)
                values[j + startIndex] = ParseString<float>(matrixElements[j + startIndex].toStdString(), 0.0f);
        }
        result.SetPos(values[0], values[1], values[2]);
        result.SetRot(values[3], values[4], values[5]);
        result.SetScale(values[6], values[7], values[8]);
    }
    Set(result, change);
}

