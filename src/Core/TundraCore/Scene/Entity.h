// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"
#include "CoreTypes.h"
#include "SceneFwd.h"
#include "IComponent.h"
#include "IAttribute.h"
#include "EntityAction.h"
#include "UniqueIdGenerator.h"

#include <kNetFwd.h>

#include <QObject>
#include <QMap>

class QDomDocument;
class QDomElement;

/// Represents a single object in a Scene.
/** An entity is just a collection of components, the components define what
    the entity is and what it does.
    Entities should not be directly created, instead use Scene::CreateEntity().

    Each component type that is added to this entity is registered as
    Q_PROPERTY as in following syntax EC_Light -> light, where EC_ is cut off
    and name is converted to low case format. This allow scripter to get access to
    component using a following code "entity.mesh.SetMesh("mesh id");"
    
    @note If there are several components that have a same typename only first component
    is accessible through Q_PROPERTY and if you want to edit other same type of components
    you should use GetComponent method instead.

    When component is removed from the entity a Q_PROPERTY connection is destroyed from
    the component. In case that there are several components with the same typename, there is 
    a name check that ensures that both components names are same before Q_PROPERTY destroyed.

    @note   Entity can have multiple components with same component type name as long as
            the component names are unique.

    \ingroup Scene_group */
class TUNDRACORE_API Entity : public QObject, public enable_shared_from_this<Entity>
{
    Q_OBJECT
    Q_PROPERTY(entity_id_t id READ Id) /**< @copydoc Id */
    Q_PROPERTY(QString name READ Name WRITE SetName) /**< @copydoc Name */
    Q_PROPERTY(QString description READ Description WRITE SetDescription) /**< @copydoc Description */
    Q_PROPERTY(bool replicated READ IsReplicated) /**< @copydoc IsReplicated */
    Q_PROPERTY(bool local READ IsLocal) /**< @copydoc IsLocal */
    Q_PROPERTY(bool unacked READ IsUnacked) /**< @copydoc IsUnacked */
    Q_PROPERTY(bool temporary READ IsTemporary WRITE SetTemporary) /**< @copydoc IsTemporary */
    Q_PROPERTY(ComponentMap components READ Components) /**< @copydoc Components */

public:
    typedef std::map<component_id_t, ComponentPtr> ComponentMap; ///< Component container.
    typedef std::vector<ComponentPtr> ComponentVector; ///< Component vector container.
    typedef QMap<QString, EntityAction *> ActionMap; ///< Action container

    /// If entity has components that are still alive, they become free-floating.
    ~Entity();

    /// Returns a component with certain type, already cast to correct type, or empty pointer if component was not found
    /** If there are several components with the specified type, returns the first component found (arbitrary). */
    template <class T>
    shared_ptr<T> GetComponent() const;

    /** Returns list of components with certain class type, already cast to correct type.
        @param T Component class type.
        @return List of components with certain class type, or empty list if no components was found. */
    template <class T>
    std::vector<shared_ptr<T> > GetComponents() const;

    /// Returns a component with certain type and name, already cast to correct type, or empty pointer if component was not found
    /** @param name name of the component */
    template <class T>
    shared_ptr<T> GetComponent(const QString& name) const;

   /** Returns pointer to the first attribute with specific name.
        @param T Type name/class of the attribute.
        @param name Name of the attribute.
        @return Pointer to the attribute.
        @note Always remember to check for null pointer. */
    template<typename T>
    Attribute<T> *GetAttribute(const QString &name) const;

    /// Returns list of attributes with specific name.
    /** @param T Type name/class of the attribute.
        @param name Name of the attribute.
        @return List of attributes, or empty list if no attributes are found. */
    template<typename T>
    std::vector<Attribute<T> > GetAttributes(const QString &name) const;

    /// In the following, deserialization functions are now disabled since deserialization can't safely
    /// process the exact same data that was serialized, or it risks receiving entity ID conflicts in the scene.
    /// \todo Implement a deserialization flow that takes that into account. In the meanwhile, use Scene
    /// functions for achieving the same.

