// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"
#include "CoreDefines.h"
#include "SceneFwd.h"
#include "AttributeChangeType.h"
#include "EntityAction.h"
#include "UniqueIdGenerator.h"
#include "Math/float3.h"
#include "SceneDesc.h"

#include <QObject>
#include <QVariant>

#include <map>

class Framework;
/// @todo Not nice: UserConnection is a class from TundraProtocolModule, so Scene core API "depends" on it currently.
/// Maybe have some kind of UserConnection interface class defined in Framework and use that instead.
class UserConnection;
class QDomDocument;

/// A collection of entities which form an observable world.
/** Acts as a factory for all entities.
    Has subsystem-specific worlds, such as rendering and physics, as dynamic properties.

    To create, access and remove scenes, see SceneAPI.

    \ingroup Scene_group */
class TUNDRACORE_API Scene : public QObject, public enable_shared_from_this<Scene>
{
    Q_OBJECT
    Q_PROPERTY(QString name READ Name)
    Q_PROPERTY(bool viewEnabled READ ViewEnabled)
    Q_PROPERTY(bool authority READ IsAuthority)
    Q_PROPERTY(EntityMap entities READ Entities)

public:
    ~Scene();

    typedef std::map<entity_id_t, EntityPtr> EntityMap; ///< Typedef for an entity map.
    typedef EntityMap::iterator iterator; ///< entity iterator, see begin() and end()
    typedef EntityMap::const_iterator const_iterator;///< const entity iterator. see begin() and end()

    /// Returns name of the scene.
    const QString &Name() const { return name_; }

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
    bool operator == (const Scene &other) const { return Name() == other.Name(); }

    /// Returns true if the two scenes have different names
    bool operator != (const Scene &other) const { return !(*this == other); }

    /// Order by scene name
    bool operator < (const Scene &other) const { return Name() < other.Name(); }

    /// Return a subsystem world (OgreWorld, PhysicsWorld)
    template <class T>
    shared_ptr<T> GetWorld() const
    {
        QVariant pr = this->property(T::PropertyName());
        QObject *qo = pr.value<QObject*>();
        T* rawPtr = checked_static_cast<T*>(qo);
        return rawPtr ? rawPtr->shared_from_this() : shared_ptr<T>();
    }

    /// Forcibly changes id of an existing entity. If there already is an entity with the new id, it will be purged
    /** @note Called by scenesync. This will not trigger any signals
        @param old_id Old id of the existing entity
        @param new_id New id to set */
    void ChangeEntityId(entity_id_t old_id, entity_id_t new_id);

    /// Starts an attribute interpolation
    /** @param attr Attribute inside a static-structured component.
        @param endvalue Same kind of attribute holding the endpoint value. You must dynamically allocate this yourself, but Scene
               will always take care of deleting it.
        @param length Time length
        @return true if successful (attribute must be in interpolated mode (set in metadata), must be in component, component 
                must be static-structured, component must be in an entity which is in a scene, scene must be us) */
    bool StartAttributeInterpolation(IAttribute* attr, IAttribute* endvalue, float length);

    /// Ends an attribute interpolation. The last set value will remain.
    /** @param attr Attribute inside a static-structured component.
        @return true if an interpolation existed */
    bool EndAttributeInterpolation(IAttribute* attr);

    /// Ends all attribute interpolations
    void EndAllAttributeInterpolations();

    /// Processes all running attribute interpolations. LocalOnly change will be used.
    /** @param frametime Time step */
    void UpdateAttributeInterpolations(float frametime);

    /// See if scene is currently performing interpolations, to differentiate between interpolative & non-interpolative attribute changes.
    bool IsInterpolating() const { return interpolating_; }

    /// Returns Framework
    Framework *GetFramework() const { return framework_; }

    /// Inspects file and returns a scene description structure from the contents of XML file.
    /** @param filename File name. */
    SceneDesc CreateSceneDescFromXml(const QString &filename) const;
    /// @overload
    /** @param data XML data to be processed.
        @param sceneDesc Initialized SceneDesc with filename prepared. */
    SceneDesc CreateSceneDescFromXml(QByteArray &data, SceneDesc &sceneDesc) const;

