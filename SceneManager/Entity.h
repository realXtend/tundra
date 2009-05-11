// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_SceneEntity_h
#define incl_SceneEntity_h

#include "EntityInterface.h"

namespace Scene
{
    class SceneModule;

    /*! \copydoc Scene::EntityInterface */
    class Entity : public Scene::EntityInterface
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
        virtual ~Entity();

        //! Shares components between entities
        EntityInterface &operator =(const EntityInterface &other)
        {
            if (&other != this)
            {
                components_ = checked_static_cast<const Entity*>(&other)->components_;
            }
            return *this;
        }

        Scene::EntityPtr Clone(const ScenePtr &scene) const;

        void AddEntityComponent(const Foundation::ComponentInterfacePtr &component);
        void RemoveEntityComponent(const Foundation::ComponentInterfacePtr &component);
        Foundation::ComponentInterfacePtr GetComponent(const std::string &name) const;

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

