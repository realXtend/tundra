// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_ComponentManager_h
#define incl_Foundation_ComponentManager_h

#include "ComponentInterface.h"

namespace Foundation
{
    class Framework;
    class ComponentInterface;

    //! Manages components. Also works as a component factory.
    class ComponentManager
    {
    public:
        //! default constructor
        ComponentManager(Framework *framework) : framework_(framework) {}
        //! destructor
        ~ComponentManager() {}

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

        //! Create a new component
        /*! Returns empty ComponentInterfacePtr if component type is not registered

            \param name name of the component to create
        */
        ComponentPtr CreateComponent(const std::string &componentName) const;

        //! Create clone of the specified component
        ComponentPtr CloneComponent(const ComponentInterfacePtr &component) const;
        
        //! Get component by entity id and component type
        ComponentPtr GetComponent(Core::entity_id_t id, const std::string &component);
        

    private:
        typedef std::map<std::string, ComponentFactoryInterfacePtr> ComponentFactoryMap;

        ComponentFactoryMap factories_;
        Framework *framework_;
    };
}

#endif 
