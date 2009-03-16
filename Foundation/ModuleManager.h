// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_ModuleManager_h
#define incl_Foundation_ModuleManager_h

#include "ModuleInterface.h"


namespace Foundation
{
    class ModuleInterface;
    class Framework;

    //! Managers modules. Modules are loaded at runtime.
    /*! Assumption is that all modules get loaded when program is started, and unloaded when program exits.
        Modules may get initialized and uninitialized any number of times during the program's life time.

        To create a new static module:
            - Create a class that inherits from ModuleInterface_Impl
            
            - In the Module's load()-function, declare all components the new module offers with DECLARE_MODULE_EC macro.
            
            - In the Module's initialize()-function, register all services the new module offers
            
            - Also unregister all services in the uninitialize()-function.
            
            - Declare the static module with DeclareStaticModule() function.
            
            - Add the type of the module to both Foundation::Module::Type enumeration and
              Foundation::Module::NameFromType function, in ModuleInterface.h.


        Additional steps to create a new shared module:
            - Do not use the DeclareStaticModule() function
            - Copy following to the implementation (cpp file) of the Module-class:

                    POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
                        POCO_EXPORT_CLASS(CLASS_NAME)
                    POCO_END_MANIFEST
                
              where CLASS_NAME is the name of the newly created class.
            
            - Create a module definition file (xml file) that matches the name of the sub-project
              and that contains the names of the module entry classes (modules that are contained in
              a single sub-project, often just one),

              With only a single module in the sub-project and whose entry class matches the name of
              the project, the xml file can be an empty file.

              Place the xml file in the sub-project directory.

            - In a post-build step, both the module and the xml file will get copied to bin/modules
              directory (or one of it's subdirectories).


        You can define multiple modules in single library file.
        With static module, just declare each module to the module manager.
        With shared module:
            - Add each new class that defines a new module to Poco export manifest:

                    POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
                        POCO_EXPORT_CLASS(CLASS_NAME_A)
                        POCO_EXPORT_CLASS(CLASS_NAME_B)
                    POCO_END_MANIFEST
              
              There should only be one Poco manifest section per library!
            - Add an entry to the module definition file for each exported class.


        The preferred way to communicate between modules is by using services,
        see ServiceManager for more information. It is also possible to communicate
        by accessing modules directly.

        Accessing modules directly using ModuleManager:
            - All classes that need to be accessed should be declared with REX_API macro.
            - MODULE_EXPORTS should be defined in both modules.
            - Add the module's directory to includes
            - Link against the module library
           

        \todo Track which modules are enabled (initialized) and which are not
    */
    class ModuleManager
    {
    public:
        ModuleManager(Framework *framework);
        ~ModuleManager();

        //! Declare a module from static library
        /*! Use 'new' to create the module. The framework will take responsibility of the
            declared module and will delete it after unloading it.

            Loads the module immediatelly.
        */
        void DeclareStaticModule(ModuleInterface *module);

        //! Specify a module that should not be loaded or initialized under any circumstances
        /*! Only works for core modules.

            \param type Type of the module that should be excluded.
        */
        void ExcludeModule(Module::Type type)
        {
            exclude_list_.insert(type);

            Foundation::RootLogInfo("Module: " + Module::NameFromType(type) + " added to exclude list.");
        }

        //! Returns true if the specified module type is exluded from being loaded
        bool IsExcluded(Module::Type type)
        {
            return (exclude_list_.find(type) != exclude_list_.end());
        }

        //! loads all available modules. Does not initialize them.
        void LoadAvailableModules();

        //! unloads all available modules. Modules does not get unloaded as such, only the module's unload() function will be called
        /*! Assumptions is that modules only get unloaded once the program exits.

            \todo Find a better way to handle unloading of modules / actually unload modules?
        */
        void UnloadModules();

        //! initialize all modules
        /*! All static modules should be declared before calling this.

            \note should only be called once, when firing up the framework
        */
        void InitializeModules();

        //! uninitialize all modules
        void UninitializeModules();

        //! perform synchronized update on all modules
        void UpdateModules();

        //! Returns module by name
        /*!
            \note The pointer may invalidate between frames, always reacquire at begin of frame update
        */
        ModuleInterface *GetModule(const std::string &name)
        {
            ModuleVector::iterator it = modules_.begin();
            for ( ; it != modules_.end() ; ++it)
            {
                if ( (*it)->Name() == name )
                    return *it;
            }
            return NULL;
        }

        //! Returns module by type
        /*!
            \note The pointer may invalidate between frames, always reacquire at begin of frame update
        */
        template <class T>
        T *GetModule(Foundation::Module::Type type)
        {
            assert (type != Module::MT_Unknown);

            ModuleVector::iterator it = modules_.begin();
            for ( ; it != modules_.end() ; ++it)
            {
                if ( (*it)->Type() == type )
                    return (static_cast<T*>(*it));
            }
            return NULL;
        }

        //! Returns true if module is loaded, false otherwise
        bool HasModule(Module::Type type) const
        {
            return HasModule(Module::NameFromType(type));
        }

        //! Returns true if module is loaded, false otherwise
        bool HasModule(const std::string &name) const
        {
            for (size_t i = 0 ; i < modules_.size() ; ++i)
            {
                if (modules_[i]->Name() == name)
                    return true;
            }
            return false;
        }

        //! Loads and initializes a module with specified name
        /*! For internal use only!

            \param name name of the module to load
        */
        void LoadModuleByName(const std::string &name);

    private:

        //! Loads module
        /*!
            \param path path to module definition file (xml)
        */
        void LoadModule(const boost::filesystem::path &path);

        //! loads module
        /*!
            \param moduleName path to the module
            \param entryPoint name of the entry class
        */
        void LoadModule(const std::string &moduleName,  const Core::StringVector &entries);

        //! Initialize the specified module
        void InitializeModule(ModuleInterface *module);

        //! Uninitialize the specified module
        void UninitializeModule(ModuleInterface *module);

        //! returns true if module is present
        bool HasModule(ModuleInterface *module);

        //! Returns a vector containing all xml files in the specified directory, scans recursively.
        Core::StringVectorPtr GetXmlFiles(const std::string &path);

        typedef std::vector<ModuleInterface*> ModuleVector;

        const std::string DEFAULT_MODULES_PATH;

        typedef std::set<Module::Type> ModuleTypeSet;

        //! list of modules managed by this manager
        ModuleVector modules_;

        //! List of modules that should be excluded
        ModuleTypeSet exclude_list_;

        Framework *framework_;
    };
}

#endif // incl_Foundation_ModuleManager_h
