// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_SceneEntity_h
#define incl_SceneEntity_h

namespace Scene
{
    class Entity;
    typedef boost::weak_ptr<Entity> EntityWeakPtr;
    typedef boost::shared_ptr<Entity> EntityPtr;

    //! Represents an entity in the world. 
    /*! An entity is just a collection of components, the components define what
        the entity is and what it does.
        Entities should not be directly created, instead use SceneManager::CreateEntity().

        \ingroup Scene_group
    */
    class Entity
    {
        friend class SceneManager;
    private:
        //! default constructor
        Entity();

        //! constructor that takes an id for the entity
        /*!
            \param id unique id for the entity.
            \param module parent module
        */
        Entity(Core::uint id);

        //! Copy constructor. Shared components between two entities.
        Entity(const Entity &other) : id_(other.id_)
        {
            components_ = other.components_;
        }

        //! Set new id
        void SetNewId(Core::entity_id_t id);

    public:
        //! component container
        typedef std::vector<Foundation::ComponentInterfacePtr> ComponentVector;

        //! destructor
        ~Entity();

        //! Shares components between entities
        Entity &operator =(const Entity &other)
        {
            if (&other != this)
            {
                components_ = other.components_;
            }
            return *this;
        }
        //! Returns true if the two entities have the same id, false otherwise
        virtual bool operator == (const Entity &other) const { return GetId() == other.GetId(); }
        //! Returns true if the two entities have differend id, false otherwise
        virtual bool operator != (const Entity &other) const { return !(*this == other); }
        //! comparison by id
        virtual bool operator < (const Entity &other) const { return GetId() < other.GetId(); }

        //! Clones the entity. The new entity will contain the same components as the old one.
        /*! The components will be shared between the two entities.

            \param scene_name Name of the scene the new entity should be in
        */
        Scene::EntityPtr Clone(const ScenePtr &scene) const;

        //! Add a new component to this entity.
        /*! Entities can contain any number of components of any type.
            It is also possible to have several components of the same type,
            although in most cases it is probably not sensible. It is even
            possible to have the exactly same component more than once.

            \param component An entity component
        */
        void AddEntityComponent(const Foundation::ComponentInterfacePtr &component);

        //! Remove the component from this entity.
        /*! 
            \param component Pointer to the component to remove
        */
        void RemoveEntityComponent(const Foundation::ComponentInterfacePtr &component);

        //! Returns a component with name 'name' or empty pointer if component was not found
        /*! If there are several components with the specified name, returns the first component found (arbitrary).

            \param name name of the component
        */
        Foundation::ComponentInterfacePtr GetComponent(const std::string &name) const;

        //! Returns the unique id of this entity
        Core::entity_id_t GetId() const { return id_; }

        //! introspection for the entity, returns all components
        const ComponentVector &GetComponentVector() const;

    private:
        //! a list of all components
        ComponentVector components_;

        //! Unique id for this entity
        Core::entity_id_t id_;
   };
}

#endif