    void SerializeToBinary(kNet::DataSerializer &dst) const;
//        void DeserializeFromBinary(kNet::DataDeserializer &src, AttributeChange::Type change);

    /// Emit EnterView signal. Called by the rendering subsystem
    void EmitEnterView(IComponent* camera);
    
    /// Emit LeaveView signal. Called by the rendering subsystem
    void EmitLeaveView(IComponent* camera);

    /// Returns true if the two entities have the same id, false otherwise
    bool operator == (const Entity &other) const { return Id() == other.Id(); }

    /// Returns true if the two entities have different id, false otherwise
    bool operator != (const Entity &other) const { return !(*this == other); }

    /// comparison by id
    bool operator < (const Entity &other) const { return Id() < other.Id(); }

    /// Forcibly changes id of an existing component. If there already is a component with the new id, it will be purged
    /** @note Called by scenesync. This will not trigger any signals
        @param old_id Old id of the existing component
        @param new_id New id to set */
    void ChangeComponentId(component_id_t old_id, component_id_t new_id);
    
    /// Create a component with predefined ID. Called by SyncManager.
    /** @param typeId Unique type ID of the component.
        @param name name of the component */
    ComponentPtr CreateComponentWithId(component_id_t compId, u32 typeId, const QString &name, AttributeChange::Type change = AttributeChange::Default);

    /// introspection for the entity, returns all components
    const ComponentMap &Components() const { return components_; }

    /// Returns attribute interface pointer to attribute with specific name.
    /** @param name Name of the attribute.
        @return IAttribute pointer to the attribute.
        @note Always remember to check for null pointer. */
    IAttribute *GetAttribute(const QString  &name) const;

    /// Returns list of attributes with specific name.
    /** @param name Name of the attribute.
        @return List of attribute interface pointers, or empty list if no attributes are found. */
    AttributeVector GetAttributes(const QString &name) const;

    /// Returns actions map for introspection/reflection.
    const ActionMap &Actions() const { return actions_; }

public slots:
    /// Returns a component by ID. This is the fastest way to query, as the components are stored in a map by id.
    ComponentPtr GetComponentById(component_id_t id) const;
    /// Returns a component with type 'typeName' or empty pointer if component was not found
    /** If there are several components with the specified type, returns the first component found (arbitrary).
        @param typeName type of the component */
    ComponentPtr GetComponent(const QString &typeName) const;
    /// @overload
    /** @param typeId Unique type ID. */
    ComponentPtr GetComponent(u32 typeId) const;
    /// @overload
    /** @param name Specifies the name of the component to fetch. This can be used to distinguish between multiple instances of components of same type. */
    ComponentPtr GetComponent(const QString &typeName, const QString &name) const;
    /// @overload
    /** @param typeId The type id of the component to get.
        @param name name of the component */
    ComponentPtr GetComponent(u32 typeId, const QString &name) const;

    /// Returns a component with type 'typeName' or creates & adds it if not found. If could not create, returns empty pointer
    /** @param typeName The type string of the component to create, obtained from IComponent::TypeName().
        @param change Change signalling mode, in case component has to be created
        @param replicated Whether new component will be replicated through network
        @return Pointer to the component, or an empty pointer if the component could be retrieved or created. */
    ComponentPtr GetOrCreateComponent(const QString &typeName, AttributeChange::Type change = AttributeChange::Default, bool replicated = true);
    /// @overload
    /** @param name If specified, the component having the given name is returned, or created if it doesn't exist. */
    ComponentPtr GetOrCreateComponent(const QString &typeName, const QString &name, AttributeChange::Type change = AttributeChange::Default, bool replicated = true);
    /// @overload
    /** @param typeId Identifies the component type to create by the id of the type instead of the name. */
    ComponentPtr GetOrCreateComponent(u32 typeId, AttributeChange::Type change = AttributeChange::Default, bool replicated = true);
    /// @overload
    /** @param name name of the component */
    ComponentPtr GetOrCreateComponent(u32 typeId, const QString &name, AttributeChange::Type change = AttributeChange::Default, bool replicated = true);

