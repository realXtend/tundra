/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   AttributeInterface.cpp
 *  @brief  Abstract base class and template class for entity-component attributes.
 */

#include "AttributeInterface.h"
#include "ComponentInterface.h"
#include "AssetInterface.h"
#include "Core.h"
#include "CoreStdIncludes.h"

#include <QVariant>

// Implementation code for some common attributes

namespace Foundation
{

AttributeInterface::AttributeInterface(ComponentInterface* owner, const char* name) :
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

template<> std::string Attribute<std::string>::ToString() const
{
    ///\todo decode/encode XML-risky characters
    return Get();
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

template<> std::string Attribute<Real>::ToString() const
{
    return ::ToString<Real>(Get());
}

template<> std::string Attribute<Vector3df>::ToString() const
{
    Vector3df value = Get();
    
    return ::ToString<Real>(value.x) + " " +
        ::ToString<Real>(value.y) + " " +
        ::ToString<Real>(value.z);
}
    
template<> std::string Attribute<Quaternion>::ToString() const
{
    Quaternion value = Get();
    
    return ::ToString<Real>(value.w) + " " +
        ::ToString<Real>(value.x) + " " +
        ::ToString<Real>(value.y) + " " +
        ::ToString<Real>(value.z);
}

template<> std::string Attribute<Color>::ToString() const
{
    Color value = Get();
    
    return ::ToString<Real>(value.r) + " " +
        ::ToString<Real>(value.g) + " " +
        ::ToString<Real>(value.b) + " " +
        ::ToString<Real>(value.a);
}

template<> std::string Attribute<AssetReference>::ToString() const
{
    AssetReference value = Get();
    
    return value.type_ + "," + value.id_;
}

template<> std::string Attribute<QVariant>::ToString() const
{
    QVariant value = Get();
    
    return value.toString().toStdString();
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

template<> std::string Attribute<Real>::TypenameToString() const
{
    return "real";
}

template<> std::string Attribute<std::string>::TypenameToString() const
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

template<> std::string Attribute<AssetReference>::TypenameToString() const
{
    return "assetreference";
}

template<> std::string Attribute<QVariant>::TypenameToString() const
{
    return "qvariant";
}

    // FROMSTRING TEMPLATE IMPLEMENTATIONS.

template<> void Attribute<std::string>::FromString(const std::string& str, AttributeChange::Type change)
{
    ///\todo decode/encode XML-risky characters
    Set(str, change);
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

template<> void Attribute<Real>::FromString(const std::string& str, AttributeChange::Type change)
{
    try
    {
        Real value = ParseString<Real>(str);
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
            value.x = ParseString<Real>(components[0]);
            value.y = ParseString<Real>(components[1]);
            value.z = ParseString<Real>(components[2]);
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
            value.r = ParseString<Real>(components[0]);
            value.g = ParseString<Real>(components[1]);
            value.b = ParseString<Real>(components[2]);
            Set(value, change);
        }
        catch (...) {}
    }
    if (components.size() == 4)
    {
        try
        {
            value.r = ParseString<Real>(components[0]);
            value.g = ParseString<Real>(components[1]);
            value.b = ParseString<Real>(components[2]);
            value.a = ParseString<Real>(components[3]);
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
            value.w = ParseString<Real>(components[0]);
            value.x = ParseString<Real>(components[1]);
            value.y = ParseString<Real>(components[2]);
            value.z = ParseString<Real>(components[3]);
            Set(value, change);
        }
        catch (...) {}
    }
}

template<> void Attribute<AssetReference>::FromString(const std::string& str, AttributeChange::Type change)
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

}
