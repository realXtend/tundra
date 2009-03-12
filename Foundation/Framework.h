// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_Framework_h
#define incl_Foundation_Framework_h

#include "ConfigurationManager.h"
#include "ServiceInterfaces.h"
#include "ServiceManager.h"

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
        ModuleManagerPtr GetModuleManager() const { return module_manager_; }
        ServiceManagerPtr GetServiceManager() const { return service_manager_; }
        PlatformPtr GetPlatform() const { return platform_; }

        //! Signal the framework to exit at first possible opportunity
        void Exit() { exit_signal_ = true; }

        //! Returns true if framework is in the process of exiting (will exit at next possible opportunity)
        bool IsExiting() { return exit_signal_; }

        //! Returns the default configuration
        const ConfigurationManager &GetDefaultConfig() const { return config_; }

        //! Returns pointer to the default configuration
        const ConfigurationManager *GetDefaultConfigPtr() const { return &config_; }

        //! Shortcut for retrieving a service. See ServiceManager for more info
        template <class T>
        __inline T *GetService(Service::Type type) { return service_manager_->GetService<T>(type); }

        //! Shortcut for retrieving a service. See ServiceManager for more info
        template <class T>
        __inline const T *GetService(Service::Type type) const { return service_manager_->GetService<T>(type); }

        //! Returns name of the configuration group used by the framework
        /*! The group name is used with ConfigurationManager, for framework specific
            settings. Alternatively a class may use it's own name as the name of the
            configuration group, if it so chooses.

            For internal use only.
        */
        static const std::string &ConfigurationGroup()
        {
            static std::string group("Foundation");
            return group;
        }

    private:
        //! Loads all available modules
        void LoadModules();
        //! Unloads all available modules
        void UnloadModules();
        //! Create logging system
        void CreateLoggingSystem();

        ModuleManagerPtr module_manager_;
        ComponentManagerPtr component_manager_;
        ServiceManagerPtr service_manager_;
        PlatformPtr platform_;

        //! if true, exit application
        bool exit_signal_;
        
        //! Logger channels
        std::vector<Poco::Channel*> log_channels_;
        //! Logger default formatter
        Poco::Formatter *log_formatter_;

        //! default configuration
        ConfigurationManager config_;
    };
}

#endif

