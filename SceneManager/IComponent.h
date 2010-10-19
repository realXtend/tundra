/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   IComponent.h
 *  @brief  Base class for all components. Inherit from this class when creating new components.
 */

#ifndef incl_Interfaces_IComponent_h
#define incl_Interfaces_IComponent_h

#include "ForwardDefines.h"
#include "AttributeChangeType.h"
#include "IAttribute.h"
#include "IEventData.h"
#include "CoreTypes.h"

#include <QObject>

#include <set>

class QDomDocument;
class QDomElement;

/// IComponent is the base class for all user-created components. Inherit your own components from this class.
/** Each Component has a compile-time specified Typename that identifies the class-name of the Component.
    This differentiates different derived implementations of the IComponent class. Each implemented Component
    must have a unique Typename.

    Additionally, each Component has a Name string, which identifies different instances of the same Component,
    if more than one is added to an Entity.

    A Component consists of a list of Attributes, which are automatically replicatable instances of scene data.
    See IAttribute for more details.

    Every Component has a state variable 'UpdateMode' that specifies a default setting for managing which objects
    get notified whenever an Attribute change event occurs. This is used to create "Local Only"-objects as well
    as when doing batch updates of Attributes (for performance or correctness). */
class IComponent : public QObject
{
    friend class ::IAttribute;

    Q_OBJECT
    Q_PROPERTY(QString Name READ Name WRITE SetName)
    Q_PROPERTY(QString TypeName READ TypeName)
    Q_PROPERTY(bool NetworkSyncEnabled READ GetNetworkSyncEnabled WRITE SetNetworkSyncEnabled)
    Q_PROPERTY(AttributeChange::Type UpdateMode READ GetUpdateMode WRITE SetUpdateMode)
    
public:
    /// Constuctor.
    explicit IComponent(Foundation::Framework* framework);

    /// Copy-constructor.
    IComponent(const IComponent& rhs);

    /// Destructor.
    virtual ~IComponent();

    /// Returns the typename of this component. The typename is the "class" type of the component,
    /// e.g. "EC_Mesh" or "EC_DynamicComponent". The typename of a component cannot be an empty string.
    /// The typename of a component never changes at runtime.
    virtual const QString &TypeName() const = 0;

    //! Returns type name hash of the component
    uint TypeNameHash() const;

    /// Returns the name of this component. The name of a component is a custom user-specified name for
    /// this component instance, and identifies separate instances of the same component in an object. 
    /// The (TypeName, Name) pairs of all components in an Entity must be unique. The Name string can be empty.
    const QString &Name() const { return name_; }

    /// Sets the name of the component. This call will silently fail if there already exists a component with the
    /// same (Typename, Name) pair in this entity. When this function changes the name of the component,
    /// the signal OnComponentNameChanged is emitted.
    /// @param name The new name for this component. This may be an empty string.
    void SetName(const QString& name);

    /// Stores a pointer of the Entity that owns this component into this component. This function is called at
    /// component initialization time to attach this component to its owning Entity.
    /// Although public, it is not intended to be called by users of IComponent.
    void SetParentEntity(Scene::Entity* entity);

public slots:
    /// Returns a pointer to the Naali framework instance.
    Foundation::Framework *GetFramework() const { return framework_; }

    /// Enables or disables network synchronization of changes that occur in the attributes of this component.
    /// By default, this flag is set for all created components.
    /// When network synchronization is disabled, changes to the attributes of this component affect
    /// only locally and will not be pushed to network.
    void SetNetworkSyncEnabled(bool enabled);

    /// Returns true if network synchronization of the attributes of this component is enabled.
    bool GetNetworkSyncEnabled() const { return network_sync_; }

    /// Sets the default mode for attribute change operations
    void SetUpdateMode(AttributeChange::Type defaultmode);
    
    /// Gets the default mode for attribute change operations
    AttributeChange::Type GetUpdateMode() const { return updatemode_; }

    /// Returns true is this component supports XML serialization. \todo In the future, ALL Naali components
    /// must support serialization, so this function will be removed. Initially, this boolean was created
    /// to avoid all Naali components from storing themselves to RexPrimData, which couldn't be handled on
    /// the network layer. To create 'local-only' components, use the above SetNetworkSyncEnabled flag instead. -jj.
    virtual bool IsSerializable() const { return false; }

