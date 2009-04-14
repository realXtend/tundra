// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_ModuleInterface_h
#define incl_Interfaces_ModuleInterface_h

#include <Poco/ClassLibrary.h>
#include <Poco/Logger.h>

#include "ComponentRegistrarInterface.h"
#include "CoreTypes.h"
#include "ForwardDefines.h"
#include "ServiceInterfaces.h"

#define DECLARE_MODULE_EC(component) \
    { Foundation::ComponentRegistrarInterfacePtr registrar = Foundation::ComponentRegistrarInterfacePtr(new component::component##Registrar); \
    DeclareComponent(registrar); } \

#define MODULE_LOGGING_FUNCTIONS                                                                              \
    static void LogFatal(const std::string &msg)    { Poco::Logger::get(NameStatic()).fatal(msg);         }   \
    static void LogCritical(const std::string &msg) { Poco::Logger::get(NameStatic()).critical(msg);      }   \
    static void LogError(const std::string &msg)    { Poco::Logger::get(NameStatic()).error(msg);         }   \
    static void LogWarning(const std::string &msg)  { Poco::Logger::get(NameStatic()).warning(msg);       }   \
    static void LogNotice(const std::string &msg)   { Poco::Logger::get(NameStatic()).notice(msg);        }   \
    static void LogInfo(const std::string &msg)     { Poco::Logger::get(NameStatic()).information(msg);   }   \
    static void LogTrace(const std::string &msg)    { Poco::Logger::get(NameStatic()).trace(msg);         }   \
    static void LogDebug(const std::string &msg)    { Poco::Logger::get(NameStatic()).debug(msg);         }


namespace Foundation
{
    class Framework;
    class EventDataInterface;

    namespace Module
    {
        //! internal module types.
        /*!
            \note if you add new internal module type, don't forget to add it's name to NameFromType()
        */
		enum Type 
		{
			MT_Scene = 0,
			MT_Renderer,
			MT_Sound,
			MT_Gui,
			MT_WorldLogic,
			MT_OpenSimProtocol,
			MT_Test,
			MT_NetTestLogic,
			MT_PythonScript,
            MT_Console,
            MT_Asset,
			MT_Communication,
			MT_CommunicationUI,
            MT_Input,
            MT_TextureDecoder,
			MT_Unknown
		};

        static const std::string &NameFromType(Type type)
        {
            assert(type != MT_Unknown);

            static const std::string type_strings[MT_Unknown] = {
                 "SceneModule", "OgreRenderingModule", "SoundModule", "GtkmmUI", "RexLogicModule", "OpenSimProtocolModule",
                 "TestModule", "NetTestLogicModule", "PythonScriptModule", "ConsoleModule", "AssetModule", "CommunicationModule",
				 "CommunicationUIModule", "InputModuleOIS", "TextureDecoderModule" };

            return type_strings[type];
        }

        //! Current module state
        enum State
        {
            MS_Unloaded = 0,
            MS_Loaded,
            MS_Initialized,
            MS_Unknown
        };
    }

    //! interface for modules
    /*! See ModuleManager for more info.
        For core modules, version is same as framework. Otherwise each module is
        responsible for assigning itself a version number by overriding
        VersionMajor() and VersionMinor() - functions.
      
        \note Every module should have a name. Only internal modules have types.
    */
    class MODULE_API ModuleInterface
    {
        friend class ModuleManager;
    public:
        ModuleInterface()  {}
        virtual ~ModuleInterface() {}

        //! Called when module is loaded into memory.Do not trust that framework can be used.
        /*!
            Components in the module should be declared here by using DECLARE_MODULE_EC(Component) macro, where
            Component is the class of the component.
        */
        virtual void Load() = 0;
        //! called when module is unloaded from memory. Do not trust that framework can be used.
        virtual void Unload() = 0;

        //! Pre-initialization for the module. Called before modules are initializated.
        virtual void PreInitialize() = 0;
        //! Initializes the module. Called when module is taken in use
        virtual void Initialize() = 0;
        //! Post-initialization for the module. At this point Initialize() has been called for all enabled modules.
        virtual void PostInitialize() = 0;
        
        //! Uninitialize the module. Called when module is removed from use
        virtual void Uninitialize() = 0;

        //! synchronized update for the module
        /*!
            \param frametime elapsed time in seconds since last frame
        */
        virtual void Update(Core::f64 frametime) = 0;

        //! Returns the name of the module. Each module also has a static accessor for the name, it's needed by the logger.
        virtual const std::string &Name() const = 0;

        //! Returns internal type of the module or MT_Unknown if module is not internal
        virtual Module::Type Type() const = 0;

        //! Returns true if module is internal, false otherwise
        virtual bool IsInternal() const = 0;

        //! Declare a component the module defines. For internal use.
        virtual void DeclareComponent(const ComponentRegistrarInterfacePtr &registrar) = 0;
        
        //! Receives an event
        /*! Should return true if the event was handled and is not to be propagated further
         */
        virtual bool HandleEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, EventDataInterface* data) = 0;

        //! Returns the state of the module
        virtual Module::State State() const = 0;

        //! By using this function for console commands, the command gets automatically
        //! registered / unregistered with the console when module is initialized / uninitialized
        virtual void AutoRegisterConsoleCommand(const Console::Command &command) = 0;

        //! Returns parent framework
        virtual Framework *GetFramework() const = 0;

        //! Returns major version as string. Override is module is not internal.
        virtual std::string VersionMajor() const = 0;

        //! Returns minor version as string. Override is module is not internal.
        virtual std::string VersionMinor() const = 0;

    private:
        virtual void SetFramework(Framework *framework) = 0;

        //! Called when module is loaded. Do not override in child classes. For internal use.
        virtual void LoadInternal() = 0;
        //! Called when module is unloaded. Do not override in child classes. For internal use.
        virtual void UnloadInternal() = 0;

        //! Initializes the module. Called when module is taken in use. Do not override in child classes. For internal use.
        virtual void InitializeInternal() = 0;
        //! Uninitialize the module. Called when module is removed from use. Do not override in child classes. For internal use.
        virtual void UninitializeInternal() = 0;
    };

    //! interface for modules, implementation
    class MODULE_API ModuleInterfaceImpl : public ModuleInterface
    {
        friend class ModuleManager;
    private:
        typedef std::vector<Console::Command> CommandVector;
    public:
    
        explicit ModuleInterfaceImpl(const std::string &name) : name_(name), type_(Module::MT_Unknown), state_(Module::MS_Unloaded), framework_(NULL)
        { 
            try
            {
                Poco::Logger::create(Name(),Poco::Logger::root().getChannel(), Poco::Message::PRIO_TRACE);    
            }
            catch (std::exception)
            {
                Foundation::RootLogError("Failed to create logger " + Name() + ".");
            }
        }

        explicit ModuleInterfaceImpl(Module::Type type) : type_(type), state_(Module::MS_Unloaded), framework_(NULL)
        { 
            try
            {
                Poco::Logger::create(Name(),Poco::Logger::root().getChannel(),Poco::Message::PRIO_TRACE);           
            }
            catch (std::exception)
            {
                Foundation::RootLogError("Failed to create logger " + Name() + ".");
            }
        }

        virtual ~ModuleInterfaceImpl()
        {
            Poco::Logger::destroy(Name());
        }

        virtual void PreInitialize() {}
        virtual void PostInitialize() {}
        
        virtual void Update(Core::f64 frametime) {}

        virtual const std::string &Name() const { return (type_ == Module::MT_Unknown ? name_ : Module::NameFromType(type_)); }
        virtual Module::Type Type() const { return type_; }
        virtual bool IsInternal() const { return type_ != Module::MT_Unknown; }

        virtual void DeclareComponent(const ComponentRegistrarInterfacePtr &registrar)
        {
            component_registrars_.push_back(registrar);
        }
        
        virtual bool HandleEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, EventDataInterface* data) { return false; }

        virtual Module::State State() const { return state_; }

        virtual void AutoRegisterConsoleCommand(const Console::Command &command)
        {
            for ( CommandVector::iterator it = console_commands_.begin() ; 
                  it != console_commands_.end() ;
                  ++it )
            {
                if (it->name_ == command.name_)
                    assert (false && "Registering console command twice");
            }

            console_commands_.push_back(command); 
        }

        virtual Framework *GetFramework() const { return framework_; }

        virtual std::string VersionMajor() const
        {
            if (IsInternal())
            {
                static const std::string version("version_major");
                return ( GetFramework()->GetDefaultConfig().GetString(Framework::ConfigurationGroup(), version) );
            }
            return std::string("0");
        }

        virtual std::string VersionMinor() const
        {
            if (IsInternal())
            {
                static const std::string version("version_minor");
                return ( GetFramework()->GetDefaultConfig().GetString(Framework::ConfigurationGroup(), version) );
            }
            return std::string("0");
        }
    protected:
        //! parent framework
        Framework *framework_;
        
    private:
        virtual void LoadInternal() { assert(state_ == Module::MS_Unloaded); Load(); state_ = Module::MS_Loaded; }
        virtual void UnloadInternal() { assert(state_ == Module::MS_Loaded); Unload(); state_ = Module::MS_Unloaded; }
        virtual void SetFramework(Framework *framework) { framework_ = framework; assert (framework_); }

        //! Registers all declared components
        virtual void InitializeInternal()
        {
            assert(framework_ != NULL);
            assert (state_ == Module::MS_Loaded);

            //! Register components
            for (size_t n=0 ; n<component_registrars_.size() ; ++n)
            {
                component_registrars_[n]->Register(framework_, this);
            }
            
            //! Register commands
            for ( CommandVector::iterator it = console_commands_.begin() ; 
                  it != console_commands_.end() ;
                  ++it )
            {
                if (framework_->GetServiceManager()->IsRegistered(Service::ST_ConsoleCommand))
                {
                    Console::CommandService *console = framework_->GetService<Console::CommandService>(Service::ST_ConsoleCommand);
                    console->RegisterCommand(*it);
                }
            }

            Initialize();
            state_ = Module::MS_Initialized;
        }

        //! Unregisters all declared components
        virtual void UninitializeInternal()
        {
            assert(framework_ != NULL);
            assert (state_ == Module::MS_Initialized);

            for (size_t n=0 ; n<component_registrars_.size() ; ++n)
            {
                component_registrars_[n]->Unregister(framework_);
            }

            //! Unregister commands
            for ( CommandVector::iterator it = console_commands_.begin() ; 
                  it != console_commands_.end() ;
                  ++it )
            {
                if (framework_->GetServiceManager()->IsRegistered(Service::ST_ConsoleCommand))
                {
                    Console::CommandService *console = framework_->GetService<Console::CommandService>(Service::ST_ConsoleCommand);
                    console->UnregisterCommand(it->name_);
                }
            }

            Uninitialize();

            // The module is now uninitialized, but it is still loaded in memory.
            // The module can now be initialized again, and InitializeInternal()
            // expects the state to be Module::MS_Loaded.
            state_ = Module::MS_Loaded;
        }

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

#endif

