// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include <Poco/Logger.h>
#include <Poco/LoggingFactory.h>
#include <Poco/FormattingChannel.h>
#include <Poco/SplitterChannel.h>
#include <Poco/LocalDateTime.h>
#include "Poco/UnicodeConverter.h"

#include "Foundation.h"

namespace Foundation
{
   const char *Framework::DEFAULT_EVENT_SUBSCRIBER_TREE_PATH = "./data/event_tree.xml";
   
    Framework::Framework() : exit_signal_(false)
    {
        platform_ = PlatformPtr(new Platform(this));

        config_.DeclareSetting(Framework::ConfigurationGroup(), "application_name", "realXtend");
        config_.DeclareSetting(Framework::ConfigurationGroup(), "log_console", true);
        config_.DeclareSetting(Framework::ConfigurationGroup(), "log_level", "information");

        platform_->PrepareApplicationDataDirectory(); // depends on config

        CreateLoggingSystem(); // depends on config and platform

        // create managers
        module_manager_ = ModuleManagerPtr(new ModuleManager(this));
        component_manager_ = ComponentManagerPtr(new ComponentManager(this));
        service_manager_ = ServiceManagerPtr(new ServiceManager(this));
        event_manager_ = EventManagerPtr(new EventManager(this));
    }

    Framework::~Framework()
    {
        Poco::Logger::shutdown();
 
        for (size_t i=0 ; i<log_channels_.size() ; ++i)
            log_channels_[i]->release();
            
        log_channels_.clear();
        log_formatter_->release();
    }

    void Framework::CreateLoggingSystem()
    {
        Poco::LoggingFactory *loggingfactory = new Poco::LoggingFactory();

        Poco::Channel *consolechannel = NULL;
        if (config_.GetBool(Framework::ConfigurationGroup(), "log_console"))
            consolechannel = loggingfactory->createChannel("ConsoleChannel");

        Poco::Channel *filechannel = loggingfactory->createChannel("FileChannel");
        
        std::wstring logfilepath_w = platform_->GetUserDocumentsDirectoryW();
        logfilepath_w += L"/" + Core::ToWString(config_.GetString(Framework::ConfigurationGroup(), "application_name")) + L".log";
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
        std::string log_level = config_.GetString(Framework::ConfigurationGroup(), "log_level");
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


    void Framework::Go()
    {
        LoadModules();

        // commands must be registered after modules are loaded and initialized
        RegisterConsoleCommands();

        // add event subscribers now, that all modules are loaded/initialized
        event_manager_->LoadEventSubscriberTree(DEFAULT_EVENT_SUBSCRIBER_TREE_PATH);

        // main loop
        while (exit_signal_ == false)
        {
            // do synchronized update for modules
            module_manager_->UpdateModules();

            // call asynchronous update on modules / do parallel tasks

            // synchronize shared data across modules
            //mChangeManager->_propagateChanges();
            
            // if we have a renderer service, render now
            if (service_manager_->IsRegistered(Service::ST_Renderer))
            {
                Foundation::RenderServiceInterface *renderer = 
                    service_manager_->GetService<RenderServiceInterface>(Service::ST_Renderer);
                renderer->Render();
            }
            
        }

         UnloadModules();
    }

    void Framework::LoadModules()
    {
        module_manager_->LoadAvailableModules();
        module_manager_->InitializeModules();
    }

    void Framework::UnloadModules()
    {
        module_manager_->UninitializeModules();
        module_manager_->UnloadModules();
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
        }

        if (!result)
            return Console::ResultFailure("Module not found.");
        
        return Console::ResultSuccess("Module " + params[0] + " unloaded.");
    }

    void Framework::RegisterConsoleCommands()
    {
        Console::CommandService *console = GetService<Console::CommandService>(Foundation::Service::ST_ConsoleCommand);
        console->RegisterCommand(Console::CreateCommand("LoadModule", 
            "Loads a module from shared library. Usage: LoadModule(lib, entry)", 
            Console::Bind(this, &Framework::ConsoleLoadModule)));

        console->RegisterCommand(Console::CreateCommand("UnloadModule", 
            "Unloads a module. Usage: LoadModule(name)", 
            Console::Bind(this, &Framework::ConsoleUnloadModule)));
    }
}

