// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"
#include "IComponent.h"
#include "IAttribute.h"

#include <QVariant>

namespace kNet
{
    class DataSerializer;
    class DataDeserializer;
}

struct DeserializeData;

class QScriptValue;

/// A component that allows adding of dynamically structured attributes at runtime.
/** <table class="header">
    <tr>
    <td>
    <h2>DynamicComponent</h2>
    Component for which user can add and delete attributes at runtime.
    <b> Name of the attributes must be unique. </b>
    It's recommend to use attribute names when you set or get your attribute values because
    indices can change while the dynamic component's attributes are added or removed.

    Use CreateAttribute for creating new attributes.

    When component is deserialized it will compare old and a new attribute values and will get difference
    between those two and use that information to remove attributes that are not in the new list and add those
    that are only in new list and only update those values that are same in both lists.

    Registered by TundraLogicModule.

    <b>No Static Attributes.</b>

    <b>Exposes the following scriptable functions:</b>
    <ul>
    <li>"CreateAttribute": @copydoc CreateAttribute
    <li>"GetAttribute": @copydoc GetAttribute
    <li>"SetAttribute": @copydoc SetAttribute
    <li>"GetAttributeName": @copydoc GetAttributeName
    <li>"ContainSameAttributes": @copydoc ContainSameAttributes
    <li>"RemoveAttribute": @copydoc RemoveAttribute
    <li>"ContainsAttribute": @copydoc ContainsAttribute
    <li>"RemoveAllAttributes": @copydoc RemoveAllAttributes
    </ul>

    Does not react on entity actions.

    Does not emit any actions.

    <b>Doesn't depend on any components</b>.

    </table> */
class TUNDRACORE_API EC_DynamicComponent : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_DynamicComponent", 25)

public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_DynamicComponent(Scene* scene);

    ~EC_DynamicComponent();

    /// IComponent override.
    void DeserializeFrom(QDomElement& element, AttributeChange::Type change);

    void DeserializeCommon(std::vector<DeserializeData>& deserializedAttributes, AttributeChange::Type change);

    /// IComponent override
    virtual void SerializeToBinary(kNet::DataSerializer& dest) const;

    /// IComponent override
    virtual void DeserializeFromBinary(kNet::DataDeserializer& source, AttributeChange::Type change);

public slots:
    /// IComponent override
    virtual bool SupportsDynamicAttributes() const { return true; }
    
    /// A factory method that constructs a new attribute of a given the type name.
    /** @param typeName Type name of the attribute.
        @param id ID of the attribute.
        @param change Change type.
        This factory is not extensible. If attribute was already created the method will return it's pointer.

        @note If multiple clients, or the client and the server, add attributes at the same time, unresolvable
        scene replication conflits will occur. The exception is filling attributes immediately after creation
        (before the component is replicated for the first time), which is supported. Prefer to either create
        all attributes at creation, or to only add new attributes on the server. 
        
        @note Name of the attribute will be assigned to same as the ID. */
    IAttribute *CreateAttribute(const QString &typeName, const QString &id, AttributeChange::Type change = AttributeChange::Default);

    /// Get attribute value as QVariant.
    /** If attribute type isn't QVariantAttribute then attribute value is returned as in string format.
        Use QVariant's isNull method to check if the variant value is initialized.
        @param index Index to attribute list.
        @return Return attribute value as QVariant if attribute has been found, else return null QVariant. */
    QVariant GetAttribute(int index) const;
    QVariant GetAttribute(const QString &id) const; /**< @overload @param id ID of the attribute. */

    /// Inserts new attribute value to attribute.
    /** @param index Index for the attribute.
        @param value Value of the attribute.
        @param change Change type. */
    void SetAttribute(int index, const QVariant &value, AttributeChange::Type change = AttributeChange::Default);
    void SetAttribute(const QString &id, const QVariant &value, AttributeChange::Type change = AttributeChange::Default); /**< @overload @param name Name of the attribute. */

    /// Returns ID of attribute with the specific @c index
    /** @param index Index of the attribute. */
    QString GetAttributeId(int index) const;

    /// Checks if a given component @c comp is holding exactly same attributes as this component.
    /** @param comp Component to be compared with.
        @return Return true if component is holding same attributes as this component else return false. */
    bool ContainSameAttributes(const EC_DynamicComponent &comp) const;

    /// Remove attribute from the component.
    /** @param id ID of the attribute. */
    void RemoveAttribute(const QString &id, AttributeChange::Type change = AttributeChange::Default);

    /// Check if component is holding an attribute by the @c name.
    /** @param id ID of attribute that we are looking for. */
    bool ContainsAttribute(const QString &id) const;

    /// Removes all attributes from the component
    void RemoveAllAttributes(AttributeChange::Type change = AttributeChange::Default);

    void AddQVariantAttribute(const QString &id, AttributeChange::Type change = AttributeChange::Default); /**< @deprecated Use CreateAttribute('QVariant') @todo Remove */
    void SetAttributeQScript(const QString &id, const QScriptValue &value, AttributeChange::Type change = AttributeChange::Default); /**< @deprecated Use SetAttribute @todo Remove */

    // DEPRECATED
    /// Returns name of attribute with the specific @c index
    /** @param index Index of the attribute. 
        @todo Remove once nothing uses it, as IDs are same as name for dynamic attributes */
    QString GetAttributeName(int index) const;

private:
    /// Convert attribute index without holes (used by client) into actual attribute index. Returns below zero if not found. Requires a linear search.
    int GetInternalAttributeIndex(int index) const;
};
