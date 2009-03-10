// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_EntityInterface_h
#define incl_Interfaces_EntityInterface_h

#include "ComponentInterface.h"

namespace Foundation
{
    //! interface for entities
    class EntityInterface
    {
    public:
        //! default constructor
        EntityInterface()  {}

        //! destructor
        virtual ~EntityInterface() {}

        //! Add new component to this entity
        virtual void AddEntityComponent(const Foundation::ComponentInterfacePtr &component) = 0;

        //! Remove the component from this entity
        virtual void RemoveEntityComponent(const Foundation::ComponentInterfacePtr &component) = 0;

        //! Returns a component with name 'name'
        virtual Foundation::ComponentInterfacePtr GetComponent(const std::string &name) const = 0;

        //! Return the unique id of this component
        virtual Core::entity_id_t GetId() const = 0;
    };

    typedef boost::shared_ptr<EntityInterface> EntityPtr;
}

#endif

