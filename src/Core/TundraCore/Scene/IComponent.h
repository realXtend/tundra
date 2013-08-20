/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   IComponent.h
    @brief  The common interface for all components, which are the building blocks the scene entities are formed of. */

#pragma once

#include "TundraCoreApi.h"
#include "SceneFwd.h"
#include "AttributeChangeType.h"
#include "IAttribute.h"
#include "LoggingFunctions.h"

#include <QObject>
#include <QVariant>

class QDomDocument;
class QDomElement;

class Framework;

/// Specifies unique type name and unique type ID of this component.
/** This #define should be instantiated inside the public slots: section of the component.
    Warning: This #define alters the current visibility specifier in the class file. */
#define COMPONENT_NAME(componentTypeName, componentTypeId)                              \
public:                                                                                 \
    static const QString &TypeNameStatic()                                              \
    {                                                                                   \
        static const QString name(componentTypeName);                                       \
        return name;                                                                    \
    }                                                                                   \
    static u32 TypeIdStatic()                                                           \
    {                                                                                   \
        return componentTypeId;                                                         \
    }                                                                                   \
    enum { ComponentTypeId = componentTypeId };                                         \
public slots:                                                                           \
    virtual const QString &TypeName() const                                             \
    {                                                                                   \
        return TypeNameStatic();                                                        \
    }                                                                                   \
    virtual u32 TypeId() const                                                          \
    {                                                                                   \
        return componentTypeId;                                                         \
    }                                                                                   \
private: // Return the class visibility specifier to the strictest form so that the user most likely catches that this macro had to change the visibility.

//Q_PROPERTY(type attribute READ get##attribute WRITE set##attribute)
/// Defines a new 'Attribute<type> attribute' member as an automatically generated QProperty of name 'attribute'.
#define DEFINE_QPROPERTY_ATTRIBUTE(type, attribute) \
    Attribute<type > attribute; \
    type get##attribute() const { return (type)attribute.Get(); } \
    void set##attribute(type value) { attribute.Set((type)value, AttributeChange::Default); }

/// Macro for constructing an attribute in the component's constructor initializer list. "id" is the property/variable name, "name" is the human-readable name used in editing.
#define INIT_ATTRIBUTE(id, name) id(this, #id, name)
/// Macro for constructing an attribute in the component's constructor initializer list. "id" is the property/variable name, "name" is the human-readable name used in editing, "value" is initial value.
#define INIT_ATTRIBUTE_VALUE(id, name, value) id(this, #id, name, value)

/// The common interface for all components, which are the building blocks the scene entities are formed of.
/** Inherit your own components from this class. Never directly allocate new components using operator new,
    but use the factory-based SceneAPI::CreateComponent functions instead.

    Each Component has a compile-time specified type name that identifies the class-name of the Component.
    This differentiates different derived implementations of the IComponent class. Each implemented Component
    must have a unique type name.

    Additionally, each Component has a Name string, which identifies different instances of the same Component,
    if more than one is added to an Entity.

    A Component consists of a list of Attributes, which are automatically replicable instances of scene data.
    See IAttribute for more details.

    To retain network protocol compatibility between Tundra versions, only add any new static attributes to
    the end of the Component's existing attributes. Note that the order in the header is what matters, not
    the order they are initialized in the constructor, but the two should still match.

    Every Component has a state variable updateMode that specifies a default setting for managing which objects
    get notified whenever an Attribute change event occurs. This is used to create "Local Only"-objects as well
    as when doing batch updates of Attributes (for performance or correctness). */