    /// Returns a component with type 'typeName' or creates & adds it as local if not found. If could not create, returns empty pointer
    ComponentPtr GetOrCreateLocalComponent(const QString &typeName);
    /// @overload
    /** Returns a component with type 'typeName' and name 'name' or creates & adds it as local if not found. If could not create, returns empty pointer */
    ComponentPtr GetOrCreateLocalComponent(const QString &typeName, const QString &name);

    /// Creates a new component and attaches it to this entity. 
    /** @param typeName type of the component
        @param change Change signalling mode, in case component has to be created
        @param replicated Whether new component will be replicated through network
        @return Retuns a pointer to the newly created component, or null if creation failed. Common causes for failing to create an component
        is that a component with the same (typename, name) pair exists, or that components of the given typename are not recognized by the system. */
    ComponentPtr CreateComponent(const QString &typeName, AttributeChange::Type change = AttributeChange::Default, bool replicated = true);
    /// @overload
    /** @param name name of the component */
    ComponentPtr CreateComponent(const QString &typeName, const QString &name, AttributeChange::Type change = AttributeChange::Default, bool replicated = true);
    /// @overload
    /** @param typeId Unique type ID of the component. */
    ComponentPtr CreateComponent(u32 typeId, AttributeChange::Type change = AttributeChange::Default, bool replicated = true);
    /// @overload
    /** @param typeId Unique type ID of the component.
        @param name name of the component */
    ComponentPtr CreateComponent(u32 typeId, const QString &name, AttributeChange::Type change = AttributeChange::Default, bool replicated = true);
    
    /// Creates a local component with type 'typeName' and adds it to the entity. If could not create, return empty pointer
    ComponentPtr CreateLocalComponent(const QString &typeName);
    /// Creates a local component with type 'typeName' and name 'name' and adds it to the entity. If could not create, return empty pointer
    ComponentPtr CreateLocalComponent(const QString &typeName, const QString &name);
    
    /// Attachs an existing parentless component to this entity.
    /** A component ID will be allocated.
        Entities can contain any number of components of any type.
        It is also possible to have several components of the same type,
        although in most cases it is probably not sensible.

        Each component type that is added to this entity is registered as
        Q_PROPERTY as in following syntax EC_Light -> light, where EC_ is cutted off
        and name is converted to low case format. This allow scripter to get access to
        component using a following code "entity.mesh.SetMesh("mesh id");"

        @param component The component to add to this entity. The component must be parentless, i.e.
                      previously created using SceneAPI::CreateComponent.
        @param change Change signalling mode */
    void AddComponent(const ComponentPtr &component, AttributeChange::Type change = AttributeChange::Default);
    /// @overload
    /** Attaches an existing parentless component to this entity, using the specific ID. This variant is used by SyncManager. */
    void AddComponent(component_id_t id, const ComponentPtr &component, AttributeChange::Type change = AttributeChange::Default);

    /// Removes component from this entity.
    /** When component is removed from the entity a Q_PROPERTY connection is destroyed from
        the component. In case where there are several components with the same typename, there is 
        a name check that ensures that both components names are same before Q_PROPERTY destroyed.
        
        @param component Pointer to the component to remove
        @param change Specifies how other parts of the system are notified of this removal.
        @sa RemoveComponentById */
    void RemoveComponent(const ComponentPtr &component, AttributeChange::Type change = AttributeChange::Default); /**< @overload */
    void RemoveComponent(const QString &typeName, AttributeChange::Type change = AttributeChange::Default) { RemoveComponent(GetComponent(typeName), change); }  /**< @overload */
    void RemoveComponent(const QString &typeName, const QString &name, AttributeChange::Type change = AttributeChange::Default) { RemoveComponent(GetComponent(typeName, name), change); }  /**< @overload */
    /// Removes component by ID.
    /** @sa RemoveComponent */
    void RemoveComponentById(component_id_t id, AttributeChange::Type change = AttributeChange::Default);

    /// Returns list of components with type 'typeName' or empty list if no components were found.
    /** @param typeName type of the component */
    ComponentVector GetComponents(const QString &typeName) const;

