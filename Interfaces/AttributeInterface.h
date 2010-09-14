/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   AttributeInterface.h
 *  @brief  Abstract base class, template class and metadata class for entity-component attributes.
 */

#ifndef incl_Interfaces_AttributeInterface_h
#define incl_Interfaces_AttributeInterface_h

#include "CoreDefines.h"
#include "CoreStringUtils.h"
#include "AttributeChangeType.h"

#include <map>

namespace Foundation
{
    class ComponentInterface;
}

//! Attribute metadata contains information about the attribute: description (e.g. "color" or "direction",
/*! possible min and max values mapping of enumeration signatures and values.
 *
 *  Usage example (we're assuming that you have attribute "Attribute<float> range" as member variable):
 *
 *  EC_Example() : range(this, "example attribute", -1.f);
 *  {
 *      static AttributeMetadata metadata("this attribute is used as an example", "-128.3", "256.7")
 *      range.SetMetadata(&metadata);
 *  }
 *
 */
class AttributeMetadata
{
public:
    typedef std::map<int, std::string> EnumDescMap_t;

    //! Default constructor.
    AttributeMetadata() {}

    //! Constructor.
    /*! \param desc Description.
     *  \param min Minimum value.
     *  \param max Maximum value.
     *  \param enum_desc Mapping of enumeration's signatures (in readable form) and actual values.
     */
    AttributeMetadata(const QString &desc, const QString &min = "",
        const QString &max = "", const QString &step = "", const EnumDescMap_t &enum_desc= EnumDescMap_t()) {}

    //! Destructor.
    ~AttributeMetadata() {}

    //! Description.
    QString description;

    //! Minimum value.
    QString min;

    //! Maximum value.
    QString max;

    //! Step value.
    QString step;

    //! Mapping of enumeration's signatures (in readable form) and actual values.
    EnumDescMap_t enums;

private:
    AttributeMetadata(const AttributeMetadata &);
    void operator=(const AttributeMetadata &);
};

//! Abstract base class for entity-component attributes.
/*! Concrete attribute classes will be subclassed out of this
 */
class AttributeInterface
{
public:
    //! Constructor
    /*! \param owner Component which this attribute will be attached to
        \param name Name of attribute
     */
    AttributeInterface(Foundation::ComponentInterface* owner, const char* name);

    //! Destructor.
    virtual ~AttributeInterface() {}

    //! Sets attribute null i.e. its value should be fetched from a parent entity
    /*! \todo Not implemented yet. Implement parent entity concept and fetching of values
     */
    void SetNull(bool enable) { null_ = enable; }

    //! Resets previous/current change to none.
    void ResetChange() { change_ = AttributeChange::None; }

    //! Returns true if the attribute is dirty i.e. it has changed either locally or because of a network message.
    /*! The dirty flag should be resetted by the replication manager after it has processed the change. However,
        LOCALONLY changes most probably are not resetted by anything by default, so that state will stay until one
        resets it manually.
     */
    bool IsDirty() const { return change_ != AttributeChange::None; }

    //! Returns true if the attribute is null i.e. its value should be fetched from a parent entity
    bool IsNull() const { return null_; }

    //! Returns attributes owner component.
    Foundation::ComponentInterface* GetOwner() const { return owner_; }

    //! Returns current/most recent change type.
    AttributeChange::Type GetChange() const { return change_; }

    //! Returns attributes name as const char *.
    const char* GetName() const { return name_.c_str(); }

    //! Returns attributes name as string.
    std::string GetNameString() const { return std::string(name_); }

    //! Convert attribute to string for XML serialization
    virtual std::string ToString() const = 0;

    //! Convert attribute from string for XML deserialization
    virtual void FromString(const std::string& str, AttributeChange::Type change) = 0;

    //! Returns the type of the data stored in this attribute.
    virtual std::string TypenameToString() const = 0;

    //! Sets attribute's metadata.
    /*! \param metadata Metadata.
     */
    void SetMetadata(AttributeMetadata *metadata) { metadata_ = metadata; }

    //! Returns attribute's metadata, or null if no metadata exists.
    AttributeMetadata *GetMetadata() const { return metadata_; }

    //! Returns true if this attribute has metadata set.
    bool HasMetadata() const { return metadata_ != 0; }

protected:
    //! Notifies owner component that the attribute has changed
    void Changed(AttributeChange::Type change);

    //! Owning component
    Foundation::ComponentInterface* owner_;

    //! Name of attribute
    std::string name_;

    //! Change type (dirty flag)
    AttributeChange::Type change_;

    //! Null flag. If attribute is null, its value should be fetched from a parent entity
    bool null_;

    //! Possible attribute metadata.
    AttributeMetadata *metadata_;
};

typedef std::vector<AttributeInterface*> AttributeVector;

//! Attribute template class
template<typename T> class Attribute : public AttributeInterface
{
public:
    /*! Constructor.
        \param owner Owner component.
        \param name Name.
    */
    Attribute(Foundation::ComponentInterface* owner, const char* name) :
        AttributeInterface(owner, name)
    {
    }

    /*! Constructor taking also value.
        \param owner Owner component.
        \param name Name.
        \param value Value.
    */
    Attribute(Foundation::ComponentInterface* owner, const char* name, const T &value) :
        AttributeInterface(owner, name),
        value_(value)
    {
    }

    //! Returns attribute's value.
    const T &Get() const { return value_; }

    /*! Sets attribute's value.
        \param new value Value.
        \param change Change type.
    */
    void Set(const T &new_value, AttributeChange::Type change)
    {
        value_ = new_value;
        change_ = change;
        Changed(change);
    }

    //! AttributeInterface override.
    virtual std::string ToString() const;

    //! AttributeInterface override.
    virtual void FromString(const std::string& str, AttributeChange::Type change);

    //! Returns the type of the data stored in this attribute.
    virtual std::string TypenameToString() const;

private:
    //! Attribute value
    T value_;
};

// Commented out to improve compilation times, as these are not used outside AttributeInterface.cpp
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
tempalte<> void Attribute<QVariant>::FromString(const std::string& str, ChangeType change);

template<> 
*/

#endif
