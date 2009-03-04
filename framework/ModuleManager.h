// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __inc_Foundation_ModuleManager_h__
#define __inc_Foundation_ModuleManager_h__

#include <boost/filesystem.hpp>

namespace Foundation
{
    class ModuleInterface;
    class Framework;

    //! Managers modules. Modules are loaded at runtime.
    /*! Assumption is that all modules get loaded when program is started, and unloaded when program exits.
        Modules may get initialized and uninitialized any number of times during the program's life time.

        To create a new module:
            - Create a class that inherits from ModuleInterface_Impl
            - Copy following to the implementation (cpp file) of the System-class:

                    POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
                        POCO_EXPORT_CLASS(CLASS_NAME)
                    POCO_END_MANIFEST
                
              where CLASS_NAME is the name of the newly created class.
            - In the System's load()-function, declare all components the new module offers with DECLARE_MODULE_EC macro.
            - In the System's initialize()-function, register all services the new module offers
            - Also unregister all services in the uninitialize()-function.
            - Copy the module library file to modules-directory. Add a module definition file (xml file) that matches
              the name of the module and that contains the module entry class name, if the name of the class that
              inherits from ModuleInterface_Impl is different from the name of the actual module. Otherwise the module
              definition file can be an empty file.

        \todo Track which modules are enabled (initialized) and which are not
        \note Do not use directly!
    */
    class ModuleManager
    {
    public:
        ModuleManager(Framework *framework);
        ~ModuleManager();

        //! loads all available modules. Does not initialize them.
        void loadAvailableModules();

        //! unloads all available modules. Modules does not get unloaded as such, only the module's unload() function will be called
        /*! Assumptions is that modules only get unloaded once the program exits.

            \todo Find a better way to handle unloading of modules / actually unload modules?
        */
        void unloadModules();

        //! initialize all modules
        void initializeModules();

        //! uninitialize all modules
        void uninitializeModules();

        //! perform synchronized update on all modules
        void updateModules();

        ////! Returns module by name
        //ModuleInterface *getModule(const std::string &name)
        //{
        //    ModuleVector::iterator it = mModules.begin();
        //    for ( ; it != mModules.end() ; ++it)
        //    {
        //        if ( (*it)->name() == name )
        //            return *it;
        //    }
        //    return NULL;
        //}

        ////! Returns module by type
        //template <class T>
        //T *getModule(Foundation::Module::Type type)
        //{
        //    assert (type != Module::Type_Unknown);

        //    ModuleVector::iterator it = mModules.begin();
        //    for ( ; it != mModules.end() ; ++it)
        //    {
        //        if ( (*it)->type() == type )
        //            return (static_cast<T*>(*it));
        //    }
        //    return NULL;
        //}

    private:

        //! Loads module
        /*!
            \param path path to module definition file (xml)
        */
        void loadModule(const boost::filesystem::path &path);

        //! loads module
        /*!
            \param moduleName path to the module
            \param entryPoint name of the entry class
        */
        void loadModule(const std::string &moduleName, const std::string &entryPoint);

        //! Initialize the specified module
        void initializeModule(ModuleInterface *module);

        void uninitializeModule(ModuleInterface *module);

        typedef std::vector<ModuleInterface*> ModuleVector;

        const std::string DEFAULT_MODULES_PATH;

        //! list of modules managed by this manager
        ModuleVector mModules;

        Framework *mFramework;
    };
}

#endif // __inc_Foundation_ModuleManager_h__
