// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MemoryLeakCheck.h"

#include "Framework.h"
#include "ModuleManager.h"
#include "ConfigurationManager.h"
#include "Poco/UnicodeConverter.h"
#include "CoreException.h"

#include <QString>

#include <Poco/Environment.h>

namespace fs = boost::filesystem;

typedef void (*SetProfilerFunc)(Foundation::Profiler *profiler);

namespace Foundation
{
    namespace Module
    {
        SharedLibrary::SharedLibrary(const std::string &path) : path_(path)
        {
            sl_.load(path); 
            cl_.loadLibrary(path_);
        }

        SharedLibrary::~SharedLibrary()
        {
            cl_.unloadLibrary(path_);
        }

        ModuleDeletor::ModuleDeletor(const std::string &entry, SharedLibraryPtr shared_library) :
            entry_(entry), shared_library_(shared_library)
        {
        }

        void ModuleDeletor::operator()(ModuleInterface *module)
        {
            if (shared_library_)
                shared_library_->cl_.destroy(entry_, module);

            delete module; // needed for modules not loaded through poco's SharedLibrary (static libs).
        }
    }

    ModuleManager::ModuleManager(Framework *framework) :
        framework_(framework),
        DEFAULT_MODULES_PATH(framework->GetDefaultConfig().DeclareSetting<std::string>("ModuleManager", "Default_Modules_Path", "./modules"))
    {
    }

    ModuleManager::~ModuleManager()
    {
        //! \todo This might cause problems, investigate.
        //Unload all loaded modules if object is truely destroyed. 
        UninitializeModules();
        UnloadModules();
    }

    void ModuleManager::DeclareStaticModule(ModuleInterface *module)
    {
        assert (module);
        if (IsExcluded(module->Name()) == false && HasModule(module) == false)
        {
            ModuleSharedPtr modulePtr = ModuleSharedPtr(module);
            Module::Entry entry = { modulePtr, module->Name(), Module::SharedLibraryPtr() };
            modules_.push_back(entry);
#ifndef _DEBUG
             
            // make it so debug messages are not logged in release mode

            std::string log_level = "information";
            if ( framework_->GetDefaultConfig().HasKey(Framework::ConfigurationGroup(), "log_level") )
                log_level = framework_->GetDefaultConfig().GetSetting<std::string>(Framework::ConfigurationGroup(), "log_level");
            else
                framework_->GetConfigManager()->SetSetting(Framework::ConfigurationGroup(), "log_level", log_level);

            Poco::Logger::get(module->Name()).setLevel(log_level);
#endif
            module->SetFramework(framework_);
            module->LoadInternal();
        }
        else
        {
            Foundation::RootLogInfo("Module: " + module->Name() + " is excluded and not loaded.");
        }
    }

    void ModuleManager::LoadAvailableModules()
    {
        PROFILE(ModuleManager_LoadAvailableModules);

        // Find all shared modules and load them
        StringVectorPtr files;
        try
        {
            files = GetXmlFiles(DEFAULT_MODULES_PATH);
        }
        catch (Exception)
        {
            throw Exception("Failed to load modules, modules directory not found."); // can be considered fatal
        }

        for(size_t i = 0 ; i < files->size() ; ++i)
        {
            const fs::path path((*files)[i]);

            try
            {
                LoadModule(path, files);
            } catch (std::exception &e) // may not be fatal, depending on which module failed
            {
                Foundation::RootLogError(std::string("Exception: ") + e.what());
                Foundation::RootLogError("Failed to load module.");
            }
        }
    }

    void ModuleManager::InitializeModules()
    {
        ModuleInterface *mod;
        {
            PROFILE(ModuleManager_PreInitModules);
            for(size_t i=0 ; i<modules_.size() ; ++i)
            {
                mod = modules_[i].module_.get();
                if (mod->State() != Foundation::Module::MS_Initialized)
                    PreInitializeModule(mod);
            }
        }

        {
            PROFILE(ModuleManager_InitModules);
            for(size_t i=0 ; i<modules_.size() ; ++i)
            {
                mod = modules_[i].module_.get();
                if (mod->State() != Foundation::Module::MS_Initialized)
                    InitializeModule(mod);
            }
        }

        {
            PROFILE(ModuleManager_PostInitModules);
            for(size_t i=0 ; i<modules_.size() ; ++i)
                PostInitializeModule(modules_[i].module_.get());
        }
    }

