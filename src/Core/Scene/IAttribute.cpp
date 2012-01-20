/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   IAttribute.cpp
    @brief  Abstract base class and template class for entity-component attributes. */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "IAttribute.h"
#include "IComponent.h"
#include "CoreTypes.h"
#include "CoreDefines.h"
#include "Transform.h"
#include "AssetReference.h"
#include "EntityReference.h"
#include "LoggingFunctions.h"
#include "Color.h"
#include "Math/Quat.h"
#include "Math/float2.h"
#include "Math/float3.h"
#include "Math/float3.h"
#include "Math/MathFunc.h"

#include <QVariant>
#include <QStringList>
#include <QScriptEngine>
#include <QSize>
#include <QPoint>

#include <kNet.h>

#include "MemoryLeakCheck.h"

using namespace kNet;

// Implementation code for some common attributes

IAttribute::IAttribute(IComponent* owner_, const char* name_) :
    name(name_),
    metadata(0),
    dynamic(false),
    owner(0),
    index(0),
    valueChanged(true)
{
    if (owner_)
        owner_->AddAttribute(this);
}

void IAttribute::Changed(AttributeChange::Type change)
{
    if (owner)
        owner->EmitAttributeChanged(this, change);
}

// Hide all template implementations from being included to public documentation
/// @cond PRIVATE

// TYPEID TEMPLATE IMPLEMENTATIONS
template<> u32 Attribute<QString>::TypeId() const { return cAttributeString; }
template<> u32 Attribute<int>::TypeId() const { return cAttributeInt; }
template<> u32 Attribute<float>::TypeId() const { return cAttributeReal; }
template<> u32 Attribute<Color>::TypeId() const { return cAttributeColor; }
template<> u32 Attribute<float2>::TypeId() const { return cAttributeFloat2; }
template<> u32 Attribute<float3>::TypeId() const { return cAttributeFloat3; }
template<> u32 Attribute<float4>::TypeId() const { return cAttributeFloat4; }
template<> u32 Attribute<bool>::TypeId() const { return cAttributeBool; }
template<> u32 Attribute<uint>::TypeId() const { return cAttributeUInt; }
template<> u32 Attribute<Quat>::TypeId() const { return cAttributeQuat; }
template<> u32 Attribute<AssetReference>::TypeId() const { return cAttributeAssetReference; }
template<> u32 Attribute<AssetReferenceList>::TypeId() const { return cAttributeAssetReferenceList; }
template<> u32 Attribute<EntityReference>::TypeId() const { return cAttributeEntityReference; }
template<> u32 Attribute<QVariant>::TypeId() const { return cAttributeQVariant; }
template<> u32 Attribute<QVariantList>::TypeId() const { return cAttributeQVariantList; }
template<> u32 Attribute<Transform>::TypeId() const { return cAttributeTransform; }
template<> u32 Attribute<QPoint>::TypeId() const { return cAttributeQPoint; }

// DefaultValue implementations
template<> QString Attribute<QString>::DefaultValue() const { return QString(); }
template<> int Attribute<int>::DefaultValue() const { return 0; }
template<> float Attribute<float>::DefaultValue() const { return 0.f; }
template<> Color Attribute<Color>::DefaultValue() const { return Color(); }
template<> float2 Attribute<float2>::DefaultValue() const { return float2::zero; }
template<> float3 Attribute<float3>::DefaultValue() const { return float3::zero; }
template<> float4 Attribute<float4>::DefaultValue() const { return float4::zero; }
template<> bool Attribute<bool>::DefaultValue() const { return false; }
template<> uint Attribute<uint>::DefaultValue() const { return 0; }
template<> Quat Attribute<Quat>::DefaultValue() const { return Quat::identity; }
template<> AssetReference Attribute<AssetReference>::DefaultValue() const { return AssetReference(); }
template<> AssetReferenceList Attribute<AssetReferenceList>::DefaultValue() const { return AssetReferenceList(); }
template<> EntityReference Attribute<EntityReference>::DefaultValue() const { return EntityReference(); }
template<> QVariant Attribute<QVariant>::DefaultValue() const { return QVariant(); }
template<> QVariantList Attribute<QVariantList>::DefaultValue() const { return QVariantList(); }
template<> Transform Attribute<Transform>::DefaultValue() const { return Transform(); }
template<> QPoint Attribute<QPoint>::DefaultValue() const { return QPoint(); }

