// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_EntityInterface_h
#define incl_Interfaces_EntityInterface_h

#include <boost/shared_ptr.hpp>

namespace Scene
{
    class SceneManager;
    class EntityInterface;
    typedef boost::weak_ptr<EntityInterface> EntityWeakPtr;
    typedef boost::shared_ptr<EntityInterface> EntityPtr;
    typedef boost::shared_ptr<SceneManager> ScenePtr;

    //! Represents an entity in the world. 
    /*! An entity is just a collection of components, the components define what
        the entity is and what it does.
        Entities should not be directly created, instead use SceneInterface::CreateEntity().

        \ingroup Scene_group
    */
    class EntityInterface
    {
    public:
        //! default constructor
        EntityInterface();

        //! destructor
        virtual ~EntityInterface();

        virtual EntityInterface &operator = (const EntityInterface &other) = 0;
        virtual bool operator == (const EntityInterface &other) const { return GetId() == other.GetId(); }
        virtual bool operator != (const EntityInterface &other) const { return !(*this == other); }
        virtual bool operator < (const EntityInterface &other) const { return GetId() < other.GetId(); }

        //! Clones the entity. The new entity will contain the same components as the old one.
        /*! The components will be shared between the two entities.

            \param scene_name Name of the scene the new entity should be in
        */
        virtual EntityPtr Clone(const ScenePtr &scene) const = 0;

        //! Add a new component to this entity.
        /*! Entities can contain any number of components of any type.
            It is also possible to have several components of the same type,
            although in most cases it is probably not sensible. It is even
            possible to have the exactly same component more than once.

            \param component An entity component
        */
        virtual void AddEntityComponent(const Foundation::ComponentInterfacePtr &component) = 0;

        //! Remove the component from this entity.
        /*! 
            \param component Pointer to the component to remove
        */
        virtual void RemoveEntityComponent(const Foundation::ComponentInterfacePtr &component) = 0;

        //! Returns a component with name 'name' or empty pointer if component was not found
        /*! If there are several components with the specified name, returns the first component found (arbitrary).

            \param name name of the component
        */
        virtual Foundation::ComponentInterfacePtr GetComponent(const std::string &name) const = 0;

        //! Return the unique id of this component
        virtual Core::entity_id_t GetId() const = 0;
    };
}

#endif

