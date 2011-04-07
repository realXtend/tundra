/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   IAttribute.cpp
 *  @brief  Abstract base class and template class for entity-component attributes.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MemoryLeakCheck.h"

#include "IAttribute.h"
#include "IComponent.h"
#include "Core.h"
#include "CoreStdIncludes.h"
#include "Transform.h"
#include "AssetReference.h"

#include <QVector3D>
#include <QVariant>
#include <QStringList>
#include <QScriptEngine>
#include <QSize>
#include <QPoint>

#include <kNet.h>

using namespace kNet;

// Implementation code for some common attributes

IAttribute::IAttribute(IComponent* owner, const char* name) :
    owner_(owner),
    name_(name),
    metadata_(0)
{
    if (owner)
        owner->AddAttribute(this);
}

void IAttribute::Changed(AttributeChange::Type change)
{
    if (owner_)
        owner_->EmitAttributeChanged(this, change);
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

template<> std::string Attribute<AssetReference>::ToString() const
{
    return Get().ref.toStdString();
}

template<> std::string Attribute<AssetReferenceList>::ToString() const
{
    std::string stringValue = "";
    AssetReferenceList values = Get();
    for(int i = 0; i < values.Size(); ++i)
    {
        stringValue += values[i].ref.toStdString();
        if (i < values.Size() - 1)
            stringValue += ";";
    }

    return stringValue;
}

template<> std::string Attribute<QVariant>::ToString() const
{
    QVariant value = Get();
    
    return value.toString().toStdString();
}

template<> std::string Attribute<QVariantList>::ToString() const
{
    QVariantList values = Get();

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
    Transform transform = Get();
    Vector3df editValues[3];
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

template<> std::string Attribute<QVector3D>::ToString() const
{
    QVector3D value = Get();
    
    return ::ToString<float>(value.x()) + " " +
        ::ToString<float>(value.y()) + " " +
        ::ToString<float>(value.z());
}

template<> std::string Attribute<QSize>::ToString() const
{
    QSize value = Get();

    return ::ToString<int>(value.width()) + " " +
        ::ToString<int>(value.height());
}

template<> std::string Attribute<QSizeF>::ToString() const
{
    QSizeF value = Get();

    return ::ToString<double>(value.width()) + " " +
        ::ToString<double>(value.height());
}

template<> std::string Attribute<QPoint>::ToString() const
{
    QPoint value = Get();

    return ::ToString<int>(value.x()) + " " +
        ::ToString<int>(value.y());
}

template<> std::string Attribute<QPointF>::ToString() const
{
    QPointF value = Get();

    return ::ToString<double>(value.x()) + " " +
        ::ToString<double>(value.y());
}

// TYPENAMETOSTRING TEMPLATE IMPLEMENTATIONS.

template<> std::string Attribute<int>::TypeName() const
{
    return "int";
}

template<> std::string Attribute<uint>::TypeName() const
{
    return "uint";
}

template<> std::string Attribute<float>::TypeName() const
{
    return "real";
}

template<> std::string Attribute<QString>::TypeName() const
{
    return "string";
}

template<> std::string Attribute<bool>::TypeName() const
{
    return "bool";
}

template<> std::string Attribute<Vector3df>::TypeName() const
{
    return "vector3df";
}

template<> std::string Attribute<Quaternion>::TypeName() const
{
    return "quaternion";
}

template<> std::string Attribute<Color>::TypeName() const
{
    return "color";
}

template<> std::string Attribute<AssetReference>::TypeName() const
{
    return "assetreference";
}

template<> std::string Attribute<AssetReferenceList>::TypeName() const
{
    return "assetreferencelist";
}

template<> std::string Attribute<QVariant>::TypeName() const
{
    return "qvariant";
}

template<> std::string Attribute<QVariantList >::TypeName() const
{
    return "qvariantlist";
}

template<> std::string Attribute<Transform>::TypeName() const
{
    return "transform";
}

template<> std::string Attribute<QVector3D>::TypeName() const
{
    return "qvector3d";
}

template<> std::string Attribute<QSize>::TypeName() const
{
    return "qsize";
}

template<> std::string Attribute<QSizeF>::TypeName() const
{
    return "qsizef";
}

template<> std::string Attribute<QPoint>::TypeName() const
{
    return "qpoint";
}

template<> std::string Attribute<QPointF>::TypeName() const
{
    return "qpointf";
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
        catch(...) {}
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
        catch(...) {}
    }
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
    QString value = QString::fromStdString(str);
    QStringList matrixElements = value.split(',');
    Transform result;
    if(matrixElements.size() == 9) //Ensure that we a have right amount of elements.
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

template<> void Attribute<QVector3D>::FromString(const std::string& str, AttributeChange::Type change)
{
    StringVector components = SplitString(str, ' ');
    if (components.size() == 3)
    {
        try
        {
            QVector3D value;
            value.setX(ParseString<float>(components[0]));
            value.setY(ParseString<float>(components[1]));
            value.setZ(ParseString<float>(components[2]));
            Set(value, change);
        }
        catch(...) {}
    }
}

template<> void Attribute<QSize>::FromString(const std::string& str, AttributeChange::Type change)
{
    StringVector components = SplitString(str, ' ');
    if (components.size() == 2)
    {
        try
        {
            QSize value;
            value.setWidth(ParseString<int>(components[0]));
            value.setHeight(ParseString<int>(components[1]));
            Set(value, change);
        }
        catch(...) {}
    }
}

template<> void Attribute<QSizeF>::FromString(const std::string& str, AttributeChange::Type change)
{
    StringVector components = SplitString(str, ' ');
    if (components.size() == 2)
    {
        try
        {
            QSizeF value;
            value.setWidth(ParseString<float>(components[0]));
            value.setHeight(ParseString<float>(components[1]));
            Set(value, change);
        }
        catch(...) {}
    }
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

template<> void Attribute<QPointF>::FromString(const std::string& str, AttributeChange::Type change)
{
    StringVector components = SplitString(str, ' ');
    if (components.size() == 2)
    {
        try
        {
            QPointF value;
            value.setX(ParseString<float>(components[0]));
            value.setY(ParseString<float>(components[1]));
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

template<> void Attribute<Vector3df>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(qvariant_cast<Vector3df>(variant), change);
}

template<> void Attribute<Color>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(qvariant_cast<Color>(variant), change);
}

template<> void Attribute<Quaternion>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(qvariant_cast<Quaternion>(variant), change);
}

template<> void Attribute<AssetReference>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(qvariant_cast<AssetReference>(variant), change);
}

template<> void Attribute<AssetReferenceList>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(qvariant_cast<AssetReferenceList>(variant), change);
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

template <> void Attribute<QVector3D>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(qvariant_cast<QVector3D>(variant), change);
}

template <> void Attribute<QSize>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(qvariant_cast<QSize>(variant), change);
}

