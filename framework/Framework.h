// For conditions of distribution and use, see copyright notice in license.txt

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
        //! default constructor
        Framework();
        //! destructor
        ~Framework();

        //! entry point for the framework
        void go();

        ChangeManagerPtr getChangeManager() const { return mChangeManager; }
        ComponentManagerPtr getComponentManager() const { return mComponentManager; }
        EntityManagerPtr getEntityManager() const { return mEntityManager; }
        ServiceManagerPtr getServiceManager() const { return mServiceManager; }


        //! implementation of raycast service, uses internally RenderingSystemInterface for the service
        void raycast();

        //! Signal the framework to exit at first possible opportunity
        void _exit() { mExitSignal = true; }

        //! Returns true if framework is in the process of exiting (will exit at next possible opportunity)
        bool isExiting() { return mExitSignal; }

    private:
        //! Loads all available modules
        void loadModules();
        //! Unloads all available modules
        void unloadModules();

        ModuleManagerPtr mModuleManager;
        ChangeManagerPtr mChangeManager;
        ComponentManagerPtr mComponentManager;
        EntityManagerPtr mEntityManager;
        ServiceManagerPtr mServiceManager;

        //! if true, exit application
        bool mExitSignal;
    };
}

#endif // __include_Framework_h__