    void ModuleManager::UninitializeModules()
    {
        for(ModuleVector::reverse_iterator it = modules_.rbegin(); it != modules_.rend(); ++it)
            UninitializeModule(it->module_.get());
    }

    void ModuleManager::UpdateModules(f64 frametime)
    {
        for(size_t i=0 ; i<modules_.size() ; ++i)
            modules_[i].module_->Update(frametime);
    }

    bool ModuleManager::LoadModuleByName(const std::string &lib, const std::string &module)
    {
        assert (lib.empty() == false);
        assert (module.empty() == false);

        for(size_t i = 0 ; i < modules_.size() ; ++i)
        {
            if (modules_[i].entry_ == module)
            {
                RootLogWarning("Attempting to load module " + module + " manually, but module already loaded.");
                return true;
            }
        }


        StringVector current_modules;
        for(size_t i = 0 ; i < modules_.size() ; ++i)
            current_modules.push_back(modules_[i].entry_);

        StringVectorPtr files = GetXmlFiles(DEFAULT_MODULES_PATH);
        for(size_t i = 0 ; i < files->size() ; ++i)
        {
            fs::path path((*files)[i]);
            const fs::path orig_path = path;

            path.replace_extension("");
            std::string filename = path.filename();
            if (filename == lib)
            {
                LoadModule(orig_path, files);
                break;
            }
        }

        //! \todo Dependencies are not handled properly still, modules need to be initialized in dependency order here.
        //!       ModuleA <- ModuleB (A <- B == B depends on A) works, but
        //!       ModuleA <- ModuleB <- ModuleC may not work, ModuleB may get initialized before ModuleA which is error.
        // first initialize dependencies
        for(size_t i = 0 ; i < modules_.size() ; ++i)
        {
            if (modules_[i].module_->State() == Module::MS_Loaded && std::find(
                current_modules.begin(), current_modules.end(), modules_[i].entry_) == current_modules.end()
                && modules_[i].entry_ != module)
            {
                modules_[i].module_->PreInitializeInternal();
            }
        }
        for(size_t i = 0 ; i < modules_.size() ; ++i)
        {
            if (modules_[i].entry_ == module)
                modules_[i].module_->PreInitializeInternal();
        }
        for(size_t i = 0 ; i < modules_.size() ; ++i)
        {
            if (modules_[i].module_->State() == Module::MS_Loaded && std::find(
                current_modules.begin(), current_modules.end(), modules_[i].entry_) == current_modules.end()
                && modules_[i].entry_ != module)
            {
                modules_[i].module_->InitializeInternal();
            }
        }
        for(size_t i = 0 ; i < modules_.size() ; ++i)
        {
            if (modules_[i].entry_ == module)
                modules_[i].module_->InitializeInternal();
        }
        for(size_t i = 0 ; i < modules_.size() ; ++i)
        {
            if (modules_[i].module_->State() == Module::MS_Loaded && std::find(
                current_modules.begin(),current_modules.end(), modules_[i].entry_) == current_modules.end()
                && modules_[i].entry_ != module)
            {
                modules_[i].module_->PostInitializeInternal();
            }
        }
        for(size_t i = 0 ; i < modules_.size() ; ++i)
        {
            if (modules_[i].entry_ == module)
            {
                modules_[i].module_->PostInitializeInternal();
                return true;
            }
        }

        return false;
    }

    bool ModuleManager::UnloadModuleByName(const std::string &module)
    {
        for ( ModuleVector::iterator it = modules_.begin(); it != modules_.end(); ++it)
        {
            if (it->module_->Name() == module)
            {
                UninitializeModule(it->module_.get());
                UnloadModule(*it);
                modules_.erase(it);
                return true;
            }
        }

        return false;
    }