// TOSTRING TEMPLATE IMPLEMENTATIONS.

template<> std::string Attribute<QString>::ToString() const
{
    ///\todo decode/encode XML-risky characters
    return Get().toStdString();
}

template<> std::string Attribute<bool>::ToString() const
{
    return Get() ? "true" : "false";
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
    
template<> std::string Attribute<Quat>::ToString() const
{
    return Get().SerializeToString();
}

template<> std::string Attribute<float2>::ToString() const
{
    return Get().SerializeToString();
}

template<> std::string Attribute<float3>::ToString() const
{
    return Get().SerializeToString();
}

template<> std::string Attribute<float4>::ToString() const
{
    return Get().SerializeToString();
}

template<> std::string Attribute<Color>::ToString() const
{
    const Color &value = Get();
    return ::ToString<float>(value.r) + " " +
        ::ToString<float>(value.g) + " " +
        ::ToString<float>(value.b) + " " +
        ::ToString<float>(value.a);
}

template<> std::string Attribute<AssetReference>::ToString() const
{
    return Get().ref.toStdString();
}

template<> std::string Attribute<AssetReferenceList>::ToString() const
{
    std::string stringValue = "";
    const AssetReferenceList &values = Get();
    for(int i = 0; i < values.Size(); ++i)
    {
        stringValue += values[i].ref.toStdString();
        if (i < values.Size() - 1)
            stringValue += ";";
    }

    return stringValue;
}

template<> std::string Attribute<EntityReference>::ToString() const
{
    return Get().ref.toStdString();
}

template<> std::string Attribute<QVariant>::ToString() const
{
    return Get().toString().toStdString();
}

template<> std::string Attribute<QVariantList>::ToString() const
{
    const QVariantList &values = Get();

    std::string stringValue = "";
    for(int i = 0; i < values.size(); i++)
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
    const Transform &transform = Get();
    float3 editValues[3];
    editValues[0] = transform.pos;
    editValues[1] = transform.rot;
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

template<> std::string Attribute<QPoint>::ToString() const
{
    const QPoint &value = Get();
    return ::ToString<int>(value.x()) + " " +
        ::ToString<int>(value.y());
}

// TYPENAMETOSTRING TEMPLATE IMPLEMENTATIONS.

template<> QString Attribute<int>::TypeName() const
{
    return "int";
}

template<> QString Attribute<uint>::TypeName() const
{
    return "uint";
}

template<> QString Attribute<float>::TypeName() const
{
    return "real";
}

template<> QString Attribute<QString>::TypeName() const
{
    return "string";
}

template<> QString Attribute<bool>::TypeName() const
{
    return "bool";
}

template<> QString Attribute<Quat>::TypeName() const
{
    return "quat";
}

template<> QString Attribute<float2>::TypeName() const
{
    return "float2";
}

template<> QString Attribute<float3>::TypeName() const
{
    return "float3";
}

template<> QString Attribute<float4>::TypeName() const
{
    return "float4";
}

template<> QString Attribute<Color>::TypeName() const
{
    return "color";
}

template<> QString Attribute<AssetReference>::TypeName() const
{
    return "assetreference";
}

template<> QString Attribute<AssetReferenceList>::TypeName() const
{
    return "assetreferencelist";
}

template<> QString Attribute<EntityReference>::TypeName() const
{
    return "entityreference";
}

template<> QString Attribute<QVariant>::TypeName() const
{
    return "qvariant";
}

template<> QString Attribute<QVariantList >::TypeName() const
{
    return "qvariantlist";
}

template<> QString Attribute<Transform>::TypeName() const
{
    return "transform";
}

template<> QString Attribute<QPoint>::TypeName() const
{
    return "qpoint";
}

// FROMSTRING TEMPLATE IMPLEMENTATIONS.

template<> void Attribute<QString>::FromString(const std::string& str, AttributeChange::Type change)
{
    ///\todo decode/encode XML-risky characters
    Set(QString::fromStdString(str), change);
}

template<> void Attribute<bool>::FromString(const std::string& str, AttributeChange::Type change)
{
    Set(ParseBool(str), change);
}

template<> void Attribute<int>::FromString(const std::string& str, AttributeChange::Type change)
{
    try
    {
        int value = ParseString<int>(str);
        Set(value, change);
    }
    catch(...) {}
}

template<> void Attribute<uint>::FromString(const std::string& str, AttributeChange::Type change)
{
    try
    {
        uint value = ParseString<uint>(str);
        Set(value, change);
    }
    catch(...) {}
}

template<> void Attribute<float>::FromString(const std::string& str, AttributeChange::Type change)
{
    try
    {
        float value = ParseString<float>(str);
        Set(value, change);
    }
    catch(...) {}
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
        catch(...) {}
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
        catch(...) {}
    }
}

template<> void Attribute<Quat>::FromString(const std::string& str, AttributeChange::Type change)
{
    Set(Quat::FromString(str), change);
}

template<> void Attribute<float2>::FromString(const std::string& str, AttributeChange::Type change)
{
    Set(float2::FromString(str), change);
}

template<> void Attribute<float3>::FromString(const std::string& str, AttributeChange::Type change)
{
    Set(float3::FromString(str), change);
}

template<> void Attribute<float4>::FromString(const std::string& str, AttributeChange::Type change)
{
    Set(float4::FromString(str), change);
}

template<> void Attribute<AssetReference>::FromString(const std::string& str, AttributeChange::Type change)
{
    Set(AssetReference(str.c_str()), change);
}

template<> void Attribute<AssetReferenceList>::FromString(const std::string& str, AttributeChange::Type change)
{
    AssetReferenceList value;
    QString strValue = QString::fromStdString(str);
    QStringList components = strValue.split(';');
    for(int i = 0; i < components.size(); i++)
        value.Append(AssetReference(components[i]));
    if (value.Size() == 1)
        if (value[0].ref.trimmed().isEmpty())
            value.RemoveLast();

    Set(value, change);
}

template<> void Attribute<EntityReference>::FromString(const std::string& str, AttributeChange::Type change)
{
    Set(EntityReference(str.c_str()), change);
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

    for(int i = 0; i < components.size(); i++)
        value.push_back(QVariant(components[i]));
    if(value.size() == 1)
        if(value[0] == "")
            value.pop_back();
    Set(value, change);
}

template<> void Attribute<Transform>::FromString(const std::string& str, AttributeChange::Type change)
{
    StringVector elements = SplitString(str, ',');
    if (elements.size() != 9)
    {
        ::LogError("Attribute<Transform>::FromString failed: Can't deserialize string \"" + str + "\"!");
        return;
    }

    float posX = ParseString<float>(elements[0], 0.0f);
    float posY = ParseString<float>(elements[1], 0.0f);
    float posZ = ParseString<float>(elements[2], 0.0f);
    float eulerX = ParseString<float>(elements[3], 0.0f);
    float eulerY = ParseString<float>(elements[4], 0.0f);
    float eulerZ = ParseString<float>(elements[5], 0.0f);
    float scaleX = ParseString<float>(elements[6], 0.0f);
    float scaleY = ParseString<float>(elements[7], 0.0f);
    float scaleZ = ParseString<float>(elements[8], 0.0f);

    Transform result;
    result.SetPos(posX, posY, posZ);
    result.SetRotation(eulerX, eulerY, eulerZ);
    result.SetScale(scaleX, scaleY, scaleZ);
    Set(result, change);
}

template<> void Attribute<QPoint>::FromString(const std::string& str, AttributeChange::Type change)
{
    StringVector components = SplitString(str, ' ');
    if (components.size() == 2)
    {
        try
        {
            QPoint value;
            value.setX(ParseString<int>(components[0]));
            value.setY(ParseString<int>(components[1]));
            Set(value, change);
        }
        catch(...) {}
    }
}

// FROMQVARIANT TEMPLATE IMPLEMENTATIONS.

template<> void Attribute<QString>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(variant.toString(), change);
}

