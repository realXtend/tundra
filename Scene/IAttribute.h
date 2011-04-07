/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   IAttribute.h
 *  @brief  Abstract base class, template class and metadata class for entity-component attributes.
 */

#ifndef incl_Interfaces_AttributeInterface_h
#define incl_Interfaces_AttributeInterface_h

#include "CoreDefines.h"
#include "CoreStringUtils.h"
#include "AttributeChangeType.h"

#include <map>

#include <QVariant>

namespace kNet
{
    class DataSerializer;
    class DataDeserializer;
}

class IComponent;
class QScriptValue;

/// Attribute metadata contains information about the attribute.
/** The metadata includes information such as description (e.g. "color" or "direction",
    possible min and max values and mapping of enumeration signatures and values.

    Usage example (we're assuming that you have attribute "Attribute<float> range" as member variable):
    @code
    EC_Example() : range(this, "example attribute", -1.f);
    {
        static AttributeMetadata metadata("this attribute is used as an example", "-128.3", "256.7")
        range.SetMetadata(&metadata);
    }
    @endcode
*/
class AttributeMetadata
{
public:
    enum InterpolationMode
    {
        None,
        Interpolate
    };

    /// ButtonInfo structure will contain all information need to create a QPushButtons to ECEditor.
    struct ButtonInfo
    {
        /// Constructor.
        /** \param object_name QPushButton's object name.
            \param button_text QPushButton's text/icon.
            \param method_name IComponent's public slot method name.
         */
        ButtonInfo(const QString &object_name, const QString &button_text, const QString &method_name):
            objectName(object_name),
            text(button_text),
            method(method_name){}

        bool operator ==(const ButtonInfo &rhs) const 
        {
            return this->objectName == rhs.objectName &&
                   this->text == rhs.text &&
                   this->method == rhs.method;
        }
        bool operator !=(const ButtonInfo &rhs) const { return !(*this == rhs); }
        bool operator <(const ButtonInfo &rhs) const { return objectName < rhs.objectName; }

        QString objectName;
        QString text;
        QString method;
    };

    typedef QList<ButtonInfo> ButtonInfoList;
    typedef std::map<int, std::string> EnumDescMap_t;

    /// Default constructor.
    AttributeMetadata() : interpolation(None), designable(true) {}

    /// Constructor.
    /** \param desc Description.
        \param min Minimum value.
        \param max Maximum value.
        \param step_ Step value.
        \param enum_desc Mapping of enumeration's signatures (in readable form) and actual values.
        \param interpolation_ Interpolation mode for clients.
        \param designable_ Indicates if Attribute should be shown in designer/editor ui.
     */
    AttributeMetadata(const QString &desc, const QString &min = "", const QString &max = "", const QString &step_ = "", 
        const EnumDescMap_t &enum_desc = EnumDescMap_t(), InterpolationMode interpolation_ = None, bool designable_ = true) :
        description(desc),
        minimum(min),
        maximum(max),
        step(step_),
        enums(enum_desc),
        interpolation(interpolation_),
        designable(designable_)
    {
    }

    /// Destructor.
    ~AttributeMetadata() {}

    /// Description.
    QString description;

    /// Minimum value.
    QString minimum;

    /// Maximum value.
    QString maximum;

    /// Step value.
    QString step;

    /// List of all buttons wanted to shown on the editor (works with string attributes).
    ButtonInfoList buttons;

    /// Describes the type for individual elements of this attribute (in case there are multiple, e.g. in the case of QVariantList).
    QString elementType;

    /// Interpolation mode for clients.
    InterpolationMode interpolation;

    /// Mapping of enumeration's signatures (in readable form) and actual values.
    EnumDescMap_t enums;

    /// Indicates if Attribute should be shown in designer/editor ui.
    bool designable;

private:
    AttributeMetadata(const AttributeMetadata &);
    void operator=(const AttributeMetadata &);
};

/// Abstract base class for entity-component attributes.
/** Concrete attribute classes will be subclassed out of this
 */
class IAttribute
{
public:
    /// Constructor
    /** \param owner Component which this attribute will be attached to
        \param name Name of attribute
     */
    IAttribute(IComponent* owner, const char* name);

    /// Destructor.
    virtual ~IAttribute() {}

    /// Returns attributes owner component.
    IComponent* GetOwner() const { return owner_; }

    /// Returns attributes name as const char *.
    const char* GetName() const { return name_.c_str(); }

    /// Returns attributes name as string.
    std::string GetNameString() const { return name_; }

    /// Write attribute to string for XML serialization
    virtual std::string ToString() const = 0;

