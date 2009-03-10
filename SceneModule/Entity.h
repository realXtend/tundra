// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_SceneEntity_h
#define incl_SceneEntity_h

//#include "ComponentInterface.h"
#include "EntityInterface.h"

namespace Scene
{
    //! Represents an entity in the world. 
    /*! Entity is just a collection of components, the components define what
        the entity is and what it does.

        Use SceneManager to create new entity, do not create directly.
    */
    class Entity : public Foundation::EntityInterface
    {    
    public:
        Entity();
        virtual ~Entity();
 
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
        Core::entity_id_t id_;
   };
}

#endif

