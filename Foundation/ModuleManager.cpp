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
        modules_.push_back(module);
    }

    void ModuleManager::LoadAvailableModules()
    {
        // First load all static modules
        ModuleVector::iterator it = modules_.begin();
        for ( ; it != modules_.end() ; ++it)
        {
            (*it)->Load();
        }

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

        //fs::path full_path = fs::system_complete(fs::path(DEFAULT_MODULES_PATH));
        //if ( !fs::exists( full_path ) || !fs::is_directory( full_path ))
        //    throw Core::Exception("Failed to load modules, modules directory not found."); // can be considered fatal

        //
        //fs::recursive_directory_iterator iter( full_path );
        //fs::recursive_directory_iterator end_iter;
        //for ( ; iter != end_iter ; ++iter )
        //{
        //    try
        //    {
        //        if ( fs::is_regular_file( iter->status() ) )
        //        {
        //            LoadModule(iter->path());
        //        }
        //    } catch (std::exception &e) // may not be fatal, depending on which module failed
        //    {
        //        Foundation::RootLogError(std::string("Exception: ") + e.what());
        //        Foundation::RootLogError("Failed to load module.");
        //    }
        //}
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

    void ModuleManager::LoadModuleByName(const std::string &name)
    {
        assert (name.empty() == false);

        Core::StringVectorPtr files = GetXmlFiles(DEFAULT_MODULES_PATH);
        for (size_t i = 0 ; i < files->size() ; ++i)
        {
            fs::path path((*files)[i]);
            const fs::path orig_path = path;

            path.replace_extension("");
            std::string filename = path.filename();
            if (filename == name)
            {
                LoadModule(orig_path);
                break;
            }
        }

        for (size_t i = 0 ; i < modules_.size() ; ++i)
        {
            if (modules_[i]->Name() == name)
            {
                modules_[i]->_Initialize(framework_);
                break;
            }
        }
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
            module->Load();

            modules_.push_back(module);

            Foundation::RootLogInfo("Module: " + *it + " loaded.");
        }
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
