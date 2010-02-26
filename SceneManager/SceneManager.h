// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_SceneSceneManager_h
#define incl_SceneSceneManager_h

#include "CoreAnyIterator.h"
#include "Entity.h"

namespace Scene
{
    class SceneManager;
    typedef boost::shared_ptr<SceneManager> ScenePtr;
    typedef boost::weak_ptr<SceneManager> SceneWeakPtr;

    //! Acts as a generic scenegraph for all entities in the world.
    /*! Contains all entities in the world in a generic fashion.
        Acts as a factory for all entities.

        To create access and remove scenes, see Framework.

        Contains iterators for iterating all entities in the scene.
        If you wish to access components of a specific type inside
        the entities, iterating with Foundation::ComponentManager is
        the preferred way.

        \ingroup Scene_group
    */
    class SceneManager
    {
        friend class Foundation::Framework;
    private:
        SceneManager();
        //! constructor that takes a name and parent module
        SceneManager(const std::string &name, Foundation::Framework *framework) :  name_(name), framework_(framework) {}
        //! copy constructor that also takes a name
        SceneManager( const SceneManager &other, const std::string &name ) : framework_(other.framework_), entities_(other.entities_) { }
        // copy constuctor
        SceneManager( const SceneManager &other);

        

        //! Current global id for entities
        static uint gid_;

    public:
        typedef std::map<entity_id_t, Scene::EntityPtr> EntityMap;
        //! entity iterator, see begin() and end()
        typedef MapIterator<EntityMap::iterator, Scene::EntityPtr> iterator;
        //! const entity iterator. see begin() and end()
        typedef MapIterator<EntityMap::const_iterator, const Scene::EntityPtr> const_iterator;

        //! destructor
        ~SceneManager() {}

        //! assignment operator. The two scenes will contain pointers to same entities after this.
        SceneManager &operator =(const SceneManager &other)
        {
            if (&other != this)
            {
                entities_ = other.entities_;
            }
            return *this;
        }

        //! Returns true if the two scenes have the same name
        bool operator == (const SceneManager &other) const { return Name() == other.Name(); }
        //! Returns true if the two scenes have different names
        bool operator != (const SceneManager &other) const { return !(*this == other); }
        //! Order by scene name
        bool operator <  (const SceneManager &other) const { return Name() < other.Name(); }

        //! Returns scene name
        const std::string &Name() const { return name_; }

        //! Make a soft clone of this scene. The new scene will contain pointers to the same entities as the old one.
        /*! 
            \param newName Name of the new scene
        */
        Scene::ScenePtr Clone(const std::string &newName) const;
        //! Creates new entity that contains the specified components
        /*! Entities should never be created directly, but instead created with this function.

            To create an empty entity omit components parameter.

            \param id Id of the new entity. Use GetNextFreeId().
            \param components Optional list of component names the entity will use. If omitted or the list is empty, creates an empty entity.
        */
        Scene::EntityPtr CreateEntity(entity_id_t id = 0, const StringVector &components = StringVector());

        //! Returns entity with the specified id
        /*!
            \note Returns a shared pointer, but it is preferable to use a weak pointer, Scene::EntityWeakPtr,
                  to avoid dangling references that prevent entities from being properly destroyed.
        */
        Scene::EntityPtr GetEntity(entity_id_t id) const;

        //! Returns true if entity with the specified id exists in this scene, false otherwise
        bool HasEntity(entity_id_t id) const
        {
            return (entities_.find(id) != entities_.end());
        }

        //! Remove entity with specified id
        /*! The entity may not get deleted if the entity is shared between multiple scenes,
            or if dangling references to a pointer to the entity exists.

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
    private:

        //! Entities in a map
        EntityMap entities_;

        //! parent framework
        Foundation::Framework *framework_;

        //! Name of the scene
        const std::string name_;
    };
}

#endif
