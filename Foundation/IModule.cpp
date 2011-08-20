/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   IModule.cpp
 *  @brief  Interface for modules.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Framework.h"
#include "IModule.h"
#include "ServiceManager.h"
#include "EventManager.h"
#include "ModuleManager.h"

#include <Poco/Logger.h>

#include "MemoryLeakCheck.h"

static const int DEFAULT_EVENT_PRIORITY = 100;

using namespace Foundation;

IModule::IModule(const std::string &name) :
    name_(name), state_(MS_Unloaded), framework_(0)
{
    try
    {
#ifdef _DEBUG
            int loggingLevel = Poco::Message::PRIO_TRACE;
#else
            int loggingLevel = Poco::Message::PRIO_INFORMATION;
#endif            
        Poco::Logger::create(Name(),Poco::Logger::root().getChannel(), loggingLevel);
    }
    catch (const std::exception &e)
    {
        RootLogError("Failed to create logger " + Name() + ":" + std::string(e.what()));
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

void IModule::InitializeInternal()
{
    assert(framework_ != 0);
    assert(state_ == MS_Loaded);

    //! Register components
    for(size_t n = 0; n < component_registrars_.size(); ++n)
        component_registrars_[n]->Register(framework_, this);

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

    // Unregister from event system
    framework_->GetEventManager()->UnregisterEventSubscriber(this);

    Uninitialize();

    // The module is now uninitialized, but it is still loaded in memory.
    // The module can now be initialized again, and InitializeInternal()
    // expects the state to be MS_Loaded.
    state_ = MS_Loaded;
}
