// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_SceneManager_SceneManager_h
#define incl_SceneManager_SceneManager_h

#include "CoreStdIncludes.h"
#include "CoreAnyIterator.h"
#include "Entity.h"
#include "ComponentInterface.h"

#include <QObject>

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
        Q_OBJECT;
        
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

        //! Current global id for entities
        static uint gid_;

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

        //! Creates new entity that contains the specified components
        /*! Entities should never be created directly, but instead created with this function.

            To create an empty entity omit components parameter.

            \param id Id of the new entity. Use GetNextFreeId().
            \param components Optional list of component names the entity will use. If omitted or the list is empty, creates an empty entity.
        */
        EntityPtr CreateEntity(entity_id_t id = 0, const StringVector &components = StringVector());

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
        */
        void RemoveEntity(entity_id_t id);

        //! Get the next free entity id. Can be used with CreateEntity().
        entity_id_t GetNextFreeId();

        iterator begin() { return iterator(entities_.begin()); }
        iterator end() { return iterator(entities_.end()); }
        const_iterator begin() const { return const_iterator(entities_.begin()); }
        const_iterator end() const { return const_iterator(entities_.end()); }

        //! Returns entity map for introspection purposes
        const EntityMap &GetEntityMap() const { return entities_; }

        //! Return list of entities with a spesific component present.
        //! \param type_name Type name of the component
        EntityList GetEntitiesWithComponent(const std::string &type_name);

        //! Emit a notification of a component's attributes changing. Called by the components themselves
        /*! \param comp Component pointer
            \param change Type of change (local, from network...)
         */
        void EmitComponentChanged(Foundation::ComponentInterface* comp, Foundation::ChangeType change);
        
        //! Emit a notification of a component being added to entity. Called by the entity
        /*! \param entity Entity pointer
            \param comp Component pointer
            \param change Type of change (local, from network...)
         */
        void EmitComponentAdded(Scene::Entity* entity, Foundation::ComponentInterface* comp, Foundation::ChangeType change);
        
        //! Emit a notification of a component being removed from entity. Called by the entity
        /*! \param entity Entity pointer
            \param comp Component pointer
            \param change Type of change (local, from network...)
         */
        void EmitComponentRemoved(Scene::Entity* entity, Foundation::ComponentInterface* comp, Foundation::ChangeType change);
        
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
        void ComponentChanged(Foundation::ComponentInterface* comp, Foundation::ChangeType change);
        //! Signal when a component is added to an entity and should possibly be replicated (if the change originates from local)
        /*! Network synchronization managers should connect to this
         */
        void ComponentAdded(Scene::Entity* entity, Foundation::ComponentInterface* comp, Foundation::ChangeType change);
        //! Signal when a component is removed from an entity and should possibly be replicated (if the change originates from local)
        /*! Network synchronization managers should connect to this
         */
        void ComponentRemoved(Scene::Entity* entity, Foundation::ComponentInterface* comp, Foundation::ChangeType change);
    };
}

#endif
