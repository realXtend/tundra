// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ModuleManager.h"
#include "ModuleInterface.h"


namespace Foundation
{
    ModuleManager::~ModuleManager()
    {
        //uninitializeModules();
        //unloadModules();
    }

    void ModuleManager::loadAvailableModules()
    {
        Poco::AutoPtr<Poco::Util::XMLConfiguration> config;

        std::string moduleconfigfile = Poco::Path::current();
        moduleconfigfile.append("modules.xml");

        try
        {
            config = new Poco::Util::XMLConfiguration(moduleconfigfile);
        }
        catch(Poco::FileNotFoundException & e)
        {
            LOGERROR("Config file not found: " + std::string(e.what()));
            return;
        }

        Poco::Util::AbstractConfiguration::Keys keys;
	    config->keys(keys);
        for (size_t i=0 ; i<keys.size() ; ++i)
        {
            const std::string &moduleName = config->getString(keys[i]);
            std::string entryPoint = moduleName;
            try
            {
                entryPoint = config->getString(keys[i] + "[@entry]");
            } catch (std::exception) {  }
            try
            {
                loadModule(moduleName, entryPoint);
            } catch (Poco::LibraryAlreadyLoadedException)
            {
                assert(false && "Attempting to load a module that is already loaded.");
            } catch(std::exception & e)
            {
                LOGERROR("Exception: " + std::string(e.what()));
                LOGERROR("Failed to load module: " + moduleName + ".");
                return;
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
