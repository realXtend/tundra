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
#include <QPoint>

#include <kNet/DataDeserializer.h>
#include <kNet/DataSerializer.h>

#include "MemoryLeakCheck.h"

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
template<> u32 TUNDRACORE_API Attribute<QString>::TypeId() const { return cAttributeString; }
template<> u32 TUNDRACORE_API Attribute<int>::TypeId() const { return cAttributeInt; }
template<> u32 TUNDRACORE_API Attribute<float>::TypeId() const { return cAttributeReal; }
template<> u32 TUNDRACORE_API Attribute<Color>::TypeId() const { return cAttributeColor; }
template<> u32 TUNDRACORE_API Attribute<float2>::TypeId() const { return cAttributeFloat2; }
template<> u32 TUNDRACORE_API Attribute<float3>::TypeId() const { return cAttributeFloat3; }
template<> u32 TUNDRACORE_API Attribute<float4>::TypeId() const { return cAttributeFloat4; }
template<> u32 TUNDRACORE_API Attribute<bool>::TypeId() const { return cAttributeBool; }
template<> u32 TUNDRACORE_API Attribute<uint>::TypeId() const { return cAttributeUInt; }
template<> u32 TUNDRACORE_API Attribute<Quat>::TypeId() const { return cAttributeQuat; }
template<> u32 TUNDRACORE_API Attribute<AssetReference>::TypeId() const { return cAttributeAssetReference; }
template<> u32 TUNDRACORE_API Attribute<AssetReferenceList>::TypeId() const { return cAttributeAssetReferenceList; }
template<> u32 TUNDRACORE_API Attribute<EntityReference>::TypeId() const { return cAttributeEntityReference; }
template<> u32 TUNDRACORE_API Attribute<QVariant>::TypeId() const { return cAttributeQVariant; }
template<> u32 TUNDRACORE_API Attribute<QVariantList>::TypeId() const { return cAttributeQVariantList; }
template<> u32 TUNDRACORE_API Attribute<Transform>::TypeId() const { return cAttributeTransform; }
template<> u32 TUNDRACORE_API Attribute<QPoint>::TypeId() const { return cAttributeQPoint; }

// DefaultValue implementations
template<> QString TUNDRACORE_API Attribute<QString>::DefaultValue() const { return QString(); }
template<> int TUNDRACORE_API Attribute<int>::DefaultValue() const { return 0; }
template<> float TUNDRACORE_API Attribute<float>::DefaultValue() const { return 0.f; }
template<> Color TUNDRACORE_API Attribute<Color>::DefaultValue() const { return Color(); }
template<> float2 TUNDRACORE_API Attribute<float2>::DefaultValue() const { return float2::zero; }
template<> float3 TUNDRACORE_API Attribute<float3>::DefaultValue() const { return float3::zero; }
template<> float4 TUNDRACORE_API Attribute<float4>::DefaultValue() const { return float4::zero; }
template<> bool TUNDRACORE_API Attribute<bool>::DefaultValue() const { return false; }
template<> uint TUNDRACORE_API Attribute<uint>::DefaultValue() const { return 0; }
template<> Quat TUNDRACORE_API Attribute<Quat>::DefaultValue() const { return Quat::identity; }
template<> AssetReference TUNDRACORE_API Attribute<AssetReference>::DefaultValue() const { return AssetReference(); }
template<> AssetReferenceList TUNDRACORE_API Attribute<AssetReferenceList>::DefaultValue() const { return AssetReferenceList(); }
template<> EntityReference TUNDRACORE_API Attribute<EntityReference>::DefaultValue() const { return EntityReference(); }
template<> QVariant TUNDRACORE_API Attribute<QVariant>::DefaultValue() const { return QVariant(); }
template<> QVariantList TUNDRACORE_API Attribute<QVariantList>::DefaultValue() const { return QVariantList(); }
template<> Transform TUNDRACORE_API Attribute<Transform>::DefaultValue() const { return Transform(); }
template<> QPoint TUNDRACORE_API Attribute<QPoint>::DefaultValue() const { return QPoint(); }

