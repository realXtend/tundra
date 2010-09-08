// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_SceneManager_Entity_h
#define incl_SceneManager_Entity_h

#include "ForwardDefines.h"
#include "CoreTypes.h"
#include "ComponentInterface.h"
#include "AttributeInterface.h"

#include <QObject>
#include <QMap>

namespace Scene
{
    class SceneManager;
    class Action;

    //! Represents an entity in the world. 
    /*! An entity is just a collection of components, the components define what
        the entity is and what it does.
        Entities should not be directly created, instead use SceneManager::CreateEntity().

        \note   Entity can have multiple components with same component type name as long as
                the component names are unique.

        \ingroup Scene_group
    */
    class Entity : public QObject
    {
        Q_OBJECT
        Q_PROPERTY (uint Id READ GetId)

        friend class SceneManager;

    private:
        //! constructor
        /*!
            \param framework Framework
            \param scene Scene this entity belongs to
        */
        Entity(Foundation::Framework* framework, SceneManager* scene);

        //! constructor that takes an id for the entity
        /*!
            \param framework Framework
            \param id unique id for the entity.
            \param scene Scene this entity belongs to
        */
        Entity(Foundation::Framework* framework, entity_id_t id, SceneManager* scene);

        //! Set new id
        void SetNewId(entity_id_t id) { id_ = id; }

        //! Set new scene
        void SetScene(SceneManager* scene) { scene_ = scene; }

    public:
        //! component container
        typedef std::vector<Foundation::ComponentInterfacePtr> ComponentVector;

        //! Action container
        typedef QMap<QString, Action *> ActionMap;

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
        void AddComponent(const Foundation::ComponentInterfacePtr &component, AttributeChange::Type change = AttributeChange::LocalOnly);

        //! Remove the component from this entity.
        /*! 
            \param component Pointer to the component to remove
        */
        void RemoveComponent(const Foundation::ComponentInterfacePtr &component, AttributeChange::Type change = AttributeChange::LocalOnly);

        //! Returns a component with type 'type_name' or empty pointer if component was not found
        /*! If there are several components with the specified type, returns the first component found (arbitrary).

            \param type_name type of the component
        */
        Foundation::ComponentInterfacePtr GetComponent(const QString &type_name) const;

        //! Returns a component with type typename and name or empty pointer if component was not found
        /*! If there are several components with the specified type, returns the first component found (arbitrary).

            \param component component that we want to find.
        */
        Foundation::ComponentInterfacePtr GetComponent(const Foundation::ComponentInterface *component) const;

        //! Returns list of components with type 'type_name' or empty list if no components were found.
        //! \param type_name type of the component
        ComponentVector GetComponents(const QString &type_name) const
        {
            ComponentVector ret;
            for(size_t i = 0; i < components_.size() ; ++i)
                if (components_[i]->TypeName() == type_name)
                    ret.push_back(components_[i]);
            return ret;
        }

        //! Returns a component with specific type and name, or empty pointer if component was not found
        /*! 
            \param type_name type of the component
            \param name name of the component
        */
        Foundation::ComponentInterfacePtr GetComponent(const QString &type_name, const QString &name) const;

        //! Returns a component with type 'type_name' or creates & adds it if not found. If could not create, returns empty pointer
        /*! 
            \param type_name type of the component
            \param change Change type for network replication, in case component has to be created
        */
        Foundation::ComponentInterfacePtr GetOrCreateComponent(const QString &type_name, AttributeChange::Type change = AttributeChange::LocalOnly);
        Foundation::ComponentInterfacePtr GetOrCreateComponent(const QString &type_name, const QString &name, AttributeChange::Type change = AttributeChange::LocalOnly);

        //! Returns a component with certain type, already cast to correct type, or empty pointer if component was not found
        /*! If there are several components with the specified type, returns the first component found (arbitrary).
        */
        template <class T>
        boost::shared_ptr<T> GetComponent() const
        {
            return boost::dynamic_pointer_cast<T>(GetComponent(T::TypeNameStatic()));
        }

        /*! Returns list of components with certain class type, already cast to correct type.
            \param T Component class type.
            \return List of components with certain class type, or empty list if no components was found.
        */
        template <class T>
        std::vector<boost::shared_ptr<T> > GetComponents() const
        {
            std::vector<boost::shared_ptr<T> > ret;
            for(size_t i = 0; i < components_.size() ; ++i)
            {
                boost::shared_ptr<T> t = boost::dynamic_pointer_cast<T>(components_[i]);
                if (t)
                    ret.push_back(t);
            }
            return ret;
        }

        //! Returns a component with certain type and name, already cast to correct type, or empty pointer if component was not found
        /*! 
            \param name name of the component
        */
        template <class T>
        boost::shared_ptr<T> GetComponent(const QString& name) const
        {
            return boost::dynamic_pointer_cast<T>(GetComponent(T::TypeNameStatic(), name));
        }