    /// Returns the number of Attributes in this component.
    int GetNumberOfAttributes() const { return attributes_.size(); }

    /// Informs this component that the value of a member Attribute of this component has changed.
    /// You may call this function manually to force Attribute change signal to
    /// occur, but it is not necessary if you use the Attribute::Set function, since
    /// it notifies this function automatically.
    /// @param attribute The attribute that was changed. The attribute passed here
    ///            must be an Attribute member of this component.
    /// @param change Informs to the component the type of change that occurred.
    ///
    /// This function calls Scene::EmitAttributeChanged and triggers the 
    /// OnAttributeChanged signal of this component.
    /// 
    /// This function is called by IAttribute::Changed whenever the value in that
    /// attribute is changed.
    void AttributeChanged(IAttribute* attribute, AttributeChange::Type change);

    /// Informs this component that the value of a member Attribute of this component has changed.
    /// You may call this function manually to force Attribute change signal to
    /// occur, but it is not necessary if you use the Attribute::Set function, since
    /// it notifies this function automatically.
    /// @param attributeName Name of the attribute that changed. Note: this is a no-op
    ///        if the named attribute is not found.
    /// @param change Informs to the component the type of change that occurred.
    ///
    /// This function calls Scene::EmitAttributeChanged and triggers the 
    /// OnAttributeChanged signal of this component.
    void AttributeChanged(const QString& attributeName, AttributeChange::Type change);

    /// Informs that every attribute in this Component has changed with the change
    /// you specify. If change is Replicate, or it is Default and the UpdateMode is Replicate,
    /// every attribute will be synced to the network.
    ///
    /// \todo Triggers also the deprecated OnChanged() signal. That will be removed in the future.
    /// Do not rely on it.
    void ComponentChanged(AttributeChange::Type change);

    /// Returns the Entity this Component is part of.
    /// \note Calling this function will return null if it is called in the ctor of this Component. This is
    ///       because the parent entity has not yet been set with a call to SetParentEntity at that point.
    Scene::Entity* GetParentEntity() const;

    //! Sets whether component is temporary. Temporary components won't be saved when the scene is saved.
    void SetTemporary(bool enable);
     
    //! Returns whether component is temporary. Temporary components won't be saved when the scene is saved.
    /*! Note: if parent entity is temporary, this returns always true regardless of the component's temporary flag
     */
    bool IsTemporary() const;
    
public:
    /// Returns the list of all Attributes in this component for reflection purposes.
    const AttributeVector& GetAttributes() const { return attributes_; }

    /// Finds and returns an attribute of type 'Attribute<T>' and given name.
    /// @param T The Attribute type to look for.
    /// @param name The name of the attribute.
    /// @return If there exists an attribute of type 'Attribute<T>' which has the given name, a pointer to
    ///         that attribute is returned, otherwise returns null.
    template<typename T> Attribute<T> *GetAttribute(const std::string &name) const
    {
        for(size_t i = 0; i < attributes_.size(); ++i)
            if (attributes_[i]->GetNameString() == name)
                return dynamic_cast<Attribute<T> *>(&attributes_[i]);
        return 0;
    }

    /// Serializes this component and all its Attributes to the given XML document.
    /// @param doc The XML document to serialize this component to.
    /// @param base_element Points to the <entity> element of the document doc. This element is the
    ///            Entity that owns this component. This component will be serialized as a child tree
    ///            of this element.
    virtual void SerializeTo(QDomDocument& doc, QDomElement& base_element) const;

    /// Deserializes this component from the given XML document.
    /// @param element Points to the <component> element that is the root of the serialized form of this Component.
    /// @param change Specifies the source of this change. This field controls whether the deserialization
    ///              was initiated locally and must be replicated to network, or if the change was received from
    ///              the network and only local application of the data suffices.
    virtual void DeserializeFrom(QDomElement& element, AttributeChange::Type change);

    //! Serialize attributes to binary
    /*! Note: does not include syncmode, typename or name. These are left for higher-level logic, and
        it depends on the situation if they are needed or not
     */
    virtual void SerializeToBinary(kNet::DataSerializer& dest) const;