template<> void Attribute<bool>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(variant.toBool(), change);
}

template<> void Attribute<int>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(variant.toInt(), change);
}

template<> void Attribute<uint>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(variant.toUInt(), change);
}

template<> void Attribute<float>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(variant.toFloat(), change);
}

template<> void Attribute<Color>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(qvariant_cast<Color>(variant), change);
}

template<> void Attribute<Quat>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(qvariant_cast<Quat>(variant), change);
}

template<> void Attribute<float2>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(qvariant_cast<float2>(variant), change);
}

template<> void Attribute<float3>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(qvariant_cast<float3>(variant), change);
}

template<> void Attribute<float4>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(qvariant_cast<float4>(variant), change);
}

template<> void Attribute<AssetReference>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(qvariant_cast<AssetReference>(variant), change);
}

template<> void Attribute<AssetReferenceList>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(qvariant_cast<AssetReferenceList>(variant), change);
}

template<> void Attribute<EntityReference>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(qvariant_cast<EntityReference>(variant), change);
}

template<> void Attribute<QVariant>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(variant, change);
}

template<> void Attribute<QVariantList>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(qvariant_cast<QVariantList>(variant), change);
}

template<> void Attribute<Transform>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(qvariant_cast<Transform>(variant), change);
}

template <> void Attribute<QPoint>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(qvariant_cast<QPoint>(variant), change);
}

