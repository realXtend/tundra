// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "SceneFwd.h"
#include "AttributeChangeType.h"
#include "EntityAction.h"
#include "SceneDesc.h"
#include "Vector3D.h"

#include <QObject>
#include <QVariant>

#include <boost/enable_shared_from_this.hpp>

class Framework;
class SceneAPI;
class QDomDocument;

/// Container for an ongoing attribute interpolation
struct AttributeInterpolation
{
    AttributeInterpolation() : dest(0), start(0), end(0), time(0.0f), length(0.0f) {}
    ///\todo The raw IAttribute pointers are unsafe.
    IAttribute* dest;
    IAttribute* start;
    IAttribute* end;
    entity_id_t entityId;
    uint compTypeId;
    QString compName;
    float time;
    float length;
};

/// Acts as a generic scene graph for all entities in the world.
/** Contains all entities in the world.
    Acts as a factory for all entities.

    To create, access and remove scenes, see SceneAPI.

    \ingroup Scene_group
*/
class SceneManager : public QObject, public boost::enable_shared_from_this<SceneManager>
{
    Q_OBJECT
    Q_PROPERTY(QString name READ Name)
    Q_PROPERTY(bool viewEnabled READ ViewEnabled)

public:
    ~SceneManager();

    typedef std::map<entity_id_t, EntityPtr> EntityMap; ///< Typedef for an entity map.
    typedef EntityMap::iterator iterator; ///< entity iterator, see begin() and end()
    typedef EntityMap::const_iterator const_iterator;///< const entity iterator. see begin() and end()

    /// Returns iterator to the beginning of the entities.
    iterator begin() { return iterator(entities_.begin()); }

    /// Returns iterator to the end of the entities.
    iterator end() { return iterator(entities_.end()); }

    /// Returns constant iterator to the beginning of the entities.
    const_iterator begin() const { return const_iterator(entities_.begin()); }

    /// Returns constant iterator to the end of the entities.
    const_iterator end() const { return const_iterator(entities_.end()); }

    /// Returns entity map for introspection purposes
    const EntityMap &Entities() const { return entities_; }

    /// Returns true if the two scenes have the same name
    bool operator == (const SceneManager &other) const { return Name() == other.Name(); }

    /// Returns true if the two scenes have different names
    bool operator != (const SceneManager &other) const { return !(*this == other); }

    /// Order by scene name
    bool operator < (const SceneManager &other) const { return Name() < other.Name(); }

    /// Return a subsystem world (OgreWorld, PhysicsWorld)
    template <class T> boost::shared_ptr<T> GetWorld() const { T* rawPtr = checked_static_cast<T*>(property(T::PropertyNameStatic()).value<QObject*>()); return rawPtr ? rawPtr->shared_from_this() : boost::shared_ptr<T>(); }

public slots:
    /// Creates new entity that contains the specified components
    /** Entities should never be created directly, but instead created with this function.

        To create an empty entity omit components parameter.

        @param id Id of the new entity. Use GetNextFreeId() or GetNextFreeIdLocal()
        @param components Optional list of component names the entity will use. If omitted or the list is empty, creates an empty entity.
        @param change Notification/network replication mode
        @param defaultNetworkSync Whether components will have network sync. Default true
    */
    EntityPtr CreateEntity(entity_id_t id = 0, const QStringList &components = QStringList(),
        AttributeChange::Type change = AttributeChange::Default, bool defaultNetworkSync = true);

    /// Creates new local entity that contains the specified components
    /** Entities should never be created directly, but instead created with this function.

        To create an empty entity omit components parameter.

        @param components Optional list of component names the entity will use. If omitted or the list is empty, creates an empty entity.
        @param change Notification/network replication mode
        @param defaultNetworkSync Whether components will have network sync. Default true
    */
    EntityPtr CreateLocalEntity(const QStringList &components = QStringList(),
        AttributeChange::Type change = AttributeChange::Default, bool defaultNetworkSync = true);

    /// Forcibly changes id of an existing entity. If there already is an entity with the new id, it will be purged
    /** Note: this is meant as a response for a server-authoritative message to change the id of a client-created entity,
        and this change in itself will not be replicated
        @param old_id Old id of the existing entity
        @param new_id New id to set
    */
    void ChangeEntityId(entity_id_t old_id, entity_id_t new_id);

