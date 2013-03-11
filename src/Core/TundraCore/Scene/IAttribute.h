/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   IAttribute.h
    @brief  Abstract base class and template class implementations for entity-component attributes. */

#pragma once

#include "TundraCoreApi.h"
#include "CoreDefines.h"
#include "CoreStringUtils.h"
#include "AttributeChangeType.h"
#include "SceneFwd.h"

namespace kNet
{
    class DataSerializer;
    class DataDeserializer;
}

class QScriptValue;
class QVariant;

/// Abstract base class for entity-component attributes.
/** Concrete attribute classes will be subclassed out of this. */
class TUNDRACORE_API IAttribute : public enable_shared_from_this<IAttribute>
{
public:
    /// Constructor
    /** @param owner Component which this attribute will be attached to.
        @param name Name of the attribute. */
    IAttribute(IComponent* owner, const char* name);

    virtual ~IAttribute() {}

    /// Returns attribute's owner component.
    IComponent* Owner() const { return owner; }

    /// Returns name of the attribute.
    const QString &Name() const { return name; }

    /// Writes attribute to string for XML serialization
    virtual std::string ToString() const = 0;

    /// Reads attribute from string for XML deserialization
    virtual void FromString(const std::string& str, AttributeChange::Type change) = 0;

    /// Returns the type name of the data stored in this attribute.
    virtual const QString &TypeName() const = 0;
    
    /// Returns the type ID of this attribute.
    virtual u32 TypeId() const = 0;

    /// Writes attribute to binary for binary serialization
    virtual void ToBinary(kNet::DataSerializer& dest) const = 0;

    /// Reads attribute from binary for binary deserialization
    virtual void FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change) = 0;

    /// Returns the value as QVariant (For scripts).
    virtual QVariant ToQVariant() const = 0;

    /// Convert QVariant to attribute value.
    virtual void FromQVariant(const QVariant &variant, AttributeChange::Type change) = 0;

    /// Convert QScriptValue to attribute value (QtScript Spesific).
    /// @deprecated FromQVariant should be used instead.
    /// @todo Remove when if possible.
    virtual void FromScriptValue(const QScriptValue &value, AttributeChange::Type change) = 0;

    /// Sets attribute's metadata.
    /** @param meta Metadata. */
    void SetMetadata(AttributeMetadata *meta) { metadata = meta; }

    /// Returns attribute's metadata, or null if no metadata exists.
    AttributeMetadata *Metadata() const { return metadata; }

    /// Returns whether attribute has been dynamically allocated. By default false
    bool IsDynamic() const { return dynamic; }
    
    /// Returns attribute's index in the parent component
    u8 Index() const { return index; }
    
    /// Notifies owner component that the attribute has changed.
    /** This function is called automatically when the Attribute value is Set(). You may call this manually
        to force a change signal to be emitted for this attribute. Calling this is equivalent to calling the
        IComponent::AttributeChanged(this->Name()) for the owner of this attribute. */
    void Changed(AttributeChange::Type change);

    /// Creates a clone of this attribute by dynamic allocation.
    /** The caller is responsible for eventually freeing the created attribute. The clone will have the same type and value, but no owner.
        This is meant to be used by network sync managers and such, that need to do interpolation/extrapolation/dead reckoning. */
    virtual IAttribute* Clone() const = 0;

    /// Copies the value from another attribute of the same type.
    virtual void CopyValue(IAttribute* source, AttributeChange::Type change) = 0;

    /// Interpolates the value of this attribute based on two values, and a lerp factor between 0 and 1
    /** The attributes given must be of the same type for the result to be defined.
        Is a no-op if the attribute (for example string) does not support interpolation.
        The value will be set using the given changetype. */
    virtual void Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change) = 0;

    /// Returns whether the value of this attribute is dirty and pending an update by the component that owns this attribute.
    /** This flag is used to optimize the attribute update events from an asymptotically Theta(n^2) operation to a Theta(n) when loading a scene or
            creating a new component with all new attributes. This also enables the components themselves to avoid having to cache "shadow" values of its attributes 
            in the component, in order to optimize its loading.
        @return If true, an external source (scene load event, UI attribute edit event, a script, received sync message from sync manager, etc.)
            has modified the value of this attribute, but the *implementation* of the component this attribute is part of has not
            yet reacted to this change.
        @note This flag is NOT to be used except by the code that is implementing a new component. Do not read this flag from client code
            that is trying to detect if an attribute has changed, use the attribute changed signal in the component instead. */
    bool ValueChanged() const { return valueChanged; }

    /// Acknowledges that the component owning this attribute has updated the component state to reflect the current value of this attribute.
    void ClearChangedFlag() { valueChanged = false; }

protected:
    friend class SceneAPI;
    friend class IComponent;
    
    IComponent* owner; ///< Owning component.
    QString name; ///< Name of attribute.
    AttributeMetadata *metadata; ///< Possible attribute metadata.
    bool dynamic; ///< Dynamic attributes must be deleted at component destruction
    u8 index; ///< Attribute index in the parent component's attribute list

    /// If true, the value of this attribute has changed, but the implementing code has not yet reacted to it.
    /// @see ValueChanged().
    bool valueChanged;
};