    /// Creates clone of the entity.
    /** @param local If true, the new entity will be local entity. If false, the entity will be replicated.
        @param temporary Will the new entity be temporary.
        @return Pointer to the new entity, or null pointer if the cloning fails. */
    EntityPtr Clone(bool local, bool temporary) const;

    /// Serializes this entity and its' components to the given XML document
    /** @param doc The XML document to serialize this entity to.
        @param base_element Points to the <scene> element of this XML document. This entity will be serialized as a child to base_element.
        @param serializeTemporary Serialize temporary entities for application-specific purposes. The default value is false. */
    void SerializeToXML(QDomDocument& doc, QDomElement& base_element, bool serializeTemporary = false) const;
//        void DeserializeFromXML(QDomElement& element, AttributeChange::Type change);

    /// Serializes this entity, and returns the generated XML as a string
    /** @param serializeTemporary Serialize temporary entities for application-specific purposes. The default value is false.
        @sa SerializeToXML */
    QString SerializeToXMLString(bool serializeTemporary = false) const;
//        bool DeserializeFromXMLString(const QString &src, AttributeChange::Type change);

    /// Sets name of the entity to EC_Name component. If the component doesn't exist, it will be created.
    /** @param name Name. */
    ///\todo Doesn't need to be slot, exposed as Q_PROPERTY
    void SetName(const QString &name);

    /// Returns name of this entity if EC_Name is available, empty string otherwise.
    ///\todo Doesn't need to be slot, exposed as Q_PROPERTY
    QString Name() const;

    /// Sets description of the entity to EC_Name component. If the component doesn't exist, it will be created.
    /** @param desc Description. */
    ///\todo Doesn't need to be slot, exposed as Q_PROPERTY
    void SetDescription(const QString &desc);

    /// Returns description of this entity if EC_Name is available, empty string otherwise.
    ///\todo Doesn't need to be slot, exposed as Q_PROPERTY
    QString Description() const;

    /// Creates and registers new action for this entity, or returns an existing action.
    /** Use this function from scripting languages.
        @param name Name of the action.
        @note Never returns null pointer
        @note Never store the returned pointer. */
    EntityAction *Action(const QString &name);

    /// Find & Delete EntityAction object from EntityActions map.
    /** Use this function from scripting languages.
        @param name Name of the action. */
    void RemoveAction(const QString &name);

    /// Connects action with a specific name to a receiver object with member slot.
    /** @param name Name of the action.
        @param receiver Receiver object.
        @param member Member slot. */
    void ConnectAction(const QString &name, const QObject *receiver, const char *member);

    /// Executes an arbitrary action for all components of this entity.
    /** The components may or may not handle the action.
        @param type Execution type(s), i.e. where the actions is executed.
        @param action Name of the action.
        @param p1 1st parameter for the action, if applicable.
        @param p2 2nd parameter for the action, if applicable.
        @param p3 3rd parameter for the action, if applicable. */
    void Exec(EntityAction::ExecTypeField type, const QString &action, const QString &p1 = "", const QString &p2 = "", const QString &p3 = "");
    /// @overload
    /** @param params List of parameters for the action. */
    void Exec(EntityAction::ExecTypeField type, const QString &action, const QStringList &params);
    /// @overload
    /** Experimental overload using QVariant. Converts the variants to strings.
        @note If called from JavaScript, syntax '<targetEntity>["Exec(EntityAction::ExecTypeField,QString,QVariantList)"](2, "name", params);' must be used. */
    void Exec(EntityAction::ExecTypeField type, const QString &action, const QVariantList &params);

    /// Sets whether entity is temporary. Temporary entities won't be saved when the scene is saved.
    /** By definition, all components of a temporary entity are temporary as well. */
    ///\todo Doesn't need to be slot, exposed as Q_PROPERTY
    void SetTemporary(bool enable);

    /// Returns whether entity is temporary. Temporary entities won't be saved when the scene is saved.
    /** By definition, all components of a temporary entity are temporary as well. */
    ///\todo Doesn't need to be slot, exposed as Q_PROPERTY
    bool IsTemporary() const { return temporary_; }

