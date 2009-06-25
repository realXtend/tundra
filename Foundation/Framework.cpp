// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include <Poco/Logger.h>
#include <Poco/LoggingFactory.h>
#include <Poco/FormattingChannel.h>
#include <Poco/SplitterChannel.h>
#include <Poco/LocalDateTime.h>
#include "Poco/UnicodeConverter.h"

#include "Foundation.h"
#include "SceneManager.h"
#include "SceneEvents.h"

#include "RexQEngine.h"

namespace Foundation
{
    const char *Framework::DEFAULT_EVENT_SUBSCRIBER_TREE_PATH = "./data/event_tree.xml";
    
    Framework::Framework() : exit_signal_(false)
    {
        ProfilerSection::SetProfiler(&profiler_);

        PROFILE(FW_Startup);
        application_ = ApplicationPtr(new Application(this));
        platform_ = PlatformPtr(new Platform(this));
	
	// Create config manager
	config_manager_ = ConfigurationManagerPtr(new ConfigurationManager(this));

        config_manager_->DeclareSetting(Framework::ConfigurationGroup(), std::string("version_major"), std::string("0"));
        config_manager_->DeclareSetting(Framework::ConfigurationGroup(), std::string("version_minor"), std::string("0.1"));
        config_manager_->DeclareSetting(Framework::ConfigurationGroup(), std::string("application_name"), std::string("realXtend"));
        config_manager_->DeclareSetting(Framework::ConfigurationGroup(), std::string("window_title"), std::string("realXtend Naali"));
        config_manager_->DeclareSetting(Framework::ConfigurationGroup(), std::string("log_console"), bool(true));
        config_manager_->DeclareSetting(Framework::ConfigurationGroup(), std::string("log_level"), std::string("information"));

        Core::uint max_fps_release = config_manager_->DeclareSetting(Framework::ConfigurationGroup(), std::string("max_fps_release"), 60);
        Core::uint max_fps_debug = config_manager_->DeclareSetting(Framework::ConfigurationGroup(), std::string("max_fps_debug"), static_cast<Core::uint>(-1));

        
        max_ticks_ = 1000 / max_fps_release;
#ifdef _DEBUG
        max_ticks_ = 1000 / max_fps_debug;
#endif

        platform_->PrepareApplicationDataDirectory(); // depends on config

        CreateLoggingSystem(); // depends on config and platform

        // create managers
        module_manager_ = ModuleManagerPtr(new ModuleManager(this));
        component_manager_ = ComponentManagerPtr(new ComponentManager(this));
        service_manager_ = ServiceManagerPtr(new ServiceManager(this));
        event_manager_ = EventManagerPtr(new EventManager(this));

        Scene::Events::RegisterSceneEvents(event_manager_);

        q_engine_ = boost::shared_ptr<RexQEngine>(new RexQEngine(this));
    }

    Framework::~Framework()
    {
        default_scene_.reset();
        scenes_.clear();
        module_manager_.reset();

        Poco::Logger::shutdown();
 
        for (size_t i=0 ; i<log_channels_.size() ; ++i)
            log_channels_[i]->release();
            
        log_channels_.clear();
        log_formatter_->release();
    }