    /// Inspects file and returns a scene description structure from the contents of binary file.
    /** @param filename File name. */
    SceneDesc CreateSceneDescFromBinary(const QString &filename) const;
    /// @overload
    /** @param data Binary data to be processed. */
    SceneDesc CreateSceneDescFromBinary(QByteArray &data, SceneDesc &sceneDesc) const;

    /// Inspects .js file content for dependencies and adds them to sceneDesc.assets
    ///@todo This function is a duplicate copy of void ScriptAsset::ParseReferences(). Delete this code. -jj.
    /** @param filePath. Path to the file that is opened for inspection.
        @param SceneDesc. Scene description struct ref, found asset dependencies will be added here.
        @todo Make one implementation of this to a common place that EC_Script and Scene can use.
        @note If the way we introduce js dependencies (!ref: and engine.IncludeFile()) changes, this function needs to change too. */
    void SearchScriptAssetDependencies(const QString &filePath, SceneDesc &sceneDesc) const;

    /// Creates scene content from scene description.
    /** @param desc Scene description.
        @param useEntityIDsFromFile If true, the created entities will use the Entity IDs from the original file.
                  If the scene contains any previous entities with conflicting IDs, those are removed. If false, the entity IDs from the files are ignored,
                  and new IDs are generated for the created entities.
        @param change Change type that will be used, when removing the old scene, and deserializing the new
        @return List of created entities. */
    QList<Entity *> CreateContentFromSceneDesc(const SceneDesc &desc, bool useEntityIDsFromFile, AttributeChange::Type change);

    /// Emits notification of an attribute changing. Called by IComponent.
    /** @param comp Component pointer
        @param attribute Attribute pointer
        @param change Change signalling mode */
    void EmitAttributeChanged(IComponent* comp, IAttribute* attribute, AttributeChange::Type change);

    /// Emits notification of an attribute having been created. Called by IComponent's with dynamic structure
    /** @param comp Component pointer
        @param attribute Attribute pointer
        @param change Change signalling mode */
    void EmitAttributeAdded(IComponent* comp, IAttribute* attribute, AttributeChange::Type change);

    /// Emits notification of an attribute about to be deleted. Called by IComponent's with dynamic structure
    /** @param comp Component pointer
        @param attribute Attribute pointer
        @param change Change signalling mode */
     void EmitAttributeRemoved(IComponent* comp, IAttribute* attribute, AttributeChange::Type change);

    /// Emits a notification of a component being added to entity. Called by the entity
    /** @param entity Entity pointer
        @param comp Component pointer
        @param change Change signalling mode */
    void EmitComponentAdded(Entity* entity, IComponent* comp, AttributeChange::Type change);

    /// Emits a notification of a component being removed from entity. Called by the entity
    /** @param entity Entity pointer
        @param comp Component pointer
        @param change Change signalling mode
        @note This is emitted before just before the component is removed. */
    void EmitComponentRemoved(Entity* entity, IComponent* comp, AttributeChange::Type change);

    /// Emits a notification of an entity being removed.
    /** @note the entity pointer will be invalid shortly after!
        @param entity Entity pointer
        @param change Change signalling mode */
    void EmitEntityRemoved(Entity* entity, AttributeChange::Type change);

    /// Emits a notification of an entity action being triggered.
    /** @param entity Entity pointer
        @param action Name of the action
        @param params Parameters
        @param type Execution type. */
    void EmitActionTriggered(Entity *entity, const QString &action, const QStringList &params, EntityAction::ExecTypeField type);

    /// Emits a notification of an entity creation acked by the server, and the entity ID changing as a result. Called by SyncManager
    void EmitEntityAcked(Entity* entity, entity_id_t oldId);

    /// Emits a notification of a component creation acked by the server, and the component ID changing as a result. Called by SyncManager
    void EmitComponentAcked(IComponent* component, component_id_t oldId);

public slots:
    /// Creates new entity that contains the specified components.
    /** Entities should never be created directly, but instead created with this function.

        To create an empty entity, omit the components parameter.

        @param id Id of the new entity. Specify 0 to use the next free ID
        @param components Optional list of component names the entity will use. If omitted or the list is empty, creates an empty entity.
        @param change Notification/network replication mode
        @param replicated Whether entity is replicated. Default true.
        @param componentsReplicated Whether components will be replicated. Default true */
    EntityPtr CreateEntity(entity_id_t id = 0, const QStringList &components = QStringList(),
        AttributeChange::Type change = AttributeChange::Default, bool replicated = true, bool componentsReplicated = true);

