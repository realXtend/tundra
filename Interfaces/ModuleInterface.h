// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_ModuleInterface_h
#define incl_Interfaces_ModuleInterface_h

#include <Poco/ClassLibrary.h>
#include "ComponentRegistrarInterface.h"

#define DECLARE_MODULE_EC(component) \
    { Foundation::ComponentRegistrarInterfacePtr registrar = Foundation::ComponentRegistrarInterfacePtr(new component::component##Registrar); \
    DeclareComponent(registrar); } \

namespace Foundation
{
    class Framework;

    class Module
    {
    public:
        //! internal module types.
        /*!
            \note if you add new internal module type, don't forget to add it's name to NameFromType()
        */
		enum Type 
		{
			Type_Scene = 0,
			Type_Renderer, 
			Type_Sound, 
			Type_Gui, 
			Type_WorldLogic, 
			Type_Network,
			Type_Test, 
			Type_Unknown
		};

        static const std::string &NameFromType(Type type)
        {
            assert(type != Type_Unknown);
            static const std::string type_strings[Type_Unknown] = { "Scene", "Renderer", "Sound", "Gui", "World Logic", "Network", "Test" };

            return type_strings[type];
        }
    };

    //! interface for modules
    /*! See ModuleManager for more info.
      
        \note Every module should have a name. Only internal modules have types.
    */
    class ModuleInterface
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

        //! Initializes the module. Called when module is taken in use
        virtual void Initialize(Framework *framework) = 0;
        //! Uninitialize the module. Called when module is removed from use
        virtual void Uninitialize(Foundation::Framework *framework) = 0;

        //! synchronized update for the module
        virtual void Update() = 0;

        //! Returns the name of the module
        virtual const std::string &Name() const = 0;
        //! Returns internal type of the module or Type_Unknown if module is not internal
        virtual Module::Type Type() const = 0;

        //! Declare a component the module defines. For internal use.
        virtual void DeclareComponent(const ComponentRegistrarInterfacePtr &registrar) = 0;
    };

    //! interface for modules, implementation
    class ModuleInterface_Impl : public ModuleInterface
    {
    public:
        explicit ModuleInterface_Impl(const std::string &name) : name_(name), type_(Module::Type_Unknown) {}
        explicit ModuleInterface_Impl(Module::Type type) : type_(type) {}
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

        virtual void Update() {}

        virtual const std::string &Name() const { return (type_ == Module::Type_Unknown ? name_ : Module::NameFromType(type_)); }
        virtual Module::Type Type() const { return type_; }

        virtual void DeclareComponent(const ComponentRegistrarInterfacePtr &registrar)
        {
            component_registrars_.push_back(registrar);
        }

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

