// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_DynamicComponent_EC_DynamicComponent_h
#define incl_EC_DynamicComponent_EC_DynamicComponent_h

#include "IComponent.h"
#include "IAttribute.h"
#include "Declare_EC.h"

namespace kNet
{
    class DataSerializer;
    class DataDeserializer;
}

#include <QVariant>

/// Component for which user can add and delete attributes at runtime.
/**
<table class="header">
<tr>
<td>
<h2>DynamicComponent</h2>
Component for which user can add and delete attributes at runtime.
It's recommend to use attribute names when you set or get your attribute values because
index can change while the dynamic componet's attributes are added or removed.

If you want to create a new attribute you can use either AddQVariantAttribute or CreateAttribute methods.
AddQVariantAttribute will create empty QVariant type of attribute and user need to set attribute value
after the attribute is added to component by using a SetAttribute method.

All component's changes should be forwarded to all clients and therefore they should be on sync.
When component is deserialized it will compare old and a new attribute values and will get difference
between those two and use that infomation to remove attributes that are not in the new list and add those
that are only in new list and only update those values that are same in both lists.

@todo Serialize is now done using a FreeData field that has character limit of 1000.
If xml file will get larger than that client will not send a new attribute values to the server.


Registered by PythonScript::PythonScriptModule.

<b>No Attributes.</b>

<b>Exposes the following scriptable functions:</b>
<ul>
<li>"AddQVariantAttribute": Create new attribute that type is QVariant.
<li>"GetAttribute":Get attribute value as QVariant. If attribute type isn't QVariantAttribute then attribute value is returned as in string format.
        @param index Index to attribute list.
        @return Return attribute value as QVariant if attribute has been found, else return null QVariant.
        Use QVariant's isNull method to check if the variant value is initialized.
<li>"SetAttribute": Insert new attribute value to attribute.
<li>"GetNumAttributes": 
<li>"GetAttributeName":
<li>"ContainSameAttribute": Check if a given component is holding exactly same attributes as this component.
    @return Return true if component is holding same attributes as this component else return false.
<li>"RemoveAttribute": Remove attribute from the component.
<li>"ContainAttribute": Check if component is hodling attribute by that name.
    @param name Name of attribute that we are looking for.
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>...
</ul>
</td>
</tr>

Does not emit any actions.

<b>Doesn't depend on any components</b>.

</table>
*/

struct DeserializeData;


class QScriptValue;

class EC_DynamicComponent : public IComponent
{
    DECLARE_EC(EC_DynamicComponent);
    Q_OBJECT

public:
    /// Destructor.
    ~EC_DynamicComponent();

    /// IComponent override. This component is serializable.
    virtual bool IsSerializable() const { return true; }

    /// IComponent override.
    void SerializeTo(QDomDocument& doc, QDomElement& base_element) const;

    /// IComponent override.
    void DeserializeFrom(QDomElement& element, AttributeChange::Type change);

    void DeserializeCommon(std::vector<DeserializeData>& deserializedAttributes, AttributeChange::Type change);

    /// Constructs a new attribute of type Attribute<T>.
    template<typename T>
    void AddAttribute(const QString &name, AttributeChange::Type change = AttributeChange::Default)
    {
        // Check if attribute has already created.
        if (!ContainsAttribute(name))
        {
            IAttribute *attribute = new Attribute<T>(this, name.toStdString().c_str());
            AttributeChanged(attribute, change);
            emit AttributeAdded(name);
        }
    }

    /// IComponent override
    virtual void SerializeToBinary(kNet::DataSerializer& dest) const;

    /// IComponent override
    virtual void DeserializeFromBinary(kNet::DataDeserializer& source, AttributeChange::Type change);

public slots:
    /// A factory method that constructs a new attribute given the typename. This factory is not extensible.
    IAttribute *CreateAttribute(const QString &typeName, const QString &name, AttributeChange::Type change = AttributeChange::Default);

    /// Create new attribute that type is QVariant.
    /** @param name Name of the attribute.
    */
    void AddQVariantAttribute(const QString &name, AttributeChange::Type change = AttributeChange::Default);

    /// Get attribute value as QVariant.
    /** If attribute type isn't QVariantAttribute then attribute value is returned as in string format.
        Use QVariant's isNull method to check if the variant value is initialized.
        @param index Index to attribute list.
        @return Return attribute value as QVariant if attribute has been found, else return null QVariant.
    */
    QVariant GetAttribute(int index) const;

    /// This is an overloaded function.
    /** @param name Name of the attribute.
    */
    QVariant GetAttribute(const QString &name) const;

    /// Inserts new attribute value to attribute. Note: this is only meant to be used through javascripts.
    /** @param name Name of the attribute.
     *  @param value Value of the attribe.
     *  @param change Change type.
     *  @todo remove this from dynamic component when possible.
     */
    void SetAttributeQScript(const QString &name, const QScriptValue &value, AttributeChange::Type change);
    /// Inserts new attribute value to attribute.
    /** @param index Index for the attribute.
        @param value Value of the attribute.
        @param change Change type.
    */
    void SetAttribute(int index, const QVariant &value, AttributeChange::Type change = AttributeChange::Default);

    /// This is an overloaded function.
    /** @param name Name of the attribute.
        @param value Value of the attribute.
        @param change Change type.
    */
    void SetAttribute(const QString &name, const QVariant &value, AttributeChange::Type change = AttributeChange::Default);

    /// Returns number of attributes in this component.
    int GetNumAttributes() const { return attributes_.size(); }

    /// Returns name of attribute with the spesific @c index
    /** @param index Index of the attribute.
    */
    QString GetAttributeName(int index) const;

    /// Checks if a given component @c comp is holding exactly same attributes as this component.
    /** @param comp Component to be compared with.
        @return Return true if component is holding same attributes as this component else return false.
    */
    bool ContainSameAttributes(const EC_DynamicComponent &comp) const;

    /// Remove attribute from the component.
    /** @param name Name of the attirbute.
    */
    void RemoveAttribute(const QString &name, AttributeChange::Type change = AttributeChange::Default);

    /// Check if component is holding an attribute by the @c name.
    /** @param name Name of attribute that we are looking for.
    */
    bool ContainsAttribute(const QString &name) const;

signals:
    /// Emitted when a new attribute is added to this component.
    /** @param attr New attribute.
    */
    void AttributeAdded(IAttribute *attr);

    /// Emitted when attribute is about to be removed.
    /** @param attr Attribute about to be removed.
    */
    void AttributeAboutToBeRemoved(IAttribute *attr);

    /// Emitted when attribute is removed from this component.
    /** @param name Name of the attribute.
        @todo REMOVE
    */
    void AttributeRemoved(const QString &name);

private:
    /// Constructor.
    /** @param module Declaring module
    */
    explicit EC_DynamicComponent(IModule *module);
};

#endif
