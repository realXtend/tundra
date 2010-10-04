// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_SceneManager_Entity_h
#define incl_SceneManager_Entity_h

#include "ForwardDefines.h"
#include "CoreTypes.h"
#include "IComponent.h"
#include "IAttribute.h"
#include "EntityAction.h"

#include <QObject>
#include <QMap>

namespace Scene
{
    class SceneManager;

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
        Q_PROPERTY (QString Name READ GetName)

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
        //! Returns a component with type 'type_name' or empty pointer if component was not found
        /*! If there are several components with the specified type, returns the first component found (arbitrary).

            \param type_name type of the component
        */
        ComponentPtr GetComponent(const QString &type_name) const;

        //! Returns a component with specific type and name, or empty pointer if component was not found
        /*! 
            \param type_name type of the component
            \param name name of the component
        */
        ComponentPtr GetComponent(const QString &type_name, const QString &name) const;

        //! Returns a component with type 'type_name' or creates & adds it if not found. If could not create, returns empty pointer
        /*! 
            \param type_name type of the component
            \param change Network replication mode, in case component has to be created
        */
        ComponentPtr GetOrCreateComponent(const QString &type_name, AttributeChange::Type change = AttributeChange::LocalOnly);
        ComponentPtr GetOrCreateComponent(const QString &type_name, const QString &name, AttributeChange::Type change = AttributeChange::LocalOnly);

        //! component container
        typedef std::vector<ComponentPtr> ComponentVector;

        //! Action container
        typedef QMap<QString, EntityAction *> ActionMap;

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
            \param change Network replication mode
        */
        void AddComponent(const ComponentPtr &component, AttributeChange::Type change = AttributeChange::LocalOnly);

        //! Remove the component from this entity.
        /*! 
            \param component Pointer to the component to remove
        */
        void RemoveComponent(const ComponentPtr &component, AttributeChange::Type change = AttributeChange::LocalOnly);

        //! Returns a component with type typename and name or empty pointer if component was not found
        /*! If there are several components with the specified type, returns the first component found (arbitrary).

            \param component component that we want to find.
        */
        ComponentPtr GetComponent(const IComponent *component) const;

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

        //! Return entity's shared pointer.
        EntityPtr GetSharedPtr() const;

        //! Returns the unique id of this entity
        entity_id_t GetId() const { return id_; }

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
        Attribute<T> *GetAttribute(const std::string &name) const
        {
            for(size_t i = 0; i < components_.size() ; ++i)
            {
                Attribute<T> *t = components_[i]->GetAttribute<T>(name);
                if (t)
                    return t;
            }
            return 0;
        }

        //! Returns attribute interface pointer to attribute with spesific name.
        /*! \param name Name of the attribute.
            \return IAttribute pointer to the attribute.
            \note Always remember to check for null pointer.
        */
        IAttribute *GetAttributeInterface(const std::string &name) const;

        //! Returns list of attributes with spesific name.
        /*! \param T Typename/class of the attribute.
            \param name Name of the attribute.
            \return List of attributes, or empty list if no attributes are found.
        */
        template<typename T>
        std::vector<Attribute<T> > GetAttributes(const std::string &name) const
        {
            std::vector<Attribute<T> > ret;
            for(size_t i = 0; i < components_.size() ; ++i)
            {
                Attribute<T> *t = components_[i]->GetAttribute<T>(name);
                if (t)
                    return ret.push_back(t);
            }
            return ret;
        }

        //! Returns list of attributes with spesific name.
        /*! \param name Name of the attribute.
            \return List of attribute interface pointers, or empty list if no attributes are found.
        */
        AttributeVector GetAttributes(const std::string &name) const;

    public slots:
        IComponent* GetComponentRaw(const QString &type_name) const { return GetComponent(type_name).get(); }
        IComponent* GetComponentRaw(const QString &type_name, const QString &name) const { return GetComponent(type_name, name).get(); }
        
        IComponent* GetOrCreateComponentRaw(const QString &type_name, AttributeChange::Type change = AttributeChange::LocalOnly) { return GetOrCreateComponent(type_name, change).get(); }
        IComponent* GetOrCreateComponentRaw(const QString &type_name, const QString &name, AttributeChange::Type change = AttributeChange::LocalOnly) { return GetOrCreateComponent(type_name, name, change).get(); }