class TUNDRACORE_API IComponent : public QObject, public enable_shared_from_this<IComponent>
{
    Q_OBJECT
    Q_PROPERTY(QString name READ Name WRITE SetName) /**< @copybrief Name */
    Q_PROPERTY(QString typeName READ TypeName) /**< @copybrief TypeName */
    Q_PROPERTY(bool replicated READ IsReplicated)  /**< @copybrief IsReplicated */
    Q_PROPERTY(bool local READ IsLocal) /**< @copybrief IsLocal */
    /// @note Use "component.updateMode = { value : <AttributeChange::Type value as int> };" syntax when settings updateMode from QtScript.
    Q_PROPERTY(AttributeChange::Type updateMode READ UpdateMode WRITE SetUpdateMode) /**< @copybrief UpdateMode */
    Q_PROPERTY (uint id READ Id) /**< @copybrief Id */
    Q_PROPERTY (bool unacked READ IsUnacked) /**< @copybrief IsUnacked */
    Q_PROPERTY (bool temporary READ IsTemporary WRITE SetTemporary) /**< @copybrief IsTemporary */
    /// @deprecated. Remove when all scripts have been converted to not refer to this
    Q_PROPERTY(bool networkSyncEnabled READ IsReplicated)

public:
    /// @cond PRIVATE
    /// Constructor.
    /** @note scene - and consecutively framework - can be null if component is created unparented
        intentionally, so always remember to perform null checks for them even in the ctor. The ParentEntitySet
        signal can used internally to know when accessing parent scene, parent entity, or framework is possible. */
    explicit IComponent(Scene* scene);
    /// @endcond PRIVATE

    /// Deletes potential dynamic attributes.
    virtual ~IComponent();

    /// Returns the typename of this component.
    /** The typename is the "class" type of the component,
        e.g. "EC_Mesh" or "EC_DynamicComponent". The type name of a component cannot be an empty string.
        The type name of a component never changes at runtime.
        @note Prefer TypeId over TypeName when inspecting the component type (performance). */
    virtual const QString &TypeName() const = 0;

    /// Returns the unique type ID of this component.
    virtual u32 TypeId() const = 0;

    /// Returns the name of this component.
    /** The name of a component is a custom user-specified name for
        this component instance, and identifies separate instances of the same component in an object. 
        The (TypeName, Name) pairs of all components in an Entity must be unique. The Name string can be empty. */
    const QString &Name() const { return name; }

    /// Sets the name of the component.
    /** This call will silently fail if there already exists a component with the
        same (TypeName, Name) pair in this entity. When this function changes the name of the component,
        the signal ComponentNameChanged is emitted.
        @param name The new name for this component. This may be an empty string. */
    void SetName(const QString& name);

    /// Stores a pointer of the Entity that owns this component into this component.
    /** This function is called at component initialization time to attach this component to its owning Entity.
        Although public, it is not intended to be called by users of IComponent. */
    void SetParentEntity(Entity* entity);

    /// Returns the list of all Attributes in this component for reflection purposes.
    /** *Warning*: because attribute reindexing is not performed when dynamic attributes are removed, you *must* be prepared for null pointers when examining this! */
    const AttributeVector& Attributes() const { return attributes; }

    /// Returns a list of all attributes with null attributes sanitated away. This is slower than Attributes().
    AttributeVector NonEmptyAttributes() const;

    /// Serializes this component and all its Attributes to the given XML document.
    /** @param doc The XML document to serialize this component to.
        @param baseElement Points to the <entity> element of the document doc. This element is the Entity that
                owns this component. This component will be serialized as a child tree of this element. 
        @param serializeTemporary Serialize temporary components for application-specific purposes. The default value is false */
    virtual void SerializeTo(QDomDocument& doc, QDomElement& baseElement, bool serializeTemporary = false) const;

    /// Deserializes this component from the given XML document.
    /** @param element Points to the <component> element that is the root of the serialized form of this Component.
        @param change Specifies the source of this change. This field controls whether the deserialization
                     was initiated locally and must be replicated to network, or if the change was received from
                     the network and only local application of the data suffices. */
    virtual void DeserializeFrom(QDomElement& element, AttributeChange::Type change);

    /// Serialize attributes to binary
    /** @note does not include syncmode, type name or name. These are left for higher-level logic, and
        it depends on the situation if they are needed or not */
    virtual void SerializeToBinary(kNet::DataSerializer& dest) const;

    /// Deserialize attributes from binary
    /** @note does not include syncmode, type name or name. These are left for higher-level logic, and
        it depends on the situation if they are needed or not. */
    virtual void DeserializeFromBinary(kNet::DataDeserializer& source, AttributeChange::Type change);

    /// Create an attribute with specified index, type and ID. Return it if successful or null if not. Called by SyncManager.
    /** Component must override SupportsDynamicAttributes() to allow creating attributes. 
        @note For dynamic attributes ID and name will be same
      */
    IAttribute* CreateAttribute(u8 index, u32 typeID, const QString& id, AttributeChange::Type change = AttributeChange::Default);