// TOQVARIANT TEMPLATE IMPLEMENTATIONS.

template<> QVariant Attribute<QString>::ToQVariant() const
{
    return QVariant(Get());
}

template<> QVariant Attribute<bool>::ToQVariant() const
{
    return QVariant(Get());
}

template<> QVariant Attribute<int>::ToQVariant() const
{
    return QVariant(Get());
}

template<> QVariant Attribute<uint>::ToQVariant() const
{
    return QVariant(Get());
}

template<> QVariant Attribute<float>::ToQVariant() const
{
    return QVariant(Get());
}
    
template<> QVariant Attribute<Quat>::ToQVariant() const
{
    return QVariant::fromValue<Quat>(Get());
}

template<> QVariant Attribute<float2>::ToQVariant() const
{
    return QVariant::fromValue<float2>(Get());
}

template<> QVariant Attribute<float3>::ToQVariant() const
{
    return QVariant::fromValue<float3>(Get());
}

template<> QVariant Attribute<float4>::ToQVariant() const
{
    return QVariant::fromValue<float4>(Get());
}

template<> QVariant Attribute<Color>::ToQVariant() const
{
    return QVariant::fromValue<Color>(Get());
}

template<> QVariant Attribute<AssetReference>::ToQVariant() const
{
    return QVariant::fromValue<AssetReference>(Get());
}

template<> QVariant Attribute<AssetReferenceList>::ToQVariant() const
{
    return QVariant::fromValue<AssetReferenceList>(Get());
}

template<> QVariant Attribute<EntityReference>::ToQVariant() const
{
    return QVariant::fromValue<EntityReference>(Get());
}

template<> QVariant Attribute<QVariant>::ToQVariant() const
{
    return Get();
}

template<> QVariant Attribute<QVariantList >::ToQVariant() const
{
    return QVariant::fromValue<QVariantList>(Get());
}

template<> QVariant Attribute<Transform>::ToQVariant() const
{
    return QVariant::fromValue<Transform>(Get());
}

template<> QVariant Attribute<QPoint>::ToQVariant() const
{
    return QVariant::fromValue<QPoint>(Get());
}

// FROMSCRIPTVALUE TEMPLATE IMPLEMENTATIONS.

template<> void Attribute<QString>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(qScriptValueToValue<QString>(value), change);
}

template<> void Attribute<bool>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(qScriptValueToValue<bool>(value), change);
}

template<> void Attribute<int>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(qScriptValueToValue<int>(value), change);
}

template<> void Attribute<uint>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(qScriptValueToValue<uint>(value), change);
}

template<> void Attribute<float>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(qScriptValueToValue<float>(value), change);
}
    
template<> void Attribute<Quat>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(value.data().toVariant().value<Quat>(), change);
}

template<> void Attribute<float2>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(value.data().toVariant().value<float2>(), change);
}

template<> void Attribute<float3>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(value.data().toVariant().value<float3>(), change);
}

template<> void Attribute<float4>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(value.data().toVariant().value<float4>(), change);
}

template<> void Attribute<Color>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(qScriptValueToValue<Color>(value), change);
}

template<> void Attribute<AssetReference>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(qScriptValueToValue<AssetReference>(value), change);
}

