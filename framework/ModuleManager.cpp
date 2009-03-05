// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ModuleManager.h"
#include "ConfigurationManager.h"
#include "Framework.h"
#include "ModuleInterface.h"

namespace fs = boost::filesystem;


namespace Foundation
{
    ModuleManager::ModuleManager(Framework *framework) : 
        framework_(framework)
      , DEFAULT_MODULES_PATH(Framework::GetDefaultConfig().DeclareSetting("ModuleManager", "Default_Modules_Path", "./modules"))
    {
    }

    ModuleManager::~ModuleManager()
    {
    }

    void ModuleManager::LoadAvailableModules()
    {
        fs::path full_path = fs::system_complete(fs::path(DEFAULT_MODULES_PATH));
        if ( !fs::exists( full_path ) || !fs::is_directory( full_path ))
            throw Core::Exception("Failed to load modules, modules directory not found."); // can be considered fatal

        
        fs::recursive_directory_iterator iter( full_path );
        fs::recursive_directory_iterator end_iter;
        for ( ; iter != end_iter ; ++iter )
        {
            try
            {
                if ( fs::is_regular_file( iter->status() ) )
                {
                    LoadModule(iter->path());
                    
                }
            } catch (std::exception &e) // may not be fatal, depending on which module failed
            {
                LOGERROR(std::string("Exception: ") + e.what());
                LOGERROR("Failed to load module.");
            }
        }
    }

    void ModuleManager::InitializeModules()
    {
        for (size_t i=0 ; i<modules_.size() ; ++i)
        {
            InitializeModule(modules_[i]);
        }
    }

    void ModuleManager::UninitializeModules()
    {
        for (size_t i=0 ; i<modules_.size() ; ++i)
        {
            UninitializeModule(modules_[i]);
        }
    }

    void ModuleManager::UpdateModules()
    {
        for (size_t i=0 ; i<modules_.size() ; ++i)
        {
            modules_[i]->Update();
        }
    }

    void ModuleManager::LoadModule(const fs::path &path)
    {
        assert (path.has_filename());


        std::string ext = path.extension();
        boost::algorithm::to_lower(ext);
        if (ext == ".xml")
        {
            fs::path modulePath(path);
            modulePath.replace_extension("");

            std::string entry = modulePath.filename();

            Poco::AutoPtr<Poco::Util::XMLConfiguration> config;
            try
            {
                config = new Poco::Util::XMLConfiguration(path.native_directory_string());
                entry = config->getString("entry", entry);

            }
            catch(Poco::FileNotFoundException &e)
            {
                LOGERROR("Module definition file not found: " + std::string(e.what()));
                return;
            }
            catch (std::exception) { /* no need to handle */ }

            LoadModule(modulePath.native_directory_string(), entry);
        }
    }

    void ModuleManager::LoadModule(const std::string &moduleName, const std::string &entryPoint)
    {
        assert(moduleName.empty() == false);

        LOG("Attempting to load module: " + moduleName + " with entry point: " + entryPoint + ".");
        std::string path(moduleName);
        path.append(Poco::SharedLibrary::suffix());

        Poco::ClassLoader<ModuleInterface> cl;
        cl.loadLibrary(path);

        if (cl.findClass(entryPoint) == NULL)
        {
            throw Core::Exception("Entry class not found from plugin");
        }

        ModuleInterface* module = cl.classFor(entryPoint).create();
        module->Load();

        modules_.push_back(module);

        LOG("Module: " + moduleName + " loaded.");
    }

    void ModuleManager::UnloadModules()
    {
        for (size_t i=0 ; i<modules_.size() ; ++i)
        {
            assert(modules_[i]);
            modules_[i]->Unload();
            delete modules_[i];
        }
        modules_.clear();
    }

    void ModuleManager::InitializeModule(ModuleInterface *module)
    {
        assert(module);
        module->_Initialize(framework_);
    }

    void ModuleManager::UninitializeModule(ModuleInterface *module)
    {
        assert(module);
        module->_Uninitialize(framework_);
    }
}
