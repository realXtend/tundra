// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_ComponentManager_h
#define incl_Foundation_ComponentManager_h

#include "ForwardDefines.h"

#include <map>

namespace Foundation
{
    class Framework;

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
        typedef std::map< std::string, ComponentList > ComponentTypeMap;
        typedef ComponentList::iterator iterator;
        typedef ComponentList::const_iterator const_iterator;
        typedef std::map<std::string, ComponentFactoryInterfacePtr> ComponentFactoryMap;

        //! default constructor
        ComponentManager(Framework *framework) : framework_(framework) {}

        //! destructor
        ~ComponentManager() { }

        //! register factory for the component
        void RegisterFactory(const std::string &component, const ComponentFactoryInterfacePtr &factory)
        {
            assert(factories_.find(component) == factories_.end());
            factories_[component] = factory;
        }

        //! Unregister the component. Removes the factory.
        void UnregisterFactory(const std::string &component)
        {
            ComponentFactoryMap::iterator iter = factories_.find(component);
            assert(iter != factories_.end());
            factories_.erase(iter);
        }

        //! Returns true if component can be created (a factory for the component has registered itself)
        /*!
            \param type_name name of the component type
            \return true if component can be created, false otherwise
        */
        bool CanCreate(const std::string &type_name);

        //! Create a new component
        /*!
            Precondition: CanCreate(componentName)

            \param type_name type of the component to create
        */
        ComponentPtr CreateComponent(const std::string &type_name);

        //! Create a new component
        /*!
            Precondition: CanCreate(componentName)

            \param type_name type of the component to create
            \param name name of the component to create
        */
        ComponentPtr CreateComponent(const std::string &type_name, const std::string &name);

        //! Create clone of the specified component
        ComponentPtr CloneComponent(const ComponentInterfacePtr &component);

        //! Get all component factories
        const ComponentFactoryMap GetComponentFactoryMap() const { return factories_; }

    private:
        //! map of component factories
        ComponentFactoryMap factories_;

        //! Framework
        Framework *framework_;
    };
}

#endif