    void Framework::CreateLoggingSystem()
    {
        PROFILE(FW_CreateLoggingSystem);
        Poco::LoggingFactory *loggingfactory = new Poco::LoggingFactory();

        Poco::Channel *consolechannel = NULL;
        if (config_manager_->GetSetting<bool>(Framework::ConfigurationGroup(), "log_console"))
            consolechannel = loggingfactory->createChannel("ConsoleChannel");

        Poco::Channel *filechannel = loggingfactory->createChannel("FileChannel");
        
        std::wstring logfilepath_w = platform_->GetUserDocumentsDirectoryW();
        logfilepath_w += L"/" + Core::ToWString(config_manager_->GetSetting<std::string>(Framework::ConfigurationGroup(), "application_name")) + L".log";
        std::string logfilepath;
        Poco::UnicodeConverter::toUTF8(logfilepath_w, logfilepath);

        filechannel->setProperty("path",logfilepath);
        filechannel->setProperty("rotation","3M");
        filechannel->setProperty("archive","number");
        filechannel->setProperty("compress","false");

        Poco::SplitterChannel *splitterchannel = new Poco::SplitterChannel();
        if (consolechannel)
            splitterchannel->addChannel(consolechannel);
        splitterchannel->addChannel(filechannel); 

        log_formatter_ = loggingfactory->createFormatter("PatternFormatter");
        log_formatter_->setProperty("pattern","%H:%M:%S [%s] %t");
        log_formatter_->setProperty("times","local");
        Poco::Channel *formatchannel = new Poco::FormattingChannel(log_formatter_,splitterchannel);
        
        try
        {
            Poco::Logger::create("",formatchannel,Poco::Message::PRIO_TRACE);    
            Poco::Logger::create("Foundation",Poco::Logger::root().getChannel() ,Poco::Message::PRIO_TRACE);
        } catch (Poco::ExistsException)
        {
            assert (false && "Somewhere, a message is pushed to log before the logger is initialized.");
        }

        Poco::LocalDateTime *currenttime = new Poco::LocalDateTime(); 
        std::string timestring = boost::lexical_cast<std::string>(currenttime->day()) + "/";
        timestring.append(boost::lexical_cast<std::string>(currenttime->month()) + "/");
        timestring.append(boost::lexical_cast<std::string>(currenttime->year()) + " ");
        timestring.append(boost::lexical_cast<std::string>(currenttime->hour()) + ":");
        timestring.append(boost::lexical_cast<std::string>(currenttime->minute()) + ":");
        timestring.append(boost::lexical_cast<std::string>(currenttime->second()));
        
        try        
        {
            Foundation::RootLogInfo("Log file opened on " + timestring);
        } catch(Poco::OpenFileException)
        {
            // Do not create the log file.
            splitterchannel->removeChannel(filechannel);
            Foundation::RootLogInfo("Poco::OpenFileException. Log file not created.");
        }

#ifndef _DEBUG
        // make it so debug messages are not logged in release mode
        std::string log_level = config_manager_->GetSetting<std::string>(Framework::ConfigurationGroup(), "log_level");
        Poco::Logger::get("Foundation").setLevel(log_level);
#endif
        
        if (consolechannel)
            log_channels_.push_back(consolechannel);
        log_channels_.push_back(filechannel);
        log_channels_.push_back(splitterchannel);
        log_channels_.push_back(formatchannel);
        
        delete currenttime;
        delete loggingfactory;
    }

    void Framework::PostInitialize()
    {
        PROFILE(FW_PostInitialize);

        LoadModules();

        // commands must be registered after modules are loaded and initialized
        RegisterConsoleCommands();

        // add event subscribers now, that all modules are loaded/initialized
        event_manager_->LoadEventSubscriberTree(DEFAULT_EVENT_SUBSCRIBER_TREE_PATH);
    }

    void Framework::ProcessOneFrame()
    {
        if (exit_signal_ == true)
            return; // We've accidentally ended up to update a frame, but we're actually quitting.

#ifdef PROFILING
        // Reset profiling data. Should be outside of any profiling blocks.
        GetProfiler().Reset();
#endif

        PROFILE(MainLoop);

        double frametime = timer.elapsed();
        
        timer.restart();
        // do synchronized update for modules
        {
            PROFILE(FW_UpdateModules);
            module_manager_->UpdateModules(frametime);
        }

        // call asynchronous update on modules / do parallel tasks

        // synchronize shared data across modules
        //mChangeManager->_propagateChanges();
        
        // process delayed events
        
        {
           PROFILE(FW_ProcessDelayedEvents);
            event_manager_->ProcessDelayedEvents(frametime);
        }
        
        // if we have a renderer service, render now
        boost::weak_ptr<Foundation::RenderServiceInterface> renderer = 
                    service_manager_->GetService<RenderServiceInterface>(Service::ST_Renderer);
		
        if (renderer.expired() == false)
        {
            PROFILE(FW_Render);
            renderer.lock()->Render();
        }

        //! \note Frame limiter disabled for now for inaccuracy of boost timer, hopefully more accurate solution can be made on Qt side
        //! \note We limit frames for the whole main thread, not just for the renderer. This is the price to pay for being an application rather than a game.
        //Core::uint elapsed_time = static_cast<Core::uint>(timer.elapsed() * 1000); // get time until this point, as we do not want to include time used in sleeping in previous frame
        //if (max_ticks_ > elapsed_time)
        //{
        //    boost::this_thread::sleep(boost::posix_time::milliseconds(max_ticks_ - elapsed_time));
        //}
    }

    std::string Framework::GetApplicationMainWindowHandle() const
    { 
        return q_engine_->GetMainWindowHandle();
    }

    QWidget *Framework::GetApplicationMainWindowQWidget() const
    {
        return q_engine_->GetMainWindowQWidget();
    }