// TOSTRING TEMPLATE IMPLEMENTATIONS.

template<> std::string TUNDRACORE_API Attribute<QString>::ToString() const
{
    ///\todo decode/encode XML-risky characters
    return Get().toStdString();
}

template<> std::string TUNDRACORE_API Attribute<bool>::ToString() const
{
    return Get() ? "true" : "false";
}

template<> std::string TUNDRACORE_API Attribute<int>::ToString() const
{
    char str[256];
    sprintf(str, "%i", Get());
    return str;
}

template<> std::string TUNDRACORE_API Attribute<uint>::ToString() const
{
    char str[256];
    sprintf(str, "%u", Get());
    return str;
}

template<> std::string TUNDRACORE_API Attribute<float>::ToString() const
{
    char str[256];
    sprintf(str, "%f", Get());
    return str;
}

template<> std::string TUNDRACORE_API Attribute<Quat>::ToString() const
{
    return Get().SerializeToString();
}

template<> std::string TUNDRACORE_API Attribute<float2>::ToString() const
{
    return Get().SerializeToString();
}

template<> std::string TUNDRACORE_API Attribute<float3>::ToString() const
{
    return Get().SerializeToString();
}

template<> std::string TUNDRACORE_API Attribute<float4>::ToString() const
{
    return Get().SerializeToString();
}

template<> std::string TUNDRACORE_API Attribute<Color>::ToString() const
{
    return Get().SerializeToString().toStdString();
}

template<> std::string TUNDRACORE_API Attribute<AssetReference>::ToString() const
{
    return Get().ref.toStdString();
}

template<> std::string TUNDRACORE_API Attribute<AssetReferenceList>::ToString() const
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

template<> std::string TUNDRACORE_API Attribute<EntityReference>::ToString() const
{
    return Get().ref.toStdString();
}

template<> std::string TUNDRACORE_API Attribute<QVariant>::ToString() const
{
    return Get().toString().toStdString();
}

template<> std::string TUNDRACORE_API Attribute<QVariantList>::ToString() const
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

template<> std::string TUNDRACORE_API Attribute<Transform>::ToString() const
{
    return Get().SerializeToString().toStdString();
}

template<> std::string TUNDRACORE_API Attribute<QPoint>::ToString() const
{
    return QString("%1 %2").arg(Get().x()).arg(Get().y()).toStdString(); /**< @todo Maybe should use ';' instead of ' ' here as a delimeter as it's used for all other attributes. */
}

// TYPENAMETOSTRING TEMPLATE IMPLEMENTATIONS.

template<> const QString TUNDRACORE_API & Attribute<int>::TypeName() const
{
    return cAttributeIntTypeName;
}

template<> const QString TUNDRACORE_API & Attribute<uint>::TypeName() const
{
    return cAttributeUIntTypeName;
}

template<> const QString TUNDRACORE_API & Attribute<float>::TypeName() const
{
    return cAttributeRealTypeName;
}

template<> const QString TUNDRACORE_API & Attribute<QString>::TypeName() const
{
    return cAttributeStringTypeName;
}

template<> const QString TUNDRACORE_API & Attribute<bool>::TypeName() const
{
    return cAttributeBoolTypeName;
}

template<> const QString TUNDRACORE_API & Attribute<Quat>::TypeName() const
{
    return cAttributeQuatTypeName;
}

template<> const QString TUNDRACORE_API & Attribute<float2>::TypeName() const
{
    return cAttributeFloat2TypeName;
}

template<> const QString TUNDRACORE_API & Attribute<float3>::TypeName() const
{
    return cAttributeFloat3TypeName;
}

template<> const QString TUNDRACORE_API & Attribute<float4>::TypeName() const
{
    return cAttributeFloat4TypeName;
}