        void RemoveComponent(const QString &type_name, AttributeChange::Type change = AttributeChange::LocalOnly) { RemoveComponent(GetComponent(type_name), change); }
        void RemoveComponent(const QString &type_name, const QString &name, AttributeChange::Type change = AttributeChange::LocalOnly) { RemoveComponent(GetComponent(type_name, name), change); }

        //! Returns list of components with type 'type_name' or if typename is empty return all components
        //! \param type_name type of the component
        QObjectList GetComponentsRaw(const QString &type_name) const
        {
            QObjectList ret;
            if(type_name.isNull())
            {
                for(size_t i = 0; i < components_.size() ; ++i)
                    ret.push_back(components_[i].get());
            }
            else
            {
                for(size_t i = 0; i < components_.size() ; ++i)
                    if (components_[i]->TypeName() == type_name)
                        ret.push_back(components_[i].get());
            }
            return ret;
        }

        //! Returns whether or not this entity has a component with certain type and name.
        //! \param type_name Type of the component.
        bool HasComponent(const QString &type_name) const;

        //! Returns whether or not this entity has a component with certain type and name.
        //! \param type_name type of the component
        //! \param name name of the component
        bool HasComponent(const QString &type_name, const QString &name) const;

        //! Returns name of this entity if EC_Name is available, empty string otherwise.
        QString GetName() const;

        //! Returns description of this entity if EC_Name is available, empty string otherwise.
        QString GetDescription() const;

        //! Return by name and type, 'cause can't call RemoveComponent with comp as shared_py
        void RemoveComponentRaw(IComponent* comp);

        //! Returns actions map for introspection/reflection.
        const ActionMap &Actions() const { return actions_; }

        /// Creates and registers new action for this entity, or returns an existing action.
        /** Use this function from scripting languages.
            @param name Name of the action.
            @note Never returns null pointer
            @note Never store the returned pointer.
        */
        EntityAction *Action(const QString &name);

        /// Connects action with a spesific name to a receiver object with member slot.
        /** @param name Name of the action.
            @param receiver Receiver object.
            @param member Member slot.
        */
        void ConnectAction(const QString &name, const QObject *receiver, const char *member);

        /// Executes an arbitrary action for all components of this entity.
        /** The components may or may not handle the action.
            @param action Name of the action.
            @param type Execution type, i.e. where the actions is executed.
        */
        void Exec(const QString &action, EntityAction::ExecutionType type = EntityAction::Local);

        /// This is an overloaded function.
        /** @param action Name of the action.
            @param Parameter for the action.
            @param type Execution type, i.e. where the actions is executed.
        */
        void Exec(const QString &action, const QString &param, EntityAction::ExecutionType type = EntityAction::Local);

        /// This is an overloaded function.
        /** @param action Name of the action.
            @param param1 1st parameter for the action.
            @param param2 2nd parameter for the action.
            @param type Execution type, i.e. where the actions is executed.
        */
        void Exec(const QString &action, const QString &param1, const QString &param2,
            EntityAction::ExecutionType type = EntityAction::Local);

        /// This is an overloaded function.
        /** @param action Name of the action.
            @param param1 1st parameter for the action.
            @param param2 2nd parameter for the action.
            @param param3 3rd parameter for the action.
            @param type Execution type, i.e. where the actions is executed.
        */
        void Exec(const QString &action, const QString &param1, const QString &param2, const QString &param3,
            EntityAction::ExecutionType type = EntityAction::Local);

        /// This is an overloaded function.
        /** @param action Name of the action.
            @param params List of parameters for the action.
            @param type Execution type, i.e. where the actions is executed.
        */
        void Exec(const QString &action, const QStringList &params, EntityAction::ExecutionType type = EntityAction::Local);

    private:
        /// Validates that the action has receivers. If not, deletes the action and removes it from the registered actions.
        /** @param action Action to be validated.
        */
        bool HasReceivers(EntityAction *action);

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