template<> void Attribute<AssetReferenceList>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(qScriptValueToValue<AssetReferenceList>(value), change);
}

template<> void Attribute<EntityReference>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(qScriptValueToValue<EntityReference>(value), change);
}

template<> void Attribute<QVariant>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(qScriptValueToValue<QVariant>(value), change);
}

template<> void Attribute<QVariantList>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(qScriptValueToValue<QVariantList>(value), change);
}

template<> void Attribute<Transform>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(qScriptValueToValue<Transform>(value), change);
}

template<> void Attribute<QPoint>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(qScriptValueToValue<QPoint>(value), change);
}

// TOBINARY TEMPLATE IMPLEMENTATIONS.

template<> void Attribute<QString>::ToBinary(kNet::DataSerializer& dest) const
{
    QByteArray utf8bytes = value.toUtf8();
    dest.Add<u16>(utf8bytes.size());
    if (utf8bytes.size())
        dest.AddArray<u8>((const u8*)utf8bytes.data(), utf8bytes.size());
}

template<> void Attribute<bool>::ToBinary(kNet::DataSerializer& dest) const
{
    if (value)
        dest.Add<u8>(1);
    else
        dest.Add<u8>(0);
}

template<> void Attribute<int>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<s32>(value);
}

template<> void Attribute<uint>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<u32>(value);
}

template<> void Attribute<float>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<float>(value);
}

template<> void Attribute<Quat>::ToBinary(kNet::DataSerializer& dest) const
{
    ///\todo Optimize here by omitting the fourth scalar.
    dest.Add<float>(value.x);
    dest.Add<float>(value.y);
    dest.Add<float>(value.z);
    dest.Add<float>(value.w);
}

template<> void Attribute<float2>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<float>(value.x);
    dest.Add<float>(value.y);
}

template<> void Attribute<float3>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<float>(value.x);
    dest.Add<float>(value.y);
    dest.Add<float>(value.z);
}

template<> void Attribute<float4>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<float>(value.x);
    dest.Add<float>(value.y);
    dest.Add<float>(value.z);
    dest.Add<float>(value.w);
}

template<> void Attribute<Color>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<float>(value.r);
    dest.Add<float>(value.g);
    dest.Add<float>(value.b);
    dest.Add<float>(value.a);
}

template<> void Attribute<AssetReference>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.AddString(value.ref.toStdString());
}

template<> void Attribute<AssetReferenceList>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<u8>(value.Size());
    for(int i = 0; i < value.Size(); ++i)
        dest.AddString(value[i].ref.toStdString());
}

template<> void Attribute<EntityReference>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.AddString(value.ref.toStdString());
}

template<> void Attribute<QVariant>::ToBinary(kNet::DataSerializer& dest) const
{
    std::string str = value.toString().toStdString();
    dest.AddString(str);
}

template<> void Attribute<QVariantList>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<u8>(value.size());
    for(uint i = 0; i < (uint)value.size(); ++i)
        dest.AddString(value[i].toString().toStdString());
}

template<> void Attribute<Transform>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<float>(value.pos.x);
    dest.Add<float>(value.pos.y);
    dest.Add<float>(value.pos.z);
    dest.Add<float>(value.rot.x);
    dest.Add<float>(value.rot.y);
    dest.Add<float>(value.rot.z);
    dest.Add<float>(value.scale.x);
    dest.Add<float>(value.scale.y);
    dest.Add<float>(value.scale.z);
}

template<> void Attribute<QPoint>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<s32>(value.x());
    dest.Add<s32>(value.y());
}

// FROMBINARY TEMPLATE IMPLEMENTATIONS.

template<> void Attribute<QString>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    QByteArray utf8bytes;
    utf8bytes.resize(source.Read<u16>());
    if (utf8bytes.size())
        source.ReadArray<u8>((u8*)utf8bytes.data(), utf8bytes.size());
    Set(QString::fromUtf8(utf8bytes.data(), utf8bytes.size()), change);
}

template<> void Attribute<bool>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    Set(source.Read<u8>() ? true : false, change);
}

template<> void Attribute<int>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    Set(source.Read<s32>(), change);
}

template<> void Attribute<uint>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    Set(source.Read<u32>(), change);
}

