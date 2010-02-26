// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_ModuleManager_h
#define incl_Foundation_ModuleManager_h

#include <boost/filesystem.hpp>
#include <Poco/SharedLibrary.h>
#include <Poco/ClassLoader.h>

#include "ModuleInterface.h"
#include "ModuleReference.h"

namespace fs = boost::filesystem;

namespace Foundation
{
    class ModuleInterface;
    class Framework;

    /*! \defgroup Module_group Module Architecture Client Interface
        \copydoc Module
    */

    //! Module specific parts of the foundation.
    /*! Generally module handling is done in the background, so there
        is little need to access many of the things provided by this
        namespace.

        For details, see \ref ModuleArchitecture "Module Architecture".
    */
    namespace Module
    {
        //! entry for shared library that may contain module(s)
        struct SharedLibrary : public boost::noncopyable
        {
            SharedLibrary();
            SharedLibrary(const std::string &path);
            ~SharedLibrary();

            //! path to the shared library
            std::string path_;
            //! shared library
            Poco::SharedLibrary sl_;
            //! class loader
            Poco::ClassLoader<ModuleInterface> cl_;
        };
        typedef boost::shared_ptr<SharedLibrary> SharedLibraryPtr;

        //! A function object that deletes a SharedLibrary object once it is not used.
        //! Used as a Deletor object for boost::shared_ptr.
        class ModuleDeletor
        {
            std::string entry_;
            SharedLibraryPtr shared_library_;

        public:
            ModuleDeletor(const std::string &entry, SharedLibraryPtr shared_library);

            void operator()(ModuleInterface *module);
        };

        //! Module entry. Contains information about a module. Useful for ModuleManager introspection.
        /*! \ingroup Foundation_group
            \ingroup Module_group
        */
        struct Entry
        {
            //! The module. Memory owned by Poco if a shared library, by us using new/delete if static library.
            ModuleSharedPtr module_;
            //! entry class of the module
            std::string entry_;
            //! shared library this module was loaded from. Null for static library
            SharedLibraryPtr shared_library_;
        };
    }

    //! Manages run-time loadable and unloadable modules.
    /*! See \ref ModuleArchitecture for details on how to use.  

        \ingroup Foundation_group
        \ingroup Module_group
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

            Foundation::RootLogDebug("Added module " + module + " to exclude list");
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
        void UpdateModules(f64 frametime);

        //! Returns module by name
        /*!
            \note The pointer may invalidate between frames, always reacquire at begin of frame update
        */
        ModuleWeakPtr GetModule(const std::string &name)
        {
            ModuleVector::iterator it = modules_.begin();
            for (; it != modules_.end() ; ++it)
            {
                if (it->module_->Name() == name)
                    return ModuleWeakPtr(it->module_);
            }
            return ModuleWeakPtr();
        }

        //! Returns module by type
        /*!
            \note The pointer may invalidate between frames, always reacquire at begin of frame update
        */
        ModuleWeakPtr GetModule(Foundation::Module::Type type)
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
        boost::weak_ptr<T> GetModule(Foundation::Module::Type type)
        {
            assert (type != Module::MT_Unknown);

            for(ModuleVector::iterator it = modules_.begin(); it != modules_.end() ; ++it)
                if (it->module_->Type() == type)
                    return boost::dynamic_pointer_cast<T>(it->module_);
//                    return boost::weak_ptr<T>(boost::shared_ptr<T>());//dynamic_cast<T*>(it->module_));
//                    return boost::weak_ptr<T>(dynamic_cast<T*>(it->module_));

            return boost::weak_ptr<T>();
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

        //! @return True if the given module library entry is present, false otherwise.
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
            \return True if the module was loaded succesfully or if it already has been loaded, false otherwise
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
        void LoadModule(const fs::path &path, const StringVectorPtr &all_files);

        //! loads module
        /*!
            \param moduleName path to the shared lib containing the modules
            \param entries name of the entry classes in the lib
        */
        void LoadModule(const std::string &moduleName,  const StringVector &entries);

        //! returns true if module is present
        bool HasModule(ModuleInterface *module) const;

        //! Returns a vector containing all xml files in the specified directory, scans recursively.
        StringVectorPtr GetXmlFiles(const std::string &path);

        //! adds needed dependency paths to process path
        void AddDependenciesToPath(const StringVector &all_additions);

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