template<> const QString TUNDRACORE_API & Attribute<Color>::TypeName() const
{
    return cAttributeColorTypeName;
}

template<> const QString TUNDRACORE_API & Attribute<AssetReference>::TypeName() const
{
    return cAttributeAssetReferenceTypeName;
}

template<> const QString TUNDRACORE_API & Attribute<AssetReferenceList>::TypeName() const
{
    return cAttributeAssetReferenceListTypeName;
}

template<> const QString TUNDRACORE_API & Attribute<EntityReference>::TypeName() const
{
    return cAttributeEntityReferenceTypeName;
}

template<> const QString TUNDRACORE_API & Attribute<QVariant>::TypeName() const
{
    return cAttributeQVariantTypeName;
}

template<> const QString TUNDRACORE_API & Attribute<QVariantList >::TypeName() const
{
    return cAttributeQVariantListTypeName;
}

template<> const QString TUNDRACORE_API & Attribute<Transform>::TypeName() const
{
    return cAttributeTransformTypeName;
}

template<> const QString TUNDRACORE_API & Attribute<QPoint>::TypeName() const
{
    return cAttributeQPointTypeName;
}

// FROMSTRING TEMPLATE IMPLEMENTATIONS.

template<> void TUNDRACORE_API Attribute<QString>::FromString(const std::string& str, AttributeChange::Type change)
{
    ///\todo decode/encode XML-risky characters
    Set(QString::fromStdString(str), change);
}

template<> void TUNDRACORE_API Attribute<bool>::FromString(const std::string& str, AttributeChange::Type change)
{
    Set(ParseBool(str), change);
}

template<> void TUNDRACORE_API Attribute<int>::FromString(const std::string& str, AttributeChange::Type change)
{
    Set(ParseInt(str, DefaultValue()), change);
}

template<> void TUNDRACORE_API Attribute<uint>::FromString(const std::string& str, AttributeChange::Type change)
{
    Set(ParseUInt(str, DefaultValue()), change);
}

template<> void TUNDRACORE_API Attribute<float>::FromString(const std::string& str, AttributeChange::Type change)
{
    Set(ParseFloat(str, DefaultValue()), change);
}

template<> void TUNDRACORE_API Attribute<Color>::FromString(const std::string& str, AttributeChange::Type change)
{
    Set(Color::FromString(str.c_str()), change);
}

template<> void TUNDRACORE_API Attribute<Quat>::FromString(const std::string& str, AttributeChange::Type change)
{
    Set(Quat::FromString(str), change);
}

template<> void TUNDRACORE_API Attribute<float2>::FromString(const std::string& str, AttributeChange::Type change)
{
    Set(float2::FromString(str), change);
}

template<> void TUNDRACORE_API Attribute<float3>::FromString(const std::string& str, AttributeChange::Type change)
{
    Set(float3::FromString(str), change);
}

template<> void TUNDRACORE_API Attribute<float4>::FromString(const std::string& str, AttributeChange::Type change)
{
    Set(float4::FromString(str), change);
}

template<> void TUNDRACORE_API Attribute<AssetReference>::FromString(const std::string& str, AttributeChange::Type change)
{
    Set(AssetReference(str.c_str()), change);
}

template<> void TUNDRACORE_API Attribute<AssetReferenceList>::FromString(const std::string& str, AttributeChange::Type change)
{
    AssetReferenceList value;
    QString strValue = QString::fromStdString(str);
    QStringList components = strValue.split(';');
    for(int i = 0; i < components.size(); i++)
        value.Append(AssetReference(components[i]));
    if (value.Size() == 1 && value[0].ref.trimmed().isEmpty())
        value.RemoveLast();

    Set(value, change);
}

template<> void TUNDRACORE_API Attribute<EntityReference>::FromString(const std::string& str, AttributeChange::Type change)
{
    Set(EntityReference(str.c_str()), change);
}

template<> void TUNDRACORE_API Attribute<QVariant>::FromString(const std::string& str, AttributeChange::Type change)
{
    Set(str.c_str(), change);
}

template<> void TUNDRACORE_API Attribute<QVariantList >::FromString(const std::string& str, AttributeChange::Type change)
{
    QVariantList value;
    QString strValue = QString::fromStdString(str);
    QStringList components = strValue.split(';');
    for(int i = 0; i < components.size(); i++)
        value.push_back(QVariant(components[i]));
    if(value.size() == 1 && value[0] == "")
        value.pop_back();

    Set(value, change);
}

template<> void TUNDRACORE_API Attribute<Transform>::FromString(const std::string& str, AttributeChange::Type change)
{
    Set(Transform::FromString(str.c_str()), change);
}

template<> void TUNDRACORE_API Attribute<QPoint>::FromString(const std::string& str, AttributeChange::Type change)
{
    const float2 value = float2::FromString(str);
    Set(QPoint(value.x, value.y), change);
}

// FROMQVARIANT TEMPLATE IMPLEMENTATIONS.

template<> void TUNDRACORE_API Attribute<QString>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(variant.toString(), change);
}

template<> void TUNDRACORE_API Attribute<bool>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(variant.toBool(), change);
}

template<> void TUNDRACORE_API Attribute<int>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(variant.toInt(), change);
}

template<> void TUNDRACORE_API Attribute<uint>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(variant.toUInt(), change);
}

template<> void TUNDRACORE_API Attribute<float>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(variant.toFloat(), change);
}

template<> void TUNDRACORE_API Attribute<Color>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(qvariant_cast<Color>(variant), change);
}

template<> void TUNDRACORE_API Attribute<Quat>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(qvariant_cast<Quat>(variant), change);
}

template<> void TUNDRACORE_API Attribute<float2>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(qvariant_cast<float2>(variant), change);
}

template<> void TUNDRACORE_API Attribute<float3>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(qvariant_cast<float3>(variant), change);
}

template<> void TUNDRACORE_API Attribute<float4>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(qvariant_cast<float4>(variant), change);
}

template<> void TUNDRACORE_API Attribute<AssetReference>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(qvariant_cast<AssetReference>(variant), change);
}

template<> void TUNDRACORE_API Attribute<AssetReferenceList>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(qvariant_cast<AssetReferenceList>(variant), change);
}

template<> void TUNDRACORE_API Attribute<EntityReference>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(qvariant_cast<EntityReference>(variant), change);
}

template<> void TUNDRACORE_API Attribute<QVariant>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(variant, change);
}

template<> void TUNDRACORE_API Attribute<QVariantList>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(qvariant_cast<QVariantList>(variant), change);
}

template<> void TUNDRACORE_API Attribute<Transform>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(qvariant_cast<Transform>(variant), change);
}

template <> void TUNDRACORE_API Attribute<QPoint>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(qvariant_cast<QPoint>(variant), change);
}

// TOQVARIANT TEMPLATE IMPLEMENTATIONS.

template<> QVariant TUNDRACORE_API Attribute<QString>::ToQVariant() const
{
    return QVariant(Get());
}

template<> QVariant TUNDRACORE_API Attribute<bool>::ToQVariant() const
{
    return QVariant(Get());
}

template<> QVariant TUNDRACORE_API Attribute<int>::ToQVariant() const
{
    return QVariant(Get());
}

template<> QVariant TUNDRACORE_API Attribute<uint>::ToQVariant() const
{
    return QVariant(Get());
}

template<> QVariant TUNDRACORE_API Attribute<float>::ToQVariant() const
{
    return QVariant(Get());
}

template<> QVariant TUNDRACORE_API Attribute<Quat>::ToQVariant() const
{
    return QVariant::fromValue<Quat>(Get());
}

template<> QVariant TUNDRACORE_API Attribute<float2>::ToQVariant() const
{
    return QVariant::fromValue<float2>(Get());
}

template<> QVariant TUNDRACORE_API Attribute<float3>::ToQVariant() const
{
    return QVariant::fromValue<float3>(Get());
}

template<> QVariant TUNDRACORE_API Attribute<float4>::ToQVariant() const
{
    return QVariant::fromValue<float4>(Get());
}

template<> QVariant TUNDRACORE_API Attribute<Color>::ToQVariant() const
{
    return QVariant::fromValue<Color>(Get());
}

template<> QVariant TUNDRACORE_API Attribute<AssetReference>::ToQVariant() const
{
    return QVariant::fromValue<AssetReference>(Get());
}

template<> QVariant TUNDRACORE_API Attribute<AssetReferenceList>::ToQVariant() const
{
    return QVariant::fromValue<AssetReferenceList>(Get());
}

template<> QVariant TUNDRACORE_API Attribute<EntityReference>::ToQVariant() const
{
    return QVariant::fromValue<EntityReference>(Get());
}

template<> QVariant TUNDRACORE_API Attribute<QVariant>::ToQVariant() const
{
    return Get();
}

template<> QVariant TUNDRACORE_API Attribute<QVariantList >::ToQVariant() const
{
    return QVariant::fromValue<QVariantList>(Get());
}

template<> QVariant TUNDRACORE_API Attribute<Transform>::ToQVariant() const
{
    return QVariant::fromValue<Transform>(Get());
}

template<> QVariant TUNDRACORE_API Attribute<QPoint>::ToQVariant() const
{
    return QVariant::fromValue<QPoint>(Get());
}

// FROMSCRIPTVALUE TEMPLATE IMPLEMENTATIONS.

template<> void TUNDRACORE_API Attribute<QString>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(qScriptValueToValue<QString>(value), change);
}

template<> void TUNDRACORE_API Attribute<bool>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(qScriptValueToValue<bool>(value), change);
}

template<> void TUNDRACORE_API Attribute<int>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(qScriptValueToValue<int>(value), change);
}

template<> void TUNDRACORE_API Attribute<uint>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(qScriptValueToValue<uint>(value), change);
}

template<> void TUNDRACORE_API Attribute<float>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(qScriptValueToValue<float>(value), change);
}

template<> void TUNDRACORE_API Attribute<Quat>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(value.data().toVariant().value<Quat>(), change);
}

template<> void TUNDRACORE_API Attribute<float2>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(value.data().toVariant().value<float2>(), change);
}

template<> void TUNDRACORE_API Attribute<float3>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(value.data().toVariant().value<float3>(), change);
}

template<> void TUNDRACORE_API Attribute<float4>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(value.data().toVariant().value<float4>(), change);
}

template<> void TUNDRACORE_API Attribute<Color>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(qScriptValueToValue<Color>(value), change);
}

template<> void TUNDRACORE_API Attribute<AssetReference>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(qScriptValueToValue<AssetReference>(value), change);
}

template<> void TUNDRACORE_API Attribute<AssetReferenceList>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(qScriptValueToValue<AssetReferenceList>(value), change);
}

template<> void TUNDRACORE_API Attribute<EntityReference>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(qScriptValueToValue<EntityReference>(value), change);
}

template<> void TUNDRACORE_API Attribute<QVariant>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(qScriptValueToValue<QVariant>(value), change);
}

template<> void TUNDRACORE_API Attribute<QVariantList>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(qScriptValueToValue<QVariantList>(value), change);
}

template<> void TUNDRACORE_API Attribute<Transform>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(qScriptValueToValue<Transform>(value), change);
}

template<> void TUNDRACORE_API Attribute<QPoint>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(qScriptValueToValue<QPoint>(value), change);
}

// TOBINARY TEMPLATE IMPLEMENTATIONS.

template<> void TUNDRACORE_API Attribute<QString>::ToBinary(kNet::DataSerializer& dest) const
{
    QByteArray utf8bytes = value.toUtf8();
    dest.Add<u16>(utf8bytes.size());
    if (utf8bytes.size())
        dest.AddArray<u8>((const u8*)utf8bytes.data(), utf8bytes.size());
}

template<> void TUNDRACORE_API Attribute<bool>::ToBinary(kNet::DataSerializer& dest) const
{
    if (value)
        dest.Add<u8>(1);
    else
        dest.Add<u8>(0);
}

template<> void TUNDRACORE_API Attribute<int>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<s32>(value);
}

template<> void TUNDRACORE_API Attribute<uint>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<u32>(value);
}

template<> void TUNDRACORE_API Attribute<float>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<float>(value);
}

template<> void TUNDRACORE_API Attribute<Quat>::ToBinary(kNet::DataSerializer& dest) const
{
    ///\todo Optimize here by omitting the fourth scalar.
    dest.Add<float>(value.x);
    dest.Add<float>(value.y);
    dest.Add<float>(value.z);
    dest.Add<float>(value.w);
}

template<> void TUNDRACORE_API Attribute<float2>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<float>(value.x);
    dest.Add<float>(value.y);
}

template<> void TUNDRACORE_API Attribute<float3>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<float>(value.x);
    dest.Add<float>(value.y);
    dest.Add<float>(value.z);
}

template<> void TUNDRACORE_API Attribute<float4>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<float>(value.x);
    dest.Add<float>(value.y);
    dest.Add<float>(value.z);
    dest.Add<float>(value.w);
}

template<> void TUNDRACORE_API Attribute<Color>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<float>(value.r);
    dest.Add<float>(value.g);
    dest.Add<float>(value.b);
    dest.Add<float>(value.a);
}

template<> void TUNDRACORE_API Attribute<AssetReference>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.AddString(value.ref.toStdString());
}

template<> void TUNDRACORE_API Attribute<AssetReferenceList>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<u8>(value.Size());
    for(int i = 0; i < value.Size(); ++i)
        dest.AddString(value[i].ref.toStdString());
}

template<> void TUNDRACORE_API Attribute<EntityReference>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.AddString(value.ref.toStdString());
}

template<> void TUNDRACORE_API Attribute<QVariant>::ToBinary(kNet::DataSerializer& dest) const
{
    std::string str = value.toString().toStdString();
    dest.AddString(str);
}

template<> void TUNDRACORE_API Attribute<QVariantList>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<u8>(value.size());
    for(uint i = 0; i < (uint)value.size(); ++i)
        dest.AddString(value[i].toString().toStdString());
}

template<> void TUNDRACORE_API Attribute<Transform>::ToBinary(kNet::DataSerializer& dest) const
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

template<> void TUNDRACORE_API Attribute<QPoint>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<s32>(value.x());
    dest.Add<s32>(value.y());
}

// FROMBINARY TEMPLATE IMPLEMENTATIONS.

template<> void TUNDRACORE_API Attribute<QString>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    QByteArray utf8bytes;
    utf8bytes.resize(source.Read<u16>());
    if (utf8bytes.size())
        source.ReadArray<u8>((u8*)utf8bytes.data(), utf8bytes.size());
    Set(QString::fromUtf8(utf8bytes.data(), utf8bytes.size()), change);
}

template<> void TUNDRACORE_API Attribute<bool>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    Set(source.Read<u8>() ? true : false, change);
}

template<> void TUNDRACORE_API Attribute<int>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    Set(source.Read<s32>(), change);
}

template<> void TUNDRACORE_API Attribute<uint>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    Set(source.Read<u32>(), change);
}

template<> void TUNDRACORE_API Attribute<float>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    Set(source.Read<float>(), change);
}

template<> void TUNDRACORE_API Attribute<Color>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    Color value;
    value.r = source.Read<float>();
    value.g = source.Read<float>();
    value.b = source.Read<float>();
    value.a = source.Read<float>();
    Set(value, change);
}

template<> void TUNDRACORE_API Attribute<Quat>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    ///\todo Optimize here by omitting the fourth scalar.
    Quat value;
    value.x = source.Read<float>();
    value.y = source.Read<float>();
    value.z = source.Read<float>();
    value.w = source.Read<float>();
    Set(value, change);
}

template<> void TUNDRACORE_API Attribute<float2>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    float2 value;
    value.x = source.Read<float>();
    value.y = source.Read<float>();
    Set(value, change);
}

template<> void TUNDRACORE_API Attribute<float3>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    float3 value;
    value.x = source.Read<float>();
    value.y = source.Read<float>();
    value.z = source.Read<float>();
    Set(value, change);
}

template<> void TUNDRACORE_API Attribute<float4>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    float4 value;
    value.x = source.Read<float>();
    value.y = source.Read<float>();
    value.z = source.Read<float>();
    value.w = source.Read<float>();
    Set(value, change);
}

template<> void TUNDRACORE_API Attribute<AssetReference>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    AssetReference value;
    value.ref = source.ReadString().c_str();
    Set(value, change);
}

template<> void TUNDRACORE_API Attribute<AssetReferenceList>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    AssetReferenceList value;
    u8 numValues = source.Read<u8>();
    for(u32 i = 0; i < numValues; ++i)
        value.Append(AssetReference(source.ReadString().c_str()));

    Set(value, change);
}

template<> void TUNDRACORE_API Attribute<EntityReference>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    EntityReference value;
    value.ref = source.ReadString().c_str();
    Set(value, change);
}

template<> void TUNDRACORE_API Attribute<QVariant>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    std::string str = source.ReadString();
    QVariant value(QString(str.c_str()));
    Set(value, change);
}

template<> void TUNDRACORE_API Attribute<QVariantList>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
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

template<> void TUNDRACORE_API Attribute<Transform>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
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

template<> void TUNDRACORE_API Attribute<QPoint>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    QPoint value;
    value.setX(source.Read<s32>());
    value.setY(source.Read<s32>());
    Set(value, change);
}

// INTERPOLATE TEMPLATE IMPLEMENTATIONS

template<> void TUNDRACORE_API Attribute<QString>::Interpolate(IAttribute* /*start*/, IAttribute* /*end*/, float /*t*/, AttributeChange::Type /*change*/)
{
    ::LogError("Attribute<QString>::Interpolate invoked! QString attributes cannot be animated!");
}

template<> void TUNDRACORE_API Attribute<bool>::Interpolate(IAttribute* /*start*/, IAttribute* /*end*/, float /*t*/, AttributeChange::Type /*change*/)
{
    ::LogError("Attribute<bool>::Interpolate invoked! bool attributes cannot be animated!");
}

template<> void TUNDRACORE_API Attribute<AssetReference>::Interpolate(IAttribute* /*start*/, IAttribute* /*end*/, float /*t*/, AttributeChange::Type /*change*/)
{
    ::LogError("Attribute<AssetReference>::Interpolate invoked! AssetReference attributes cannot be animated!");
}

template<> void TUNDRACORE_API Attribute<AssetReferenceList>::Interpolate(IAttribute* /*start*/, IAttribute* /*end*/, float /*t*/, AttributeChange::Type /*change*/)
{
    ::LogError("Attribute<AssetReferenceList>::Interpolate invoked! AssetReferenceList attributes cannot be animated!");
}

template<> void TUNDRACORE_API Attribute<EntityReference>::Interpolate(IAttribute* /*start*/, IAttribute* /*end*/, float /*t*/, AttributeChange::Type /*change*/)
{
    ::LogError("Attribute<EntityReference>::Interpolate invoked! EntityReference attributes cannot be animated!");
}

template<> void TUNDRACORE_API Attribute<QVariant>::Interpolate(IAttribute* /*start*/, IAttribute* /*end*/, float /*t*/, AttributeChange::Type /*change*/)
{
    ::LogError("Attribute<QVariant>::Interpolate invoked! QVariant attributes cannot be animated!");
}

template<> void TUNDRACORE_API Attribute<QVariantList>::Interpolate(IAttribute* /*start*/, IAttribute* /*end*/, float /*t*/, AttributeChange::Type /*change*/)
{
    ::LogError("Attribute<QVariantList>::Interpolate invoked! QVariantList attributes cannot be animated!");
}

template<> void TUNDRACORE_API Attribute<int>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
    Attribute<int>* startInt = dynamic_cast<Attribute<int>*>(start);
    Attribute<int>* endInt = dynamic_cast<Attribute<int>*>(end);
    if (startInt && endInt)
        Set(RoundInt(Lerp((float)startInt->Get(), (float)endInt->Get(), t)), change);
}

template<> void TUNDRACORE_API Attribute<uint>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
    Attribute<uint>* startUint = dynamic_cast<Attribute<uint>*>(start);
    Attribute<uint>* endUint = dynamic_cast<Attribute<uint>*>(end);
    if (startUint && endUint)
        Set((uint)RoundInt(Lerp((float)startUint->Get(), (float)endUint->Get(), t)), change);
}

template<> void TUNDRACORE_API Attribute<float>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
    Attribute<float>* startFloat = dynamic_cast<Attribute<float>*>(start);
    Attribute<float>* endFloat = dynamic_cast<Attribute<float>*>(end);
    if (startFloat && endFloat)
        Set(Lerp(startFloat->Get(), endFloat->Get(), t), change);
}

template<> void TUNDRACORE_API Attribute<Quat>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
    Attribute<Quat>* startQuat = dynamic_cast<Attribute<Quat>*>(start);
    Attribute<Quat>* endQuat = dynamic_cast<Attribute<Quat>*>(end);
    if (startQuat && endQuat)
        Set(Slerp(startQuat->Get(), endQuat->Get(), t), change);
}

template<> void TUNDRACORE_API Attribute<float2>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
    Attribute<float2>* startVec = dynamic_cast<Attribute<float2>*>(start);
    Attribute<float2>* endVec = dynamic_cast<Attribute<float2>*>(end);
    if (startVec && endVec)
        Set(Lerp(startVec->Get(), endVec->Get(), t), change);
}

template<> void TUNDRACORE_API Attribute<float3>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
    Attribute<float3>* startVec = dynamic_cast<Attribute<float3>*>(start);
    Attribute<float3>* endVec = dynamic_cast<Attribute<float3>*>(end);
    if (startVec && endVec)
        Set(Lerp(startVec->Get(), endVec->Get(), t), change);
}

template<> void TUNDRACORE_API Attribute<float4>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
    Attribute<float4>* startVec = dynamic_cast<Attribute<float4>*>(start);
    Attribute<float4>* endVec = dynamic_cast<Attribute<float4>*>(end);
    if (startVec && endVec)
        Set(Lerp(startVec->Get(), endVec->Get(), t), change);
}

template<> void TUNDRACORE_API Attribute<Color>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
    Attribute<Color>* startColor = dynamic_cast<Attribute<Color>*>(start);
    Attribute<Color>* endColor = dynamic_cast<Attribute<Color>*>(end);
    if (startColor && endColor)
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

template<> void TUNDRACORE_API Attribute<Transform>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
    Attribute<Transform>* startTrans = dynamic_cast<Attribute<Transform>*>(start);
    Attribute<Transform>* endTrans = dynamic_cast<Attribute<Transform>*>(end);
    if (startTrans && endTrans)
    {
        const Transform& startValue = startTrans->Get();
        const Transform& endValue = endTrans->Get();
        Transform newTrans;
        newTrans.pos = Lerp(startValue.pos, endValue.pos, t);
        newTrans.SetOrientation(Slerp(startValue.Orientation(), endValue.Orientation(), t));
        newTrans.scale = Lerp(startValue.scale, endValue.scale, t);
        Set(newTrans, change);
    }
}

template<> void TUNDRACORE_API Attribute<QPoint>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
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