    /// Starts an attribute interpolation
    /** @param attr Attribute inside a static-structured component.
        @param endvalue Same kind of attribute holding the endpoint value. You must dynamically allocate this yourself, but SceneManager
               will always take care of deleting it.
        @param length Time length
        @return true if successful (attribute must be in interpolated mode (set in metadata), must be in component, component 
                must be static-structured, component must be in an entity which is in a scene, scene must be us)
    */
    bool StartAttributeInterpolation(IAttribute* attr, IAttribute* endvalue, float length);

    /// Ends an attribute interpolation. The last set value will remain.
    /** @param attr Attribute inside a static-structured component.
        @return true if an interpolation existed
    */
    bool EndAttributeInterpolation(IAttribute* attr);

    /// Ends all attribute interpolations
    void EndAllAttributeInterpolations();

    /// Processes all running attribute interpolations. LocalOnly change will be used.
    /** @param frametime Time step
    */
    void UpdateAttributeInterpolations(float frametime);

    /// See if scene is currently performing interpolations, to differentiate between interpolative & non-interpolative attribute changes.
    bool IsInterpolating() const { return interpolating_; }

    /// Returns Framework
    Framework *GetFramework() const { return framework_; }

    /// Inspects file and returns a scene description structure from the contents of XML file.
    /** @param filename File name.
    */
    SceneDesc GetSceneDescFromXml(const QString &filename) const;

    /// Inspects xml data and returns a scene description structure from the contents of XML data.
    /** @param data Data to be processed.
        @param sceneDesc Initialized SceneDesc with filename and enum type prepared.
    */
    SceneDesc GetSceneDescFromXml(QByteArray &data, SceneDesc &sceneDesc) const;

    /// Inspects file and returns a scene description structure from the contents of binary file.
    /** @param filename File name.
    */
    SceneDesc GetSceneDescFromBinary(const QString &filename) const;

    /// Inspects binary data and returns a scene description structure from the contents of binary data.
    /** @param data Data to be processed.
    */
    SceneDesc GetSceneDescFromBinary(QByteArray &data, SceneDesc &sceneDesc) const;

    /// Inspects .js file content for dependencies and adds them to sceneDesc.assets
    ///@todo This function is a duplicate copy of void ScriptAsset::ParseReferences(). Delete this code. -jj.
    /** @param filePath. Path to the file that is opened for inspection.
        @param SceneDesc. Scene description struct ref, found asset dependencies will be added here.
        @todo Make one implementation of this to a common place that EC_Script and SceneManager can use.
        @note If the way we introduce js dependencies (!ref: and engine.IncludeFile()) changes, this function needs to change too.
    */
    void SearchScriptAssetDependencies(const QString &filePath, SceneDesc &sceneDesc) const;

    /// Returns scene up vector. For now it is a compile-time constant
    Vector3df GetUpVector() const;

    /// Returns scene right vector. For now it is a compile-time constant
    Vector3df GetRightVector() const;

    /// Returns scene forward vector. For now it is a compile-time constant
    Vector3df GetForwardVector() const;

    /// @todo Clean these overload functions created for PythonQt and QtScript compatibility as much as possible.
    //  For documentation, see the plain C++ public methods above.

    /// @todo These can be removed when otherwise a good time, the entity_id_t has been exposed to js.
    bool HasEntityId(uint id) const { return HasEntity((entity_id_t)id); }
    uint NextFreeId() { return (uint)GetNextFreeId(); }
    uint NextFreeIdLocal() { return (uint)GetNextFreeIdLocal(); }
    
    Entity* CreateEntityRaw(uint id = 0, const QStringList &components = QStringList(), AttributeChange::Type change = AttributeChange::Default, bool defaultNetworkSync = true) 
        { return CreateEntity((entity_id_t)id, components, change, defaultNetworkSync).get(); }
    Entity* CreateEntityLocalRaw(const QStringList &components = QStringList(), AttributeChange::Type change = AttributeChange::LocalOnly, bool defaultNetworkSync = false)
        { return CreateEntity(NextFreeIdLocal(), components, change, defaultNetworkSync).get(); }

    Entity* GetEntityRaw(uint id) { return GetEntity(id).get(); }
    QVariantList GetEntityIdsWithComponent(const QString &type_name) const;
    QList<Entity*> GetEntitiesWithComponentRaw(const QString &type_name) const;

    void DeleteEntityById(uint id, AttributeChange::Type change = AttributeChange::Default) { RemoveEntity((entity_id_t)id, change); }