    void ModuleManager::LoadModule(const fs::path &path, const StringVectorPtr &all_files)
    {
        PROFILE(ModuleManager_LoadModule);
        assert (path.has_filename());

        std::string ext = path.extension();
        boost::algorithm::to_lower(ext);
        if (ext == ".xml")
        {
            Foundation::RootLogDebug("LOADING " + path.file_string());
            fs::path modulePath(path);
            modulePath.replace_extension("");

            StringVector entries;
            StringVector dependencies;
            StringVector relativePathDependencyAddititions;

            Poco::AutoPtr<Poco::Util::XMLConfiguration> config;
            try
            {
                config = new Poco::Util::XMLConfiguration(path.native_directory_string());
                Poco::Util::AbstractConfiguration::Keys keys;
                config->keys(keys);

                for( Poco::Util::AbstractConfiguration::Keys::const_iterator it = keys.begin(); it != keys.end(); it++ )
                {
                    if ((*it).find("dependency_dir") != std::string::npos)
                        relativePathDependencyAddititions.push_back( config->getString(*it) );
                    else if ((*it).find("dependency") != std::string::npos)
                        dependencies.push_back( config->getString(*it) );
                    else if ((*it).find("entry") != std::string::npos)
                        entries.push_back( config->getString(*it) );
                }
            }
            catch(std::exception)
            {
            }

            if (!relativePathDependencyAddititions.empty())
                AddDependenciesToPath(relativePathDependencyAddititions);

            if (entries.empty())
                entries.push_back(modulePath.filename());

            // Recurse to load dependencies (if any)
            for(StringVector::const_iterator it = dependencies.begin(); it != dependencies.end(); ++it )
            {
                bool found = false;
                // Try to find the dependency from the all module paths list
                for (StringVector::const_iterator it2 = all_files->begin(); it2 != all_files->end(); ++it2)
                {
                    if ((*it2).find((*it)) != std::string::npos)
                    {
                        const fs::path path((*it2));
                        LoadModule(path, all_files);

                        found = true;
                    }
                }
                if (!found)
                    Foundation::RootLogWarning("Failed to find dependency " + *it); 
            }

            // Then load the module itself
            LoadModule(modulePath.native_directory_string(), entries);
        }
    }

    void ModuleManager::LoadModule(const std::string &name, const StringVector &entries)
    {
        assert(name.empty() == false);

        std::string path(name);
        path.append(Poco::SharedLibrary::suffix());

        Module::SharedLibraryPtr library;

        // See if shared library is already loaded, and if it is, use it
        for(ModuleVector::const_iterator it = modules_.begin(); it != modules_.end(); ++it)
            if (it->shared_library_ && it->shared_library_->path_ == path)
                library = it->shared_library_;

        if (!library)
        {
            try
            {
                library = Module::SharedLibraryPtr(new Module::SharedLibrary(path));
                if (!library->sl_.hasSymbol("SetProfiler"))
                    throw Poco::Exception("Function SetProfiler() need to be exported from the shared library for profiling to work properly!");

                {
                    SetProfilerFunc setProfiler = (SetProfilerFunc) library->sl_.getSymbol("SetProfiler");
                    setProfiler(&framework_->GetProfiler());
                }
            } 
            catch (Poco::Exception &e)
            {
                Foundation::RootLogError(e.displayText());
                Foundation::RootLogError("Failed to load dynamic library: " + path);
                return;
            }
        }

        for(StringVector::const_iterator it = entries.begin(); it != entries.end(); ++it)
        {
            if (HasModuleEntry(*it))
            {
                Foundation::RootLogDebug(">> " + *it + " already loaded");
                continue;
            }

            if (IsExcluded(*it))
            {
                Foundation::RootLogDebug(">> " + *it + " excluded and not loaded");
                continue;
            }

            Foundation::RootLogDebug(">> Attempting to load module " + *it);

            if (library->cl_.findClass(*it) == 0)
                throw Exception("Entry class not found from module");

            ModuleInterface* module = library->cl_.create(*it);
            Module::ModuleDeletor md(*it, library);
            ModuleSharedPtr modulePtr(module, md);

            if (IsExcluded(module->Name()) == false)
            {
                assert (HasModule(module) == false);

#ifndef _DEBUG
                // make it so debug messages are not logged in release mode

                std::string log_level = "information";
                if ( framework_->GetDefaultConfig().HasKey(Framework::ConfigurationGroup(), "log_level") )
                    log_level = framework_->GetDefaultConfig().GetSetting<std::string>(Framework::ConfigurationGroup(), "log_level");
                else
                    framework_->GetConfigManager()->SetSetting(Framework::ConfigurationGroup(), "log_level", log_level);

                Poco::Logger::get(module->Name()).setLevel(log_level);
#endif

                module->SetFramework(framework_);
                module->LoadInternal();

                Module::Entry entry = { modulePtr, *it, library };

                modules_.push_back(entry);

                //Foundation::RootLogInfo("   > " + *it + " loaded");
            }
            else
            {
                Foundation::RootLogInfo("   > Module " + module->Name() + " is excluded and not loaded.");
                //SAFE_DELETE (module);
            }
        }
    }

