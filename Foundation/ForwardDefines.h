// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Framework_ForwardDefines_h
#define incl_Framework_ForwardDefines_h

namespace Foundation
{
    class ModuleManager;
    class ComponentManager;
    class ComponentFactoryInterface;
    class ServiceManager;

    typedef boost::shared_ptr<ModuleManager> ModuleManagerPtr;
    typedef boost::shared_ptr<ComponentManager> ComponentManagerPtr;
    typedef boost::shared_ptr<ComponentFactoryInterface> ComponentFactoryInterfacePtr;
    typedef boost::shared_ptr<ServiceManager> ServiceManagerPtr;
}

#endif