    /// Remove an attribute at the specified index. Called by network sync.
    void RemoveAttribute(u8 index, AttributeChange::Type change);
    
    /// Enables or disables network synchronization of changes that occur in the attributes of this component.
    /** True by default. Can only be changed before the component is added to an entity, because the replication determines the ID range to use. */
    void SetReplicated(bool enable);

    /// Finds and returns an attribute of type 'Attribute<T>' and given name
    /** @param T The Attribute type to look for.
        @param name The name of the attribute.
        @return If there exists an attribute of type 'Attribute<T>' which has the given name, a pointer to
                that attribute is returned, otherwise returns null. 
        Note: attribute names are human-readable (shown in editor) and may be subject to change, while id's
        (property / variable names) should be fixed. */
    template<typename T>
    Attribute<T> *AttributeByName(const QString &name) const
    {
        for(size_t i = 0; i < attributes.size(); ++i)
            if (attributes[i] && !attributes[i]->Name().compare(name, Qt::CaseInsensitive))
                return dynamic_cast<Attribute<T> *>(&attributes[i]);
        return 0;
    }
    
    /// Finds and returns an attribute of type 'Attribute<T>' and given ID
    /** @param T The Attribute type to look for.
        @param id The ID of the attribute.
        @return If there exists an attribute of type 'Attribute<T>' which has the given ID, a pointer to
                that attribute is returned, otherwise returns null.   */
    template<typename T>
    Attribute<T> *AttributeById(const QString &id) const
    {
        for(size_t i = 0; i < attributes.size(); ++i)
            if (attributes[i] && !attributes[i]->Id().compare(id, Qt::CaseInsensitive))
                return dynamic_cast<Attribute<T> *>(&attributes[i]);
        return 0;
    }
    
    /// Returns a pointer to the Framework instance.
    Framework *GetFramework() const { return framework; }

    /// Returns an Attribute of this component with the given ID.
    /** This function iterates through the attribute vector and tries to find a member attribute with the given ID
        @param The ID of the attribute to look for.
        @return A pointer to the attribute, or null if no attribute with the given ID exists */
    IAttribute* AttributeById(const QString &name) const;
    
    /// Returns an Attribute of this component with the given name.
    /** This function iterates through the attribute vector and tries to find a member attribute with the given name.
        @param The name of the attribute to look for.
        @return A pointer to the attribute, or null if no attribute with the given name exists. 
        @note attribute names are human-readable (shown in editor) and may be subject to change, while id's
        (property / variable names) should be fixed. */
    IAttribute* AttributeByName(const QString &name) const;
    
    // Deprecated, use AttributeByName instead
    IAttribute* GetAttribute(const QString &name) const;

    // Deprecated, use AttributeByName<T> instead
    /// Finds and returns an attribute of type 'Attribute<T>' and given name
    /** @param T The Attribute type to look for.
        @param name The name of the attribute.
        @return If there exists an attribute of type 'Attribute<T>' which has the given name, a pointer to
                that attribute is returned, otherwise returns null. 
        Note: attribute names are human-readable (shown in editor) and may be subject to change, while id's
        (property / variable names) should be fixed. */
    template<typename T>
    Attribute<T> *GetAttribute(const QString &name) const
    {
        LogWarning("IComponent::GetAttribute<T> is deprecated and will be removed. Use AttributeByName<T> or AttributeById<T> instead");
        for(size_t i = 0; i < attributes.size(); ++i)
            if (attributes[i] && !attributes[i]->Name().compare(name, Qt::CaseInsensitive))
                return dynamic_cast<Attribute<T> *>(&attributes[i]);
        return 0;
    }
    
public slots:
    /// Returns true if network synchronization of the attributes of this component is enabled.
    /// A component is always either local or replicated, but not both.
    /// @todo Doesn't need to be a slot, exposed as Q_PROPERTY.
    bool IsReplicated() const { return replicated; }

    /// Returns true if network synchronization of the attributes of this component is NOT enabled.
    /// A component is always either local or replicated, but not both.
    /// @todo Doesn't need to be a slot, exposed as Q_PROPERTY.
    bool IsLocal() const { return !replicated; }

