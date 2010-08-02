// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_DynamicComponent_EC_DynamicComponent_h
#define incl_EC_DynamicComponent_EC_DynamicComponent_h

#include "ComponentInterface.h"
#include "AttributeInterface.h"
#include "Declare_EC.h"

class QDomDocument;
class QDomElement;

#include <QVariant>
#include <map>
#include <string>

/*class AttributeProduct
{
public:
    AttributeProduct(){}
    virtual ~AttributeProduct(){}
    std::string GetName() const {return name_;}
    std::string GetTypeName() const {return typeName_;}
    Foundation::ComponentInterface *GetOwner() const {return owner_;}
protected:
    virtual AttributeProduct *CreateAttribute();
private:
    std::string typeName_;
    std::string name_;
    Foundation::ComponentInterface *owner_;
};*/

class EC_DynamicComponent : public Foundation::ComponentInterface
{
    DECLARE_EC(EC_DynamicComponent);
    Q_OBJECT

signals:
    void AttributeAdded(const QString &name);
    void AttributeRemoved(const QString &name);

public:
    struct DeserializeData
    {
        std::string name_;
        std::string type_;
        std::string value_;
        DeserializeData(const std::string name = std::string(""),
                        const std::string type = std::string(""),
                        const std::string value = std::string("")):
        name_(name),
        type_(type),
        value_(value)
        {
        }

        //! Checks if any of data structure's values are null.
        bool isNull() const
        {
            return name_ == "" || type_ == "" || value_ == "";
        }
    };

    //! Destructor.
    ~EC_DynamicComponent();

    virtual bool IsSerializable() const { return true; }

    void SerializeTo(QDomDocument& doc, QDomElement& base_element) const;

    void DeserializeFrom(QDomElement& element, AttributeChange::Type change);

    /// Constructs a new attribute of type Attribute<T>.
    template<typename T>
    void AddAttribute(const QString &name)
    {
        //Check if attribute has already created.
        if(!ContainAttribute(name))
        {
            Foundation::AttributeInterface *attribute = new Foundation::Attribute<T>(this, name.toStdString().c_str());
            emit AttributeAdded(name);
        }
    }

    //! A factory method that constructs a new attribute given the typename. This factory is not extensible.
    Foundation::AttributeInterface *CreateAttribute(const QString &typeName, const QString &name);

public slots:
    //! Will handle ComponentChanged event to ComponentInterface.
    //! @param changeType Accepts following strings {LocalOnly, Local and Network}.
    void ComponentChanged(const QString &changeType);

    //! Create new attribute that type is QVariant.
    void AddQVariantAttribute(const QString &name);

    //! Get attribute value as QVariant. If attribute type isn't QVariantAttribute then attribute value is returned as in string format.
    //! @param index Index to attribute list.
    //! @return Return attribute value as QVariant if attribute has been found, else return null QVariant (Use QVariant's isNull method to check if the variant value is initialized).
    QVariant GetAttribute(int index) const;
    QVariant GetAttribute(const QString &name) const;

    //! Insert new attribute value to attribute.
    void SetAttribute(int index, const QVariant &value, AttributeChange::Type change = AttributeChange::Local);
    void SetAttribute(const QString &name, const QVariant &value, AttributeChange::Type change = AttributeChange::Local);

    int GetNumAttributes() const {return attributes_.size();}
    QString GetAttributeName(int index) const;

    //! Check if a given component is holding exactly same attributes as this component.
    //! @return Return true if component is holding same attributes as this component else return false.
    bool ContainSameAttributes(const EC_DynamicComponent &comp) const;

    /// \todo Remove.
    /// quick hack, should use GetParentEntity in ComponentInterface and add qt things to Entity (and eventualy get rid of PyEntity)
    uint GetParentEntityId() const;
    
    /// Remove attribute from the component.
    void RemoveAttribute(const QString &name);

    /// Check if component is hodling attribute by that name.
    /// @param name Name of attribute that we are looking for.
    bool ContainAttribute(const QString &name) const;
private:
    explicit EC_DynamicComponent(Foundation::ModuleInterface *module);
};

//! Function that is used by std::sort algorithm to sort attributes by their name.
bool CmpAttributeByName(const Foundation::AttributeInterface *a, const Foundation::AttributeInterface *b);
//! Function that is used by std::Sort algorithm to sort DeserializeData by their name.
bool CmpAttributeDataByName(const EC_DynamicComponent::DeserializeData &a, const EC_DynamicComponent::DeserializeData &b);

#endif
