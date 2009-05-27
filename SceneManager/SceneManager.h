// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_SceneSceneManager_h
#define incl_SceneSceneManager_h

namespace Scene
{
    //! Acts as a generic scenegraph for all entities in the world.
    /*! Contains all entities in the world in a generic fashion.
        Acts as a factory for all entities.

        To create access and remove scenes, see Framework.

        Contains iterators for iterating all entities in the scene.
        If you wish to access components of a specific type inside
        the entities, iterating with Foundation::ComponentManager is
        the preferred way.
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
        static Core::uint gid_;

    public:
        typedef std::map<Core::entity_id_t, Scene::EntityPtr> EntityMap;
        typedef Core::MapIterator<EntityMap::iterator, Scene::EntityPtr> iterator;
        typedef Core::MapIterator<EntityMap::const_iterator, const Scene::EntityPtr> const_iterator;


        ~SceneManager() {}

        SceneManager &operator =(const SceneManager &other)
        {
            if (&other != this)
            {
                entities_ = other.entities_;
            }
            return *this;
        }

        bool operator == (const SceneManager &other) const { return Name() == other.Name(); }
        bool operator != (const SceneManager &other) const { return !(*this == other); }
        bool operator <  (const SceneManager &other) const { return Name() < other.Name(); }

        const std::string &Name() const { return name_; }

        Scene::ScenePtr Clone(const std::string &newName) const;
        Scene::EntityPtr CreateEntity(Core::entity_id_t id = 0, const Core::StringVector &components = Core::StringVector());
        Scene::EntityPtr CloneEntity(const Scene::EntityPtr &entity);
        Scene::EntityPtr GetEntity(Core::entity_id_t id) const;

        bool HasEntity(Core::entity_id_t id) const
        {
            return (entities_.find(id) != entities_.end());
        }

        void RemoveEntity(Core::entity_id_t id);

        Core::entity_id_t GetNextFreeId();

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
