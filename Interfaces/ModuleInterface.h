// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_ModuleInterface_h
#define incl_Interfaces_ModuleInterface_h

#include <Poco/ClassLibrary.h>
#include <Poco/Logger.h>

#include "ComponentRegistrarInterface.h"
#include "CoreTypes.h"

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
    static void LogTrace(const std::string &msg)    { Poco::Logger::get(NameStatic()).trace(msg);         }   

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
			MT_Network,
			MT_Test,
			MT_NetTest,
			MT_Python,
            MT_Console,
			MT_Unknown
		};

        static const std::string &NameFromType(Type type)
        {
            assert(type != MT_Unknown);

            static const std::string type_strings[MT_Unknown] = { "SceneModule", "Renderer", "Sound", "Gui", "World Logic", "Network", "Test", "NetTest", "Python", "Console" };

            return type_strings[type];
        }
    }

    //! interface for modules
    /*! See ModuleManager for more info.
      
        \note Every module should have a name. Only internal modules have types.
    */
    class REX_API ModuleInterface
    {
    public:
        ModuleInterface()  {}
        virtual ~ModuleInterface() {}

        //! called when module is loaded into memory.Do not trust that framework can be used.
        /*!
            Components in the module should be declared here by using DECLARE_MODULE_EC(Component) macro, where
            Component is the class of the component.
        */
        virtual void Load() = 0;
        //! called when module is unloaded from memory. Do not trust that framework can be used.
        virtual void Unload() = 0;

        //! Initialized the module. Called when module is taken in use. Do not override in child classes. For internal use.
        virtual void _Initialize(Framework *framework) = 0;
        //! Uninitialize the module. Called when module is removed from use. Do not override in child classes. For internal use.
        virtual void _Uninitialize(Foundation::Framework *framework) = 0;

        //! Pre-initialization for the module. Called before modules are initializated.
        virtual void PreInitialize(Framework *framework) = 0;
        //! Initializes the module. Called when module is taken in use
        virtual void Initialize(Framework *framework) = 0;
        //! Post-initialization for the module. At this point Initialize() has been called for all enabled modules.
        virtual void PostInitialize(Framework *framework) = 0;
        
        //! Uninitialize the module. Called when module is removed from use
        virtual void Uninitialize(Foundation::Framework *framework) = 0;

        //! synchronized update for the module
        virtual void Update() = 0;

        //! Returns the name of the module. Each module also has a static accessor for the name, it's needed by the logger.
        virtual const std::string &Name() const = 0;
        //! Returns internal type of the module or MT_Unknown if module is not internal
        virtual Module::Type Type() const = 0;

        //! Declare a component the module defines. For internal use.
        virtual void DeclareComponent(const ComponentRegistrarInterfacePtr &registrar) = 0;
        
        //! Receives an event
        /*! Should return true if the event was handled and is not to be propagated further
         */
        virtual bool HandleEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, EventDataInterface* data) = 0;
    };

    //! interface for modules, implementation
    class REX_API ModuleInterface_Impl : public ModuleInterface
    {
    public:
    
        explicit ModuleInterface_Impl(const std::string &name) : name_(name), type_(Module::MT_Unknown) 
        { 
            Poco::Logger::create(Name(),Poco::Logger::root().getChannel(),Poco::Message::PRIO_TRACE);     
        }

        explicit ModuleInterface_Impl(Module::Type type) : type_(type) 
        { 
            Poco::Logger::create(Name(),Poco::Logger::root().getChannel(),Poco::Message::PRIO_TRACE);           
        }

        virtual ~ModuleInterface_Impl() {}

        //! Registers all declared components
        virtual void _Initialize(Framework *framework)
        {
            assert(framework != NULL);

            for (size_t n=0 ; n<component_registrars_.size() ; ++n)
            {
                component_registrars_[n]->Register(framework);
            }
            Initialize(framework);
        }

        //! Unregisters all declared components
        virtual void _Uninitialize(Foundation::Framework *framework)
        {
            assert(framework != NULL);

            for (size_t n=0 ; n<component_registrars_.size() ; ++n)
            {
                component_registrars_[n]->Unregister(framework);
            }

            Uninitialize(framework);
        }

        virtual void PreInitialize(Framework *framework) {}
        virtual void PostInitialize(Framework *framework) {}
        
        virtual void Update() {}

        virtual const std::string &Name() const { return (type_ == Module::MT_Unknown ? name_ : Module::NameFromType(type_)); }
        virtual Module::Type Type() const { return type_; }

        virtual void DeclareComponent(const ComponentRegistrarInterfacePtr &registrar)
        {
            component_registrars_.push_back(registrar);
        }
        
        virtual bool HandleEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, EventDataInterface* data) { return false; }
        
    private:
        typedef std::vector<ComponentRegistrarInterfacePtr> RegistrarVector;

        RegistrarVector component_registrars_;

        //! name of the module
        const std::string name_;
        //! type of the module if inbuild, unknown otherwise
        const Module::Type type_;
    };
}

#endif

