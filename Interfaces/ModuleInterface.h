/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   ModuleInterface.h
 *  @brief  Interface for Naali modules.
 *          See @ref ModuleArchitecture for details.
 */

#ifndef incl_Interfaces_ModuleInterface_h
#define incl_Interfaces_ModuleInterface_h

// Disable C4251 warnings in MSVC: 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif

#include "ComponentRegistrarInterface.h"
#include "CoreTypes.h"
#include "ForwardDefines.h"
#include "ConsoleCommand.h"
#include "CoreModuleApi.h"

/// this define can be used to make component declaration automatic when the parent module gets loaded / unloaded.
#define DECLARE_MODULE_EC(component) \
    { Foundation::ComponentRegistrarInterfacePtr registrar = Foundation::ComponentRegistrarInterfacePtr(new component::component##Registrar); \
    DeclareComponent(registrar); } \

namespace Console
{
    struct Command;
}

namespace Foundation
{
    class Framework;
    class EventDataInterface;

    namespace Module
    {
        /** Possible module states
            @ingroup Module_group
        */
        enum State
        {
            /// Module has been unloaded from memory
            MS_Unloaded = 0,
            /// Module is loaded into memory, but not yet initialized (and probably not yet usable)
            MS_Loaded,
            /// Module is initialized and ready for use
            MS_Initialized,
            /// Module state is unkown
            MS_Unknown
        };
    }

    /** Interface for modules. When creating new modules, inherit from this class.
        See @ref ModuleArchitecture for details.
        @ingroup Foundation_group
        @ingroup Module_group
    */
    class ModuleInterface
    {
        friend class ModuleManager;

    public:
        /** Constructor. Creates logger for the module.
            @param name Module name.
        */
        explicit ModuleInterface(const std::string &name);

        /// Destructor. Destroys logger of the module.
        virtual ~ModuleInterface();

        /** Called when module is loaded into memory. Do not trust that framework can be used.
            Override in your own module. Do not call.
            Components in the module should be declared here by using DECLARE_MODULE_EC(Component) macro, where
            Component is the class of the component.
        */
        virtual void Load() {}

        /// Pre-initialization for the module. Called before modules are initializated.
        /// Only override if you need. Do not call.
        virtual void PreInitialize() {}

        /// Initializes the module. Called when module is taken in use.
        /// Override in your own module. Do not call.
        virtual void Initialize() {}

        /// Post-initialization for the module. At this point Initialize() has been called for all enabled modules.
        /// Only override if you need. Do not call.
        virtual void PostInitialize() {}

        /** Uninitialize the module. Called when module is removed from use
            Override in your own module. Do not call.
        */
        virtual void Uninitialize() {}

        /** Called when module is unloaded from memory. Do not trust that framework can be used.
            Override in your own module. Do not call.
        */
        virtual void Unload() {}

        /** Synchronized update for the module
            Override in your own module if you want to perform synchronized update. Do not call.
            @param frametime elapsed time in seconds since last frame
        */
        virtual void Update(f64 frametime) {}

        /** Receives an event
            Should return true if the event was handled and is not to be propagated further
            Override in your own module if you want to receive events. Do not call.
            See @ref EventSystem.
            @param category_id Category id of the event
            @param event_id Id of the event
            @param data Event data, or 0 if no data passed.
        */
        virtual bool HandleEvent(event_category_id_t category_id, event_id_t event_id, EventDataInterface* data) { return false; }

        /// Declare a component the module defines. For internal use.
        void DeclareComponent(const ComponentRegistrarInterfacePtr &registrar) { component_registrars_.push_back(registrar); }

        /// Returns the name of the module. Each module also has a static accessor for the name, it's needed by the logger.
        const std::string &Name() const { return name_; }

        /// Returns the state of the module.
        Module::State State() const { return state_; }

        /// Returns parent framework.
        Framework *GetFramework() const;

    protected:
        /// Parent framework
        Framework *framework_;

        /** Registers console command for this module.
            @param command Console command.
        */
        void RegisterConsoleCommand(const Console::Command &command);

    private:
        // Modules are noncopyable.
        ModuleInterface(const ModuleInterface &);
        void operator=(const ModuleInterface &);

        /// Only for internal use.
        void SetFramework(Framework *framework) { framework_ = framework; assert (framework_); }

        /// Called when module is loaded. For internal use.
        void LoadInternal() { assert(state_ == Module::MS_Unloaded); Load(); state_ = Module::MS_Loaded; }

        /// Called when module is unloaded. For internal use.
        void UnloadInternal() { assert(state_ == Module::MS_Loaded); Unload(); state_ = Module::MS_Unloaded; }

        /// PreInitializes the module. Unused
        void PreInitializeInternal() { PreInitialize(); }

        /// Initializes the module. Called when module is taken in use. For internal use.
        /// Registers all declared components
        void InitializeInternal();

        /// PostInitializes the module. Sets internal state to "initialized"
        void PostInitializeInternal() { PostInitialize(); state_ = Foundation::Module::MS_Initialized; }

        /// Uninitialize the module. Called when module is removed from use. For internal use.
        /// Unregisters all declared components
        void UninitializeInternal();

        /// Component registrars
        RegistrarVector component_registrars_;

        typedef std::vector<Console::Command> CommandVector;

        /// list of console commands that should be registered / unregistered automatically
        CommandVector console_commands_;

        /// name of the module
        const std::string name_;

        /// Current state of the module
        Module::State state_;
    };
}

#ifdef _MSC_VER
#pragma warning( pop )
///\todo Try to find a way not disable C4275 warnings for good
// Disable C4275 warnings in MSVC for good: non – DLL-interface classkey 'identifier' used as base for DLL-interface classkey 'identifier'
#pragma warning( disable : 4275 )
#endif

#endif
