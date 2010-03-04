// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MemoryLeakCheck.h"

#include "Framework.h"
#include "ModuleManager.h"
#include "ConfigurationManager.h"
#include "Poco/UnicodeConverter.h"
#include "CoreException.h"

#include <algorithm>
#include <sstream>
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

    void ModuleManager::SortModuleLoadOrder(std::vector<ModuleLoadDescription> &modules)
    {
        for(size_t i = 0; i < modules.size(); ++i)
            for(size_t j = 0; j < modules.size(); ++j)
                if (i != j)
                {
                    size_t A = std::min(i, j);
                    size_t B = std::max(i, j);
                    if (modules[B].Precedes(modules[A]))
                        std::swap(modules[A], modules[B]);
                }

        for(size_t i = 0; i < modules.size(); ++i)
            for(size_t j = i+1; j < modules.size(); ++j)
                if (modules[j].Precedes(modules[i]))
                {
                    Foundation::RootLogCritical(std::string("A cyclic dependency found in module dependencies! Could not satisfy dependency ") + 
                        modules[j].ToString() + " < " + modules[i].ToString() + ". Continuing nevertheless.");
                }
    }

    const ModuleManager::ModuleLoadDescription *ModuleManager::FindModuleLoadDescriptionWithEntry(const std::vector<ModuleLoadDescription> &modules, const std::string &entryName)
    {
        for(size_t i = 0; i < modules.size(); ++i)
            for(size_t j = 0; j < modules[i].moduleNames.size(); ++j)
                if (modules[i].moduleNames[j] == entryName)
                    return &modules[i];

        return 0;
    }

    void ModuleManager::CheckDependencies(const std::vector<ModuleLoadDescription> &modules)
    {
        for(size_t i = 0; i < modules.size(); ++i)
            for(size_t j = 0; j < modules[i].dependencies.size(); ++j)
            {
                const ModuleLoadDescription *dependee = FindModuleLoadDescriptionWithEntry(modules, modules[i].dependencies[j]);
                if (!dependee)
                {
                    Foundation::RootLogCritical(std::string("Could not satisfy dependency ") + 
                        modules[i].ToString() + " -> " + modules[i].dependencies[j] + ". Trying to load without.");
                }
            }
    }

    void ModuleManager::LoadAvailableModules()
    {
        // Find all known module XML definition files.
        StringVectorPtr files;
        try
        {
            files = GetXmlFiles(DEFAULT_MODULES_PATH);
        }
        catch (Exception)
        {
            throw Exception("Failed to load modules, modules directory not found."); // can be considered fatal
        }

        // Now parse all the definition files we found.
        std::vector<ModuleLoadDescription> moduleDescriptions;
        StringVector relativePathDependencyAdditions;
        for(size_t i = 0; i < files->size(); ++i)
            ParseModuleXMLFile((*files)[i], moduleDescriptions, relativePathDependencyAdditions);

        // If any module needs any new directories in the path, add those there. But first, remove any duplicate entries.
        std::sort(relativePathDependencyAdditions.begin(), relativePathDependencyAdditions.end());
        relativePathDependencyAdditions.erase(std::unique(relativePathDependencyAdditions.begin(), relativePathDependencyAdditions.end()),
            relativePathDependencyAdditions.end());
        AddDependenciesToPath(relativePathDependencyAdditions);

        // Find a suitable module load order.
        SortModuleLoadOrder(moduleDescriptions);

        // Check and warn if any module dependencies could not be satisfied.
        CheckDependencies(moduleDescriptions);

        // Finally, load up all modules. The module description list is now sorted in a topological order, so that the dependencies
        // are satisfied when traversing begin()->end().
        for(std::vector<ModuleLoadDescription>::iterator iter = moduleDescriptions.begin(); iter != moduleDescriptions.end(); ++iter)
        {
            try
            {
                LoadModule(iter->moduleDescFilename.native_directory_string(), iter->moduleNames);
            } catch (std::exception &e) // may not be fatal, depending on which module failed
            {
                Foundation::RootLogError(std::string("Trying to load module ") + iter->ToString() + " threw an exception: " + e.what());
            }
        }
    }

    /// @return True if this module directly depends on the module rhs, i.e. if this module absolutely needs to be loaded before rhs.
    bool ModuleManager::ModuleLoadDescription::Precedes(const ModuleLoadDescription &rhs) const
    {
        for(size_t i = 0; i < rhs.dependencies.size(); ++i)
            for(size_t j = 0; j < moduleNames.size(); ++j)
                if (rhs.dependencies[i] == moduleNames[j])
                    return true;

        return false;
    }

    /// @return A readable string presentation of the given module description.
    std::string ModuleManager::ModuleLoadDescription::ToString() const
    {
        std::stringstream ss;
        ss << moduleDescFilename << ": ";
        if (moduleNames.size() == 0)
            ss << "(null)";
        else
            for(size_t i = 0; i < moduleNames.size(); ++i)
            {
                if (i != 0)
                    ss << ", ";
                ss << moduleNames[i];
            }
        return ss.str();
    }

    /// Parses the module xml file stored in the file pointed by the parameter path. Adds a new module load description structure to the end of the vector out,
    /// as well as adding into relativePathDependencyAdditions any path dependencies needed by that module.
    void ModuleManager::ParseModuleXMLFile(const fs::path &path, std::vector<ModuleLoadDescription> &out, StringVector &relativePathDependencyAdditions)
    {
        assert(path.has_filename());

        // Check the extension to confirm that we are passed a .xml file.
        std::string ext = path.extension();
        boost::algorithm::to_lower(ext);
        if (ext != ".xml")
        {
            Foundation::RootLogWarning("Tried to parse a module XML file with path " + path.string() + ". Extension " + ext + " is not allowed. Should have .xml!");
            return;
        }

        Foundation::RootLogDebug("Parsing module file " + path.file_string());
        fs::path modulePath(path);
        modulePath.replace_extension("");

        StringVector entries;
        StringVector dependencies;

        try
        {
            // Open the xml file and read all data inside.
            Poco::AutoPtr<Poco::Util::XMLConfiguration> config = new Poco::Util::XMLConfiguration(path.native_directory_string());
            Poco::Util::AbstractConfiguration::Keys keys;
            config->keys(keys);

            for(Poco::Util::AbstractConfiguration::Keys::const_iterator it = keys.begin(); it != keys.end(); ++it)
            {
                if (it->find("dependency_dir") != std::string::npos)
                    relativePathDependencyAdditions.push_back(config->getString(*it));
                else if (it->find("dependency") != std::string::npos)
                    dependencies.push_back(config->getString(*it));
                else if (it->find("entry") != std::string::npos)
                    entries.push_back(config->getString(*it));
            }
        }
        catch(const std::exception &e)
        {
            Foundation::RootLogWarning(std::string("Exception thrown when parsing a module XML file: ") + e.what());
            return;
        }

        // The commented-out loop would add each individual entry(module) inside this shared library as a separate module load description object,
        // but for now we load all modules from a single shared library in one go, so this is now ignored. That can be done as long as all the modules
        // inside that shared library have the same dependencies.
//        for(StringVector::iterator iter = entries.begin(); iter != entries.end(); ++iter)
//        {
            ModuleLoadDescription desc;
            desc.moduleDescFilename = modulePath;
            desc.moduleNames = entries;
//            desc.moduleName = *iter;
            /// \note Currently cannot specify in a single XML file several modules that would have separate dependencies. They all share the same!
            ///       Though, this is not currently in any way seen critical. (just write two xml files if you need separate dependencies)
            desc.dependencies = dependencies; 
            out.push_back(desc);
//        }
    }

    void ModuleManager::InitializeModules()
    {
        for(size_t i = 0; i < modules_.size(); ++i)
        {
            ModuleInterface *mod = modules_[i].module_.get();
            if (mod->State() != Foundation::Module::MS_Initialized)
                PreInitializeModule(mod);
        }

        for(size_t i = 0; i < modules_.size(); ++i)
        {
            ModuleInterface *mod = modules_[i].module_.get();
            if (mod->State() != Foundation::Module::MS_Initialized)
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
            modules_[i].module_->Update(frametime);
    }

    bool ModuleManager::LoadModuleByName(const std::string &lib, const std::string &module)
    {
        assert (lib.empty() == false);
        assert (module.empty() == false);

        // Just return with success if the module was already loaded.
        for(size_t i = 0 ; i < modules_.size() ; ++i)
            if (modules_[i].entry_ == module)
            {
                RootLogWarning("Attempting to load module " + module + " manually, but module already loaded.");
                return true;
            }

        StringVector current_modules;
        for(size_t i = 0; i < modules_.size(); ++i)
            current_modules.push_back(modules_[i].entry_);

        StringVector moduleNames;
        moduleNames.push_back(module);

        // Read all known module load description XML files and find if one matches with the load request we've been given.
        StringVectorPtr files = GetXmlFiles(DEFAULT_MODULES_PATH);
        for(size_t i = 0; i < files->size(); ++i)
        {
            fs::path path((*files)[i]);
            const fs::path orig_path = path;

            path.replace_extension("");
            std::string filename = path.filename();
            if (filename == lib)
            {
//                LoadModule(orig_path.string(), files);
                LoadModule(orig_path.string(), moduleNames);
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

    void ModuleManager::LoadModule(const std::string &name, const StringVector &entries)
    {
        assert(name.empty() == false);

        std::string path(name);
        path.append(Poco::SharedLibrary::suffix());

        Module::SharedLibraryPtr library;

        // See if shared library is already loaded, and if it is, use it
        for(ModuleVector::const_iterator it = modules_.begin(); it != modules_.end(); ++it)
            if (it->shared_library_ && it->shared_library_->path_ == path)
            {
                library = it->shared_library_;
                break;
            }

        // If we haven't loaded this library yet, load it up.
        if (!library)
        {
            try
            {
                library = Module::SharedLibraryPtr(new Module::SharedLibrary(path));
                if (!library->sl_.hasSymbol("SetProfiler"))
                    throw Poco::Exception("Function SetProfiler() need to be exported from the shared library for profiling to work properly!");

                // Each shared library containing modules must have a SetProfiler function exposed so that time profiling can be performed in it.
                ///\todo This could be made optional, but no real hurry since profiling is included in release versions as well.
                SetProfilerFunc setProfiler = (SetProfilerFunc) library->sl_.getSymbol("SetProfiler");
                setProfiler(&framework_->GetProfiler());
            } 
            catch (Poco::Exception &e)
            {
                Foundation::RootLogError(e.displayText());
                Foundation::RootLogError("Failed to load dynamic library: " + path);
                return;
            }
        }

        /// Load each module in this shared library.
        for(StringVector::const_iterator it = entries.begin(); it != entries.end(); ++it)
        {
            if (HasModuleEntry(*it))
            {
                Foundation::RootLogDebug(">> " + *it + " already loaded.");
                continue;
            }

            if (IsExcluded(*it))
            {
                Foundation::RootLogDebug(">> " + *it + " excluded and not loaded.");
                continue;
            }

            Foundation::RootLogDebug(">> Loading module " + *it + ".");

            if (library->cl_.findClass(*it) == 0)
                throw Exception("Entry class not found from module");

            ModuleInterface *module = library->cl_.create(*it);
            assert(module);
            Module::ModuleDeletor md(*it, library);
            ModuleSharedPtr modulePtr(module, md);

            if (IsExcluded(module->Name()))
            {
                Foundation::RootLogInfo("   > Module " + module->Name() + " is excluded and not loaded.");
                continue;
            }

            assert(HasModule(module) == false);

#ifndef _DEBUG
            // Remove logging of debug messages in release mode. \todo This is not applied for modules loaded explicitly in LoadModuleByName.
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

            // Send a log message in the log channel of the module we just loaded.
            Poco::Logger::get(module->Name()).information(module->Name() + " loaded.");
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
        Foundation::RootLogDebug("Preinitializing module " + module->Name());
        module->PreInitializeInternal();

        // Do not log preinit success here to avoid extraneous logging.
    }

    void ModuleManager::InitializeModule(ModuleInterface *module)
    {
        assert(module);
        assert(module->State() == Foundation::Module::MS_Loaded);
        Foundation::RootLogDebug("Initializing module " + module->Name());
        module->InitializeInternal();

        // Send a log message in the log channel of the module we just initialized.
        Poco::Logger::get(module->Name()).information(module->Name() + " initialized.");
    }

    void ModuleManager::PostInitializeModule(ModuleInterface *module)
    {
        assert(module);
        assert(module->State() == Foundation::Module::MS_Loaded);
        Foundation::RootLogDebug("Postinitializing module " + module->Name());
        module->PostInitializeInternal();

        // Do not log postinit success here to avoid extraneous logging.
    }

    void ModuleManager::UninitializeModule(ModuleInterface *module)
    {
        assert(module);
        Foundation::RootLogDebug("Uninitializing module " + module->Name() + ".");
        module->UninitializeInternal();

        // Send a log message in the log channel of the module we just uninitialized.
        Poco::Logger::get(module->Name()).information(module->Name() + " uninitialized.");
    }

    void ModuleManager::UnloadModule(Module::Entry &entry)
    {
        assert(entry.module_);

        std::string moduleName = entry.module_->Name();
        std::string moduleNameStatic = entry.module_->Name();

        Foundation::RootLogDebug("Unloading module " + moduleName + ".");

        entry.module_->UnloadInternal();
        entry.module_.reset(); // Triggers the deletion of the ModuleInterface object. (either causes operator delete or Poco's module free to be called)

        // Send a log message in the log channel of the module we just unloaded. (the channel is in Poco and not in the module, so this is ok)
        Poco::Logger::get(moduleNameStatic).information(moduleName + " unloaded.");
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
        if (all_additions.size() == 0)
            return;

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
