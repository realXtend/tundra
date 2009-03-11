// For conditions of distribution and use, see copyright notice in license.txt

#include <Poco/Logger.h>
#include <Poco/LoggingFactory.h>
#include <Poco/FormattingChannel.h>
#include <Poco/SplitterChannel.h>
#include <Poco/LocalDateTime.h>

#include "StableHeaders.h"
#include "Foundation.h"

namespace Foundation
{
    Framework::Framework() : exit_signal_(false), config_(new ConfigurationManager(ConfigurationManager::CT_DEFAULT))
    {
        CreateLoggingSystem();

        // create managers
        module_manager_ = ModuleManagerPtr(new ModuleManager(this));
        component_manager_ = ComponentManagerPtr(new ComponentManager(this));
        service_manager_ = ServiceManagerPtr(new ServiceManager(this));
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

        Poco::Channel *consolechannel = loggingfactory->createChannel("ConsoleChannel");
        Poco::Channel *filechannel = loggingfactory->createChannel("FileChannel");
             
        std::string logfilepath = Poco::Path::current();
        logfilepath.append("rex.log");
        filechannel->setProperty("path",logfilepath);
        filechannel->setProperty("rotation","3M");
        filechannel->setProperty("archive","number");
        filechannel->setProperty("compress","false");

        Poco::SplitterChannel *splitterchannel = new Poco::SplitterChannel();
        splitterchannel->addChannel(consolechannel);
        splitterchannel->addChannel(filechannel); 

        log_formatter_ = loggingfactory->createFormatter("PatternFormatter");
        log_formatter_->setProperty("pattern","%H:%M:%S [%s] %t");
        log_formatter_->setProperty("times","local");
        Poco::Channel *formatchannel = new Poco::FormattingChannel(log_formatter_,splitterchannel);
        
        Poco::Logger::create("",formatchannel,Poco::Message::PRIO_TRACE);    
        Poco::Logger::create("Foundation",Poco::Logger::root().getChannel() ,Poco::Message::PRIO_TRACE);

        Poco::LocalDateTime *currenttime = new Poco::LocalDateTime(); 
        std::string timestring = boost::lexical_cast<std::string>(currenttime->day()) + "/";
        timestring.append(boost::lexical_cast<std::string>(currenttime->month()) + "/");
        timestring.append(boost::lexical_cast<std::string>(currenttime->year()) + " ");
        timestring.append(boost::lexical_cast<std::string>(currenttime->hour()) + ":");
        timestring.append(boost::lexical_cast<std::string>(currenttime->minute()) + ":");
        timestring.append(boost::lexical_cast<std::string>(currenttime->second()));                
        Foundation::RootLogInfo("Log file opened on " + timestring);
        
        log_channels_.push_back(consolechannel);
        log_channels_.push_back(filechannel);
        log_channels_.push_back(splitterchannel);
        log_channels_.push_back(formatchannel);
        
        delete currenttime;
        delete loggingfactory;
    }


    void Framework::Go()
    {
        Platform::PrepareApplicationDataDirectory();

        LoadModules();

        // main loop
        while (exit_signal_ == false)
        {
            // do synchronized update for modules
            module_manager_->UpdateModules();

            // call asynchronous update on modules / do parallel tasks

            // synchronize shared data across modules
            //mChangeManager->_propagateChanges();
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
}

