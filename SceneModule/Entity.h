// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_SceneEntity_h
#define incl_SceneEntity_h

#include "EntityInterface.h"
#include "SceneModuleApi.h"

namespace Scene
{
    class SceneModule;

    /*! \copydoc Foundation::EntityInterface */
    class SCENE_MODULE_API Entity : public Foundation::EntityInterface
    {
        friend class Generic;
    private:
        //! default constructor
        Entity();

        //! constructor that takes a module
        /*!
            \param id unique id for the entity.
            \param module parent module
        */
        Entity(Core::uint id, SceneModule *module);

        //! Copy constructor. Shared components between two entities.
        Entity(const Entity &other) : module_(other.module_), id_(other.id_)
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
        const Entity &operator =(const Entity &other)
        {
            if (&other != this)
            {
                components_ = other.components_;
            }
            return *this;
        }

        virtual Foundation::EntityPtr Clone(const std::string &scene_name) const;

        virtual void AddEntityComponent(const Foundation::ComponentInterfacePtr &component);
        virtual void RemoveEntityComponent(const Foundation::ComponentInterfacePtr &component);
        virtual Foundation::ComponentInterfacePtr GetComponent(const std::string &name) const;

        virtual Core::entity_id_t GetId() const { return id_; }

        //! introspection for the entity, returns all components
        const ComponentVector &GetComponentVector() const;

    protected:
        //! a list of all components
        ComponentVector components_;

    private:
        //! Unique id for this entity
        Core::entity_id_t id_;

        //! parent module
        SceneModule *module_;
   };
}

#endif

