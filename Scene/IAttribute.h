/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   IAttribute.h
 *  @brief  Abstract base class, template class and metadata class for entity-component attributes.
 */

#pragma once

#include "CoreDefines.h"
#include "CoreStringUtils.h"
#include "AttributeChangeType.h"

#include <map>
#include <boost/enable_shared_from_this.hpp>

#include <QVariant>

namespace kNet
{
    class DataSerializer;
    class DataDeserializer;
}

class AttributeMetadata;
class IComponent;
class QScriptValue;

/// Abstract base class for entity-component attributes.
/** Concrete attribute classes will be subclassed out of this. */
class IAttribute : public boost::enable_shared_from_this<IAttribute>
{
public:
    /// Constructor
    /** @param owner Component which this attribute will be attached to
        @param name Name of attribute */
    IAttribute(IComponent* owner, const char* name);

    /// Destructor.
    virtual ~IAttribute() {}

    /// Returns attribute's owner component.
    IComponent* Owner() const { return owner_; }

    /// Returns attributes name as a C string.
    const char* Name() const { return name_.c_str(); }

    /// Writes attribute to string for XML serialization
    virtual std::string ToString() const = 0;

    /// Reads attribute from string for XML deserialization
    virtual void FromString(const std::string& str, AttributeChange::Type change) = 0;

    /// Returns the type name of the data stored in this attribute.
    virtual QString TypeName() const = 0;

    /// Writes attribute to binary for binary serialization
    virtual void ToBinary(kNet::DataSerializer& dest) const = 0;
    
    /// Reads attribute from binary for binary deserialization
    virtual void FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change) = 0;

    /// Returns the value as QVariant (For scripts).
    virtual QVariant ToQVariant() const = 0;

    /// Convert QVariant to attribute value.
    virtual void FromQVariant(const QVariant &variant, AttributeChange::Type change) = 0;

    /// Convert QScriptValue to attribute value (QtScript Spesific).
    /// /todo Remove when if possible.
    virtual void FromScriptValue(const QScriptValue &value, AttributeChange::Type change) = 0;

    /// Sets attribute's metadata.
    /** @param metadata Metadata. */
    void SetMetadata(AttributeMetadata *metadata) { metadata_ = metadata; }

    /// Returns attribute's metadata, or null if no metadata exists.
    AttributeMetadata *Metadata() const { return metadata_; }

    /// Returns true if this attribute has metadata set.
    bool HasMetadata() const { return metadata_ != 0; }

    /// Notifies owner component that the attribute has changed.
    /** This function is called automatically when the Attribute value is Set(). You may call this manually
        to force a change signal to be emitted for this attribute. Calling this is equivalent to calling the
        IComponent::AttributeChanged(this->GetName()) for the owner of this attribute. */
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

    /// Sets attribute null i.e. its value should be fetched from a parent entity
    //void SetNull(bool enable) { null_ = enable; }

    /// Returns true if the attribute is null i.e. its value should be fetched from a parent entity
    //bool IsNull() const { return null_; }

protected:
    IComponent* owner_; ///< Owning component
    std::string name_; ///< Name of attribute
    AttributeMetadata *metadata_; ///< Possible attribute metadata.

    /// Null flag. If attribute is null, its value should be fetched from a parent entity
    /** \todo To be thinked about more thoroughly in the future, and then possibly implemented
     */
    // bool null_;
};

typedef std::vector<IAttribute*> AttributeVector;

/// Attribute template class
template<typename T>
class Attribute : public IAttribute
{
public:
    /** Constructor.
        @param owner Owner component.
        @param name Name. */
    Attribute(IComponent* owner, const char* name) :
        IAttribute(owner, name)
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
        Changed(change);
    }
    
    /// IAttribute override
    virtual IAttribute* Clone() const
    {
        Attribute<T>* new_attr = new Attribute<T>(0, name_.c_str());
        new_attr->metadata_ = metadata_;
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
    
    /// IAttribute override.
    virtual std::string ToString() const;

    /// IAttribute override.
    virtual void FromString(const std::string& str, AttributeChange::Type change);

    /// IAttribute override.
    virtual void ToBinary(kNet::DataSerializer& dest) const;

    /// IAttribute override.
    virtual void FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change);

    /// IAttribute override
    virtual void Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change);
    
    /// Returns the type of the data stored in this attribute.
    virtual QString TypeName() const;

    /// Returns the value as QVariant (For scripts).
    virtual QVariant ToQVariant() const;

    /// Convert QVariant to attribute value.
    virtual void FromQVariant(const QVariant &variant, AttributeChange::Type change);

    /// Convert QScriptValue to attribute value (QtScript Spesific).
    /// /todo Remove this when possible.
    virtual void FromScriptValue(const QScriptValue &value, AttributeChange::Type change);

private:
    T value; ///< Attribute's value.
};

// Commented out to improve compilation times, as these are not used outside IAttribute.cpp
// currently. Can be added back if needed.

/*template<> std::string Attribute<std::string>::ToString() const;
template<> std::string Attribute<bool>::ToString() const;
template<> std::string Attribute<int>::ToString() const;
template<> std::string Attribute<uint>::ToString() const;
template<> std::string Attribute<float>::ToString() const;
template<> std::string Attribute<Vector3df>::ToString() const;
template<> std::string Attribute<Quaternion>::ToString() const;
template<> std::string Attribute<Color>::ToString() const;
template<> std::string Attribute<AssetReference>::ToString() const;
tempalte<> std::string Attribute<QVariant>::ToString() const;
template<> std::string Attribute<EntityReference>::ToString() const;

template<> void Attribute<std::string>::FromString(const std::string& str, ChangeType change);
template<> void Attribute<bool>::FromString(const std::string& str, ChangeType change);
template<> void Attribute<int>::FromString(const std::string& str, ChangeType change);
template<> void Attribute<uint>::FromString(const std::string& str, ChangeType change);
template<> void Attribute<float>::FromString(const std::string& str, ChangeType change);
template<> void Attribute<Vector3df>::FromString(const std::string& str, ChangeType change);
template<> void Attribute<Color>::FromString(const std::string& str, ChangeType change);
template<> void Attribute<Quaternion>::FromString(const std::string& str, ChangeType change);
template<> void Attribute<AssetReference>::FromString(const std::string& str, ChangeType change);
template<> void Attribute<QVariant>::FromString(const std::string& str, ChangeType change);
template<> void Attribute<EntityReference>::FromString(const std::string& str, ChangeType change);

template<> 
*/

