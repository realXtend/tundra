// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __inc_Entity_h__
#define __inc_Entity_h__

#include "ComponentInterface.h"

namespace Foundation
{
    class Framework;

    //! Represents an entity in the world. 
    /*! Entity is just a collection of components, the components define what
        the entity is and what it does.

        Use EntityManager to create new entity, do not create directly.
    */
    class Entity
    {    
    public:
        Entity() { assert (false); }
        Entity(Framework *framework);
        virtual ~Entity();
 
        //! Add new component to this entity
        void addEntityComponent(const ComponentInterfacePtr &component);

        //! Remove the component from this entity
        void removeEntityComponent(const ComponentInterfacePtr &component);

        //! Return the unique id of this component
        Core::entity_id_t getId() const { return mId; }
    protected:
        typedef std::vector<ComponentInterfacePtr> ComponentVector;
        ComponentVector mComponents;

        static Core::uint smGID;

    private:
        Framework *mFramework;

        //! Unique id for this entity
        Core::entity_id_t mId;
   };
}

#endif // __include_Entity_h__
