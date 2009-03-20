// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ModuleManager.h"
#include "ConfigurationManager.h"
#include "Framework.h"

namespace fs = boost::filesystem;


namespace Foundation
{
    ModuleManager::ModuleManager(Framework *framework) : 
        framework_(framework)
      , DEFAULT_MODULES_PATH(framework->GetDefaultConfig().DeclareSetting("ModuleManager", "Default_Modules_Path", "./modules"))
    {
    }

    ModuleManager::~ModuleManager()
    {
    }

    void ModuleManager::DeclareStaticModule(ModuleInterface *module)
    {
        assert (module);
        if (IsExcluded(module->Name()) == false && HasModule(module) == false)
        {
            Module::Entry entry = { module, module->Name(), std::string() };
            modules_.push_back(entry);
            module->Load();
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
                LoadModule(path);
            } catch (std::exception &e) // may not be fatal, depending on which module failed
            {
                Foundation::RootLogError(std::string("Exception: ") + e.what());
                Foundation::RootLogError("Failed to load module.");
            }
        }
    }

    void ModuleManager::InitializeModules()
    {
        for (size_t i=0 ; i<modules_.size() ; ++i)
        {
            PreInitializeModule(modules_[i].module_);
        }
        
        for (size_t i=0 ; i<modules_.size() ; ++i)
        {
            InitializeModule(modules_[i].module_);
        }
        
        for (size_t i=0 ; i<modules_.size() ; ++i)
        {
            PostInitializeModule(modules_[i].module_);
        }
    }

    void ModuleManager::UninitializeModules()
    {
        for (size_t i=0 ; i<modules_.size() ; ++i)
        {
            UninitializeModule(modules_[i].module_);
        }
    }

    void ModuleManager::UpdateModules()
    {
        for (size_t i=0 ; i<modules_.size() ; ++i)
        {
            modules_[i].module_->Update();
        }
    }

    bool ModuleManager::LoadModuleByName(const std::string &lib, const std::string &module)
    {
        assert (lib.empty() == false);
        assert (module.empty() == false);

        Core::StringVectorPtr files = GetXmlFiles(DEFAULT_MODULES_PATH);
        for (size_t i = 0 ; i < files->size() ; ++i)
        {
            fs::path path((*files)[i]);
            const fs::path orig_path = path;

            path.replace_extension("");
            std::string filename = path.filename();
            if (filename == lib)
            {
                LoadModule(orig_path);
                break;
            }
        }

        for (size_t i = 0 ; i < modules_.size() ; ++i)
        {
            if (modules_[i].module_->Name() == module)
            {
                modules_[i].module_->PreInitialize(framework_);
                modules_[i].module_->InitializeInternal(framework_);
                modules_[i].module_->PostInitialize(framework_);
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
                UninitializeModule(it->module_);
                UnloadModule(it->module_);
                modules_.erase(it);
                return true;
            }
        }

        return false;
    }

    void ModuleManager::LoadModule(const fs::path &path)
    {
        assert (path.has_filename());


        std::string ext = path.extension();
        boost::algorithm::to_lower(ext);
        if (ext == ".xml")
        {
            Foundation::RootLogInfo("Attempting to load module definition file: " + path.file_string());
            fs::path modulePath(path);
            modulePath.replace_extension("");

            Core::StringVector entries;
            

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
                    entries.push_back( config->getString(*it) );
                }
            }
            catch(std::exception)
            {
                entries.push_back(modulePath.filename());
            }

            LoadModule(modulePath.native_directory_string(), entries);
        }
    }

    void ModuleManager::LoadModule(const std::string &moduleName, const Core::StringVector &entries)
    {
        assert(moduleName.empty() == false);

        std::string path(moduleName);
        path.append(Poco::SharedLibrary::suffix());

        Poco::ClassLoader<ModuleInterface> cl;
        try
        {
            cl.loadLibrary(path);
        } catch (std::exception &e)
        {
            Foundation::RootLogError(e.what());
            Foundation::RootLogError("Failed to load dynamic library: " + moduleName + ".");
        }

        for ( Core::StringVector::const_iterator it = entries.begin() ; 
              it != entries.end() ; 
              ++it )
        {
            Foundation::RootLogInfo("Attempting to load module: " + *it + ".");

            if (cl.findClass(*it) == NULL)
            {
                throw Core::Exception("Entry class not found from module.");
            }

            ModuleInterface* module = cl.classFor(*it).create();

            if (IsExcluded(module->Name()) == false && HasModule(module) == false)
            {
                module->Load();

                Module::Entry entry = { module, *it, moduleName };

                modules_.push_back(entry);

#ifndef _DEBUG
                // make it so debug messages are not logged in release mode
                Poco::Logger::get(module->Name()).setLevel("information");
#endif

                Foundation::RootLogInfo("Module: " + *it + " loaded.");
            } else
            {
                Foundation::RootLogInfo("Module: " + module->Name() + " is excluded and not loaded.");
                SAFE_DELETE (module);
            }
        }
    }

    void ModuleManager::UnloadModules()
    {
        for (size_t i=0 ; i<modules_.size() ; ++i)
        {
            UnloadModule(modules_[i].module_);
        }

        modules_.clear();
        assert (modules_.empty());
    }

    void ModuleManager::PreInitializeModule(ModuleInterface *module)
    {
        assert(module);
        module->PreInitialize(framework_);
    }
    
    void ModuleManager::InitializeModule(ModuleInterface *module)
    {
        assert(module);
        module->InitializeInternal(framework_);
    }

    void ModuleManager::PostInitializeModule(ModuleInterface *module)
    {
        assert(module);
        module->PostInitialize(framework_);
    }

    void ModuleManager::UninitializeModule(ModuleInterface *module)
    {
        assert(module);
        module->UninitializeInternal(framework_);
    }

    void ModuleManager::UnloadModule(ModuleInterface *module)
    {
        assert(module);

        module->Unload();
        delete module;
    }

    bool ModuleManager::HasModule(ModuleInterface *module)
    {
        assert (module);

        
        for ( ModuleVector::iterator it = modules_.begin() ; 
              it != modules_.end() ; 
              ++it )
        {
            if (it->module_ == module)
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
