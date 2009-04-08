// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_ModuleManager_h
#define incl_Foundation_ModuleManager_h

#include "ModuleInterface.h"


namespace Foundation
{
    class ModuleInterface;
    class Framework;

    namespace Module
    {
        //! entry for shared library that may contain module(s)
        struct SharedLibrary : public boost::noncopyable
        {
            SharedLibrary();
            SharedLibrary(const std::string &path) : path_(path) { cl_.loadLibrary(path_); }
            ~SharedLibrary() { cl_.unloadLibrary(path_); }

            //! path to the shared library
            std::string path_;
            //! class loader
            Poco::ClassLoader<ModuleInterface> cl_;
        };
        typedef boost::shared_ptr<SharedLibrary> SharedLibraryPtr;

        //! Module entry. Contains information about a module.
        struct Entry
        {
            //! the module
            ModuleInterface *module_;
            //! entry class of the module
            std::string entry_;
            //! shared library this module was loaded from. Null for static library
            SharedLibraryPtr shared_library_;
        };
    }

    //! Managers modules. Modules are loaded at runtime.
    /*! Assumption is that all modules get loaded when program is started, and unloaded when program exits.
        Modules may get initialized and uninitialized any number of times during the program's life time.

        Module versioning: core modules have the same version as the framework. Other modules are
                           responsible for handling their own versioning information.

        To create a new static module:
            - Create a class that inherits from ModuleInterfaceImpl
            
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
              
            - Module dependencies can also be specified in the module definition file. The dependencies
              will get loaded/initialized before the module itself. In this case, 
              the module definition file should look like this
              \verbatim
              <config>
                   <entry>YourEntryClassName</entry>
                   <dependency>ModuleName</dependency>
              </config>
              \endverbatim

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
            - All classes that need to be accessed should be declared with MODULE_API macro.
            - MODULE_EXPORTS should be defined in both modules.
            - Add the module's directory to includes
            - Link against the module library
           

        \todo Track which modules are enabled (initialized) and which are not
    */
    class ModuleManager
    {
    public:
        typedef std::vector<Module::Entry> ModuleVector;

        ModuleManager(Framework *framework);
        ~ModuleManager();

        //! Declare a module from static library
        /*! Use 'new' to create the module. The framework will take responsibility of the
            declared module and will delete it after unloading it.

            Loads the module immediatelly.
        */
        void DeclareStaticModule(ModuleInterface *module);

        //! Specify a module by type that should not be loaded or initialized under any circumstances
        /*! 
            \note Only call during application preinit phase.

            \param type Type of the module that should be excluded.
        */
        void ExcludeModule(Module::Type type)
        {
            ExcludeModule(Module::NameFromType(type));
        }

        //! Specify a module by name that should not be loaded or initialized under any circumstances
        /*! 
            \note Only call during application preinit phase.

            \param module Name of the module that should be excluded.
        */
        void ExcludeModule(const std::string &module)
        {
            assert (module.empty() == false);
            exclude_list_.insert(module);

            Foundation::RootLogInfo("Module: " + module + " added to exclude list.");
        }

        //! Returns true if the specified module type is exluded from being loaded
        bool IsExcluded(Module::Type type) const
        {
            return IsExcluded(Module::NameFromType(type));
        }

        //! Returns true if the specified module is excluded from being loaded
        bool IsExcluded(const std::string &module) const
        {
            return (exclude_list_.find(module) != exclude_list_.end());
        }

        //! loads all available modules. Does not initialize them.
        void LoadAvailableModules();

        //! unloads all available modules. Modules does not get unloaded as such, only the module's unload() function will be called
        /*! Assumptions is that modules only get unloaded once the program exits.

            PostCondition: HasModule(module) == false for any module

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
        void UpdateModules(double frametime);

        //! Returns module by name
        /*!
            \note The pointer may invalidate between frames, always reacquire at begin of frame update
        */
        ModuleInterface *GetModule(const std::string &name)
        {
            ModuleVector::iterator it = modules_.begin();
            for ( ; it != modules_.end() ; ++it)
            {
                if ( it->module_->Name() == name )
                    return it->module_;
            }
            return NULL;
        }

        //! Returns module by type
        /*!
            \note The pointer may invalidate between frames, always reacquire at begin of frame update
        */
        ModuleInterface *GetModule(Foundation::Module::Type type)
        {
            return GetModule(Module::NameFromType(type));
        }

        //! Returns module by type
        /*!
            \note The pointer may invalidate between frames, always reacquire at begin of frame update

            \param type type of the module
            \return The module, or null if the module of type 'type' was not found, or if dynamic cast fails
        */
        template <class T>
        T *GetModule(Foundation::Module::Type type)
        {
            assert (type != Module::MT_Unknown);

            ModuleVector::iterator it = modules_.begin();
            for ( ; it != modules_.end() ; ++it)
            {
                if ( it->module_->Type() == type )
                    return (dynamic_cast<T*>(it->module_));
            }
            return NULL;
        }

        //! @return A list of all modules in the system, for reflection purposes. If you need non-const access to
        //!         a module, call GetModule with the proper name or type.
        const ModuleVector &GetModuleList() const { return modules_; }

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
                if (modules_[i].module_->Name() == name)
                    return true;
            }
            return false;
        }

        bool HasModuleEntry(const std::string &entry) const
        {
            for (size_t i = 0 ; i < modules_.size() ; ++i)
            {
                if (modules_[i].entry_ == entry)
                    return true;
            }
            return false;
        }

        //! Loads and initializes a module with specified name
        /*! For internal use only!

            \note currently works only if one module is contained in the lib!

            \param lib name of the shared library to load from, without extension or d postfix
            \param module name of the module to load from the lib
            \return True if the module was loaded succesfully, false otherwise
        */
        bool LoadModuleByName(const std::string &lib, const std::string &module);

        //! Unloads a specific module by name
        /*! Precondition: HasModule(module)
            Postcondition: HasModule(module) == false

            \param module name of the module
        */
        bool UnloadModuleByName(const std::string &module);

        //! Pre-initialize the specified module
        void PreInitializeModule(ModuleInterface *module);
        
        //! Initialize the specified module
        void InitializeModule(ModuleInterface *module);

        //! Post-initialize the specified module
        void PostInitializeModule(ModuleInterface *module);

        //! Uninitialize the specified module
        void UninitializeModule(ModuleInterface *module);

        //! Unloads and deletes the module.
        //! \note Does not remove from modules_
        void UnloadModule(Module::Entry &entry);

    private:

        //! Loads module
        /*!
            \param path path to module definition file (xml)
            \param all_files stringvector of all module definitions, so that dependencies can be processed
        */
        void LoadModule(const boost::filesystem::path &path, Core::StringVectorPtr all_files);

        //! loads module
        /*!
            \param moduleName path to the shared lib containing the modules
            \param entries name of the entry classes in the lib
        */
        void LoadModule(const std::string &moduleName,  const Core::StringVector &entries);

        //! returns true if module is present
        bool HasModule(ModuleInterface *module) const;

        //! Returns a vector containing all xml files in the specified directory, scans recursively.
        Core::StringVectorPtr GetXmlFiles(const std::string &path);

        const std::string DEFAULT_MODULES_PATH;

        typedef std::set<std::string> ModuleTypeSet;

        //! list of modules managed by this manager
        ModuleVector modules_;

        //! List of modules that should be excluded
        ModuleTypeSet exclude_list_;
        
        Framework *framework_;
    };
}

#endif // incl_Foundation_ModuleManager_h
