// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_Framework_h
#define incl_Foundation_Framework_h

#include "ConfigurationManager.h"
#include "ServiceInterfaces.h"
#include "ServiceManager.h"
#include "Profiler.h"

namespace Foundation
{
    //! contains entry point for the framework.
    /*! Allows access to various managers and services

        \ingroup Foundation_group
    */
    class Framework
    {
    public:
        typedef std::map<std::string, Scene::ScenePtr> SceneMap;

        //! default constructor
        Framework();
        //! destructor
        ~Framework();

        //! Do post-initialization steps. No need to call if using Framework::Go().
        void PostInitialize();

        //! entry point for the framework
        void Go();

        ComponentManagerPtr GetComponentManager() const { return component_manager_; }
        ModuleManagerPtr GetModuleManager() const { return module_manager_; }
        ServiceManagerPtr GetServiceManager() const { return service_manager_; }
        EventManagerPtr GetEventManager() const { return event_manager_; }
        PlatformPtr GetPlatform() const { return platform_; }
        ConfigurationManagerPtr GetConfigManager() { return config_manager_;}

        //! Signal the framework to exit at first possible opportunity
        void Exit() { exit_signal_ = true; }

        //! Returns true if framework is in the process of exiting (will exit at next possible opportunity)
        bool IsExiting() { return exit_signal_; }

        //! Returns the default configuration
        ConfigurationManager &GetDefaultConfig() { return *(config_manager_.get()); }

        //! Returns pointer to the default configuration
        ConfigurationManager *GetDefaultConfigPtr() { return config_manager_.get(); }

        //! Shortcut for retrieving a service. See ServiceManager::GetService() for more info
        template <class T>
        __inline boost::weak_ptr<T> GetService(Core::service_type_t type) { return service_manager_->GetService<T>(type); }

        //! Shortcut for retrieving a service. See ServiceManager::GetService() for more info
        template <class T>
        __inline const boost::weak_ptr<T> GetService(Core::service_type_t type) const { return service_manager_->GetService<T>(type); }

        //! Creates new empty scene.
        /*! 
            \param name name of the new scene
            \return The new scene, or empty pointer if scene with the specified name already exists.
        */
        Scene::ScenePtr CreateScene(const std::string &name);

        //! Removes a scene with the specified name.
        /*! The scene may not get deleted since there may be dangling references to it.
            If the scene does get deleted, removes all entities which are not shared with
            another existing scene.

            Does nothing if scene with the specified name doesn't exist.

            \param name name of the scene to delete
        */
        void RemoveScene(const std::string &name);

        //! Returns a pointer to a scene
        /*! Manage the pointer carefully, as scenes may not get deleted properly if
            references to the pointer are left alive.

            \note Returns a shared pointer, but it is preferable to use a weak pointer, Scene::SceneWeakPtr,
                  to avoid dangling references that prevent scenes from being properly destroyed.

            \param name Name of the scene to return
            \return The scene, or empty pointer if the scene with the specified name could not be found
        */
        Scene::ScenePtr GetScene(const std::string &name) const;

        //! Returns true if specified scene exists, false otherwise
        bool HasScene(const std::string &name) const { return scenes_.find(name) != scenes_.end(); }

        //! Returns the currently set default world scene, for convinience
        const Scene::ScenePtr &GetDefaultWorldScene() const { return default_scene_; }

        //! Sets the default world scene, for convinient retrieval with GetDefaultWorldScene().
        void SetDefaultWorldScene(const Scene::ScenePtr &scene) { default_scene_ = scene; }

        //! Returns the scene map for self reflection / introspection.
        const SceneMap &GetSceneMap() const { return scenes_; }

        //! Returns the default profiler used by all normal profiling blocks. For profiling code, use PROFILE-macro.
//        Profiler &GetProfiler() { return *ProfilerSection::GetProfiler(); }
        Profiler &GetProfiler() { return profiler_; }

        //! load and init module
        Console::CommandResult ConsoleLoadModule(const Core::StringVector &params);

        //! uninit and unload a module
        Console::CommandResult ConsoleUnloadModule(const Core::StringVector &params);

        //! List all loaded modules
        Console::CommandResult ConsoleListModules(const Core::StringVector &params);

        //! send event
        Console::CommandResult ConsoleSendEvent(const Core::StringVector &params);

        //! Output profiling data
        Console::CommandResult ConsoleProfile(const Core::StringVector &params);

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

        //! Loads all available modules. Do not call normally.
        void LoadModules();
        //! Unloads all available modules. Do not call normally.
        void UnloadModules();

    private:
        //! Registers framework specific console commands
        //! Should be called after modules are loaded and initialized
        void RegisterConsoleCommands();

        //! default event subscriber tree XML file path
        static const char *DEFAULT_EVENT_SUBSCRIBER_TREE_PATH;
        
        //! Create logging system
        void CreateLoggingSystem();

        ModuleManagerPtr module_manager_;
        ComponentManagerPtr component_manager_;
        ServiceManagerPtr service_manager_;
        EventManagerPtr event_manager_;
        PlatformPtr platform_;
		 
		//! default configuration
        ConfigurationManagerPtr config_manager_;

        //! application data
        ApplicationPtr application_;

        //! if true, exit application
        bool exit_signal_;
        
        //! Logger channels
        std::vector<Poco::Channel*> log_channels_;
        //! Logger default formatter
        Poco::Formatter *log_formatter_;

        //! map of scenes
        SceneMap scenes_;

        //! Current 'default' scene
        Scene::ScenePtr default_scene_;

        //! maximum number of ticks (milliseconds) per frame for frame limiter
        Core::uint max_ticks_;

        Profiler profiler_;
    };
}

#endif

