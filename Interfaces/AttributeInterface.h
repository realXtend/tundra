/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   AttributeInterface.h
 *  @brief  Abstract base class and template class for entity-component attributes.
 */

#ifndef incl_Interfaces_AttributeInterface_h
#define incl_Interfaces_AttributeInterface_h

#include "CoreDefines.h"
#include "CoreStringUtils.h"
#include "AttributeChangeType.h"

namespace Foundation
{
    class ComponentInterface;

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
        AttributeInterface(ComponentInterface* owner, const char* name);

        //! Destructor.
        virtual ~AttributeInterface() {}

        //! Sets attribute null i.e. its value is fetched from a parent entity
        void SetNull(bool enable) { null_ = enable; }

        //! Resets previous/current change to none.
        void ResetChange() { change_ = AttributeChange::None; }

        //! Returns true if the attribute is dirty.
        bool IsDirty() const { return change_ != AttributeChange::None; }

        //! Returns true if the attribute is null i.e. its value is fetched from a parent entity
        bool IsNull() const { return null_; }

        //! Returns attributes owner component.
        ComponentInterface* GetOwner() const { return owner_; }

        //! Returns current/most recent change type.
        AttributeChange::Type GetChange() const { return change_; }

        //! Returns attributes name as const char *.
        const char* GetName() const { return name_; }

        //! Returns attributes name as string.
        std::string GetNameString() const { return std::string(name_); }

        //! Convert attribute to string for XML serialization
        virtual std::string ToString() const = 0;

        //! Convert attribute from string for XML deserialization
        virtual void FromString(const std::string& str, AttributeChange::Type change) = 0;

    protected:
        //! Owning component
        ComponentInterface* owner_;

        //! Name of attribute
        const char* name_;

        //! Change type (dirty flag)
        AttributeChange::Type change_;

        //! Null flag. If attribute is null, its value should be fetched from a parent entity
        bool null_;
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
        Attribute(ComponentInterface* owner, const char* name) :
            AttributeInterface(owner, name)
        {
        }

        /*! Constructor taking also value.
            \param owner Owner component.
            \param name Name.
            \param value Value.
        */
        Attribute(ComponentInterface* owner, const char* name, T value) :
            AttributeInterface(owner, name),
            value_(value)
        {
        }

        //! Returns attribute's value.
        T Get() const { return value_; }

        /*! Sets attribute's value.
            \param new value Value.
            \param change Change type.
        */
        void Set(T new_value, AttributeChange::Type change)
        {
            value_ = new_value;
            change_ = change;
        }

        //! AttributeInterface override.
        virtual std::string ToString() const;

        //! AttributeInterface override.
        virtual void FromString(const std::string& str, AttributeChange::Type change);

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
    template<> std::string Attribute<Real>::ToString() const;
    template<> std::string Attribute<Vector3df>::ToString() const;
    template<> std::string Attribute<Quaternion>::ToString() const;
    template<> std::string Attribute<Color>::ToString() const;
    template<> std::string Attribute<AssetReference>::ToString() const;

    template<> void Attribute<std::string>::FromString(const std::string& str, ChangeType change);
    template<> void Attribute<bool>::FromString(const std::string& str, ChangeType change);
    template<> void Attribute<int>::FromString(const std::string& str, ChangeType change);
    template<> void Attribute<uint>::FromString(const std::string& str, ChangeType change);
    template<> void Attribute<Real>::FromString(const std::string& str, ChangeType change);
    template<> void Attribute<Vector3df>::FromString(const std::string& str, ChangeType change);
    template<> void Attribute<Color>::FromString(const std::string& str, ChangeType change);
    template<> void Attribute<Quaternion>::FromString(const std::string& str, ChangeType change);
    template<> void Attribute<AssetReference>::FromString(const std::string& str, ChangeType change);*/
}

#endif