    void Framework::Go()
    {
        PROFILE(FW_Go);
        PostInitialize();

#ifdef USE_QT
        q_engine_->Go();
#else
        // main loop
        while (exit_signal_ == false)
            ProcessOneFrame();
#endif

         UnloadModules();
    }
    
    void Framework::Exit()
    {
        exit_signal_ = true;
#ifdef USE_QT
        if (q_engine_)
            q_engine_->SendQAppQuitMessage();
#endif
    }

    void Framework::LoadModules()
    {
        {
            PROFILE(FW_LoadModules);
            module_manager_->LoadAvailableModules();
        }
        {
            PROFILE(FW_InitializeModules);
            module_manager_->InitializeModules();
        }
    }

    void Framework::UnloadModules()
    {
        module_manager_->UninitializeModules();
        scenes_.clear();
        module_manager_->UnloadModules();
    }

    Scene::ScenePtr Framework::CreateScene(const std::string &name)
    {
        if (HasScene(name))
            return Scene::ScenePtr();

        Scene::ScenePtr new_scene = Scene::ScenePtr(new Scene::SceneManager(name, this));
        scenes_[name] = new_scene;

        return new_scene;
    }

    void Framework::RemoveScene(const std::string &name)
    {
        SceneMap::iterator scene = scenes_.find(name);
        if (scene != scenes_.end())
        {
            scenes_.erase(scene);
        }
    }
    
    Scene::ScenePtr Framework::GetScene(const std::string &name) const
    {
        SceneMap::const_iterator scene = scenes_.find(name);
        if (scene != scenes_.end())
            return scene->second;

        return Scene::ScenePtr();
    }

    Console::CommandResult Framework::ConsoleLoadModule(const Core::StringVector &params)
    {
        if (params.size() != 2 && params.size() != 1)
            return Console::ResultInvalidParameters();

        std::string lib = params[0];
        std::string entry = params[0];
        if (params.size() == 2)
            entry = params[1];
        
        bool result = module_manager_->LoadModuleByName(lib, entry);
        event_manager_->ValidateEventSubscriberTree();

        if (!result)
            return Console::ResultFailure("Library or module not found.");
        
        return Console::ResultSuccess("Module " + entry + " loaded.");
    }

    Console::CommandResult Framework::ConsoleUnloadModule(const Core::StringVector &params)
    {
        if (params.size() != 1)
            return Console::ResultInvalidParameters();
        
        bool result = false;
        if (module_manager_->HasModule(params[0]))
        {
            result = module_manager_->UnloadModuleByName(params[0]);
            event_manager_->ValidateEventSubscriberTree();
        }

        if (!result)
            return Console::ResultFailure("Module not found.");
        
        return Console::ResultSuccess("Module " + params[0] + " unloaded.");
    }

    Console::CommandResult Framework::ConsoleListModules(const Core::StringVector &params)
    {
        boost::shared_ptr<Console::ConsoleServiceInterface> console = GetService<Console::ConsoleServiceInterface>(Foundation::Service::ST_Console).lock();
        if (console)
        {
            console->Print("Loaded modules:");
            const ModuleManager::ModuleVector &modules = module_manager_->GetModuleList();
            for (size_t i = 0 ; i < modules.size() ; ++i)
            {
                console->Print(modules[i].module_->Name());
            }
        }

        return Console::ResultSuccess();
    }

    Console::CommandResult Framework::ConsoleSendEvent(const Core::StringVector &params)
    {
        if (params.size() != 2)
            return Console::ResultInvalidParameters();

        Core::event_category_id_t event_category = event_manager_->QueryEventCategory(params[0]);
        if (event_category == Core::IllegalEventCategory)
        {
            return Console::ResultFailure("Event category not found.");
        } else
        {
            event_manager_->SendEvent(
                event_category,
                Core::ParseString<Core::event_id_t>(params[1]),
                NULL);

            return Console::ResultSuccess();
        }
    }

    static std::string FormatTime(double time)
    {
        char str[128];
        if (time >= 60.0)
        {
            double seconds = fmod(time, 60.0);
            int minutes = (int)(time / 60.0);
            sprintf(str, "%dmin %2.2fs", minutes, (float)seconds);
        }
        else if (time >= 1.0)
            sprintf(str, "%2.2fs", (float)time);
        else
            sprintf(str, "%2.2fms", (float)time*1000.f);

        return std::string(str);
    }