template <> void Attribute<QSizeF>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(qvariant_cast<QSizeF>(variant), change);
}

template <> void Attribute<QPoint>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(qvariant_cast<QPoint>(variant), change);
}

template <> void Attribute<QPointF>::FromQVariant(const QVariant &variant, AttributeChange::Type change)
{
    Set(qvariant_cast<QPointF>(variant), change);
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

template<> QVariant Attribute<Vector3df>::ToQVariant() const
{
    return QVariant::fromValue<Vector3df>(Get());
}
    
template<> QVariant Attribute<Quaternion>::ToQVariant() const
{
    return QVariant::fromValue<Quaternion>(Get());
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

template<> QVariant Attribute<QVector3D>::ToQVariant() const
{
    return QVariant::fromValue<QVector3D>(Get());
}

template<> QVariant Attribute<QSize>::ToQVariant() const
{
    return QVariant::fromValue<QSize>(Get());
}

template<> QVariant Attribute<QSizeF>::ToQVariant() const
{
    return QVariant::fromValue<QSizeF>(Get());
}

template<> QVariant Attribute<QPoint>::ToQVariant() const
{
    return QVariant::fromValue<QPoint>(Get());
}

template<> QVariant Attribute<QPointF>::ToQVariant() const
{
    return QVariant::fromValue<QPointF>(Get());
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

template<> void Attribute<Vector3df>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(qScriptValueToValue<Vector3df>(value), change);
}
    
template<> void Attribute<Quaternion>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(qScriptValueToValue<Quaternion>(value), change);
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

template<> void Attribute<QVector3D>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(qScriptValueToValue<QVector3D>(value), change);
}

template<> void Attribute<QSize>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(qScriptValueToValue<QSize>(value), change);
}

template<> void Attribute<QSizeF>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(qScriptValueToValue<QSizeF>(value), change);
}

