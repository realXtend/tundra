// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_SceneManager_SceneManager_h
#define incl_SceneManager_SceneManager_h

#include "CoreStdIncludes.h"
#include "CoreAnyIterator.h"
#include "Entity.h"
#include "ComponentInterface.h"
#include <QObject>
#include <qvariant.h>
#include <QStringList>

namespace Foundation
{
    class AttributeInterface;
}

namespace Scene
{
    class SceneManager;
    typedef boost::shared_ptr<SceneManager> ScenePtr;
    typedef boost::weak_ptr<SceneManager> SceneWeakPtr;
    typedef std::list<EntityPtr> EntityList;
    typedef std::list<EntityPtr>::iterator EntityListIterator;

    //! Acts as a generic scenegraph for all entities in the world.
    /*! Contains all entities in the world in a generic fashion.
        Acts as a factory for all entities.

        To create access and remove scenes, see Framework.

        \ingroup Scene_group
    */
    class SceneManager : public QObject
    {
        Q_OBJECT
        
        friend class Foundation::Framework;
    private:
        //! default constructor
        SceneManager();

        //! constructor that takes a name and parent module
        SceneManager(const std::string &name, Foundation::Framework *framework) :  name_(name), framework_(framework) {}

        //! copy constructor that also takes a name
        SceneManager(const SceneManager &other, const std::string &name ) : framework_(other.framework_), entities_(other.entities_) { }

        //! copy constuctor
        SceneManager(const SceneManager &other);

        //! Current global id for networked entities
        uint gid_;

        //! Current id for local entities
        uint gid_local_;

    public slots:
        QVariantList GetEntityIdsWithComponent(const QString &type_name);
        Scene::Entity* GetEntityRaw(uint id) { return GetEntity(id).get(); }

    public:
        //! destructor
        ~SceneManager();
        
        //! entity map
        typedef std::map<entity_id_t, EntityPtr> EntityMap;

        //! entity iterator, see begin() and end()
        typedef MapIterator<EntityMap::iterator, EntityPtr> iterator;

        //! const entity iterator. see begin() and end()
        typedef MapIterator<EntityMap::const_iterator, const Scene::EntityPtr> const_iterator;

        //! Returns true if the two scenes have the same name
        bool operator == (const SceneManager &other) const { return Name() == other.Name(); }

        //! Returns true if the two scenes have different names
        bool operator != (const SceneManager &other) const { return !(*this == other); }

        //! Order by scene name
        bool operator < (const SceneManager &other) const { return Name() < other.Name(); }

        //! Returns scene name
        const std::string &Name() const { return name_; }

        //! Creates new local entity that contains the specified components
        /*! Entities should never be created directly, but instead created with this function.

            To create an empty entity omit components parameter.

            \param id Id of the new entity. Use GetNextFreeId() or GetNextLocalFreeId()
            \param components Optional list of component names the entity will use. If omitted or the list is empty, creates an empty entity.
            \param change Origin of change regards to network replication
        */
        EntityPtr CreateLocalEntity(entity_id_t id = 0, const QStringList &components = QStringList::QStringList(),
             AttributeChange::Type change = AttributeChange::LocalOnly);

        //! Creates new entity that contains the specified components
        /*! Entities should never be created directly, but instead created with this function.

            To create an empty entity omit components parameter.

            \param id Id of the new entity. Use GetNextFreeId() or GetNextLocalFreeId()
            \param components Optional list of component names the entity will use. If omitted or the list is empty, creates an empty entity.
            \param change Origin of change regards to network replication
        */
        EntityPtr CreateEntity(entity_id_t id = 0, const QStringList &components = QStringList::QStringList(),
             AttributeChange::Type change = AttributeChange::LocalOnly);

        //! Forcibly changes id of an existing entity. If there already is an entity with the new id, it will be purged
        /*! Note: this is meant as a response for a server-authoritative message to change the id of a client-created entity,
            and this change in itself will not be replicated
            \param old_id Old id of the existing entity
            \param new_id New id to set
         */ 
        void ChangeEntityId(entity_id_t old_id, entity_id_t new_id);
        
        //! Returns entity with the specified id
        /*!
            \note Returns a shared pointer, but it is preferable to use a weak pointer, Scene::EntityWeakPtr,
                  to avoid dangling references that prevent entities from being properly destroyed.
        */
        EntityPtr GetEntity(entity_id_t id) const;

        //! Returns true if entity with the specified id exists in this scene, false otherwise
        bool HasEntity(entity_id_t id) const { return (entities_.find(id) != entities_.end()); }

        //! Remove entity with specified id
        /*! The entity may not get deleted if dangling references to a pointer to the entity exists.
            \param id Id of the entity to remove
            \param change Origin of change regards to network replication
        */
        void RemoveEntity(entity_id_t id, AttributeChange::Type change = AttributeChange::LocalOnly);

        //! Remove all entities
        /*! The entities may not get deleted if dangling references to a pointer to them exist.
            \param send_events whether to send events & signals of each delete
         */
        void RemoveAllEntities(bool send_events = true, AttributeChange::Type change = AttributeChange::LocalOnly);
        
        //! Get the next free entity id. Can be used with CreateEntity(). 
        /* These will be for networked entities, and should be assigned only by a point of authority (server)
         */
        entity_id_t GetNextFreeId();