        //! Returns whether or not this entity has a component with certain type and name.
        //! \param type_name Type of the component.
        bool HasComponent(const QString &type_name) const;

        //! Returns whether or not this entity has a component with certain type and name.
        //! \param type_name type of the component
        //! \param name name of the component
        bool HasComponent(const QString &type_name, const QString &name) const;

        //! Return entity's shared pointer.
        EntityPtr GetSharedPtr() const;

        //! Returns the unique id of this entity
        entity_id_t GetId() const { return id_; }

        //! Returns name of this entity if EC_Name is available, empty string otherwise.
        QString GetName() const;

        //! Returns description of this entity if EC_Name is available, empty string otherwise.
        QString GetDescription() const;

        //! introspection for the entity, returns all components
        const ComponentVector &GetComponentVector() const { return components_; }

        //! Returns framework
        Foundation::Framework *GetFramework() const { return framework_; }

        //! Returns scene
        SceneManager* GetScene() const { return scene_; }

        /*! Returns pointer to the first attribute with spesific name.
            \param T Typename/class of the attribute.
            \param name Name of the attribute.
            \return Pointer to the attribute.
            \note Always remember to check for null pointer.
        */
        template<typename T>
        Foundation::Attribute<T> *GetAttribute(const std::string &name) const
        {
            for(size_t i = 0; i < components_.size() ; ++i)
            {
                Foundation::Attribute<T> *t = components_[i]->GetAttribute<T>(name);
                if (t)
                    return t;
            }
            return 0;
        }

        /*! Returns attribute interface pointer to attribute with spesific name.
            \param name Name of the attribute.
            \return AttributeInterface pointer to the attribute.
            \note Always remember to check for null pointer.
        */
        Foundation::AttributeInterface *GetAttributeInterface(const std::string &name) const
        {
            for(size_t i = 0; i < components_.size() ; ++i)
            {
                Foundation::AttributeInterface *attr = components_[i]->GetAttribute(name);
                if (attr)
                    return attr;
            }
            return 0;
        }

        /*! Returns list of attributes with spesific name.
            \param T Typename/class of the attribute.
            \param name Name of the attribute.
            \return List of attributes, or empty list if no attributes are found.
        */
        template<typename T>
        std::vector<Foundation::Attribute<T> > GetAttributes(const std::string &name) const
        {
            std::vector<Foundation::Attribute<T> > ret;
            for(size_t i = 0; i < components_.size() ; ++i)
            {
                Foundation::Attribute<T> *t = components_[i]->GetAttribute<T>(name);
                if (t)
                    return ret.push_back(t);
            }
            return ret;
        }

        /*! Returns list of attributes with spesific name.
            \param name Name of the attribute.
            \return List of attribute interface pointers, or empty list if no attributes are found.
        */
        Foundation::AttributeVector GetAttributes(const std::string &name) const
        {
            std::vector<Foundation::AttributeInterface *> ret;
            for(size_t i = 0; i < components_.size() ; ++i)
            {
                Foundation::AttributeInterface *attr = components_[i]->GetAttribute(name);
                if (attr)
                    ret.push_back(attr);
            }
            return ret;
        }

        //! Returns actions map for introspection/reflection.
        const ActionMap &Actions() const { return actions_; }

public slots:
        /** Creates and registers new action for this entity, or returns an existing action.
            @param name Name of the action.
        */
        Action *RegisterAction(const QString &name);

        /** Connects action with a spesific name to a receiver object with member slot.
            @param name Name of the action.
            @param receiver Receiver object.
            @param member Member slot.
        */
        void ConnectAction(const QString &name, const QObject *receiver, const char *member);

        /** Executes an arbitrary action for all components of this entity.
            The components may or may not handle the action.
            @param action Name of the action.
        */
        void Exec(const QString &action);

        /** This is an overloaded function.
            @param action Name of the action.
            @param Parameter for the action.
        */
        void Exec(const QString &action, const QString &param);

        /** This is an overloaded function.
            @param action Name of the action.
            @param param1 1st parameter for the action.
            @param param2 2nd parameter for the action.
        */
        void Exec(const QString &action, const QString &param1, const QString &param2);

        /** This is an overloaded function.
            @param action Name of the action.
            @param param1 1st parameter for the action.
            @param param2 2nd parameter for the action.
            @param param3 3rd parameter for the action.
        */
        void Exec(const QString &action, const QString &param1, const QString &param2, const QString &param3);

        /** This is an overloaded function.
            @param action Name of the action.
            @param params List of parameters for the action.
        */
        void Exec(const QString &action, const QStringVector &params);

    private:
        //! a list of all components
        ComponentVector components_;

        //! Unique id for this entity
        entity_id_t id_;

        //! Pointer to framework
        Foundation::Framework* framework_;

        //! Pointer to scene
        SceneManager* scene_;

        //! Map of registered entity actions.
        ActionMap actions_;
   };
}

#endif