    /// Return a scene document with just the desired entity
    QByteArray GetEntityXml(Entity *entity);

    void EmitEntityCreated(Entity *entity, AttributeChange::Type change = AttributeChange::Default);
    void EmitEntityCreatedRaw(QObject *entity, AttributeChange::Type change = AttributeChange::Default);

    void RemoveEntityRaw(int entityid, AttributeChange::Type change = AttributeChange::Default) { RemoveEntity(entityid, change); }

    /// Returns IDs of loaded entities
    QVariantList LoadSceneXMLRaw(const QString &filename, bool clearScene, bool useEntityIDsFromFile, AttributeChange::Type change);

    /// Is scene view enabled (i.e. rendering-related components actually create stuff).
    bool ViewEnabled() const { return viewEnabled_; }

    /// Is scene authoritative ie. a server or standalone scene
    bool IsAuthority() const { return authority_; }
    
    /// Returns name of the scene.
    const QString &Name() const { return name_; }

    /// Returns entity with the specified id
    /** @note Returns a shared pointer, but it is preferable to use a weak pointer, EntityWeakPtr,
        to avoid dangling references that prevent entities from being properly destroyed. */
    EntityPtr GetEntity(entity_id_t id) const;

    /// Returns entity with the specified name.
    /** @note The name of the entity is stored in a component EC_Name. If this component is not present in the entity, it has no name.
        @note Returns a shared pointer, but it is preferable to use a weak pointer, EntityWeakPtr,
              to avoid dangling references that prevent entities from being properly destroyed. */
    EntityPtr GetEntityByName(const QString& name) const;

    /// Return whether name is unique within the scene, ie. is only encountered once, or not at all.
    bool IsUniqueName(const QString& name) const;

    /// Returns true if entity with the specified id exists in this scene, false otherwise
    bool HasEntity(entity_id_t id) const { return (entities_.find(id) != entities_.end()); }

    /// Remove entity with specified id
    /** The entity may not get deleted if dangling references to a pointer to the entity exists.
        @param id Id of the entity to remove
        @param change Origin of change regards to network replication
    */
    void RemoveEntity(entity_id_t id, AttributeChange::Type change = AttributeChange::Default);

    /// Remove all entities
    /** The entities may not get deleted if dangling references to a pointer to them exist.
        @param send_events whether to send events & signals of each delete
    */
    void RemoveAllEntities(bool send_events = true, AttributeChange::Type change = AttributeChange::Default);

    /// Get the next free entity id. Can be used with CreateEntity(). 
    /* These will be for networked entities, and should be assigned only by a point of authority (server) */
    entity_id_t GetNextFreeId();

    /// Get the next free local entity id. Can be used with CreateEntity().
    /* As local entities will not be network synced, there should be no conflicts in assignment. */
    entity_id_t GetNextFreeIdLocal();

    /// Return list of entities with a specific component present.
    /** @param typeName Type name of the component
        @param name Name of the component, optional.
    */
    EntityList GetEntitiesWithComponent(const QString &typeName, const QString &name = "") const;

    /// Emit notification of an attribute changing. Called by IComponent.
    /** @param comp Component pointer
        @param attribute Attribute pointer
        @param change Network replication mode
    */
    void EmitAttributeChanged(IComponent* comp, IAttribute* attribute, AttributeChange::Type change);

    /// Emit notification of an attribute having been created. Called by IComponent's with dynamic structure
    /** @param comp Component pointer
        @param attribute Attribute pointer
        @param change Network replication mode
    */
    void EmitAttributeAdded(IComponent* comp, IAttribute* attribute, AttributeChange::Type change);

    /// Emit notification of an attribute about to be deleted. Called by IComponent's with dynamic structure
    /** @param comp Component pointer
        @param attribute Attribute pointer
        @param change Network replication mode
     */
     void EmitAttributeRemoved(IComponent* comp, IAttribute* attribute, AttributeChange::Type change);
     
    /// Emit a notification of a component being added to entity. Called by the entity
    /** @param entity Entity pointer
        @param comp Component pointer
        @param change Network replication mode
    */
    void EmitComponentAdded(Entity* entity, IComponent* comp, AttributeChange::Type change);

    /// Emit a notification of a component being removed from entity. Called by the entity
    /** @param entity Entity pointer
        @param comp Component pointer
        @param change Network replication mode
        @note This is emitted before just before the component is removed.
    */
    void EmitComponentRemoved(Entity* entity, IComponent* comp, AttributeChange::Type change);

