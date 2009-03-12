// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_SceneGeneric_h
#define incl_SceneGeneric_h

#include "SceneInterface.h"

namespace Foundation
{
    class Framework;
}

namespace Scene
{
    //! Acts as a generic scenegraph for all entities in the world
    /*!
        Contains all entities in the world in a generic fashion.
        Acts as a factory for all entities.
    */
    class Generic : public Foundation::SceneInterface
    {
        friend class SceneManager;
    private:
        Generic();
        //! constructor that takes a framework
        Generic(Foundation::Framework *framework) : SceneInterface(), framework_(framework) {}



    public:
        virtual ~Generic() {}

        //! Creates new entity that contains the specified components
        /*!
            \param components list of component names the entity will use
        */
        virtual Foundation::EntityPtr CreateEntity(const Core::StringVector &components);

        //! Creates an empty entity
        virtual Foundation::EntityPtr CreateEntity();
        
        //! Returns entity with the specified id
        virtual Foundation::EntityPtr GetEntity(Core::entity_id_t id) const;

        virtual bool HasEntity(Core::entity_id_t id) const
        {
            return (entities_.find(id) != entities_.end());
        }
    
    private:
        typedef std::map<Core::entity_id_t, Foundation::EntityPtr> EntityMap;

        //! Entities in a map
        EntityMap entities_;

        Foundation::Framework *framework_;
    };
}

#endif
