// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_EntityInterface_h
#define incl_Interfaces_EntityInterface_h

//#include "ComponentInterface.h"

namespace Foundation
{
    class EntityInterface;    
    typedef boost::shared_ptr<EntityInterface> EntityPtr;


    //! interface for entities
    class MODULE_API EntityInterface
    {
    public:
        //! default constructor
        EntityInterface()  {}

        //! destructor
        virtual ~EntityInterface() {}

        virtual bool operator == (const EntityInterface &other) const = 0;
        virtual bool operator != (const EntityInterface &other) const = 0;
        virtual bool operator < (const EntityInterface &other) const = 0;

        //! Clones the entity. The new entity will contain the same components as the old one.
        /*!
            \param scene_name Name of the scene the new entity should be in
        */
        virtual EntityPtr Clone(const std::string &scene_name) const = 0;

        //! Add new component to this entity
        virtual void AddEntityComponent(const Foundation::ComponentInterfacePtr &component) = 0;

        //! Remove the component from this entity
        virtual void RemoveEntityComponent(const Foundation::ComponentInterfacePtr &component) = 0;

        //! Returns a component with name 'name'
        virtual Foundation::ComponentInterfacePtr GetComponent(const std::string &name) const = 0;

        //! Return the unique id of this component
        virtual Core::entity_id_t GetId() const = 0;
    };
}

#endif

