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
    Framework::Framework() : exit_signal_(false)
    {
       // create loggers
        Poco::LoggingFactory *loggingfactory = new Poco::LoggingFactory();
        
        Poco::Channel *consolechannel = loggingfactory->createChannel("ConsoleChannel");
        Poco::Channel *filechannel = loggingfactory->createChannel("FileChannel");
             
        std::string logfilepath = Poco::Path::current();
        logfilepath.append("rex.log");
        filechannel->setProperty("path",logfilepath);
        filechannel->setProperty("rotation","never");

        Poco::SplitterChannel *splitterchannel = new Poco::SplitterChannel();
        splitterchannel->addChannel(consolechannel);
        splitterchannel->addChannel(filechannel); 

        Poco::Formatter *defaultformatter = loggingfactory->createFormatter("PatternFormatter");
        defaultformatter->setProperty("pattern","%H:%M:%S [%s] %t");
        defaultformatter->setProperty("times","local");
        Poco::Channel *formatchannel = new Poco::FormattingChannel(defaultformatter,splitterchannel);
        
        Poco::Logger::create("",formatchannel,Poco::Message::PRIO_TRACE);    
        foundation_logger_ = &Poco::Logger::create("Foundation",Poco::Logger::root().getChannel() ,Poco::Message::PRIO_TRACE);

        Poco::LocalDateTime *currenttime = new Poco::LocalDateTime(); 
        std::string timestring = boost::lexical_cast<std::string>(currenttime->day()) + "/";
        timestring.append(boost::lexical_cast<std::string>(currenttime->month()) + "/");
        timestring.append(boost::lexical_cast<std::string>(currenttime->year()) + " ");
        timestring.append(boost::lexical_cast<std::string>(currenttime->hour()) + ":");
        timestring.append(boost::lexical_cast<std::string>(currenttime->minute()) + ":");
        timestring.append(boost::lexical_cast<std::string>(currenttime->second()));                
        Foundation::RootLogInfo("Log file opened on " + timestring);

        // create managers
        module_manager_ = ModuleManagerPtr(new ModuleManager(this));
        component_manager_ = ComponentManagerPtr(new ComponentManager(this));
        service_manager_ = ServiceManagerPtr(new ServiceManager(this));
    }

    Framework::~Framework()
    {
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

