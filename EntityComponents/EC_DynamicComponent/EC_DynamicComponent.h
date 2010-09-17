// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_DynamicComponent_EC_DynamicComponent_h
#define incl_EC_DynamicComponent_EC_DynamicComponent_h

#include "ComponentInterface.h"
#include "AttributeInterface.h"
#include "Declare_EC.h"

#include <QVariant>

//! EC_DynamicComponent is a component where user can add or delete attributes in a runtime.
/*! It's recommend to use attribute names while you set or get your attribute values cause 
 *  index can change while the dynamic componet's attributes are added or removed. If you want to
 *  create a new attribute you can use either AddQVariantAttribute or CreateAttribute methods.
 *  AddQVariantAttribute will create empty QVariant type of attribute and user need to set attribute value
 *  after the attribute is added to component by using a SetAttribute mehtod. All component's changes should
 *  be forwarded to all clients and thenfor they should be on sync. When component is deserialized it will
 *  compare old and a new attribute values and will get difference between those two and use that infomation
 *  to remove attributes that are not in the new list and add those that are only in new list and only update
 *  those values that are same in both lists.
 *  @todo Serialize is now done using a FreeData field that has character limit of 1000. If xml file will get larger
 *  than that client will not send a new attribute values to the server.
 */
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
            AttributeInterface *attribute = new Attribute<T>(this, name.toStdString().c_str());
            emit AttributeAdded(name);
        }
    }

    //! A factory method that constructs a new attribute given the typename. This factory is not extensible.
    AttributeInterface *CreateAttribute(const QString &typeName, const QString &name);

public slots:
    //! Will handle ComponentChanged event to ComponentInterface.
    //! @param changeType Accepts following strings {LocalOnly, Local and Network}.
    void ComponentChanged(const QString &changeType);

    //! Create new attribute that type is QVariant.
    void AddQVariantAttribute(const QString &name);

    /** Get attribute value as QVariant. If attribute type isn't QVariantAttribute then attribute value is returned as in string format.
        @param index Index to attribute list.
        @return Return attribute value as QVariant if attribute has been found, else return null QVariant.
        Use QVariant's isNull method to check if the variant value is initialized.
    */
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
    explicit EC_DynamicComponent(IModule *module);
};

#endif