    /// Creates new local entity that contains the specified components
    /** Entities should never be created directly, but instead created with this function.

        To create an empty entity omit components parameter.

        @param components Optional list of component names the entity will use. If omitted or the list is empty, creates an empty entity.
        @param change Notification/network replication mode
        @param defaultNetworkSync Whether components will have network sync. Default true */
    EntityPtr CreateLocalEntity(const QStringList &components = QStringList(),
        AttributeChange::Type change = AttributeChange::Default, bool componentsReplicated = true);

    /// Returns scene up vector. For now it is a compile-time constant
    float3 UpVector() const;

    /// Returns scene right vector. For now it is a compile-time constant
    float3 RightVector() const;

    /// Returns scene forward vector. For now it is a compile-time constant
    float3 ForwardVector() const;

    /// Is scene view enabled (i.e. rendering-related components actually create stuff).
    /** @todo Exposed as Q_PROPERTY, doesn't need to be a slot. */
    bool ViewEnabled() const { return viewEnabled_; }

    /// Is scene authoritative ie. a server or standalone scene
    /** @todo Exposed as Q_PROPERTY, doesn't need to be a slot. */
    bool IsAuthority() const { return authority_; }

    /// Returns entity with the specified id
    /** @note Returns a shared pointer, but it is preferable to use a weak pointer, EntityWeakPtr,
        to avoid dangling references that prevent entities from being properly destroyed.
        @note O(log n)
        @sa EntityByName*/
    EntityPtr EntityById(entity_id_t id) const;

    /// Returns entity with the specified name.
    /** @note The name of the entity is stored in a component EC_Name. If this component is not present in the entity, it has no name.
        @note Returns a shared pointer, but it is preferable to use a weak pointer, EntityWeakPtr,
              to avoid dangling references that prevent entities from being properly destroyed.
        @note @note O(n)
        @sa EntityByName */
    EntityPtr EntityByName(const QString &name) const;

    /// Returns whether name is unique within the scene, ie. is only encountered once, or not at all.
    /** @note O(n) */
    bool IsUniqueName(const QString& name) const;

    /// Returns true if entity with the specified id exists in this scene, false otherwise
    /** @note O(log n) */
    bool HasEntity(entity_id_t id) const { return (entities_.find(id) != entities_.end()); }

    /// Removes entity with specified id
    /** The entity may not get deleted if dangling references to a pointer to the entity exists.
        @param id Id of the entity to remove
        @param change Origin of change regards to network replication.
        @return Was the entity found and removed. */
    bool RemoveEntity(entity_id_t id, AttributeChange::Type change = AttributeChange::Default);

    /// Removes all entities
    /** The entities may not get deleted if dangling references to a pointer to them exist.
        @param signal Whether to send signals of each delete. */
    void RemoveAllEntities(bool signal = true, AttributeChange::Type change = AttributeChange::Default);

    /// Gets and allocates the next free entity id.
    entity_id_t NextFreeId();

    /// Gets and allocates the next free entity id.
    entity_id_t NextFreeIdLocal();

    /// Returns list of entities with a specific component present.
    /** @param typeName Type name of the component
        @param name Name of the component, optional.
        @note O(n) */
    EntityList EntitiesWithComponent(const QString &typeName, const QString &name = "") const;

    /// Performs a regular expression matching through the entities, and returns a list of the matched entities
    /** @param pattern Regular expression to be matched
        @note Wildcards can be escaped with '\' character*/
    EntityList FindEntities(const QRegExp &pattern) const;
    EntityList FindEntities(const QString &pattern) const; /**< @overload @param pattern String pattern with wildcards*/

    /// Performs a search through the entities, and returns a list of all the entities that contain 'substring' in their names
    /** @param substring String to be searched*/
    EntityList FindEntitiesContaining(const QString &substring) const;

    /// Loads the scene from XML.
    /** @param filename File name
        @param clearScene Do we want to clear the existing scene.
        @param useEntityIDsFromFile If true, the created entities will use the Entity IDs from the original file. 
                  If the scene contains any previous entities with conflicting IDs, those are removed. If false, the entity IDs from the files are ignored,
                  and new IDs are generated for the created entities.
        @param change Change type that will be used, when removing the old scene, and deserializing the new
        @return List of created entities. */
    QList<Entity *> LoadSceneXML(const QString& filename, bool clearScene, bool useEntityIDsFromFile, AttributeChange::Type change);

