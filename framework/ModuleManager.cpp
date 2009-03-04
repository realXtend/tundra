// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ModuleManager.h"
#include "ModuleInterface.h"
#include "ConfigurationManager.h"
#include "Framework.h"

namespace fs = boost::filesystem;


namespace Foundation
{
//    const char *ModuleManager::DEFAULT_MODULES_PATH = "./modules";

    ModuleManager::ModuleManager(Framework *framework) : 
        mFramework(framework)
      , DEFAULT_MODULES_PATH(Framework::getDefaultConfig().declareSetting("ModuleManager", "Default_Modules_Path", "./modules"))
    {
    }

    ModuleManager::~ModuleManager()
    {
    }

    void ModuleManager::loadAvailableModules()
    {
        fs::path full_path = fs::system_complete(fs::path(DEFAULT_MODULES_PATH));
        if ( !fs::exists( full_path ) || !fs::is_directory( full_path ))
            throw std::exception("Failed to load modules, modules directory not found."); // can be considered fatal

        
        fs::recursive_directory_iterator iter( full_path );
        fs::recursive_directory_iterator end_iter;
        for ( ; iter != end_iter ; ++iter )
        {
            try
            {
                if ( fs::is_regular_file( iter->status() ) )
                {
                    loadModule(iter->path());
                    
                }
            } catch (std::exception &e) // may not be fatal, depending on which module failed
            {
                LOGERROR(std::string("Exception: ") + e.what());
                LOGERROR("Failed to load module.");
            }
        }
    }

    void ModuleManager::initializeModules()
    {
        for (size_t i=0 ; i<mModules.size() ; ++i)
        {
            initializeModule(mModules[i]);
        }
    }

    void ModuleManager::uninitializeModules()
    {
        for (size_t i=0 ; i<mModules.size() ; ++i)
        {
            uninitializeModule(mModules[i]);
        }
    }

    void ModuleManager::updateModules()
    {
        for (size_t i=0 ; i<mModules.size() ; ++i)
        {
            mModules[i]->update();
        }
    }

    void ModuleManager::loadModule(const fs::path &path)
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

            loadModule(modulePath.native_directory_string(), entry);
        }
    }

    void ModuleManager::loadModule(const std::string &moduleName, const std::string &entryPoint)
    {
        assert(moduleName.empty() == false);

        LOG("Attempting to load module: " + moduleName + " with entry point: " + entryPoint + ".");
        std::string path(moduleName);
        path.append(Poco::SharedLibrary::suffix());

        Poco::ClassLoader<ModuleInterface> cl;
        cl.loadLibrary(path);

        if (cl.findClass(entryPoint) == NULL)
        {
            throw std::exception("Entry class not found from plugin");
        }

        ModuleInterface* module = cl.classFor(entryPoint).create();
        module->load();

        mModules.push_back(module);

        LOG("Module: " + moduleName + " loaded.");
    }

    void ModuleManager::unloadModules()
    {
        for (size_t i=0 ; i<mModules.size() ; ++i)
        {
            assert(mModules[i]);
            mModules[i]->unload();
            delete mModules[i];
        }
        mModules.clear();
    }

    void ModuleManager::initializeModule(ModuleInterface *module)
    {
        assert(module);
        module->_initialize(mFramework);
    }

    void ModuleManager::uninitializeModule(ModuleInterface *module)
    {
        assert(module);
        module->_uninitialize(mFramework);
    }
}
