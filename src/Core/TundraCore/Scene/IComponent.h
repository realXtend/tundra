/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   IComponent.h
    @brief  The common interface for all components, which are the building blocks the scene entities are formed of. */

#pragma once

#include "TundraCoreApi.h"
#include "SceneFwd.h"
#include "AttributeChangeType.h"
#include "IAttribute.h"

#include <QObject>
#include <QVariant>

class QDomDocument;
class QDomElement;

class Framework;

/// Specifies the name and ID of this component.
/** This #define should be instantiated inside the public slots: section of the component.
    Warning: This #define alters the current visibility specifier in the class file. */
#define COMPONENT_NAME(componentName, componentTypeId)                                  \
public:                                                                                 \
    static const QString &TypeNameStatic()                                              \
    {                                                                                   \
        static const QString name(componentName);                                       \
        return name;                                                                    \
    }                                                                                   \
    static u32 TypeIdStatic()                                                           \
    {                                                                                   \
        return componentTypeId;                                                         \
    }                                                                                   \
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
/// Exposes an existing 'Attribute<type> attribute' member as an automatically generated QProperty of name 'attribute'.
#define EXPOSE_ATTRIBUTE_AS_QPROPERTY(type, attribute) \
    type get##attribute() const { return (type)attribute.Get(); } \
    void set##atribute(type value) { attribute.Set((type)value, AttributeChange::Default); }

//Q_PROPERTY(type attribute READ get##attribute WRITE set##attribute)
/// Defines a new 'Attribute<type> attribute' member as an automatically generated QProperty of name 'attribute'.
#define DEFINE_QPROPERTY_ATTRIBUTE(type, attribute) \
    Attribute<type > attribute; \
    type get##attribute() const { return (type)attribute.Get(); } \
    void set##attribute(type value) { attribute.Set((type)value, AttributeChange::Default); }

/// The common interface for all components, which are the building blocks the scene entities are formed of.
/** Inherit your own components from this class.
    Each Component has a compile-time specified type name that identifies the class-name of the Component.
    This differentiates different derived implementations of the IComponent class. Each implemented Component
    must have a unique type name.

    Additionally, each Component has a Name string, which identifies different instances of the same Component,
    if more than one is added to an Entity.

    A Component consists of a list of Attributes, which are automatically replicatable instances of scene data.
    See IAttribute for more details.

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
    /// Constructor.
    explicit IComponent(Scene* scene);

    /// Deletes potential dynamic attributes.
    virtual ~IComponent();

    /// Returns the typename of this component.
    /** The typename is the "class" type of the component,
        e.g. "EC_Mesh" or "EC_DynamicComponent". The typename of a component cannot be an empty string.
        The typename of a component never changes at runtime. */
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
    
    /// Finds and returns an attribute of type 'Attribute<T>' and given name.
    /** @param T The Attribute type to look for.
        @param name The name of the attribute.
        @return If there exists an attribute of type 'Attribute<T>' which has the given name, a pointer to
                that attribute is returned, otherwise returns null. */
    template<typename T>
    Attribute<T> *GetAttribute(const QString &name) const
    {
        for(size_t i = 0; i < attributes.size(); ++i)
            if (attributes[i] && attributes[i]->Name() == name)
                return dynamic_cast<Attribute<T> *>(&attributes[i]);
        return 0;
    }

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
    /** @note does not include syncmode, typename or name. These are left for higher-level logic, and
        it depends on the situation if they are needed or not */
    virtual void SerializeToBinary(kNet::DataSerializer& dest) const;

    /// Deserialize attributes from binary
    /** @note does not include syncmode, typename or name. These are left for higher-level logic, and
        it depends on the situation if they are needed or not. */
    virtual void DeserializeFromBinary(kNet::DataDeserializer& source, AttributeChange::Type change);

    /// Returns an Attribute of this component with the given @c name.
    /** This function iterates through the attribute vector and tries to find a member attribute with the given name.
        @param The name of the attribute to look for.
        @return A pointer to the attribute, or null if no attribute with the given name exists. */
    IAttribute* GetAttribute(const QString &name) const;
    
    /// Create an attribute with specifed index, type and name. Return it if successful or null if not. Called by SyncManager.
    IAttribute* CreateAttribute(u8 index, u32 typeID, const QString& name, AttributeChange::Type change = AttributeChange::Default);
    
    /// Remove an attribute at the specified index. Called by network sync.
    void RemoveAttribute(u8 index, AttributeChange::Type change);
    
    /// Enables or disables network synchronization of changes that occur in the attributes of this component.
    /** True by default. Can only be changed before the component is added to an entity, because the replication determines the ID range to use. */
    void SetReplicated(bool enable);

