
#ifndef __inc_Foundation_Framework_h__
#define __inc_Foundation_Framework_h__

#include "ForwardDefines.h"


namespace Foundation
{
    //! contains entry point for the framework.
    /*! Allows access to various managers and services
    */
    class Framework
    {
    public:
        Framework();
        ~Framework();

        //! entry point for the framework
        void go();

        ChangeManagerPtr getChangeManager() const { return mChangeManager; }
        ComponentManagerPtr getComponentManager() const { return mComponentManager; }
        EntityManagerPtr getEntityManager() const { return mEntityManager; }
        ServiceManagerPtr getServiceManager() const { return mServiceManager; }


        //! implementation of raycast service, uses internally RenderingSystemInterface for the service
        void raycast();

    private:
        void loadModules();
        void unloadModules();

        ModuleManagerPtr mModuleManager;
        ChangeManagerPtr mChangeManager;
        ComponentManagerPtr mComponentManager;
        EntityManagerPtr mEntityManager;
        ServiceManagerPtr mServiceManager;
    };
}

#endif // __include_Framework_h__

