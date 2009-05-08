// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Framework.h"
#include "ModuleManager.h"
#include "ConfigurationManager.h"


namespace fs = boost::filesystem;


namespace Foundation
{
    ModuleManager::ModuleManager(Framework *framework) :
        framework_(framework)
            , DEFAULT_MODULES_PATH(framework->GetDefaultConfig().DeclareSetting<std::string>("ModuleManager", "Default_Modules_Path",
            "./modules"))
    {
    }

    ModuleManager::~ModuleManager()
    {
		//Unload all loaded modules if object is truely destroyed. 
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
        } else
        {
            Foundation::RootLogInfo("Module: " + module->Name() + " is excluded and not loaded.");
        }
    }

    void ModuleManager::LoadAvailableModules()
    {
        // Find all shared modules and load them
        Core::StringVectorPtr files;
        try
        {
            files = GetXmlFiles(DEFAULT_MODULES_PATH);
        } catch (Core::Exception)
        {
            throw Core::Exception("Failed to load modules, modules directory not found."); // can be considered fatal
        }

        for (size_t i = 0 ; i < files->size() ; ++i)
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

        for (size_t i=0 ; i<modules_.size() ; ++i)
        {
            mod = modules_[i].module_.get();
            if (mod->State() != Foundation::Module::MS_Initialized)
                PreInitializeModule(mod);
        }
        
        for (size_t i=0 ; i<modules_.size() ; ++i)
        {
            mod = modules_[i].module_.get();
            if (mod->State() != Foundation::Module::MS_Initialized)
                InitializeModule(mod);
        }
        
        for (size_t i=0 ; i<modules_.size() ; ++i)
        {
            PostInitializeModule(modules_[i].module_.get());
        }
    }

    void ModuleManager::UninitializeModules()
    {
        for (ModuleVector::reverse_iterator it = modules_.rbegin() ; 
             it != modules_.rend() ; 
             ++it)
        {
            UninitializeModule(it->module_.get());
        }
    }

    void ModuleManager::UpdateModules(Core::f64 frametime)
    {
        for (size_t i=0 ; i<modules_.size() ; ++i)
        {
            modules_[i].module_->Update(frametime);
        }
    }

    bool ModuleManager::LoadModuleByName(const std::string &lib, const std::string &module)
    {
        assert (lib.empty() == false);
        assert (module.empty() == false);

        Core::StringVector current_modules;
        for (size_t i = 0 ; i < modules_.size() ; ++i)
            current_modules.push_back(modules_[i].entry_);

        Core::StringVectorPtr files = GetXmlFiles(DEFAULT_MODULES_PATH);
        for (size_t i = 0 ; i < files->size() ; ++i)
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

        for (size_t i = 0 ; i < modules_.size() ; ++i)
        {
            if (modules_[i].module_->State() == Module::MS_Loaded && std::find(current_modules.begin(), current_modules.end(), modules_[i].entry_) == current_modules.end())
            {
                modules_[i].module_->PreInitializeInternal();
                modules_[i].module_->InitializeInternal();
                modules_[i].module_->PostInitializeInternal();

                if (modules_[i].entry_ == module)
                    return true;
            }
        }
        return false;
    }

    bool ModuleManager::UnloadModuleByName(const std::string &module)
    {
        for ( ModuleVector::iterator it = modules_.begin() ; 
             it != modules_.end() ; 
             ++it )
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

    void ModuleManager::LoadModule(const fs::path &path, const Core::StringVectorPtr &all_files)
    {
        assert (path.has_filename());

        std::string ext = path.extension();
        boost::algorithm::to_lower(ext);
        if (ext == ".xml")
        {
            Foundation::RootLogDebug("Attempting to load module definition file: " + path.file_string());
            fs::path modulePath(path);
            modulePath.replace_extension("");

            Core::StringVector entries;
            Core::StringVector dependencies;

            Poco::AutoPtr<Poco::Util::XMLConfiguration> config;
            try
            {
                config = new Poco::Util::XMLConfiguration(path.native_directory_string());
                Poco::Util::AbstractConfiguration::Keys keys;
	            config->keys(keys);

                for ( Poco::Util::AbstractConfiguration::Keys::const_iterator it = keys.begin() ; 
                      it != keys.end() ;
                      it++ )
                {
                    if ((*it).find("entry") != std::string::npos)
                        entries.push_back( config->getString(*it) );
                        
                   if ((*it).find("dependency") != std::string::npos)
                        dependencies.push_back( config->getString(*it) );
                }
            }
            catch(std::exception)
            {
            }
            
            if (entries.empty())
                entries.push_back(modulePath.filename());
                
            // Recurse to load dependencies (if any)
            for ( Core::StringVector::const_iterator it = dependencies.begin() ; 
              it != dependencies.end() ; ++it )
            {
                bool found = false;
                // Try to find the dependency from the all module paths list
                for (Core::StringVector::const_iterator it2 = all_files->begin();
                    it2 != all_files->end(); ++it2)
                {
                    if ((*it2).find((*it)) != std::string::npos)
                    {
                        const fs::path path((*it2));
                        LoadModule(path, all_files);

                        found = true;
                    }
                }
                if (!found)
                    Foundation::RootLogWarning("Failed to find dependency " + *it + "."); 
            }

            // Then load the module itself
            LoadModule(modulePath.native_directory_string(), entries);
        }
    }

    void ModuleManager::LoadModule(const std::string &name, const Core::StringVector &entries)
    {
        assert(name.empty() == false);

        std::string path(name);
        path.append(Poco::SharedLibrary::suffix());

        Module::SharedLibraryPtr library;

        // See if shared library is already loaded, and if it is, use it
        for (ModuleVector::const_iterator it = modules_.begin(); 
                it != modules_.end(); 
                ++it)
        {
            if (it->shared_library_ && it->shared_library_->path_ == path)
            {
                library = it->shared_library_;
            }
        }

        if (!library)
        {
            try
            {
                library = Module::SharedLibraryPtr(new Module::SharedLibrary(path));
            } 
            catch (Poco::Exception &e)
            {
                Foundation::RootLogError(e.displayText());
                Foundation::RootLogError("Failed to load dynamic library: " + path + ".");
                return;
            }
        }

        for (Core::StringVector::const_iterator it = entries.begin(); 
                it != entries.end(); 
                ++it)
        {   
            if (HasModuleEntry(*it))
            {
                Foundation::RootLogDebug("Module " + *it + " already loaded.");
                continue;
            }

            Foundation::RootLogInfo("Attempting to load module: " + *it + ".");


            if (library->cl_.findClass(*it) == NULL)
            {
                throw Core::Exception("Entry class not found from module.");
            }


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

                Foundation::RootLogInfo("Module " + *it + " loaded.");
            } 
            else
            {
                Foundation::RootLogInfo("Module " + module->Name() + " is excluded and not loaded.");
//                SAFE_DELETE (module);
            }
        }
    }

    void ModuleManager::UnloadModules()
    {
        for (ModuleVector::reverse_iterator it = modules_.rbegin() ; 
             it != modules_.rend() ; 
             ++it)
        {
            UnloadModule(*it);
        }

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
        Foundation::RootLogInfo("Initializing module " + module->Name() + ".");
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
        Foundation::RootLogInfo("Uninitializing module " + module->Name() + ".");
        module->UninitializeInternal();
    }

    void ModuleManager::UnloadModule(Module::Entry &entry)
    {
        assert(entry.module_);

        entry.module_->UnloadInternal();
        entry.module_.reset(); // Triggers the deletion of the ModuleInterface object. (either causes operator delete or Poco's module free to be called)
/*
        if (!entry.shared_library_) // If our module is a static library (and not a dynamic one allocated using Poco), we need to free with delete.
        {
            delete entry.module_;
        }
        else // This is a shared library that is owned by Poco, need to use Poco's destroy.
        {
            entry.shared_library_->cl_.destroy(entry.entry_, entry.module_);
            SAFE_DELETE(entry.module_);
        }*/
    }

    bool ModuleManager::HasModule(ModuleInterface *module) const
    {
        assert (module);

        
        for ( ModuleVector::const_iterator it = modules_.begin() ; 
              it != modules_.end() ; 
              ++it )
        {
            if (it->module_->Name() == module->Name())
                return true;
        }
        return false;
    }

    Core::StringVectorPtr ModuleManager::GetXmlFiles(const std::string &path)
    {
        Core::StringVectorPtr files(new Core::StringVector);

        // Find all xml files recursively
        fs::path full_path = fs::system_complete(fs::path(DEFAULT_MODULES_PATH));
        if ( !fs::exists( full_path ) || !fs::is_directory( full_path ))
            throw Core::Exception("Path not found!"); // can be considered fatal

        fs::recursive_directory_iterator iter( full_path );
        fs::recursive_directory_iterator end_iter;
        for ( ; iter != end_iter ; ++iter )
        {
            if ( fs::is_regular_file( iter->status() ) )
            {
                std::string ext = iter->path().extension();
                boost::algorithm::to_lower(ext);
                if (ext == ".xml")
                {
                    files->push_back(iter->path().string());
                }
            }
        }

        return files;
    }
}
