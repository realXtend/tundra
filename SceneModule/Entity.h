// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_SceneEntity_h
#define incl_SceneEntity_h

#include "EntityInterface.h"

namespace Scene
{
    class SceneModule;

    //! Represents an entity in the world. 
    /*! Entity is just a collection of components, the components define what
        the entity is and what it does.

        Use SceneManager to create new entity, do not create directly.
    */
    class MODULE_API Entity : public Foundation::EntityInterface
    {
        friend class Generic;
    private:
        //! default constructor
        Entity();

        //! constructor that takes a module
        /*!
            \param module parent module
        */
        Entity(SceneModule *module);

        //! copy constructor
        Entity(const Entity &other) : module_(other.module_), id_(gid_)
        {
            gid_ = (gid_ + 1) % static_cast<Core::uint>(-1);
            components_ = other.components_;
        }

    public:
        virtual ~Entity();

        const Entity &operator =(const Entity &other)
        {
            if (&other != this)
            {
                components_ = other.components_;
            }
            return *this;
        }
        virtual bool operator == (const EntityInterface &other) const { return GetId() == other.GetId(); }
        virtual bool operator != (const EntityInterface &other) const { return !(*this == other); }
        virtual bool operator < (const EntityInterface &other) const { return GetId() < other.GetId(); }

        //! Clones the entity. The new entity will contain the same components as the old one.
        /*!
            \param scene_name Name of the scene the new entity should be in
        */
        virtual Foundation::EntityPtr Clone(const std::string &scene_name) const;
 
        //! Add new component to this entity
        virtual void AddEntityComponent(const Foundation::ComponentInterfacePtr &component);

        //! Remove the component from this entity
        virtual void RemoveEntityComponent(const Foundation::ComponentInterfacePtr &component);

        //! Returns a component with name 'name'
        virtual Foundation::ComponentInterfacePtr GetComponent(const std::string &name) const;

        //! Return the unique id of this component
        virtual Core::entity_id_t GetId() const { return id_; }
    protected:
        typedef std::vector<Foundation::ComponentInterfacePtr> ComponentVector;
        ComponentVector components_;

        //! Current global id for entities
        static Core::uint gid_;

    private:
        //! Unique id for this entity
        const Core::entity_id_t id_;

        //! parent module
        SceneModule *module_;
   };
}

#endif

