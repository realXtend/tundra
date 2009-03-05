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
        void Go();

        ComponentManagerPtr GetComponentManager() const { return component_manager_; }
        EntityManagerPtr GetEntityManager() const { return entity_manager_; }
        ModuleManagerPtr GetModuleManager() const { return module_manager_; }
        ServiceManagerPtr GetServiceManager() const { return service_manager_; }

        //! Signal the framework to exit at first possible opportunity
        void Exit() { exit_signal_ = true; }

        //! Returns true if framework is in the process of exiting (will exit at next possible opportunity)
        bool IsExiting() { return exit_signal_; }

        //! Returns the default configuration manager
        static ConfigurationManager &GetDefaultConfig()
        {
            static ConfigurationManager manager(ConfigurationManager::CT_DEFAULT);
            return manager;
        }

    private:
        //! Loads all available modules
        void LoadModules();
        //! Unloads all available modules
        void UnloadModules();

        ModuleManagerPtr module_manager_;
        ComponentManagerPtr component_manager_;
        EntityManagerPtr entity_manager_;
        ServiceManagerPtr service_manager_;

        //! if true, exit application
        bool exit_signal_;
    };
}

#endif

