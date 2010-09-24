// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_ComponentManager_h
#define incl_Foundation_ComponentManager_h

#include "ForwardDefines.h"

#include <map>

namespace Foundation
{
    //! Scenegraph, entity and component model that together form a generic, extendable, lightweight scene model.
    /*! See \ref SceneModelPage "Scenes, entities and components" for details about the viewer's scene model.
    
        \defgroup Scene_group Scene Model Client Interface
    */

    //! Manages components. Also works as a component factory.
    /*! \ingroup Foundation_group
        \ingroup Scene_group
    */
    class ComponentManager
    {
    public:
        typedef std::list<ComponentWeakPtr> ComponentList;
        typedef std::map<std::string, ComponentList> ComponentTypeMap;
        typedef ComponentList::iterator iterator;
        typedef ComponentList::const_iterator const_iterator;
        typedef std::map<QString, ComponentFactoryPtr> ComponentFactoryMap;
        typedef std::map<uint, ComponentFactoryPtr> ComponentFactoryHashMap;
        
        //! default constructor
        ComponentManager(Framework *framework);

        //! destructor
        ~ComponentManager() { }

        //! register factory for the component
        void RegisterFactory(const QString &component, const ComponentFactoryPtr &factory);

        //! Unregister the component. Removes the factory.
        void UnregisterFactory(const QString &component);

        //! Returns true if component can be created (a factory for the component has registered itself)
        /*!
            \param type_name name of the component type
            \return true if component can be created, false otherwise
        */
        bool CanCreate(const QString &type_name);

        //! Returns true if component can be created (a factory for the component has registered itself)
        /*!
            \param type_hash String hash of the component typename
            \return true if component can be created, false otherwise
        */
        bool CanCreate(uint type_hash);

        //! Create a new component
        /*!
            Precondition: CanCreate(componentName)

            \param type_name type of the component to create
        */
        ComponentPtr CreateComponent(const QString &type_name);

        //! Create a new component
        /*!
            Precondition: CanCreate(componentName)

            \param type_hash String hash of the component typename
        */
        ComponentPtr CreateComponent(uint type_hash);

        //! Create a new component
        /*!
            Precondition: CanCreate(componentName)

            \param type_name type of the component to create
            \param name name of the component to create
        */
        ComponentPtr CreateComponent(const QString &type_name, const QString &name);

        //! Create a new component
        /*!
            Precondition: CanCreate(componentName)

            \param type_hash String hash of the component typename
            \param name name of the component to create
        */
        ComponentPtr CreateComponent(uint type_hash, const QString &name);
        
        //! Create clone of the specified component
        ComponentPtr CloneComponent(const ComponentInterfacePtr &component);

        //! Create new attribute for spesific component.
        IAttribute *CreateAttribute(IComponent *owner, const std::string &typeName, const std::string &name);

        //! Returns list of supported attribute types.
        StringVector GetAttributeTypes() const;

        //! Get all component factories
        const ComponentFactoryMap GetComponentFactoryMap() const { return factories_; }

        //! Get component typename from typename hash
        /*! \param type_hash String hash of component typename
            \return Typename, or empty if not found
         */
        const QString& GetComponentTypeName(uint type_hash) const;
        
    private:
        //! Map of component factories
        ComponentFactoryMap factories_;
        //! Map of component factories by hash
        ComponentFactoryHashMap factories_hash_;
        
        //! Map of component typename hash to typename
        std::map<uint, QString> hashToTypeName_;

        //! List of supported attribute types.
        StringVector attributeTypes_;

        //! Framework
        Framework *framework_;
    };
}

#endif