template<> void Attribute<float>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    Set(source.Read<float>(), change);
}

template<> void Attribute<Color>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    Color value;
    value.r = source.Read<float>();
    value.g = source.Read<float>();
    value.b = source.Read<float>();
    value.a = source.Read<float>();
    Set(value, change);
}

template<> void Attribute<Quat>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    ///\todo Optimize here by omitting the fourth scalar.
    Quat value;
    value.x = source.Read<float>();
    value.y = source.Read<float>();
    value.z = source.Read<float>();
    value.w = source.Read<float>();
    Set(value, change);
}

template<> void Attribute<float2>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    float2 value;
    value.x = source.Read<float>();
    value.y = source.Read<float>();
    Set(value, change);
}

template<> void Attribute<float3>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    float3 value;
    value.x = source.Read<float>();
    value.y = source.Read<float>();
    value.z = source.Read<float>();
    Set(value, change);
}

template<> void Attribute<float4>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    float4 value;
    value.x = source.Read<float>();
    value.y = source.Read<float>();
    value.z = source.Read<float>();
    value.w = source.Read<float>();
    Set(value, change);
}

template<> void Attribute<AssetReference>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    AssetReference value;
    value.ref = source.ReadString().c_str();
    Set(value, change);
}

template<> void Attribute<AssetReferenceList>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    AssetReferenceList value;
    u8 numValues = source.Read<u8>();
    for(u32 i = 0; i < numValues; ++i)
        value.Append(AssetReference(source.ReadString().c_str()));

    Set(value, change);
}

template<> void Attribute<EntityReference>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    EntityReference value;
    value.ref = source.ReadString().c_str();
    Set(value, change);
}

template<> void Attribute<QVariant>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    std::string str = source.ReadString();
    QVariant value(QString(str.c_str()));
    Set(value, change);
}

template<> void Attribute<QVariantList>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    QVariantList value;
    
    u8 numValues = source.Read<u8>();
    for(u32 i = 0; i < numValues; ++i)
    {
        std::string str = source.ReadString();
        value.append(QVariant(QString(str.c_str())));
    }
    
    Set(value, change);
}

template<> void Attribute<Transform>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    Transform value;
    value.pos.x = source.Read<float>();
    value.pos.y = source.Read<float>();
    value.pos.z = source.Read<float>();
    value.rot.x = source.Read<float>();
    value.rot.y = source.Read<float>();
    value.rot.z = source.Read<float>();
    value.scale.x = source.Read<float>();
    value.scale.y = source.Read<float>();
    value.scale.z = source.Read<float>();
    Set(value, change);
}

template<> void Attribute<QPoint>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    QPoint value;
    value.setX(source.Read<s32>());
    value.setY(source.Read<s32>());
    Set(value, change);
}

// INTERPOLATE TEMPLATE IMPLEMENTATIONS

template<> void Attribute<QString>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
    ::LogError("Attribute<QString>::Interpolate invoked! QString attributes cannot be animated!");
}

template<> void Attribute<bool>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
    ::LogError("Attribute<bool>::Interpolate invoked! bool attributes cannot be animated!");
}

template<> void Attribute<AssetReference>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
    ::LogError("Attribute<AssetReference>::Interpolate invoked! AssetReference attributes cannot be animated!");
}

template<> void Attribute<AssetReferenceList>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
    ::LogError("Attribute<AssetReferenceList>::Interpolate invoked! AssetReferenceList attributes cannot be animated!");
}

template<> void Attribute<EntityReference>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
    ::LogError("Attribute<EntityReference>::Interpolate invoked! EntityReference attributes cannot be animated!");
}

template<> void Attribute<QVariant>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
    ::LogError("Attribute<QVariant>::Interpolate invoked! QVariant attributes cannot be animated!");
}

template<> void Attribute<QVariantList>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
    ::LogError("Attribute<QVariantList>::Interpolate invoked! QVariantList attributes cannot be animated!");
}

template<> void Attribute<int>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
    Attribute<int>* startInt = dynamic_cast<Attribute<int>*>(start);
    Attribute<int>* endInt = dynamic_cast<Attribute<int>*>(end);
    if (startInt && endInt)
        Set(RoundInt(Lerp((float)startInt->Get(), (float)endInt->Get(), t)), change);
}