template<> void Attribute<QPoint>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(qScriptValueToValue<QPoint>(value), change);
}

template<> void Attribute<QPointF>::FromScriptValue(const QScriptValue &value, AttributeChange::Type change)
{
    Set(qScriptValueToValue<QPointF>(value), change);
}

// TOBINARY TEMPLATE IMPLEMENTATIONS.

template<> void Attribute<QString>::ToBinary(kNet::DataSerializer& dest) const
{
    QByteArray utf8bytes = value_.toUtf8();
    dest.Add<u16>(utf8bytes.size());
    if (utf8bytes.size())
        dest.AddArray<u8>((const u8*)utf8bytes.data(), utf8bytes.size());
}

template<> void Attribute<bool>::ToBinary(kNet::DataSerializer& dest) const
{
    if (value_)
        dest.Add<u8>(1);
    else
        dest.Add<u8>(0);
}

template<> void Attribute<int>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<s32>(value_);
}

template<> void Attribute<uint>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<u32>(value_);
}

template<> void Attribute<float>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<float>(value_);
}

template<> void Attribute<Vector3df>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<float>(value_.x);
    dest.Add<float>(value_.y);
    dest.Add<float>(value_.z);
}

template<> void Attribute<QVector3D>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<float>(value_.x());
    dest.Add<float>(value_.y());
    dest.Add<float>(value_.z());
}

template<> void Attribute<Quaternion>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<float>(value_.x);
    dest.Add<float>(value_.y);
    dest.Add<float>(value_.z);
    dest.Add<float>(value_.w);
}

template<> void Attribute<Color>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<float>(value_.r);
    dest.Add<float>(value_.g);
    dest.Add<float>(value_.b);
    dest.Add<float>(value_.a);
}

template<> void Attribute<AssetReference>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.AddString(value_.ref.toStdString());
}

template<> void Attribute<AssetReferenceList>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<u8>(value_.Size());
    for(int i = 0; i < value_.Size(); ++i)
        dest.AddString(value_[i].ref.toStdString());
}

template<> void Attribute<QVariant>::ToBinary(kNet::DataSerializer& dest) const
{
    std::string str = value_.toString().toStdString();
    dest.AddString(str);
}

template<> void Attribute<QVariantList>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<u8>(value_.size());
    for(uint i = 0; i < value_.size(); ++i)
        dest.AddString(value_[i].toString().toStdString());
}

template<> void Attribute<Transform>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<float>(value_.position.x);
    dest.Add<float>(value_.position.y);
    dest.Add<float>(value_.position.z);
    dest.Add<float>(value_.rotation.x);
    dest.Add<float>(value_.rotation.y);
    dest.Add<float>(value_.rotation.z);
    dest.Add<float>(value_.scale.x);
    dest.Add<float>(value_.scale.y);
    dest.Add<float>(value_.scale.z);
}

template<> void Attribute<QSize>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<s32>(value_.width());
    dest.Add<s32>(value_.height());
}

template<> void Attribute<QSizeF>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<double>(value_.width());
    dest.Add<double>(value_.height());
}

template<> void Attribute<QPoint>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<s32>(value_.x());
    dest.Add<s32>(value_.y());
}

template<> void Attribute<QPointF>::ToBinary(kNet::DataSerializer& dest) const
{
    dest.Add<double>(value_.x());
    dest.Add<double>(value_.y());
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

template<> void Attribute<Vector3df>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    Vector3df value;
    value.x = source.Read<float>();
    value.y = source.Read<float>();
    value.z = source.Read<float>();
    Set(value, change);
}

template<> void Attribute<QVector3D>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    QVector3D value;
    value.setX(source.Read<float>());
    value.setY(source.Read<float>());
    value.setZ(source.Read<float>());
    Set(value, change);
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

template<> void Attribute<Quaternion>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    Quaternion value;
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
    value.position.x = source.Read<float>();
    value.position.y = source.Read<float>();
    value.position.z = source.Read<float>();
    value.rotation.x = source.Read<float>();
    value.rotation.y = source.Read<float>();
    value.rotation.z = source.Read<float>();
    value.scale.x = source.Read<float>();
    value.scale.y = source.Read<float>();
    value.scale.z = source.Read<float>();
    Set(value, change);
}

