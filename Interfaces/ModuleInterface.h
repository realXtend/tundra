// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_ModuleInterface_h
#define incl_Interfaces_ModuleInterface_h

#include <Poco/ClassLibrary.h>
#include "ComponentRegistrarInterface.h"

#define DECLARE_MODULE_EC(component) \
    { Foundation::ComponentRegistrarInterfacePtr registrar = Foundation::ComponentRegistrarInterfacePtr(new component::component##Registrar); \
    declareComponent(registrar); } \

namespace Foundation
{
    class Framework;

    namespace Module
    {
        //! internal module types.
        /*!
            \note if you add new internal module type, don't forget to add it's name to nameFromType()
        */
        enum Type { Type_Geometry = 0, Type_Renderer, Type_Sound, Type_Gui, Type_WorldLogic, Type_Test, Type_Unknown };

        const std::string &nameFromType(Type type)
        {
            assert(type != Type_Unknown);

            static const std::string type_strings[Type_Unknown] = { "Geometry", "Renderer", "Sound", "Gui", "World Logic", "Test" };
            return type_strings[type];
        }
    }

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
        virtual void load() = 0;
        //! called when module is unloaded from memory. Do not trust that framework can be used.
        virtual void unload() = 0;

        //! Initialized the module. Called when module is taken in use. Do not override in child classes.
        virtual void _initialize(Framework *framework) = 0;
        //! Uninitialize the module. Called when module is removed from use. Do not override in child classes.
        virtual void _uninitialize(Foundation::Framework *framework) = 0;

        //! Initialized the module. Called when module is taken in use
        virtual void initialize(Framework *framework) = 0;
        //! Uninitialize the module. Called when module is removed from use
        virtual void uninitialize(Foundation::Framework *framework) = 0;

        //! synchronized update for the module
        virtual void update() = 0;

        //! Returns the name of the module
        virtual const std::string &name() const = 0;
        //! Returns internal type of the module or Type_Unknown if module is not internal
        virtual Module::Type type() const = 0;

        virtual void declareComponent(const ComponentRegistrarInterfacePtr &registrar) = 0;
    };

    //! interface for modules, implementation
    class ModuleInterface_Impl : public ModuleInterface
    {
    public:
        explicit ModuleInterface_Impl(const std::string &name) : mName(name), mType(Module::Type_Unknown) {}
        explicit ModuleInterface_Impl(Module::Type type) : mType(type) {}
        virtual ~ModuleInterface_Impl() {}

        //! Registers all declared components
        virtual void _initialize(Framework *framework)
        {
            assert(framework != NULL);

            for (size_t n=0 ; n<ComponentRegistrars.size() ; ++n)
            {
                ComponentRegistrars[n]->_register(framework);
            }
            initialize(framework);
        }

        //! Unregisters all declared components
        virtual void _uninitialize(Foundation::Framework *framework)
        {
            assert(framework != NULL);

            for (size_t n=0 ; n<ComponentRegistrars.size() ; ++n)
            {
                ComponentRegistrars[n]->_unregister(framework);
            }

            uninitialize(framework);
        }

        virtual void update() {}

        virtual const std::string &name() const { return (mType == Module::Type_Unknown ? mName : Module::nameFromType(mType)); }
        virtual Module::Type type() const { return mType; }

        virtual void declareComponent(const ComponentRegistrarInterfacePtr &registrar)
        {
            ComponentRegistrars.push_back(registrar);
        }

    private:
        typedef std::vector<ComponentRegistrarInterfacePtr> RegistrarVector;

        RegistrarVector ComponentRegistrars;

        //! name of the module
        const std::string mName;
        //! type of the module if inbuild, unknown otherwise
        const Module::Type mType;
    };
}

#endif