    /// Emit a notification of an entity having been created
    /** @param entity Entity pointer
        @param change Network replication mode
    */
    void EmitEntityCreated(EntityPtr entity, AttributeChange::Type change = AttributeChange::Default);

    /// Emit a notification of an entity being removed. 
    /** Note: the entity pointer will be invalid shortly after!
        @param entity Entity pointer
        @param change Network replication mode
    */
    void EmitEntityRemoved(Entity* entity, AttributeChange::Type change);

    /// Emits a notification of an entity action being triggered.
    /** @param entity Entity pointer
        @param action Name of the action
        @param params Parameters
        @param type Execution type.
    */
    void EmitActionTriggered(Entity *entity, const QString &action, const QStringList &params, EntityAction::ExecutionTypeField type);

    /// Loads the scene from XML.
    /** @param filename File name
        @param clearScene Do we want to clear the existing scene.
        @param useEntityIDsFromFile If true, the created entities will use the Entity IDs from the original file. 
                  If the scene contains any previous entities with conflicting IDs, those are removed. If false, the entity IDs from the files are ignored,
                  and new IDs are generated for the created entities.
        @param change Change type that will be used, when removing the old scene, and deserializing the new
        @return List of created entities.
    */
    QList<Entity *> LoadSceneXML(const QString& filename, bool clearScene, bool useEntityIDsFromFile, AttributeChange::Type change);

    /// Returns scene content as an XML string.
    /** @param getTemporary Are temporary entities wanted to be included.
        @param getLocal Are local entities wanted to be included.
        @return The scene XML as a byte array string.
    */
    QByteArray GetSceneXML(bool getTemporary, bool getLocal) const;

    /// Saves the scene to XML.
    /** @param filename File name
        @param saveTemporary Are temporary entities wanted to be included.
        @param saveLocal Are local entities wanted to be included.
        @return true if successful
    */
    bool SaveSceneXML(const QString& filename, bool saveTemporary, bool saveLocal);

    /// Loads the scene from a binary file.
    /** Note: will remove all existing entities
        @param filename File name
        @param clearScene Do we want to clear the existing scene.
        @param useEntityIDsFromFile If true, the created entities will use the Entity IDs from the original file. 
                  If the scene contains any previous entities with conflicting IDs, those are removed. If false, the entity IDs from the files are ignored,
                  and new IDs are generated for the created entities.
        @param change Change type that will be used, when removing the old scene, and deserializing the new
        @return List of created entities.
    */
    QList<Entity *> LoadSceneBinary(const QString& filename, bool clearScene, bool useEntityIDsFromFile, AttributeChange::Type change);

    /// Save the scene to binary
    /** @param filename File name
        @param saveTemporary Are temporary entities wanted to be included.
        @param saveLocal Are local entities wanted to be included.
        @return true if successful
    */
    bool SaveSceneBinary(const QString& filename, bool saveTemporary, bool saveLocal);

    /// Creates scene content from XML.
    /** @param xml XML document as string.
        @param useEntityIDsFromFile If true, the created entities will use the Entity IDs from the original file. 
                  If the scene contains any previous entities with conflicting IDs, those are removed. If false, the entity IDs from the files are ignored,
                  and new IDs are generated for the created entities.
        @param change Change type that will be used, when removing the old scene, and deserializing the new
        @return List of created entities.
    */
    QList<Entity *> CreateContentFromXml(const QString &xml, bool useEntityIDsFromFile, AttributeChange::Type change);

    /// This is an overloaded function.
    /** @param xml XML document.
        @param useEntityIDsFromFile If true, the created entities will use the Entity IDs from the original file. 
                  If the scene contains any previous entities with conflicting IDs, those are removed. If false, the entity IDs from the files are ignored,
                  and new IDs are generated for the created entities.
        @param change Change type that will be used, when removing the old scene, and deserializing the new
        @return List of created entities.
    */
    QList<Entity *> CreateContentFromXml(const QDomDocument &xml, bool useEntityIDsFromFile, AttributeChange::Type change);