    /// Outputs a hierarchical list of all PROFILE() blocks onto the given console.
    /// @param node The root node where to start the printing.
    /// @param showUnused If true, even blocks that haven't been called will be included. If false, only
    ///        the blocks that were actually recently called are included. 
    void PrintTimingsToConsole(const Console::ConsolePtr &console, const ProfilerNodeTree *node, bool showUnused)
    {
        const ProfilerNode *timings_node = dynamic_cast<const ProfilerNode*>(node);

        // Controls whether we will recursively call self to also print all child nodes.
        bool recurseToChildren = true;

        static int level = -2;

        if (timings_node)
        {
            level += 2;
            assert (level >= 0);

            double average = timings_node->num_called_total_ == 0 ? 0.0 : timings_node->total_ / timings_node->num_called_total_;

            if (timings_node->num_called_ == 0 && !showUnused)
                recurseToChildren = false;
            else
            {
                char str[512];
                // If we've spent less than 1/10th of a millisecond, show condensed.
                if (!showUnused && timings_node->elapsed_ * 1000.0 < 0.1) 
                    sprintf(str, "%s: called: %d.", timings_node->Name().c_str(), timings_node->num_called_);
                else
                    sprintf(str, "%s: called total: %d, elapsed total: %s, called: %d, elapsed: %s, avg: %s",
                        timings_node->Name().c_str(), timings_node->num_called_total_,
                        FormatTime(timings_node->total_).c_str(), timings_node->num_called_,
                        FormatTime(timings_node->elapsed_).c_str(), FormatTime(average).c_str());

                std::string timings;
                timings.append(level, ' ');
                timings += str;
    /*
                timings += timings_node->Name();
                timings += ": called total " + Core::ToString(timings_node->num_called_total_);
                timings += ", elapsed total " + Core::ToString(timings_node->total_);
                timings += ", called " + Core::ToString(timings_node->num_called_);
                timings += ", elapsed " + Core::ToString(timings_node->elapsed_);
                timings += ", average " + Core::ToString(average);
    */
                console->Print(timings);
            }
        }

        if (recurseToChildren)
        {
            const ProfilerNodeTree::NodeList &children = node->GetChildren();
            for (ProfilerNodeTree::NodeList::const_iterator it = children.begin() ; 
                 it != children.end() ;
                 ++it)
            {
                PrintTimingsToConsole(console, *it, showUnused);
            }
        }
        if (timings_node)
        {
            level -= 2;
        }
    }

    Console::CommandResult Framework::ConsoleProfile(const Core::StringVector &params)
    {
        boost::shared_ptr<Console::ConsoleServiceInterface> console = GetService<Console::ConsoleServiceInterface>(Foundation::Service::ST_Console).lock();
        if (console)
        {
            Profiler &profiler = GetProfiler();
            ProfilerNodeTree *node = profiler.Lock().get();
            PrintTimingsToConsole(console, node, false);
            profiler.Release();
        }
        return Console::ResultSuccess();
    }

    Console::CommandResult Framework::ConsoleLimitFrames(const Core::StringVector &params)
    {
        if (params.size() != 1)
            return Console::ResultInvalidParameters();

        int max_fps = Core::clamp(Core::ParseString<int>(params[0]), 1, 1000000);
        max_ticks_ = 1000 / max_fps;

        return Console::ResultSuccess();
    }


    void Framework::RegisterConsoleCommands()
    {
        boost::shared_ptr<Console::CommandService> console = GetService<Console::CommandService>(Foundation::Service::ST_ConsoleCommand).lock();
        if (console)
        {
            console->RegisterCommand(Console::CreateCommand("LoadModule", 
                "Loads a module from shared library. Usage: LoadModule(lib, entry)", 
                Console::Bind(this, &Framework::ConsoleLoadModule)));

            console->RegisterCommand(Console::CreateCommand("UnloadModule", 
                "Unloads a module. Usage: UnloadModule(name)", 
                Console::Bind(this, &Framework::ConsoleUnloadModule)));

            console->RegisterCommand(Console::CreateCommand("ListModules", 
                "Lists all loaded modules.", 
                Console::Bind(this, &Framework::ConsoleListModules)));

            console->RegisterCommand(Console::CreateCommand("SendEvent", 
                "Sends an internal event. Only for events that contain no data. Usage: SendEvent(event category name, event id)", 
                Console::Bind(this, &Framework::ConsoleSendEvent)));

            console->RegisterCommand(Console::CreateCommand("Profile", 
                "Outputs profiling data. Usage: Profile() for full, or Profile(name) for specific profiling block", 
                Console::Bind(this, &Framework::ConsoleProfile)));

            console->RegisterCommand(Console::CreateCommand("FrameLimit", 
                "Limit fps. Usage: FrameLimit(max_frames)", 
                Console::Bind(this, &Framework::ConsoleLimitFrames)));
        }
    }
}