    /// Returns scene content as an XML string.
    /** @param getTemporary Are temporary entities wanted to be included.
        @param getLocal Are local entities wanted to be included.
        @return The scene XML as a byte array string. */
    QByteArray GetSceneXML(bool getTemporary, bool getLocal) const;

    /// Saves the scene to XML.
    /** @param filename File name
        @param saveTemporary Are temporary entities wanted to be included.
        @param saveLocal Are local entities wanted to be included.
        @return true if successful */
    bool SaveSceneXML(const QString& filename, bool saveTemporary, bool saveLocal);

    /// Loads the scene from a binary file.
    /** @param filename File name
        @param clearScene Do we want to clear the existing scene.
        @param useEntityIDsFromFile If true, the created entities will use the Entity IDs from the original file. 
                  If the scene contains any previous entities with conflicting IDs, those are removed. If false, the entity IDs from the files are ignored,
                  and new IDs are generated for the created entities.
        @param change Change type that will be used, when removing the old scene, and deserializing the new
        @return List of created entities. */
    QList<Entity *> LoadSceneBinary(const QString& filename, bool clearScene, bool useEntityIDsFromFile, AttributeChange::Type change);

    /// Save the scene to binary
    /** @param filename File name
        @param saveTemporary Are temporary entities wanted to be included.
        @param saveLocal Are local entities wanted to be included.
        @return true if successful */
    bool SaveSceneBinary(const QString& filename, bool saveTemporary, bool saveLocal);

    /// Creates scene content from XML.
    /** @param xml XML document as string.
        @param useEntityIDsFromFile If true, the created entities will use the Entity IDs from the original file.
                  If the scene contains any previous entities with conflicting IDs, those are removed. If false, the entity IDs from the files are ignored,
                  and new IDs are generated for the created entities.
        @param change Change type that will be used, when removing the old scene, and deserializing the new
        @return List of created entities. */
    QList<Entity *> CreateContentFromXml(const QString &xml, bool useEntityIDsFromFile, AttributeChange::Type change);
    QList<Entity *> CreateContentFromXml(const QDomDocument &xml, bool useEntityIDsFromFile, AttributeChange::Type change); /**< @overload @param xml XML document. */

    /// Creates scene content from binary file.
    /** @param filename File name.
        @param useEntityIDsFromFile If true, the created entities will use the Entity IDs from the original file.
                  If the scene contains any previous entities with conflicting IDs, those are removed. If false, the entity IDs from the files are ignored,
                  and new IDs are generated for the created entities.
        @param change Change type that will be used, when removing the old scene, and deserializing the new
        @return List of created entities. */
    QList<Entity *> CreateContentFromBinary(const QString &filename, bool useEntityIDsFromFile, AttributeChange::Type change);
    QList<Entity *> CreateContentFromBinary(const char *data, int numBytes, bool useEntityIDsFromFile, AttributeChange::Type change); /**< @overload @param data Data buffer @param numBytes Data size. */

    /// Checks whether editing an entity is allowed.
    /** Emits AboutToModifyEntity.
        @user entity Connection that is requesting permission to modify an entity.
        @param entity Entity that is requested to be modified. */
    bool AllowModifyEntity(UserConnection *user, Entity *entity);

    /// Emits a notification of an entity having been created
    /** Creates are also automatically signalled at the end of frame, so you do not necessarily need to call this.
        @param entity Entity pointer
        @param change Change signalling mode */
    void EmitEntityCreated(Entity *entity, AttributeChange::Type change = AttributeChange::Default);