typedef std::vector<IAttribute*> AttributeVector;

/// Attribute template class
template<typename T>
class Attribute : public IAttribute
{
public:
    /** Constructor.
        value is initialiazed to DefaultValue.
        @param owner Owner component.
        @param name Name. */
    Attribute(IComponent* owner, const char* name) :
        IAttribute(owner, name),
        value(DefaultValue())
    {
    }

    /** Constructor taking also value.
        @param owner Owner component.
        @param name Name.
        @param val Value. */
    Attribute(IComponent* owner, const char* name, const T &val) :
        IAttribute(owner, name),
        value(val)
    {
    }

    /// Returns attribute's value.
    const T &Get() const { return value; }

    /** Sets attribute's value.
        @param value New value.
        @param change Change type. */
    void Set(const T &value, AttributeChange::Type change)
    {
        this->value = value;
        valueChanged = true; // Signal to IComponent owning this attribute that the value of this attribute has changed.
        Changed(change);
    }
    
    /// IAttribute override
    virtual IAttribute* Clone() const
    {
        Attribute<T>* new_attr = new Attribute<T>(0, name.toStdString().c_str());
        new_attr->metadata = metadata;
        // The new attribute has no owner, so the Changed function will have no effect, and therefore the changetype does not actually matter
        new_attr->Set(Get(), AttributeChange::Disconnected);
        return static_cast<IAttribute*>(new_attr);
    }
    
    /// IAttribute override
    virtual void CopyValue(IAttribute* source, AttributeChange::Type change)
    {
        Attribute<T>* source_attr = dynamic_cast<Attribute<T>*>(source);
        if (source_attr)
            Set(source_attr->Get(), change);
    }

    virtual std::string ToString() const; ///< IAttribute override
    virtual void FromString(const std::string& str, AttributeChange::Type change); ///< IAttribute override
    virtual void ToBinary(kNet::DataSerializer& dest) const; ///< IAttribute override
    virtual void FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change); ///< IAttribute override
    virtual void Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change); ///< IAttribute override
    virtual const QString &TypeName() const; ///< IAttribute override
    virtual u32 TypeId() const; ///< IAttribute override
    virtual QVariant ToQVariant() const; ///< IAttribute override
    virtual void FromQVariant(const QVariant &variant, AttributeChange::Type change); ///< IAttribute override
    virtual void FromScriptValue(const QScriptValue &value, AttributeChange::Type change); ///< IAttribute override

    /// Returns pre-defined default value for the attribute.
    /** Usually zero for primitive data types and for classes/structs that are collections of primitive data types (e.g. float3::zero), or the default consturctor. */
    T DefaultValue() const;

private:
    T value; ///< The value of this Attribute.
};

static const u32 cAttributeNone = 0;
static const u32 cAttributeString = 1;
static const u32 cAttributeInt = 2;
static const u32 cAttributeReal = 3;
static const u32 cAttributeColor = 4;
static const u32 cAttributeFloat2 = 5;
static const u32 cAttributeFloat3 = 6;
static const u32 cAttributeFloat4 = 7;
static const u32 cAttributeBool = 8;
static const u32 cAttributeUInt = 9;
static const u32 cAttributeQuat = 10;
static const u32 cAttributeAssetReference = 11;
static const u32 cAttributeAssetReferenceList = 12;
static const u32 cAttributeEntityReference = 13;
static const u32 cAttributeQVariant = 14;
static const u32 cAttributeQVariantList = 15;
static const u32 cAttributeTransform = 16;
static const u32 cAttributeQPoint = 17;
static const u32 cNumAttributeTypes = 18;

static const QString cAttributeNoneTypeName = "";
static const QString cAttributeStringTypeName = "string";
static const QString cAttributeIntTypeName = "int";
static const QString cAttributeRealTypeName = "real";
static const QString cAttributeColorTypeName = "color"; /**< @todo "Color" */
static const QString cAttributeFloat2TypeName = "float2";
static const QString cAttributeFloat3TypeName = "float3";
static const QString cAttributeFloat4TypeName = "float4";
static const QString cAttributeBoolTypeName = "bool";
static const QString cAttributeUIntTypeName = "uint";
static const QString cAttributeQuatTypeName = "quat"; /**< @todo "Quat" */
static const QString cAttributeAssetReferenceTypeName = "assetreference"; /**< @todo "AssetReference" */
static const QString cAttributeAssetReferenceListTypeName = "assetreferencelist"; /**< @todo "AssetReferenceList" */
static const QString cAttributeEntityReferenceTypeName = "entityreference"; /**< @todo "EntityReference" */
static const QString cAttributeQVariantTypeName = "qvariant"; /**< @todo "QVariant" */
static const QString cAttributeQVariantListTypeName = "qvariantlist"; /**< @todo "QVariantList" */
static const QString cAttributeTransformTypeName = "transform"; /**< @todo "Transform" */
static const QString cAttributeQPointTypeName = "qpoint"; /**< @todo "QPoint" */