    void ModuleManager::UnloadModules()
    {
        for(ModuleVector::reverse_iterator it = modules_.rbegin(); it != modules_.rend(); ++it)
            UnloadModule(*it);

        modules_.clear();
        assert (modules_.empty());
    }

    void ModuleManager::PreInitializeModule(ModuleInterface *module)
    {
        assert(module);
        assert(module->State() == Foundation::Module::MS_Loaded);
        module->PreInitializeInternal();
    }

    void ModuleManager::InitializeModule(ModuleInterface *module)
    {
        assert(module);
        assert(module->State() == Foundation::Module::MS_Loaded);
        Foundation::RootLogDebug("INITIALIZING module " + module->Name());
        module->InitializeInternal();
    }

    void ModuleManager::PostInitializeModule(ModuleInterface *module)
    {
        assert(module);
        assert(module->State() == Foundation::Module::MS_Loaded);
        module->PostInitializeInternal();
    }

    void ModuleManager::UninitializeModule(ModuleInterface *module)
    {
        assert(module);
        Foundation::RootLogDebug("UNINITIALIZING module " + module->Name());
        module->UninitializeInternal();
    }

    void ModuleManager::UnloadModule(Module::Entry &entry)
    {
        assert(entry.module_);
        entry.module_->UnloadInternal();
        entry.module_.reset(); // Triggers the deletion of the ModuleInterface object. (either causes operator delete or Poco's module free to be called)
    }

    bool ModuleManager::HasModule(ModuleInterface *module) const
    {
        assert (module);
        for(ModuleVector::const_iterator it = modules_.begin(); it != modules_.end(); ++it)
            if (it->module_->Name() == module->Name())
                return true;

        return false;
    }

    StringVectorPtr ModuleManager::GetXmlFiles(const std::string &path)
    {
        StringVectorPtr files(new StringVector);

        // Find all xml files recursively
        //fs::path full_path = fs::system_complete(fs::path(DEFAULT_MODULES_PATH));
        //if ( !fs::exists( full_path ) || !fs::is_directory( full_path ))
        //    throw Exception("Path not found!"); // can be considered fatal
        fs::path rel_path(path);
        if (!fs::exists(rel_path) || !fs::is_directory(rel_path))
            throw Exception("Path not found!"); // can be considered fatal

        fs::recursive_directory_iterator iter( rel_path );
        fs::recursive_directory_iterator end_iter;
        for (; iter != end_iter; ++iter )
        {
            if (fs::is_regular_file(iter->status()))
            {
                std::string ext = iter->path().extension();
                boost::algorithm::to_lower(ext);
                if (ext == ".xml")
                    files->push_back(iter->path().string());
            }
        }

        return files;
    }

    void ModuleManager::AddDependenciesToPath(const StringVector &all_additions)
    {
        // Do this in windows only for now,
        // untill linux crash caused by this is resolved
        #ifdef Q_WS_WIN

        std::string path = Poco::Environment::get("path", "");
        std::string cwd = Poco::Path::current();
        char separator = Poco::Path::pathSeparator();

        // Add additions to env
        for(StringVector::const_iterator it = all_additions.begin(); it != all_additions.end(); ++it )
        {
            std::string absolutePath = cwd + (*it);
            absolutePath.push_back(separator);
            path.insert(0, absolutePath);
        }

        // Set additions to process path
        Poco::Environment::set("path", path);

        #endif
    }
}
