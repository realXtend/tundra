// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_SceneInterface_h
#define incl_Interfaces_SceneInterface_h

#include "EntityInterface.h"

namespace Foundation
{
    class SceneInterface;
    typedef boost::shared_ptr<SceneInterface> ScenePtr;

    //! Acts as a generic scenegraph for all entities in the world
    /*!
        Contains all entities in the world in a generic fashion.
        Acts as a factory for all entities.
    */
    class VIEWER_API SceneInterface
    {
    public:
        //! default constructor
        SceneInterface() {}

        //! destructor
        virtual ~SceneInterface() {}

        virtual bool operator == (const SceneInterface &other) const = 0;
        virtual bool operator != (const SceneInterface &other) const = 0;
        virtual bool operator < (const SceneInterface &other) const = 0;

        //! Returns scene name
        virtual const std::string &Name() const = 0;

        //! Make a soft clone of this scene. The new scene will contain the same entities as the old one.
        /*! 
            \param newName Name of the new scene
        */
        virtual ScenePtr Clone(const std::string &newName) const = 0;

        //! Creates new entity that contains the specified components
        /*!
            \param components list of component names the entity will use
        */
        virtual EntityPtr CreateEntity(const Core::StringVector &components) = 0;

        //! Creates an empty entity
        virtual Foundation::EntityPtr CreateEntity() = 0;

        //! Makes a soft clone of the entity. The new entity will be placed in this scene.
        /*! The entity need not be contained in this scene

            \param entity Entity to be cloned
        */
        virtual Foundation::EntityPtr CloneEntity(const EntityPtr &entity) = 0;
        
        //! Returns entity with the specified id
        virtual Foundation::EntityPtr GetEntity(Core::entity_id_t id) const = 0;

        //! Returns true if entity with the specified id exists in this scene, false otherwise
        virtual bool HasEntity(Core::entity_id_t id) const = 0;
    };

    //! Acts as a generic scenegraph for all entities in the world
    /*!
        Contains all entities in the world in a generic fashion.
        Acts as a factory for all entities.
    */
    class VIEWER_API SceneInterface_Impl : public SceneInterface
    {
    public:
        //! default constructor
        SceneInterface_Impl() {}

        //! constructor that takes a string as name
        SceneInterface_Impl(const std::string &name) : name_(name) {}

        //! copy constructor
        SceneInterface_Impl( const SceneInterface_Impl &other ) : SceneInterface(), name_(other.name_) { }

        //! destructor
        virtual ~SceneInterface_Impl() {}
        
        //! Returns scene name
        virtual const std::string &Name() const { return name_; }

    private:
        //! name of the module
        const std::string name_;
    };
}

#endif

