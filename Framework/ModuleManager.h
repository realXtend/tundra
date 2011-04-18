// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_ModuleManager_h
#define incl_Foundation_ModuleManager_h

#include <set>
#include <boost/filesystem.hpp>

#include "IModule.h"
#include "ModuleReference.h"
#include "LoggingFunctions.h"

namespace fs = boost::filesystem;

namespace Foundation
{
    class Framework;
}


/// Manages run-time loadable and unloadable modules.
/** See \ref ModuleArchitecture for details on how to use.  

    \ingroup Foundation_group
    \ingroup Module_group
*/
class ModuleManager
{
public:
    typedef std::vector<ModuleSharedPtr> ModuleVector;

    explicit ModuleManager(Foundation::Framework *framework);
    ~ModuleManager();

    /// Declare a module from static library
    /** Use 'new' to create the module. The framework will take responsibility of the
        declared module and will delete it after unloading it.

        Loads the module immediatelly. */
    void DeclareStaticModule(IModule *module);

    /// unloads all available modules. Modules does not get unloaded as such, only the module's unload() function will be called
    /** Assumptions is that modules only get unloaded once the program exits.

        PostCondition: HasModule(module) == false for any module */
    void UnloadModules();

    /// initialize all modules
    /** All static modules should be declared before calling this.

        \note should only be called once, when firing up the framework */
    void InitializeModules();

    /// uninitialize all modules
    void UninitializeModules();

    /// perform synchronized update on all modules
    void UpdateModules(f64 frametime);

    /// Returns module by name
    /// \note The pointer may invalidate between frames, always reacquire at begin of frame update
    ModuleSharedPtr GetModule(const std::string &name);

    /** Returns module by class T.
        @param T class type of the module.
        @return The module, or null if the module doesn't exist and dynamic cast fails.
        @note The pointer may invalidate between frames, always reacquire at begin of frame update */
    template <class T>
    boost::shared_ptr<T> GetModule()
    {
        for(ModuleVector::iterator it = modules_.begin(); it != modules_.end(); ++it)
        {
            boost::shared_ptr<T> module = boost::dynamic_pointer_cast<T>(*it);
            if (module)
                return module;
        }

        return boost::shared_ptr<T>();
    }

    /// @return A list of all modules in the system, for reflection purposes. If you need non-const access to
    ///         a module, call GetModule with the proper name or type.
    const ModuleVector &GetModuleList() const { return modules_; }

    /// Pre-initialize the specified module
    void PreInitializeModule(IModule *module);
    
    /// Initialize the specified module
    void InitializeModule(IModule *module);

    /// Post-initialize the specified module
    void PostInitializeModule(IModule *module);

    /// Uninitialize the specified module
    void UninitializeModule(IModule *module);

    /// Unloads and deletes the module.
    /// \note Does not remove from modules_
//    void UnloadModule(Module::Entry &entry);

private:
    /// list of modules managed by this manager
    ModuleVector modules_;

    /// Framework pointer.
    Foundation::Framework *framework_;
};

#endif
