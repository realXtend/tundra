// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ModuleManager.h"
#include "Framework.h"
#include "LoggingFunctions.h"

#include "ConfigAPI.h"
#include "CoreException.h"

#include <algorithm>
#include <sstream>

#include <QDir>

#include "MemoryLeakCheck.h"

namespace fs = boost::filesystem;

typedef void (*SetProfilerFunc)(Foundation::Profiler *profiler);

ModuleManager::ModuleManager(Foundation::Framework *framework) :
    framework_(framework)
{
}

ModuleManager::~ModuleManager()
{
    /// \todo This might cause problems, investigate.
    //Unload all loaded modules if object is truely destroyed. 
    UninitializeModules();
    UnloadModules();
}

void ModuleManager::DeclareStaticModule(IModule *module)
{
    assert (module);
    if (IsExcluded(module->Name()) == false && HasModule(module) == false)
    {
        ModuleSharedPtr modulePtr = ModuleSharedPtr(module);
        Module::Entry entry = { modulePtr, module->Name(), Module::SharedLibraryPtr() };
        modules_.push_back(entry);
#ifndef _DEBUG
        // make it so debug messages are not logged in release mode
        /// \todo why is this even here? does not seem to do anything?
        QString log_level = framework_->Config()->Get(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_FRAMEWORK, "log level").toString();
        /*
        if (framework_->GetDefaultConfig().HasKey(Foundation::Framework::ConfigurationGroup(), "log_level"))
            log_level = framework_->GetDefaultConfig().GetSetting<std::string>(Foundation::Framework::ConfigurationGroup(), "log_level");
        else
            framework_->GetConfigManager()->SetSetting(Foundation::Framework::ConfigurationGroup(), "log_level", log_level);
        */
#endif
        module->SetFramework(framework_);
        module->LoadInternal();
    }
    else
    {
        LogInfo("Module: " + module->Name() + " is excluded and not loaded.");
    }
}

void ModuleManager::InitializeModules()
{
    for(size_t i = 0; i < modules_.size(); ++i)
    {
        IModule *mod = modules_[i].module_.get();
        if (mod->State() != MS_Initialized)
            PreInitializeModule(mod);
    }

    for(size_t i = 0; i < modules_.size(); ++i)
    {
        IModule *mod = modules_[i].module_.get();
        if (mod->State() != MS_Initialized)
            InitializeModule(mod);
    }

    for(size_t i = 0; i < modules_.size(); ++i)
        PostInitializeModule(modules_[i].module_.get());
}

void ModuleManager::UninitializeModules()
{
    for(ModuleVector::reverse_iterator it = modules_.rbegin(); it != modules_.rend(); ++it)
        UninitializeModule(it->module_.get());
}

void ModuleManager::UpdateModules(f64 frametime)
{
    for(size_t i = 0; i < modules_.size(); ++i)
    {
        try
        {
            modules_[i].module_->Update(frametime);
        }
        catch(const std::exception &e)
        {
            std::cout << "UpdateModules caught an exception while updating module " << modules_[i].module_->Name()
                << ": " << (e.what() ? e.what() : "(null)") << std::endl;
            LogCritical(std::string("UpdateModules caught an exception while updating module " + modules_[i].module_->Name()
                + ": " + (e.what() ? e.what() : "(null)")));
            throw;
        }
        catch(...)
        {
            std::cout << "UpdateModules caught an unknown exception while updating module " << modules_[i].module_->Name() << std::endl;
            LogCritical(std::string("UpdateModules caught an unknown exception while updating module " + modules_[i].module_->Name()));
            throw;
        }
    }
}

ModuleWeakPtr ModuleManager::GetModule(const std::string &name)
{
    for(ModuleVector::iterator it = modules_.begin(); it != modules_.end() ; ++it)
        if (it->module_->Name() == name)
            return ModuleWeakPtr(it->module_);
    return ModuleWeakPtr();
}

ModuleWeakPtr ModuleManager::GetModule(IModule* rawptr)
{
    for(ModuleVector::iterator it = modules_.begin(); it != modules_.end() ; ++it)
        if (it->module_.get() == rawptr)
            return ModuleWeakPtr(it->module_);
    return ModuleWeakPtr();
}

bool ModuleManager::HasModule(const std::string &name) const
{
    for(size_t i = 0 ; i < modules_.size() ; ++i)
        if (modules_[i].module_->Name() == name)
            return true;
    return false;
}

void ModuleManager::UnloadModules()
{
    ///\todo Reimplement.
    /*
    for(ModuleVector::reverse_iterator it = modules_.rbegin(); it != modules_.rend(); ++it)
        UnloadModule(*it);

    modules_.clear();
    assert (modules_.empty());
    */
}

void ModuleManager::PreInitializeModule(IModule *module)
{
    assert(module);
    assert(module->State() == MS_Loaded);
    LogDebug("Preinitializing module " + module->Name());
    module->PreInitializeInternal();

    // Do not log preinit success here to avoid extraneous logging.
}

void ModuleManager::InitializeModule(IModule *module)
{
    assert(module);
    assert(module->State() == MS_Loaded);
    LogDebug("Initializing module " + module->Name());
    module->InitializeInternal();
}

void ModuleManager::PostInitializeModule(IModule *module)
{
    assert(module);
    assert(module->State() == MS_Loaded);
    LogDebug("Postinitializing module " + module->Name());
    module->PostInitializeInternal();

    // Do not log postinit success here to avoid extraneous logging.
}

void ModuleManager::UninitializeModule(IModule *module)
{
    assert(module);
    LogDebug("Uninitializing module " + module->Name() + ".");
    module->UninitializeInternal();
}

bool ModuleManager::HasModule(IModule *module) const
{
    assert (module);
    for(ModuleVector::const_iterator it = modules_.begin(); it != modules_.end(); ++it)
        if (it->module_->Name() == module->Name())
            return true;
    return false;
}
