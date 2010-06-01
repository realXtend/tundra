// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_SceneEntity_h
#define incl_SceneEntity_h

#include "ForwardDefines.h"
#include "CoreTypes.h"
#include "ComponentInterface.h"

#include <QObject>

namespace Foundation
{
    class Framework;
}

namespace Scene
{
    class Entity;
    typedef boost::weak_ptr<Entity> EntityWeakPtr;
    typedef boost::shared_ptr<Entity> EntityPtr;
    
    class SceneManager;
    
    //! Represents an entity in the world. 
    /*! An entity is just a collection of components, the components define what
        the entity is and what it does.
        Entities should not be directly created, instead use SceneManager::CreateEntity().

        \ingroup Scene_group
    */
    class Entity : public QObject
    {
        Q_OBJECT

        friend class SceneManager;

    private:
        //! constructor
        /*!
            \param framework Framework
            \param scene Scene this entity belongs to
        */
        explicit Entity(Foundation::Framework* framework, SceneManager* scene);

        //! constructor that takes an id for the entity
        /*!
            \param framework Framework
            \param id unique id for the entity.
            \param scene Scene this entity belongs to
        */
        Entity(Foundation::Framework* framework, uint id, SceneManager* scene);

        //! Set new id
        void SetNewId(entity_id_t id) { id_ = id; }
        //! Set new scene
        void SetScene(SceneManager* scene) { scene_ = scene; }
        
    public:
        //! component container
        typedef std::vector<Foundation::ComponentInterfacePtr> ComponentVector;

        //! destructor
        ~Entity();

        //! Returns true if the two entities have the same id, false otherwise
        virtual bool operator == (const Entity &other) const { return GetId() == other.GetId(); }
        //! Returns true if the two entities have differend id, false otherwise
        virtual bool operator != (const Entity &other) const { return !(*this == other); }
        //! comparison by id
        virtual bool operator < (const Entity &other) const { return GetId() < other.GetId(); }
        
        //! Add a new component to this entity.
        /*! Entities can contain any number of components of any type.
            It is also possible to have several components of the same type,
            although in most cases it is probably not sensible.

            \param component An entity component
            \param change Origin of change for network replication
        */
        void AddComponent(const Foundation::ComponentInterfacePtr &component, Foundation::ComponentInterface::ChangeType change = Foundation::ComponentInterface::LocalOnly);

        //! Remove the component from this entity.
        /*! 
            \param component Pointer to the component to remove
        */
        void RemoveComponent(const Foundation::ComponentInterfacePtr &component, Foundation::ComponentInterface::ChangeType change = Foundation::ComponentInterface::LocalOnly);

        //! Returns a component with type 'type_name' or empty pointer if component was not found
        /*! If there are several components with the specified type, returns the first component found (arbitrary).

            \param type_name type of the component
        */
        Foundation::ComponentInterfacePtr GetComponent(const std::string &type_name) const;

        //! Returns a component with specific type and name, or empty pointer if component was not found
        /*! 
            \param type_name type of the component
            \param name name of the component
        */
        Foundation::ComponentInterfacePtr GetComponent(const std::string &type_name, const std::string& name) const;

        //! Returns a component with type 'type_name' or creates & adds it if not found. If could not create, returns empty pointer
        /*! 
            \param type_name type of the component
            \param change Change type for network replication, in case component has to be created
        */
        Foundation::ComponentInterfacePtr GetOrCreateComponent(const std::string &type_name, Foundation::ComponentInterface::ChangeType change = Foundation::ComponentInterface::LocalOnly);

        //! Returns a component with certain type, already cast to correct type, or empty pointer if component was not found
        /*! If there are several components with the specified type, returns the first component found (arbitrary).
        */
        template <class T> boost::shared_ptr<T> GetComponent() const
        {
            return boost::dynamic_pointer_cast<T>(GetComponent(T::TypeNameStatic()));
        }

        //! Returns a component with certain type and name, already cast to correct type, or empty pointer if component was not found
        /*! 
            \param name name of the component
        */
        template <class T> boost::shared_ptr<T> GetComponent(const std::string& name) const
        {
            return boost::dynamic_pointer_cast<T>(GetComponent(T::TypeNameStatic(), name));
        }

        //! Returns whether or not this entity has a component with certain type and name.
        //! \param type_name Type of the component.
        bool HasComponent(const std::string &type_name) const;

        //! Returns whether or not this entity has a component with certain type and name.
        //! \param type_name type of the component
        //! \param name name of the component
        bool HasComponent(const std::string &type_name, const std::string &name) const;

        //! Returns the unique id of this entity
        entity_id_t GetId() const { return id_; }

        //! introspection for the entity, returns all components
        const ComponentVector &GetComponentVector() const { return components_; }

        //! Returns framework
        Foundation::Framework *GetFramework() const { return framework_; }

        //! Returns scene
        SceneManager* GetScene() const { return scene_; }
        
    private:
        //! a list of all components
        ComponentVector components_;

        //! Unique id for this entity
        entity_id_t id_;

        //! Pointer to framework
        Foundation::Framework* framework_;
        
        //! Pointer to scene
        SceneManager* scene_;
   };
}

#endif

