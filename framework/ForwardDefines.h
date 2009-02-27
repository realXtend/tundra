
#ifndef __incl_Framework_ForwardDefines_h__
#define __incl_Framework_ForwardDefines_h__

namespace Foundation
{
    class ModuleManager;
    class ChangeManager;
    class ComponentManager;
    class ComponentFactoryInterface;
    class Entity;
    class EntityManager;
    class ServiceManager;

    typedef boost::shared_ptr<ModuleManager> ModuleManagerPtr;
    typedef boost::shared_ptr<ChangeManager> ChangeManagerPtr;
    typedef boost::shared_ptr<ComponentManager> ComponentManagerPtr;
    typedef boost::shared_ptr<ComponentFactoryInterface> ComponentFactoryInterfacePtr;
    typedef boost::shared_ptr<Entity> EntityPtr;
    typedef boost::shared_ptr<EntityManager> EntityManagerPtr;
    typedef boost::shared_ptr<ServiceManager> ServiceManagerPtr;
}

#endif