    /// Returns if this entity's changes will NOT be sent over the network.
    /// An Entity is always either local or replicated, but not both.
    ///\todo Doesn't need to be slot, exposed as Q_PROPERTY
    bool IsLocal() const { return id_ >= UniqueIdGenerator::FIRST_LOCAL_ID; }

    /// Returns if this entity's changes will be sent over the network.
    /// An Entity is always either local or replicated, but not both.
    ///\todo Doesn't need to be slot, exposed as Q_PROPERTY
    bool IsReplicated() const { return id_ < UniqueIdGenerator::FIRST_LOCAL_ID; }

    /// Returns if this entity is pending a proper ID assignment from the server.
    ///\todo Doesn't need to be slot, exposed as Q_PROPERTY
    bool IsUnacked() const { return id_ >= UniqueIdGenerator::FIRST_UNACKED_ID && id_ < UniqueIdGenerator::FIRST_LOCAL_ID; }

    /// Returns the identifier string for the entity.
    /** Syntax of the string: 'Entity ID <id>' or 'Entity "<name>" (ID: <id>)' if entity has a name. */
    QString ToString() const;

    /// Same as ToString. This function is implemented to facilitate the automatic QtScript QScriptValue to string conversion.
    QString toString() const { return ToString(); }

    /// Returns the unique id of this entity
    ///\todo Doesn't need to be slot, exposed as Q_PROPERTY
    entity_id_t Id() const { return id_; }

    /// Returns framework
    Framework *GetFramework() const { return framework_; }

    /// Returns parent scene of this entity.
    Scene* ParentScene() const { return scene_; }

    // DEPRECATED:
    QObjectList ComponentsList() const; /**< @deprecated Use Components @todo Remove */
    QObjectList GetComponentsRaw(const QString &typeName) const; /**< @deprecated Use GetComponents or Components instead */
    void RemoveComponentRaw(QObject* comp); /**< @deprecated Use RemoveComponent or RemoveComponentById. */
    ComponentMap Components() /*non-const intentionally*/ { return components_; } /**< @deprecated use const version Components or 'components' instead. @todo Add deprecation print. @todo Remove. */

signals:
    /// A component has been added to the entity
    /** @note When this signal is received on new entity creation, the attributes might not be filled yet! */ 
    void ComponentAdded(IComponent* component, AttributeChange::Type change);

    /// A component has been removed from the entity
    /** @note When this signal is received on new entity creation, the attributes might not be filled yet! */ 
    void ComponentRemoved(IComponent* component, AttributeChange::Type change);

    /// Signal when this entity is deleted
    void EntityRemoved(Entity* entity, AttributeChange::Type change);

    /// Signal when this entity's temporary state has been toggled
    void TemporaryStateToggled(Entity *);

    /// The entity has entered a camera's view. Triggered by the rendering subsystem.
    void EnterView(IComponent* camera);

    /// The entity has left a camera's view. Triggered by the rendering subsystem.
    void LeaveView(IComponent* camera);

private:
    friend class Scene;

    /// Constructor
    /** @param framework Framework
        @param scene Scene this entity belongs to */
    Entity(Framework* framework, Scene* scene);

    /// Constructor that takes an id for the entity
    /** @param framework Framework
        @param id unique id for the entity.
        @param scene Scene this entity belongs to */
    Entity(Framework* framework, entity_id_t id, Scene* scene);

    /// Set new id
    void SetNewId(entity_id_t id) { id_ = id; }

    /// Set new scene
    void SetScene(Scene* scene) { scene_ = scene; }

    /// Emit a entity deletion signal. Called from Scene
    void EmitEntityRemoved(AttributeChange::Type change);

    UniqueIdGenerator idGenerator_; ///< Component ID generator
    ComponentMap components_; ///< a list of all components
    entity_id_t id_; ///< Unique id for this entity
    Framework* framework_; ///< Pointer to framework
    Scene* scene_; ///< Pointer to scene
    ActionMap actions_; ///< Map of registered entity actions.
    bool temporary_; ///< Temporary-flag
};

#include "Entity.inl"