    // DEPRECATED function signatures
    EntityPtr GetEntity(entity_id_t id) const { return EntityById(id); } /**< @deprecated Use EntityById @todo Add warning print, remove in some distant future */
    EntityPtr GetEntityByName(const QString& name) const { return EntityByName(name); } /**< @deprecated Use EntityByName  @todo Add warning print, remove in some distant future */
    EntityList GetEntitiesWithComponent(const QString &typeName, const QString &name = "") const { return EntitiesWithComponent(typeName, name); } ///< @deprecated Use EntitiesWithComponent @todo Add warning print, remove in some distant future */
    EntityList GetAllEntities() const; /**< @deprecated @todo Add warning print, remove in some distant future */
    QVariantList GetEntityIdsWithComponent(const QString &typeName) const; /**< @deprecated Use EntitiesWithComponent instead @todo Remove. */
    Entity* GetEntityRaw(uint id) const { return GetEntity(id).get(); } /**< @deprecated Use EntityById @todo Remove */
    bool DeleteEntityById(uint id, AttributeChange::Type change = AttributeChange::Default) { return RemoveEntity((entity_id_t)id, change); } /**< @deprecated Use RemoveEntity @todo Remove */
    bool RemoveEntityRaw(int entityid, AttributeChange::Type change = AttributeChange::Default) { return RemoveEntity(entityid, change); } /**< @deprecated Use RemoveEntity @todo Remove */
    EntityMap Entities() /*non-const intentionally*/ { return entities_; } /**< @deprecated use const version Entities or 'entities' instead. @todo Add deprecation print. @todo Remove. */
    QByteArray GetEntityXml(Entity *entity) const; /**< @deprecated Use Entity::SerializeToXMLString. @todo Remove */

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
    /** @note Entity::IsTemporary() information might not be accurate yet, as it depends on the method that was used to create the entity. */
    void EntityCreated(Entity* entity, AttributeChange::Type change);

    /// Signal when an entity deleted
    void EntityRemoved(Entity* entity, AttributeChange::Type change);

    /// A entity creation has been acked by the server and assigned a proper replicated ID
    void EntityAcked(Entity* entity, entity_id_t oldId);

    /// An entity's temporary state has been toggled
    void EntityTemporaryStateToggled(Entity* entity);

    /// A component creation into an entity has been acked by the server and assigned a proper replicated ID
    void ComponentAcked(IComponent* comp, component_id_t oldId);

    /// Emitted when entity action is triggered.
    /** @param entity Entity for which action was executed.
        @param action Name of action that was triggered.
        @param params Parameters of the action.
        @param type Execution type.

        @note Use case-insensitive comparison for checking name of the @c action ! */
    void ActionTriggered(Entity *entity, const QString &action, const QStringList &params, EntityAction::ExecTypeField type);

    /// Emitted when an entity is about to be modified:
    void AboutToModifyEntity(ChangeRequest* req, UserConnection* user, Entity* entity);

    /// Emitted when being destroyed
    void Removed(Scene* scene);

    /// Signal when the whole scene is cleared
    void SceneCleared(Scene* scene);

private slots:
    /// Handle frame update. Signal this frame's entity creations.
    void OnUpdated(float frameTime);

private:
    Q_DISABLE_COPY(Scene);
    friend class ::SceneAPI;
    
    /// Constructor.
    /** @param name Name of the scene.
        @param fw Parent framework.
        @param viewEnabled Whether the scene is view enabled.
        @param authority Whether the scene has authority ie. a singleuser or server scene, false for network client scenes */
    Scene(const QString &name, Framework *fw, bool viewEnabled, bool authority);

    /// Container for an ongoing attribute interpolation
    struct AttributeInterpolation
    {
        AttributeInterpolation() : dest(0), start(0), end(0), time(0.0f), length(0.0f) {}
        ///\todo The raw IAttribute pointers are unsafe. Access to them must be guarded by first checking if the component weak pointer has not expired.
        IAttribute* dest;
        IAttribute* start;
        IAttribute* end;
        ComponentWeakPtr comp;
        float time;
        float length;
    };

    UniqueIdGenerator idGenerator_; ///< Entity ID generator
    EntityMap entities_; ///< All entities in the scene.
    Framework *framework_; ///< Parent framework.
    QString name_; ///< Name of the scene.
    bool viewEnabled_; ///< View enabled -flag.
    bool interpolating_; ///< Currently doing interpolation-flag.
    bool authority_; ///< Authority -flag
    std::vector<AttributeInterpolation> interpolations_; ///< Running attribute interpolations.
    std::vector<std::pair<EntityWeakPtr, AttributeChange::Type> > entitiesCreatedThisFrame_; ///< Entities to signal for creation at frame end.
};
