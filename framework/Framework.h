// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_Framework_h
#define incl_Foundation_Framework_h

#include "ForwardDefines.h"
#include "ConfigurationManager.h"


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

        ComponentManagerPtr getComponentManager() const { return mComponentManager; }
        EntityManagerPtr getEntityManager() const { return mEntityManager; }
        ModuleManagerPtr getModuleManager() const { return mModuleManager; }
        ServiceManagerPtr getServiceManager() const { return mServiceManager; }

        //! Signal the framework to exit at first possible opportunity
        void _exit() { mExitSignal = true; }

        //! Returns true if framework is in the process of exiting (will exit at next possible opportunity)
        bool isExiting() { return mExitSignal; }

        //! Returns the default configuration manager
        static ConfigurationManager &getDefaultConfig()
        {
            static ConfigurationManager manager(ConfigurationManager::CT_DEFAULT);
            return manager;
        }

    private:
        //! Loads all available modules
        void loadModules();
        //! Unloads all available modules
        void unloadModules();

        ModuleManagerPtr mModuleManager;
        ComponentManagerPtr mComponentManager;
        EntityManagerPtr mEntityManager;
        ServiceManagerPtr mServiceManager;

        //! if true, exit application
        bool mExitSignal;
    };
}

#endif // __include_Framework_h