    /// Returns true if this component is pending a replicated ID assignment from the server.
    /// @todo Doesn't need to be a slot, exposed as Q_PROPERTY.
    bool IsUnacked() const;

    /// Sets the default mode for attribute change operations
    /// @todo Doesn't need to be a slot, exposed as Q_PROPERTY.
    void SetUpdateMode(AttributeChange::Type defaultmode);

    /// Gets the default mode for attribute change operations
    /// @todo Doesn't need to be a slot, exposed as Q_PROPERTY.
    AttributeChange::Type UpdateMode() const { return updateMode; }

    /// Returns component id, which is unique within the parent entity
    /// @todo Doesn't need to be a slot, exposed as Q_PROPERTY.
    component_id_t Id() const { return id; }

    /// Returns whether this component supports adding dynamic attributes. False by default.
    /** Components that do *not* support dynamic attributes (most of them) are resilient to versioning mismatches between client/server
        as long as the new attributes are added to the end of the static attributes list. In contrast, components with dynamic attributes
        are not resilient to mismatches, except if they use *only* dynamic attributes, like EC_DynamicComponent. */
    virtual bool SupportsDynamicAttributes() const { return false; }
    
    /// Returns the total number of attributes in this component. Does not count holes in the attribute vector
    int NumAttributes() const;

    /// Returns the number of static (i.e. not dynamically allocated) attributes in this component. These are always in the beginning of the attribute vector.
    int NumStaticAttributes() const;

    /// Informs this component that the value of a member Attribute of this component has changed.
    /** You may call this function manually to force Attribute change signal to
        occur, but it is not necessary if you use the Attribute::Set function, since
        it notifies this function automatically.
        @param attribute The attribute that was changed. The attribute passed here must be an Attribute member of this component.
        @param change Informs to the component the type of change that occurred.

        This function calls EmitAttributeChanged and triggers the 
        OnAttributeChanged signal of this component.

        This function is called by IAttribute::Changed whenever the value in that
        attribute is changed. */
    void EmitAttributeChanged(IAttribute* attribute, AttributeChange::Type change);

    /// Informs this component that the metadata of a member Attribute has changed.
    /** @param The attribute of which metadata was changed. The attribute passed here must be an Attribute member of this component. */
    void EmitAttributeMetadataChanged(IAttribute* attribute);

    /// @overload
    /** @param attributeName Name of the attribute that changed. @note this is a no-op if the named attribute is not found.
        @param change Informs to the component the type of change that occurred. */
    void EmitAttributeChanged(const QString& attributeName, AttributeChange::Type change);

    /// Informs that every attribute in this Component has changed with the change
    /** you specify. If change is Replicate, or it is Default and the UpdateMode is Replicate,
        every attribute will be synced to the network. */
    void ComponentChanged(AttributeChange::Type change);

    /// Returns the Entity this Component is part of.
    /** @note Calling this function will return null if it is called in the ctor or dtor of this Component.
        This is because the parent entity has not yet been set with a call to SetParentEntity at that point,
        and parent entity is set to null before the actual Component is destroyed.
        @sa ParentScene */
    Entity* ParentEntity() const;

    /// Returns the scene this Component is part of.
    /** May return null if component is not in an entity or entity is not in a scene.
        @sa ParentEntity */
    Scene* ParentScene() const;

    /// Sets whether component is temporary. Temporary components won't be saved when the scene is saved.
    /// @todo Doesn't need to be a slot, exposed as Q_PROPERTY.
    void SetTemporary(bool enable);

    /// Returns whether component is temporary. Temporary components won't be saved when the scene is saved.
    /** @note if parent entity is temporary, this returns always true regardless of the component's temporary flag. */
    /// @todo Doesn't need to be a slot, exposed as Q_PROPERTY.
    bool IsTemporary() const;

    /// Returns whether the component is in a view-enabled scene, or not.
    /** If the information is not available (component is not yet in a scene, will guess "true. */
    bool ViewEnabled() const;

    /// Returns value of an attribute as a QVariant.
    /** @param id ID or name of the attribute, case-insensitive. */
    QVariant GetAttributeQVariant(const QString &id) const;

    /// Returns list of attribute names of the component
    QStringList GetAttributeNames() const;

