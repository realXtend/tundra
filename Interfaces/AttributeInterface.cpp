// For conditions of distribution and use, see copyright notice in license.txt

#include "AttributeInterface.h"
#include "AssetInterface.h"
#include "Core.h"
#include "CoreStdIncludes.h"
#include "Framework.h"

// Implementation code for some common attributes

namespace Foundation
{

template<> std::string Attribute<std::string>::ToString() const
{
    // Todo decode/encode XML-risky characters
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
    
    return value.type_ + ":" + value.id_;
}

template<> void Attribute<std::string>::FromString(const std::string& str, ChangeType change)
{
    // Todo decode/encode XML-risky characters
    Set(str, change);
}

template<> void Attribute<bool>::FromString(const std::string& str, ChangeType change)
{
    std::string str_lower = str;
    boost::algorithm::to_lower(str_lower);
    bool value = (boost::algorithm::starts_with(str_lower, "true") || boost::algorithm::starts_with(str_lower, "1"));
    
    Set(value, change);
}

template<> void Attribute<int>::FromString(const std::string& str, ChangeType change)
{
    try
    {
        int value = ParseString<int>(str);
        Set(value, change);
    }
    catch (...) {}
}

template<> void Attribute<uint>::FromString(const std::string& str, ChangeType change)
{
    try
    {
        uint value = ParseString<uint>(str);
        Set(value, change);
    }
    catch (...) {}
}

template<> void Attribute<Real>::FromString(const std::string& str, ChangeType change)
{
    try
    {
        Real value = ParseString<Real>(str);
        Set(value, change);
    }
    catch (...) {}
}

template<> void Attribute<Vector3df>::FromString(const std::string& str, ChangeType change)
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

template<> void Attribute<Color>::FromString(const std::string& str, ChangeType change)
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

template<> void Attribute<Quaternion>::FromString(const std::string& str, ChangeType change)
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

template<> void Attribute<AssetReference>::FromString(const std::string& str, ChangeType change)
{
    // We store type first, then ":", then asset id
    std::string::size_type pos = str.find(':');
    if (pos == std::string::npos)
        return;
    std::string type = str.substr(0, pos);
    std::string id = str.substr(pos + 1);
    
    Foundation::AssetReference value(id, type);
    Set(value, change);
}

}