        //! Get the next free local entity id. Can be used with CreateEntity().
        /* As local entities will not be network synced, there should be no conflicts in assignment
         */
        entity_id_t GetNextFreeIdLocal();

        //! Returns iterator to the beginning of the entities.
        iterator begin() { return iterator(entities_.begin()); }

        //! Returns iterator to the end of the entities.
        iterator end() { return iterator(entities_.end()); }

        //! Returns constant iterator to the beginning of the entities.
        const_iterator begin() const { return const_iterator(entities_.begin()); }

        //! Returns constant iterator to the end of the entities.
        const_iterator end() const { return const_iterator(entities_.end()); }

        //! Returns entity map for introspection purposes
        const EntityMap &GetEntityMap() const { return entities_; }

        //! Return list of entities with a spesific component present.
        //! \param type_name Type name of the component
        EntityList GetEntitiesWithComponent(const QString &type_name);

        //! Emit a notification of a component's attributes changing. Called by the components themselves
        /*! \param comp Component pointer
            \param change Type of change (local, from network...)
         */
        void EmitComponentChanged(Foundation::ComponentInterface* comp, AttributeChange::Type change);
        
        //! Emit notification of an attribute changing
        /*! \param comp Component pointer
            \param attribute Attribute pointer
            \param change Type of change (local, from network...)
         */
        void EmitAttributeChanged(Foundation::ComponentInterface* comp, Foundation::AttributeInterface* attribute, AttributeChange::Type change);
        
        //! Emit a notification of a component being added to entity. Called by the entity
        /*! \param entity Entity pointer
            \param comp Component pointer
            \param change Type of change (local, from network...)
         */
        void EmitComponentAdded(Scene::Entity* entity, Foundation::ComponentInterface* comp, AttributeChange::Type change);

        //void EmitComponentInitialized(Foundation::ComponentInterface* comp); //, AttributeChange::Type change);
        
        //! Emit a notification of a component being removed from entity. Called by the entity
        /*! \param entity Entity pointer
            \param comp Component pointer
            \param change Type of change (local, from network...)
            \note This is emitted before just before the component is removed.
         */
        void EmitComponentRemoved(Scene::Entity* entity, Foundation::ComponentInterface* comp, AttributeChange::Type change);
        //! Emit a notification of an entity having been created
        /*! Note: local EntityCreated notifications should not be used for replicating entity creation to server, as the client 
            should not usually decide the entityID itself.
            \param entity Entity pointer
            \param change Type of change (local, from network...)
         */
        void EmitEntityCreated(Scene::Entity* entity, AttributeChange::Type change);
        
        //! Emit a notification of an entity being removed. 
        /*! Note: the entity pointer will be invalid shortly after!
            \param entity Entity pointer
            \param change Type of change (local, from network...)
         */
        void EmitEntityRemoved(Scene::Entity* entity, AttributeChange::Type change);

        //! Load the scene (from an XML file for now, and only serializable components)
        /*! Note: will remove all existing entities
            \param filename File name
            \param change Changetype that will be used, when removing the old scene, and deserializing the new
           
            \return true if successful
         */
        bool LoadScene(const std::string& filename, AttributeChange::Type change);
        
        //! Save the scene (into an XML file for now, and only serializable components)
        /*! \param filename File name
            \return true if successful
         */
        bool SaveScene(const std::string& filename);
        
    private:
        SceneManager &operator =(const SceneManager &other);

        //! Entities in a map
        EntityMap entities_;

        //! parent framework
        Foundation::Framework *framework_;

        //! Name of the scene
        const std::string name_;

    signals:
        //! Signal when a component is changed and should possibly be replicated (if the change originates from local)
        /*! Network synchronization managers should connect to this
         */
        void ComponentChanged(Foundation::ComponentInterface* comp, AttributeChange::Type change);

        //! Signal when an attribute of a component has changed
        void AttributeChanged(Foundation::ComponentInterface* comp, Foundation::AttributeInterface* attribute, AttributeChange::Type change);

        //! Signal when a component is added to an entity and should possibly be replicated (if the change originates from local)
        /*! Network synchronization managers should connect to this
         */
        void ComponentAdded(Scene::Entity* entity, Foundation::ComponentInterface* comp, AttributeChange::Type change);

        //! Signal when a component is removed from an entity and should possibly be replicated (if the change originates from local)
        /*! Network synchronization managers should connect to this
         */
        void ComponentRemoved(Scene::Entity* entity, Foundation::ComponentInterface* comp, AttributeChange::Type change);

        //! Signal when a component is initialized.
        /*! Python and Javascript handlers use this instead of subclassing and overriding the component constructor
         *! -- not used now 'cause ComponentAdded is also emitted upon initialization (loading from server ) 
         void ComponentInitialized(Foundation::ComponentInterface* comp);*/

        //! Signal when an entity created
        /*! Note: currently there is also Naali scene event that duplicates this notification
         */
        void EntityCreated(Scene::Entity* entity, AttributeChange::Type change);

        //! Signal when an entity deleted
        /*! Note: currently there is also Naali scene event that duplicates this notification
         */
        void EntityRemoved(Scene::Entity* entity, AttributeChange::Type change);
    };
}

#endif