    /// Returns list of attribute IDs of the component.
    QStringList GetAttributeIds() const;

    /// Crafts a component type name string that is guaranteed not to thave the "EC_" prefix.
    static QString EnsureTypeNameWithoutPrefix(const QString &tn) { return (tn.startsWith("EC_", Qt::CaseInsensitive) ? tn.mid(3) : tn); }
    /// Crafts a component type name string that is guaranteed to have the "EC_" prefix.
    static QString EnsureTypeNameWithPrefix(const QString &tn) { return (tn.startsWith("EC_", Qt::CaseInsensitive) ? tn : "EC_" + tn); }

    // DEPRECATED
    void SetNetworkSyncEnabled(bool enable); /**< @deprecated Currently a no-op, as replication mode can not be changed after adding to an entity. @todo Remove! */

signals:
    /// This signal is emitted when an Attribute of this Component has changed. 
    void AttributeChanged(IAttribute* attribute, AttributeChange::Type change);
    
    /// This signal is emitted when metadata of an Attribute in this Component has changed.
    void AttributeMetadataChanged(IAttribute *attribute, const AttributeMetadata *metadata);

    ///\todo In the future, provide a method of listening to a change of specific Attribute, instead of having to
    /// always connect to the above function and if(...)'ing if it was the change we were interested in.

    /// This signal is emitted when the name of this Component has changed.
    /** Use this signal to keep track of a component with specified custom name.*/
    void ComponentNameChanged(const QString &newName, const QString &oldName);

    /// This signal is emitted when this Component is attached to its owning Entity.
    /** When this signal is emitted, the framework pointer is guaranteed to valid. */
    void ParentEntitySet();

    /// This signal is emitted when this Component is detached from its parent, i.e. the new parent is set to null.
    void ParentEntityDetached();

    /// Emitted when a new attribute is added to this component.
    /** @param attr New attribute. */
    void AttributeAdded(IAttribute *attr);

    /// Emitted when attribute is about to be removed.
    /** @param attr Attribute about to be removed.
        @todo Scripts cannot access IAttribute; consider maybe using name or something else in the signature. */
    void AttributeAboutToBeRemoved(IAttribute *attr);
    
protected:
    /// Helper function for starting component serialization.
    /** This function creates an XML element <component> with the name of this component, adds it to the document, and returns it. 
        If serializeTemporary is true, the attribute 'temporary' is added to the XML element. Default is false. */
    QDomElement BeginSerialization(QDomDocument& doc, QDomElement& baseElement, bool serializeTemporary = false) const;

    /// Helper function for adding an attribute and it's type to the component XML serialization.
    void WriteAttribute(QDomDocument& doc, QDomElement& compElement, const QString& name, const QString& id, const QString& value, const QString &type) const;

    /// Helper function for starting deserialization.
    /** Checks that XML element contains the right kind of EC, and if it is right, sets the component name.
        Otherwise returns false and does nothing. */
    bool BeginDeserialization(QDomElement& compElement);

    /// Add attribute to this component.
    void AddAttribute(IAttribute* attr);

    /// Add attribute to this component at specified index, creating new holes if necessary. Static attributes can not be overwritten. Return true if successful
    bool AddAttribute(IAttribute* attr, u8 index);

    Entity* parentEntity; ///< The Entity this Component is part of, or null if this Component is not attached to any Entity.
    QString name; ///< The name of this component, by default an empty string.
    AttributeVector attributes; ///< Attributes of the component.
    component_id_t id; ///< Component id, unique within the parent entity
    bool replicated; ///< Network sync enabled -flag
    AttributeChange::Type updateMode; ///< Default update mode for attribute changes
    Framework* framework; ///< Needed to be able to perform important uninitialization etc. even when not in an entity.
    bool temporary; ///< Temporary-flag

private:
    friend class ::IAttribute;
    friend class Entity;
    
    /// This function is called by the base class (IComponent) to signal to the derived class that one or more
    /// of its attributes have changed, and it should update its internal state accordingly.
    /// The derived class can call IAttribute::ValueChanged() to query which attributes have changed value,
    /// and after reacting to the change, call IAttribute::ClearChangedFlag().
    virtual void AttributesChanged() {}

    /// Set component id. Called by Entity
    void SetNewId(component_id_t newId);
};