template<> void Attribute<uint>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
    Attribute<uint>* startUint = dynamic_cast<Attribute<uint>*>(start);
    Attribute<uint>* endUint = dynamic_cast<Attribute<uint>*>(end);
    if (startUint && endUint)
        Set((uint)RoundInt(Lerp((float)startUint->Get(), (float)endUint->Get(), t)), change);
}

template<> void Attribute<float>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
    Attribute<float>* startFloat = dynamic_cast<Attribute<float>*>(start);
    Attribute<float>* endFloat = dynamic_cast<Attribute<float>*>(end);
    if (startFloat && endFloat)
        Set(Lerp(startFloat->Get(), endFloat->Get(), t), change);
}

template<> void Attribute<Quat>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
    Attribute<Quat>* startQuat = dynamic_cast<Attribute<Quat>*>(start);
    Attribute<Quat>* endQuat = dynamic_cast<Attribute<Quat>*>(end);
    if (startQuat && endQuat)
        Set(Slerp(startQuat->Get(), endQuat->Get(), t), change);
}

template<> void Attribute<float2>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
    Attribute<float2>* startVec = dynamic_cast<Attribute<float2>*>(start);
    Attribute<float2>* endVec = dynamic_cast<Attribute<float2>*>(end);
    if (startVec && endVec)
        Set(Lerp(startVec->Get(), endVec->Get(), t), change);
}

template<> void Attribute<float3>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
    Attribute<float3>* startVec = dynamic_cast<Attribute<float3>*>(start);
    Attribute<float3>* endVec = dynamic_cast<Attribute<float3>*>(end);
    if (startVec && endVec)
        Set(Lerp(startVec->Get(), endVec->Get(), t), change);
}

template<> void Attribute<float4>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
    Attribute<float4>* startVec = dynamic_cast<Attribute<float4>*>(start);
    Attribute<float4>* endVec = dynamic_cast<Attribute<float4>*>(end);
    if (startVec && endVec)
        Set(Lerp(startVec->Get(), endVec->Get(), t), change);
}

template<> void Attribute<Color>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
    Attribute<Color>* startColor = dynamic_cast<Attribute<Color>*>(start);
    Attribute<Color>* endColor = dynamic_cast<Attribute<Color>*>(end);
    if ((startColor) && (endColor))
    {
        const Color& startValue = startColor->Get();
        const Color& endValue = endColor->Get();
        Color newColor;
        newColor.r = Lerp(startValue.r, endValue.r, t);
        newColor.g = Lerp(startValue.g, endValue.g, t);
        newColor.b = Lerp(startValue.b, endValue.b, t);
        newColor.a = Lerp(startValue.a, endValue.a, t);
        Set(newColor, change);
    }
}

template<> void Attribute<Transform>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
    Attribute<Transform>* startTrans = dynamic_cast<Attribute<Transform>*>(start);
    Attribute<Transform>* endTrans = dynamic_cast<Attribute<Transform>*>(end);
    if ((startTrans) && (endTrans))
    {
        const Transform& startValue = startTrans->Get();
        const Transform& endValue = endTrans->Get();
        Transform newTrans;
        
        // Position
        newTrans.pos = Lerp(startValue.pos, endValue.pos, t);
        
        // Rotation
        newTrans.SetOrientation(Slerp(startValue.Orientation(), endValue.Orientation(), t));
        
        // Scale
        newTrans.scale = Lerp(startValue.scale, endValue.scale, t);
        
        Set(newTrans, change);
    }
}

template<> void Attribute<QPoint>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
    Attribute<QPoint>* startPoint = dynamic_cast<Attribute<QPoint>*>(start);
    Attribute<QPoint>* endPoint = dynamic_cast<Attribute<QPoint>*>(end);
    if (startPoint && endPoint)
    {
        const QPoint& startValue = startPoint->Get();
        const QPoint& endValue = endPoint->Get();
        QPoint newValue;
        newValue.setX((int)(startValue.x() * (1.f - t) + endValue.x() * t + 0.5f));
        newValue.setY((int)(startValue.y() * (1.f - t) + endValue.y() * t + 0.5f));
        Set(newValue, change);
    }
}

/// @endcond
