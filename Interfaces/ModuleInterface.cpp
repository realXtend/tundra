// For conditions of distribution and use, see copyright notice in license.txt

#include <boost/smart_ptr.hpp>
#include <Poco/Logger.h>
#include <Framework.h>
#include "ModuleInterface.h"
#include "ConfigurationManager.h"
#include "ServiceManager.h"
#include "EventManager.h"
#include "ModuleManager.h"
#include "ConsoleCommandServiceInterface.h"

namespace Foundation
{

static const int DEFAULT_EVENT_PRIORITY = 100;

ModuleInterfaceImpl::ModuleInterfaceImpl(const std::string &name) :
    name_(name), type_(Module::MT_Unknown), state_(Module::MS_Unloaded), framework_(0)
{
    try
    {
        Poco::Logger::create(Name(),Poco::Logger::root().getChannel(), Poco::Message::PRIO_TRACE);
    }
    catch (std::exception)
    {
        Foundation::RootLogError("Failed to create logger " + Name() + ".");
    }
}

ModuleInterfaceImpl::ModuleInterfaceImpl(Module::Type type) :
    type_(type), state_(Module::MS_Unloaded), framework_(0)
{
    try
    {
        Poco::Logger::create(Name(),Poco::Logger::root().getChannel(),Poco::Message::PRIO_TRACE);
    }
    catch (std::exception)
    {
        Foundation::RootLogError("Failed to create logger " + Name() + ".");
    }
}

ModuleInterfaceImpl::~ModuleInterfaceImpl()
{
    Poco::Logger::destroy(Name());
}

void ModuleInterfaceImpl::AutoRegisterConsoleCommand(const Console::Command &command)
{
    assert (State() == Module::MS_Unloaded && "AutoRegisterConsoleCommand function can only be used when loading the module.");

    Poco::Logger::get(Name()).warning("Warning: Use of AutoRegisterConsoleCommand is deprecated. Use RegisterConsoleCommand instead.");

    for(CommandVector::iterator it = console_commands_.begin() ; it != console_commands_.end(); ++it )
    {
        if (it->name_ == command.name_)
            assert (false && "Registering console command twice");
    }

    console_commands_.push_back(command); 
}
Framework *ModuleInterfaceImpl::GetFramework() const
{
    return framework_;
}

std::string ModuleInterfaceImpl::VersionMajor() const
{
    if (IsInternal())
    {
        static const std::string version("version_major");
        return ( GetFramework()->GetDefaultConfig().GetSetting<std::string>(Framework::ConfigurationGroup(), version) );
    }
    return std::string("0");
}

std::string ModuleInterfaceImpl::VersionMinor() const
{
    if (IsInternal())
    {
        static const std::string version("version_minor");
        return ( GetFramework()->GetDefaultConfig().GetSetting<std::string>(Framework::ConfigurationGroup(), version) );
    }
    return std::string("0");
}

void ModuleInterfaceImpl::RegisterConsoleCommand(const Console::Command &command)
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

void ModuleInterfaceImpl::InitializeInternal()
{
    assert(framework_ != 0);
    assert(state_ == Module::MS_Loaded);

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
        {
            for(CommandVector::iterator it = console_commands_.begin(); it != console_commands_.end(); ++it)
                console->RegisterCommand(*it);
        }
    }
    
    // Register to event system with default priority
    framework_->GetEventManager()->RegisterEventSubscriber(framework_->GetModuleManager()->GetModule(this), DEFAULT_EVENT_PRIORITY);
    
    Initialize();
}

void ModuleInterfaceImpl::UninitializeInternal()
{
    assert(framework_ != 0);
    assert (state_ == Module::MS_Initialized);

    for(size_t n=0 ; n<component_registrars_.size() ; ++n)
        component_registrars_[n]->Unregister(framework_);

    // Unregister commands
    for (CommandVector::iterator it = console_commands_.begin(); it != console_commands_.end(); ++it)
    {
        if (framework_->GetServiceManager()->IsRegistered(Service::ST_ConsoleCommand))
        {
            boost::weak_ptr<Console::CommandService> console = framework_->GetService<Console::CommandService>(Service::ST_ConsoleCommand);
            console.lock()->UnregisterCommand(it->name_);
        }
    }

    // Unregister from event system
    framework_->GetEventManager()->UnregisterEventSubscriber(this);
    
    Uninitialize();

    // The module is now uninitialized, but it is still loaded in memory.
    // The module can now be initialized again, and InitializeInternal()
    // expects the state to be Module::MS_Loaded.
    state_ = Module::MS_Loaded;
}

}