    /// Read attribute from string for XML deserialization
    virtual void FromString(const std::string& str, AttributeChange::Type change) = 0;

    /// Returns the type name of the data stored in this attribute.
    virtual std::string TypeName() const = 0;
    /// Write attribute to binary for binary serialization
    virtual void ToBinary(kNet::DataSerializer& dest) const = 0;
    
    /// Read attribute from binary for binary deserialization
    virtual void FromBinary(kNet::DataDeserializer& source, AttributeChange::Type change) = 0;

    /// Returns the value as QVariant (For scripts).
    virtual QVariant ToQVariant() const = 0;

    /// Convert QVariant to attribute value.
    virtual void FromQVariant(const QVariant &variant, AttributeChange::Type change) = 0;

    /// Convert QScriptValue to attribute value (QtScript Spesific).
    /// /todo Remove when if possible.
    virtual void FromScriptValue(const QScriptValue &value, AttributeChange::Type change) = 0;

    /// Sets attribute's metadata.
    /** \param metadata Metadata.
     */
    void SetMetadata(AttributeMetadata *metadata) { metadata_ = metadata; }

    /// Returns attribute's metadata, or null if no metadata exists.
    AttributeMetadata *GetMetadata() const { return metadata_; }

    /// Returns true if this attribute has metadata set.
    bool HasMetadata() const { return metadata_ != 0; }

    /// Notifies owner component that the attribute has changed. This function is called automatically
    /// when the Attribute value is Set(). You may call this manually to force a change signal to 
    /// be emitted for this attribute.
    /// Calling this is equivalent to calling the IComponent::AttributeChanged(this->GetName()) for the owner
    /// of this attribute.
    void Changed(AttributeChange::Type change);

    /// Creates a clone of this attribute by dynamic allocation.
    /** The caller is responsible for eventually freeing the created attribute. The clone will have the same type and value, but no owner.
        This is meant to be used by network sync managers and such, that need to do interpolation/extrapolation/dead reckoning.
     */
    virtual IAttribute* Clone() const = 0;
    
    /// Copies the value from another attribute of the same type.
    virtual void CopyValue(IAttribute* source, AttributeChange::Type change) = 0;
    
    /// Interpolates the value of this attribute based on two values, and a lerp factor between 0 and 1
    /** The attributes given must be of the same type for the result to be defined.
        Is a no-op if the attribute (for example string) does not support interpolation.
        The value will be set using the given changetype.
     */
    virtual void Interpolate(IAttribute* start, IAttribute* end, float t, AttributeChange::Type change) = 0;
    
    /// Sets attribute null i.e. its value should be fetched from a parent entity
    //void SetNull(bool enable) { null_ = enable; }

    /// Returns true if the attribute is null i.e. its value should be fetched from a parent entity
    //bool IsNull() const { return null_; }
    
protected:
    /// Owning component
    IComponent* owner_;

    /// Name of attribute
    std::string name_;

    /// Possible attribute metadata.
    AttributeMetadata *metadata_;

    /// Null flag. If attribute is null, its value should be fetched from a parent entity
    /** \todo To be thinked about more thoroughly in the future, and then possibly implemented
     */
    // bool null_;
};

typedef std::vector<IAttribute*> AttributeVector;

/// Attribute template class
template<typename T> class Attribute : public IAttribute
{
public:
    /** Constructor.
        \param owner Owner component.
        \param name Name.
    */
    Attribute(IComponent* owner, const char* name) :
        IAttribute(owner, name)
    {
    }

    /** Constructor taking also value.
        \param owner Owner component.
        \param name Name.
        \param value Value.
    */
    Attribute(IComponent* owner, const char* name, const T &value) :
        IAttribute(owner, name),
        value_(value)
    {
    }

    /// Returns attribute's value.
    const T &Get() const { return value_; }

    /** Sets attribute's value.
        \param new value Value.
        \param change Change type.
    */
    void Set(const T &new_value, AttributeChange::Type change)
    {
        value_ = new_value;
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
    virtual std::string TypeName() const;

    /// Returns the value as QVariant (For scripts).
    virtual QVariant ToQVariant() const;

    /// Convert QVariant to attribute value.
    virtual void FromQVariant(const QVariant &variant, AttributeChange::Type change);

    /// Convert QScriptValue to attribute value (QtScript Spesific).
    /// /todo Remove this when possible.
    virtual void FromScriptValue(const QScriptValue &value, AttributeChange::Type change);

private:
    /// Attribute value
    T value_;
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

template<> 
*/

#endif
