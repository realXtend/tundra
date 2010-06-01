// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_ModuleInterface_h
#define incl_Interfaces_ModuleInterface_h

// Disable C4251 warnings in MSVC.
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif

#include "ComponentRegistrarInterface.h"
#include "CoreTypes.h"
#include "ForwardDefines.h"
#include "ConsoleCommand.h"
//#include "ConsoleCommandServiceInterface.h"
#include "CoreModuleApi.h"

//#include <Poco/ClassLibrary.h>

//! this define can be used to make component declaration automatic when the parent module gets loaded / unloaded.
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
        //! Core module types, provided for convenience of accessing modules.
        /*! Core modules are developed by RealXtend core developers. For modules created by other developers,
            use string names instead of the enum below. This way modules can be created without the need to
            touch the Foundation.

            \note if you add new core module type, don't forget to add its name to NameFromType()
            \note   DO NOT ADD ANYMORE NEW ENUMS FOR MODULES. USE STRINGS INSTEAD.
                    THIS ENUM WILL BE DEPRECATED AT SOME POINT IN THE FUTURE.

            \ingroup Module_group
        */

        //! Module enumeration.
        //! \note   DO NOT ADD ANYMORE NEW ENUMS FOR MODULES. USE STRINGS INSTEAD.
        //!         THIS ENUM WILL BE DEPRECATED AT SOME POINT IN THE FUTURE.
        enum Type 
        {
            MT_Renderer = 0,
            MT_RendererWindow,
            MT_Sound,
            MT_Gui,
            MT_WorldLogic,
            MT_OpenSimProtocol,
            MT_Test,
            MT_NetTestLogic,
            MT_PythonScript,
            MT_QtScript,
            MT_Console,
            MT_Asset,
            MT_TelepathyIM,
            MT_Communications,
            MT_Input,
            MT_TextureDecoder,
            MT_Inventory,
            MT_TaigaProtocol,
            MT_OgreAssetEditor,
            MT_UiServices,
            MT_Environment,
            MT_LegacyAvatar,
            MT_Unknown
        };

        //! Returns string from type enum.
        /*!
            \ingroup Module_group
        */
        static const std::string &NameFromType(Type type)
        {
            assert(type != MT_Unknown);

            static const std::string type_strings[MT_Unknown] = {
                 "OgreRenderingModule", "OgreGtkWindowModule", "OpenALAudioModule", "QtModule", "RexLogicModule", "OpenSimProtocolModule",
                 "TestModule", "NetTestLogicModule", "PythonScriptModule", "QtScriptModule", "ConsoleModule", "AssetModule",
                 "TelepathyIMModule", "CommunicationsModule", "WorldInput", "TextureDecoderModule", "InventoryModule",
                 "TaigaProtocolModule", "OgreAssetEditorModule", "UiServices", "EnvironmentModule", "LegacyAvatarModule" };

            return type_strings[type];
        }

        //! Possible module states
        /*!
            \ingroup Module_group
        */
        enum State
        {
            //! Module has been unloaded from memory
            MS_Unloaded = 0,
            //! Module is loaded into memory, but not yet initialized (and probably not yet usable)
            MS_Loaded,
            //! Module is initialized and ready for use
            MS_Initialized,
            //! Module state is unkown
            MS_Unknown
        };
    }

    //! Interface for modules. When creating new modules, do not inherit from this class, inherit from ModuleInterfaceImpl instead.
    /*! See \ref ModuleArchitecture for details.
        
        \note Every module should have a name. Only internal modules have types.

        \ingroup Foundation_group
        \ingroup Module_group
    */
    class MODULE_API ModuleInterface
    {
        friend class ModuleManager;
    public:
        ModuleInterface()  {}
        virtual ~ModuleInterface() {}

        //! Called when module is loaded into memory. Do not trust that framework can be used.
        /*! Override in your own module. Do not call.

            Components in the module should be declared here by using DECLARE_MODULE_EC(Component) macro, where
            Component is the class of the component.
        */
        virtual void Load() = 0;

        //! called when module is unloaded from memory. Do not trust that framework can be used.
        //! Override in your own module. Do not call.
        virtual void Unload() = 0;

        //! Pre-initialization for the module. Called before modules are initializated.
        //! Only override if you need. Do not call.
        virtual void PreInitialize() = 0;

        //! Initializes the module. Called when module is taken in use.
        //! Override in your own module. Do not call.
        virtual void Initialize() = 0;

        //! Post-initialization for the module. At this point Initialize() has been called for all enabled modules.
        //! Only override if you need. Do not call.
        virtual void PostInitialize() = 0;

        //! Uninitialize the module. Called when module is removed from use
        //! Override in your own module. Do not call.
        virtual void Uninitialize() = 0;

        //! synchronized update for the module
        /*!
            Override in your own module if you want to perform synchronized update. Do not call.
            \param frametime elapsed time in seconds since last frame
        */
        virtual void Update(f64 frametime) = 0;

        //! Receives an event
        /*! Should return true if the event was handled and is not to be propagated further
            Override in your own module if you want to receive events. Do not call.

            See \ref EventSystem.

            \param category_id Category id of the event
            \param event_id Id of the event
            \param data Event data, or 0 if no data passed.
         */
        virtual bool HandleEvent(event_category_id_t category_id, event_id_t event_id, EventDataInterface* data) = 0;

        //! Returns major version as string. Override only if module is not internal.
        virtual std::string VersionMajor() const = 0;

        //! Returns minor version as string. Override only if module is not internal.
        virtual std::string VersionMinor() const = 0;

        //! Returns the name of the module. Each module also has a static accessor for the name, it's needed by the logger.
        virtual const std::string &Name() const = 0;

        //! Returns internal type of the module or MT_Unknown if module is not internal
        //! do not override
        virtual Module::Type Type() const = 0;

        //! Returns true if module is internal, false otherwise
        //! do not override
        virtual bool IsInternal() const = 0;

        //! Declare a component the module defines. For internal use.
        virtual void DeclareComponent(const ComponentRegistrarInterfacePtr &registrar) = 0;

        //! Returns the state of the module
        //! do not override
        virtual Module::State State() const = 0;

        //! By using this function for console commands, the command gets automatically
        //! registered / unregistered with the console when module is initialized / uninitialized
        //! Do not override.
        virtual void AutoRegisterConsoleCommand(const Console::Command &command) = 0;

        //! Returns parent framework
        //! do not override
        virtual Framework *GetFramework() const = 0;

    private:
        //! Only for internal use.
        virtual void SetFramework(Framework *framework) = 0;

        //! Called when module is loaded. Do not override in child classes. For internal use.
        virtual void LoadInternal() = 0;

        //! Called when module is unloaded. Do not override in child classes. For internal use.
        virtual void UnloadInternal() = 0;

        //! PreInitializes the module. 
        virtual void PreInitializeInternal() = 0;

        //! Initializes the module. Called when module is taken in use. Do not override in child classes. For internal use.
        virtual void InitializeInternal() = 0;

        //! PostInitializes the module.
        virtual void PostInitializeInternal() = 0;

        //! Uninitialize the module. Called when module is removed from use. Do not override in child classes. For internal use.
        virtual void UninitializeInternal() = 0;
    };

    //! Interface for modules, implementation. When creating new modules, inherit from this class.
    /*!
        \ingroup Foundation_group
        \ingroup Module_group
    */
    class MODULE_API ModuleInterfaceImpl : public ModuleInterface
    {
        friend class ModuleManager;

    private:
        typedef std::vector<Console::Command> CommandVector;

    public:
        /// Constructor.
        /// @param name Module name.
        explicit ModuleInterfaceImpl(const std::string &name);

        /// Constructor.
        /// @param name type Module type.
        explicit ModuleInterfaceImpl(Module::Type type);

        /// Destructor.
        virtual ~ModuleInterfaceImpl();

        virtual void Load() {}

        virtual void Unload() {}

        virtual void Initialize() {}

        virtual void Uninitialize() {}

        /// ModuleInterface override.
        virtual void PreInitialize() {}

        /// ModuleInterface override.
        virtual void PostInitialize() {}

        /// ModuleInterface override.
        virtual void Update(f64 frametime) {}

        /// ModuleInterface override.
        virtual const std::string &Name() const { return (type_ == Module::MT_Unknown ? name_ : Module::NameFromType(type_)); }

        /// ModuleInterface override.
        virtual Module::Type Type() const { return type_; }

        /// ModuleInterface override.
        virtual bool IsInternal() const { return type_ != Module::MT_Unknown; }

        /// ModuleInterface override.
        virtual void DeclareComponent(const ComponentRegistrarInterfacePtr &registrar) { component_registrars_.push_back(registrar); }

        /// ModuleInterface override.
        virtual bool HandleEvent(event_category_id_t category_id, event_id_t event_id, EventDataInterface* data) { return false; }

        /// ModuleInterface override.
        virtual Module::State State() const { return state_; }

        /// ModuleInterface override.
        virtual void AutoRegisterConsoleCommand(const Console::Command &command);

        /// ModuleInterface override.
        virtual Framework *GetFramework() const;

        /// ModuleInterface override.
        virtual std::string VersionMajor() const;

        /// ModuleInterface override.
        virtual std::string VersionMinor() const;

    protected:
        //! parent framework
        Framework *framework_;

        void RegisterConsoleCommand(const Console::Command &command);

    private:
        virtual void LoadInternal() { assert(state_ == Module::MS_Unloaded); Load(); state_ = Module::MS_Loaded; }
        virtual void UnloadInternal() { assert(state_ == Module::MS_Loaded); Unload(); state_ = Module::MS_Unloaded; }
        virtual void SetFramework(Framework *framework) { framework_ = framework; assert (framework_); }

        //! Unused
        virtual void PreInitializeInternal() { PreInitialize(); }

        //! Registers all declared components
        virtual void InitializeInternal();

        //! Sets internal state to "initialized"
        virtual void PostInitializeInternal()
        {
            PostInitialize();
            state_ = Foundation::Module::MS_Initialized;
        }

        //! Unregisters all declared components
        virtual void UninitializeInternal();

        typedef std::vector<ComponentRegistrarInterfacePtr> RegistrarVector;

        //! Component registrars
        RegistrarVector component_registrars_;

        //! list of console commands that should be registered / unregistered automatically
        CommandVector console_commands_;

        //! name of the module
        const std::string name_;

        //! type of the module if inbuild, unknown otherwise
        const Module::Type type_;

        //! Current state of the module
        Module::State state_;
    };
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif
