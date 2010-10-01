/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   IModule.cpp
 *  @brief  Interface for Naali modules.
 *          See @ref ModuleArchitecture for details.
 */
 
#include "StableHeaders.h"
#include "Framework.h"
#include "IModule.h"
#include "ServiceManager.h"
#include "EventManager.h"
#include "ModuleManager.h"
#include "ConsoleCommandServiceInterface.h"

#include <Poco/Logger.h>

static const int DEFAULT_EVENT_PRIORITY = 100;

using namespace Foundation;

IModule::IModule(const std::string &name) :
    name_(name), state_(MS_Unloaded), framework_(0)
{
    try
    {
        Poco::Logger::create(Name(),Poco::Logger::root().getChannel(), Poco::Message::PRIO_TRACE);
    }
    catch (const std::exception &e)
    {
        Foundation::RootLogError("Failed to create logger " + Name() + ":" + std::string(e.what()));
    }
}

IModule::~IModule()
{
    Poco::Logger::destroy(Name());
}

Framework *IModule::GetFramework() const
{
    return framework_;
}

void IModule::RegisterConsoleCommand(const Console::Command &command)
{
    boost::shared_ptr<Console::CommandService> console = framework_->GetService<Console::CommandService>(Service::ST_ConsoleCommand).lock();
    //assert(console.get());
    if (!console.get())
    {
        Poco::Logger::get(Name()).error(std::string("Error: Unable to register console command ") + command.name_ + std::string(". Console service not loaded!"));
        return;
    }

    console->RegisterCommand(command);
}

void IModule::InitializeInternal()
{
    assert(framework_ != 0);
    assert(state_ == MS_Loaded);

    //! Register components
    for(size_t n = 0; n < component_registrars_.size(); ++n)
        component_registrars_[n]->Register(framework_, this);

    //! Register commands
    if (console_commands_.size() > 0)
    {
        Poco::Logger::get(Name()).warning("Warning: Use of AutoRegisterConsoleCommand is deprecated. Use RegisterConsoleCommand instead.");
        Poco::Logger::get(Name()).warning("Warning: console_commands_ is deprecated and will be phased out soon.");
        boost::shared_ptr<Console::CommandService> console = framework_->GetService<Console::CommandService>(Service::ST_ConsoleCommand).lock();
        // If you hit this assert, you're trying to register console commands before the console service even exists.
        // Workarounds: 1) Add a dependency to ConsoleModule into your module XML file (not preferred)
        //              2) Use RegisterConsoleCommand instead of AutoRegisterConsoleCommand to register the console command.
        if (!console.get())
            Poco::Logger::get(Name()).error("Critical: Console service not loaded yet! Console command registration failed!");
//        assert(console.get()); 
        if (GetFramework()->GetServiceManager()->IsRegistered(Service::ST_ConsoleCommand) && console.get())
            for(CommandVector::iterator it = console_commands_.begin(); it != console_commands_.end(); ++it)
                console->RegisterCommand(*it);
    }

    // Register to event system with default priority
    framework_->GetEventManager()->RegisterEventSubscriber(this, DEFAULT_EVENT_PRIORITY);

    Initialize();
}

void IModule::UninitializeInternal()
{
    assert(framework_ != 0);
    if (state_ != MS_Initialized)
    {
        Poco::Logger::get(Name()).error("Uninitialize called on non-initialized module");
        // Initialization failed somehow, can't do proper uninit
        return;
    }

    for(size_t n=0 ; n<component_registrars_.size() ; ++n)
        component_registrars_[n]->Unregister(framework_);

    // Unregister commands
    for (CommandVector::iterator it = console_commands_.begin(); it != console_commands_.end(); ++it)
        if (framework_->GetServiceManager()->IsRegistered(Service::ST_ConsoleCommand))
        {
            boost::weak_ptr<Console::CommandService> console = framework_->GetService<Console::CommandService>(Service::ST_ConsoleCommand);
            console.lock()->UnregisterCommand(it->name_);
        }

    // Unregister from event system
    framework_->GetEventManager()->UnregisterEventSubscriber(this);

    Uninitialize();

    // The module is now uninitialized, but it is still loaded in memory.
    // The module can now be initialized again, and InitializeInternal()
    // expects the state to be MS_Loaded.
    state_ = MS_Loaded;
}
