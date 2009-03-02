// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __inc_Foundation_ComponentManager_h__
#define __inc_Foundation_ComponentManager_h__

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
        ComponentManager(Framework *framework) : mFramework(framework) {}
        //! destructor
        ~ComponentManager() {}

        //! register factory for the component
        void registerFactory(const std::string &component, const ComponentFactoryInterfacePtr &factory)
        {
            assert(mFactories.find(component) == mFactories.end());

            mFactories[component] = factory;
        }

        //! Unregister the component. Removes the factory.
        void unregisterFactory(const std::string &component)
        {
            ComponentFactoryMap::iterator iter = mFactories.find(component);
            assert(iter != mFactories.end());

            mFactories.erase(iter);
        }

        //! Create a new component
        /*! Returns empty ComponentInterfacePtr if component type is not registered

            \param name name of the component to create
        */
        ComponentPtr createComponent(const std::string &componentName) const;

        //! Create clone of the specified component
        ComponentPtr cloneComponent(const ComponentInterfacePtr &component) const;
        
        //! Get component by entity id and component type
        ComponentPtr getComponent(Core::entity_id_t id, const std::string &component);
        

    private:
        typedef std::map<std::string, ComponentFactoryInterfacePtr> ComponentFactoryMap;

        ComponentFactoryMap mFactories;
        Framework *mFramework;
    };
}

#endif 