public slots:
    /// Returns a pointer to the Framework instance.
    Framework *GetFramework() const { return framework; }

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

    /// Returns the total number of attributes in this component. Does not count holes in the attribute vector
    int NumAttributes() const;

    /// Returns the number of static (ie. not dynamically allocated) attributes in this component. These are always in the beginning of the attribute vector.
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
        and parent entity is set to null before the actual Component is destroyed. */
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

    /// Returns an attribute of this component as a QVariant
    /** @param name of attribute
        @return values of the attribute */
    QVariant GetAttributeQVariant(const QString &name) const;

    /// Returns list of attribute names of the component
    QStringList GetAttributeNames() const;

    /** @deprecated Currently a no-op, as replication mode can not be changed after adding to an entity.
        @todo Removed once scripts converted to not call this */
    void SetNetworkSyncEnabled(bool enable);

signals:
    /// This signal is emitted when an Attribute of this Component has changed. 
    void AttributeChanged(IAttribute* attribute, AttributeChange::Type change);

    ///\todo In the future, provide a method of listening to a change of specific Attribute, instead of having to
    /// always connect to the above function and if(...)'ing if it was the change we were interested in.

    /// This signal is emitted when the name of this Component has changed.
    /** Use this signal to keep track of a component with specified custom name.*/
    void ComponentNameChanged(const QString &newName, const QString &oldName);

    /// This signal is emitted when this Component is attached to its owning Entity.
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

    /// Helper function for adding an attribute to the component xml serialization.
    void WriteAttribute(QDomDocument& doc, QDomElement& compElement, const QString& name, const QString& value) const;

    /// Helper function for adding an attribute and it's type to the component xml serialization.
    void WriteAttribute(QDomDocument& doc, QDomElement& compElement, const QString& name, const QString& value, const QString &type) const;

    /// Helper function for starting deserialization. 
    /** Checks that XML element contains the right kind of EC, and if it is right, sets the component name.
        Otherwise returns false and does nothing. */
    bool BeginDeserialization(QDomElement& compElement);

    /// Helper function for getting an attribute from serialized component.
    QString ReadAttribute(QDomElement& compElement, const QString &name) const;

    /// Helper function for getting a attribute type from serialized component.
    QString ReadAttributeType(QDomElement& compElement, const QString &name) const;

    /// Add attribute to this component.
    void AddAttribute(IAttribute* attr);

    /// Add attribute to this component at specified index, creating new holes if necessary. Static attributes can not be overwritten. Return true if successful
    bool AddAttribute(IAttribute* attr, u8 index);

    /// Points to the Entity this Component is part of, or null if this Component is not attached to any Entity.
    Entity* parentEntity;

    /// The name of this component, by default an empty string.
    QString name;

    /// Attribute list for introspection/reflection.
    AttributeVector attributes;

    /// Component id, unique within the parent entity
    component_id_t id;
    
    /// Network sync enable flag
    bool replicated;

    /// Default update mode for attribute changes
    AttributeChange::Type updateMode;

    /// Framework pointer. Needed to be able to perform important uninitialization etc. even when not in an entity.
    Framework* framework;

    /// Temporary-flag
    bool temporary;

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