template<> void Attribute<QSize>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    QSize value;
    value.setWidth(source.Read<s32>());
    value.setHeight(source.Read<s32>());
    Set(value, change);
}

template<> void Attribute<QSizeF>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    QSizeF value;
    value.setWidth(source.Read<double>());
    value.setHeight(source.Read<double>());
    Set(value, change);
}

template<> void Attribute<QPoint>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    QPoint value;
    value.setX(source.Read<s32>());
    value.setY(source.Read<s32>());
    Set(value, change);
}

template<> void Attribute<QPointF>::FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    QPointF value;
    value.setX(source.Read<double>());
    value.setY(source.Read<double>());
    Set(value, change);
}

// INTERPOLATE TEMPLATE IMPLEMENTATIONS

template<> void Attribute<QString>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
}

template<> void Attribute<bool>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
}

template<> void Attribute<int>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
    Attribute<int>* startInt = dynamic_cast<Attribute<int>*>(start);
    Attribute<int>* endInt = dynamic_cast<Attribute<int>*>(end);
    if ((startInt) && (endInt))
    {
        Set(lerp(startInt->Get(), endInt->Get(), t), change);
    }
}

template<> void Attribute<uint>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
    Attribute<uint>* startUint = dynamic_cast<Attribute<uint>*>(start);
    Attribute<uint>* endUint = dynamic_cast<Attribute<uint>*>(end);
    if ((startUint) && (endUint))
    {
        Set(lerp(startUint->Get(), endUint->Get(), t), change);
    }
}

template<> void Attribute<float>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
    Attribute<float>* startFloat = dynamic_cast<Attribute<float>*>(start);
    Attribute<float>* endFloat = dynamic_cast<Attribute<float>*>(end);
    if ((startFloat) && (endFloat))
    {
        Set(lerp(startFloat->Get(), endFloat->Get(), t), change);
    }
}

template<> void Attribute<Vector3df>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
    Attribute<Vector3df>* startVec = dynamic_cast<Attribute<Vector3df>*>(start);
    Attribute<Vector3df>* endVec = dynamic_cast<Attribute<Vector3df>*>(end);
    if ((startVec) && (endVec))
    {
        Set(lerp(startVec->Get(), endVec->Get(), t), change);
    }
}

template<> void Attribute<QVector3D>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
    // Not implemented. Favor the above instead.
}

template<> void Attribute<Quaternion>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
    Attribute<Quaternion>* startQuat = dynamic_cast<Attribute<Quaternion>*>(start);
    Attribute<Quaternion>* endQuat = dynamic_cast<Attribute<Quaternion>*>(end);
    if ((startQuat) && (endQuat))
    {
        Quaternion newQuat;
        newQuat.slerp(startQuat->Get(), endQuat->Get(), t);
        Set(newQuat, change);
    }
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
        newColor.r = lerp(startValue.r, endValue.r, t);
        newColor.g = lerp(startValue.g, endValue.g, t);
        newColor.b = lerp(startValue.b, endValue.b, t);
        newColor.a = lerp(startValue.a, endValue.a, t);
        Set(newColor, change);
    }
    std::cout << "Interpolated color" << std::endl;
}

template<> void Attribute<AssetReference>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
}

template<> void Attribute<AssetReferenceList>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
}

template<> void Attribute<QVariant>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
}

template<> void Attribute<QVariantList>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
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
        newTrans.position = lerp(startValue.position, endValue.position, t);
        
        // Rotation
        Quaternion startRot(DEGTORAD * startValue.rotation.x, DEGTORAD * startValue.rotation.y, DEGTORAD * startValue.rotation.z);
        Quaternion endRot(DEGTORAD * endValue.rotation.x, DEGTORAD * endValue.rotation.y, DEGTORAD * endValue.rotation.z);
        Quaternion newRot;
        newRot.slerp(startRot, endRot, t);
        Vector3df newRotEuler;
        newRot.toEuler(newRotEuler);
        newTrans.SetRot(newRotEuler.x * RADTODEG, newRotEuler.y * RADTODEG, newRotEuler.z * RADTODEG);
        
        // Scale
        newTrans.scale = lerp(startValue.scale, endValue.scale, t);
        
        Set(newTrans, change);
    }
}

template<> void Attribute<QSize>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
}

template<> void Attribute<QSizeF>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
}

template<> void Attribute<QPoint>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
}

template<> void Attribute<QPointF>::Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change)
{
}
