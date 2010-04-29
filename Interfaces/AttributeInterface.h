// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_AttributeInterface_h
#define incl_Interfaces_AttributeInterface_h

#include "CoreDefines.h"
#include "CoreStringUtils.h"
#include "ComponentInterface.h"

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
        AttributeInterface(ComponentInterface* owner, const char* name) :
            owner_(owner),
            name_(name),
            change_(None),
            null_(false)
        {
            if (owner)
                owner->AddAttribute(this);
        }
        
        virtual ~AttributeInterface() {}
        
        void SetNull(bool enable) { null_ = enable; }
        void ResetChange() { change_ = None; }
        
        bool IsDirty() const { return change_ != None; }
        bool IsNull() const { return null_; }
        ComponentInterface* GetOwner() const { return owner_; }
        ChangeType GetChange() const { return change_; }
        const char* GetName() const { return name_; }
        std::string GetNameString() const { return std::string(name_); }
        
        //! Convert attribute to string for XML serialization
        virtual std::string ToString() const = 0;
        //! Convert attribute from string for XML deserialization
        virtual void FromString(const std::string& str, ChangeType change) = 0;
        
    protected:
        //! Owning component
        ComponentInterface* owner_;
        //! Name of attribute
        const char* name_;
        //! Change type (dirty flag)
        ChangeType change_;
        //! Null flag. If attribute is null, its value should be fetched from a parent entity
        bool null_;
    };
    
    typedef std::vector<AttributeInterface*> AttributeVector;
    
    //! Attribute template class
    template<typename T> class Attribute : public AttributeInterface
    {
    public:
        Attribute(ComponentInterface* owner, const char* name) :
            AttributeInterface(owner, name)
        {
        }
        
        Attribute(ComponentInterface* owner, const char* name, T value) :
            AttributeInterface(owner, name),
            value_(value)
        {
        }
        
        T Get() const { return value_; }
        
        void Set(T new_value, ChangeType change)
        {
            value_ = new_value;
            change_ = change;
            if (owner_)
                owner_->AttributeChanged(this);
        }
        
        virtual std::string ToString() const;
        virtual void FromString(const std::string& str, ChangeType change);
        
    private:
        //! Attribute value
        T value_;
    };
}

#endif
