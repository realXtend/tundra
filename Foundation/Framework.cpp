// For conditions of distribution and use, see copyright notice in license.txt

#include <Poco/Logger.h>
#include <Poco/LoggingFactory.h>
#include <Poco/FormattingChannel.h>


#include "StableHeaders.h"
#include "Foundation.h"

namespace Foundation
{
    Framework::Framework() : exit_signal_(false)
    {
        // create managers
        module_manager_ = ModuleManagerPtr(new ModuleManager(this));
        component_manager_ = ComponentManagerPtr(new ComponentManager(this));
        service_manager_ = ServiceManagerPtr(new ServiceManager(this));

        // create main logger
        Poco::LoggingFactory *loggingfactory = new Poco::LoggingFactory();
        
        Poco::Formatter *defaultformatter = loggingfactory->createFormatter("PatternFormatter");
        defaultformatter->setProperty("pattern","%H:%M:%S [%s] %p: %t");
        defaultformatter->setProperty("times","local");

        Poco::Channel *consolechannel = loggingfactory->createChannel("ConsoleChannel");
        Poco::Channel *formatchannel = new Poco::FormattingChannel(defaultformatter,consolechannel);
        main_logger_ = &Poco::Logger::create("",formatchannel,Poco::Message::PRIO_TRACE);
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

