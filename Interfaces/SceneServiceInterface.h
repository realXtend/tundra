// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_SceneServiceInterface_h
#define incl_Interfaces_SceneServiceInterface_h

#include "ServiceInterface.h"
#include "EntityInterface.h"

namespace Foundation
{
    //! Acts as a generic scenegraph for all entities in the world
    /*!
        Contains all entities in the world in a generic fashion.
        Acts as a factory for all entities.
    */
    class SceneServiceInterface : public ServiceInterface
    {
    public:
        //! default constructor
        SceneServiceInterface() {}

        //! destructor
        virtual ~SceneServiceInterface() {}

        //! Creates new entity that contains the specified components
        /*!
            \param components list of component names the entity will use
        */
        virtual EntityPtr CreateEntity(const Core::StringVector &components) = 0;

        //! Creates an empty entity
        virtual Foundation::EntityPtr CreateEntity() = 0;
        
        //! Returns entity with the specified id
        virtual Foundation::EntityPtr GetEntity(Core::entity_id_t id) const = 0;

        //! Returns true if entity with the specified id exists in this scene, false otherwise
        virtual bool HasEntity(Core::entity_id_t id) const = 0;
    };

    typedef boost::shared_ptr<SceneServiceInterface> ScenePtr;
}

#endif

