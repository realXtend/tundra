
#ifndef __inc_Foundation_ModuleManager_h__
#define __inc_Foundation_ModuleManager_h__

namespace Foundation
{
    class ModuleInterface;
    class Framework;

    //! Managers modules or plugins. Modules are loaded runtime.
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
            - Add the module to the modules.xml file so it gets loaded runtime. If the name of the class that
              inherits from ModuleInterface_Impl is different from the name of the actual module, add attribute
              'entry="CLASS_NAME".

        \todo Track which modules are enabled (initialized) and which are not
        \note Do not use directly!
    */
    class ModuleManager
    {
    public:
        ModuleManager(Framework *framework) : mFramework(framework) {}
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
    private:

        void loadModule(const std::string &moduleName, const std::string &entryPoint);

        //! Initialize the specified module
        void initializeModule(ModuleInterface *module);

        void uninitializeModule(ModuleInterface *module);

        //! list of modules managed by this manager
        std::vector<ModuleInterface*> mModules;

        Framework *mFramework;
    };
}

#endif // __inc_Foundation_ModuleManager_h__