    //! Deserialize attributes from binary
    /*! Note: does not include syncmode, typename or name. These are left for higher-level logic, and
        it depends on the situation if they are needed or not
     */
    virtual void DeserializeFromBinary(kNet::DataDeserializer& source, AttributeChange::Type change);

    /// Handles an event. Override in your own module if you want to receive events. Do not call.
    /** @param category_id Category id of the event
        @param event_id Id of the event
        @param data Event data, or 0 if no data passed.
        @return True if the event was handled and is not to be propagated further.
        For more information, see @ref EventSystem.
        @note This member is deprecated and will be removed in the future.
    */
    virtual bool HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data) { return false; }

    /// Returns an Attribute of this component with the given Name. This function iterates through the 
    /// attribute vector and tries to find a member attribute with the given name.
    /// @param The name of the attribute to look for.
    /// @return A pointer to the attribute, or null if no attribute with the given name exists.
    IAttribute* GetAttribute(const QString &name) const;
    // NOTE: was made a slot, but interfered with a slot with the same name in EC_DynamicComponent, and this version
    // doesn't work right for py&js 'cause doesn't return a QVariant .. so not a slot now as a temporary measure.

signals:
    /// This signal is emitted when any Attribute of this component has changed.
    /// Users may freely register to this signal to get notifications of the changes to this Component.
    /// \todo This member is deprecated and will be removed. You can achieve the exact same thing by 
    /// connecting to OnAttributeChanged and just ignoring the parameters of that signal.
    void OnChanged();

    /// This signal is emitted when an Attribute of this Component has changed. 
    void OnAttributeChanged(IAttribute* attribute, AttributeChange::Type change);

    ///\todo In the future, provide a method of listening to a change of specific Attribute, instead of having to
    /// always connect to the above function and if(...)'ing if it was the change we were interested in.

    /// This signal is emitted when the name of this Component has changed. Use this signal to keep track of
    /// a component with specified custom name.
    void OnComponentNameChanged(const QString &new_name, const QString &old_name);

    /// This signal is emitted when this Component is attached to its owning Entity.
    void ParentEntitySet();

    /// This signal is emitted when this Component is detached from its parent, i.e. the new parent is set to null.
    void ParentEntityDetached();

protected:
    /// Helper function for starting component serialization.
    /// This function creates an XML element <component> with the name of this component, adds it to the document, and returns it.
    QDomElement BeginSerialization(QDomDocument& doc, QDomElement& base_element) const;

    /// Helper function for adding an attribute to the component xml serialization.
    void WriteAttribute(QDomDocument& doc, QDomElement& comp_element, const QString& name, const QString& value) const;

    /// Helper function for adding an attribute and it's type to the component xml serialization.
    void WriteAttribute(QDomDocument& doc, QDomElement& comp_element, const QString& name, const QString& value, const QString &type) const;

    /// Helper function for starting deserialization. 
    /// Checks that xml element contains the right kind of EC, and if it is right, sets the component name.
    /// Otherwise returns false and does nothing.
    bool BeginDeserialization(QDomElement& comp_element);

    /// Helper function for getting an attribute from serialized component.
    QString ReadAttribute(QDomElement& comp_element, const QString &name) const;

    /// Helper function for getting a attribute type from serialized component.
    QString ReadAttributeType(QDomElement& comp_element, const QString &name) const;

    /// Points to the Entity this Component is part of, or null if this Component is not attached to any Entity.
    Scene::Entity* parent_entity_;

    /// The name of this component. By default the name of a component is an empty string.
    QString name_;

    /// Attribute list for introspection/reflection.
    AttributeVector attributes_;

    /// Network sync enable flag
    bool network_sync_;
    
    /// Default update mode for attribute changes
    AttributeChange::Type updatemode_;

    /// Framework pointer. Needed to be able to perform important uninitialization etc. even when not in an entity.
    Foundation::Framework* framework_;

    /// Temporary-flag
    bool temporary_;
    
private:
    /// Called by IAttribute on initialization of each attribute
    void AddAttribute(IAttribute* attr) { attributes_.push_back(attr); }
    
};

#endif