    /// Creates scene content from binary file.
    /** @param filename File name.
        @param useEntityIDsFromFile If true, the created entities will use the Entity IDs from the original file. 
                  If the scene contains any previous entities with conflicting IDs, those are removed. If false, the entity IDs from the files are ignored,
                  and new IDs are generated for the created entities.
        @param change Change type that will be used, when removing the old scene, and deserializing the new
        @return List of created entities.
    */
    QList<Entity *> CreateContentFromBinary(const QString &filename, bool useEntityIDsFromFile, AttributeChange::Type change);

public:
    /// This is an overloaded function.
    /** @param data Data buffer.
        @param numBytes Data size.
        @param useEntityIDsFromFile If true, the created entities will use the Entity IDs from the original file. 
                  If the scene contains any previous entities with conflicting IDs, those are removed. If false, the entity IDs from the files are ignored,
                  and new IDs are generated for the created entities.
        @param change Change type that will be used, when removing the old scene, and deserializing the new
        @return List of created entities.
    */
    QList<Entity *> CreateContentFromBinary(const char *data, int numBytes, bool useEntityIDsFromFile, AttributeChange::Type change);

    /// Creates scene content from scene description.
    /** @param desc Scene description.
        @param useEntityIDsFromFile If true, the created entities will use the Entity IDs from the original file. 
                  If the scene contains any previous entities with conflicting IDs, those are removed. If false, the entity IDs from the files are ignored,
                  and new IDs are generated for the created entities.
        @param change Change type that will be used, when removing the old scene, and deserializing the new
        @return List of created entities.
    */
    QList<Entity *> CreateContentFromSceneDesc(const SceneDesc &desc, bool useEntityIDsFromFile, AttributeChange::Type change);

signals:
    /// Signal when an attribute of a component has changed
    /** Network synchronization managers should connect to this. */
    void AttributeChanged(IComponent* comp, IAttribute* attribute, AttributeChange::Type change);

    /// Signal when an attribute of a component has been added (dynamic structure components only)
    /** Network synchronization managers should connect to this. */
    void AttributeAdded(IComponent* comp, IAttribute* attribute, AttributeChange::Type change);

    /// Signal when an attribute of a component has been added (dynamic structure components only)
    /** Network synchronization managers should connect to this. */
    void AttributeRemoved(IComponent* comp, IAttribute* attribute, AttributeChange::Type change);

    /// Signal when a component is added to an entity and should possibly be replicated (if the change originates from local)
    /** Network synchronization managers should connect to this. */
    void ComponentAdded(Entity* entity, IComponent* comp, AttributeChange::Type change);

    /// Signal when a component is removed from an entity and should possibly be replicated (if the change originates from local)
    /** Network synchronization managers should connect to this */
    void ComponentRemoved(Entity* entity, IComponent* comp, AttributeChange::Type change);

    /// Signal when an entity created
    /** @note Entity::IsTemporary() information might not accurate yet, as it depends on the method that was used to create the entity. */
    void EntityCreated(Entity* entity, AttributeChange::Type change);

    /// Signal when an entity deleted
    void EntityRemoved(Entity* entity, AttributeChange::Type change);

    /// Emitted when entity action is triggered.
    /** @param entity Entity for which action was executed.
        @param action Name of action that was triggered.
        @param params Parameters of the action.
        @param type Execution type.

        @note Use case-insensitive comparison for checking name of the @c action !
    */
    void ActionTriggered(Entity *entity, const QString &action, const QStringList &params, EntityAction::ExecutionTypeField type);

    /// Emitted when being destroyed
    void Removed(SceneManager* scene);

    /// Signal when the whole scene is cleared
    void SceneCleared(SceneManager* scene);

private:
    Q_DISABLE_COPY(SceneManager);
    friend class ::SceneAPI;

    /// Default constructor, name will be empty string and authority-flag false.
    SceneManager();

    /// Constructor.
    /** @param name Name of the scene.
        @param fw Framework Parent framework.
        @param viewEnabled Whether the scene is view enabled.
        @param authority Whether the scene has authority ie. a singleuser or server scene, false for network client scenes
    */
    SceneManager(const QString &name, Framework *fw, bool viewEnabled, bool authority   );

    uint gid_; ///< Current global id for networked entities
    uint gid_local_; ///< Current id for local entities.
    EntityMap entities_; ///< All entities in the scene.
    Framework *framework_; ///< Parent framework.
    QString name_; ///< Name of the scene.
    bool viewEnabled_; ///< View enabled -flag.
    bool interpolating_; ///< Currently doing interpolation-flag.
    bool authority_; ///< Authority -flag
    std::vector<AttributeInterpolation> interpolations_; ///< Running attribute interpolations.
};
