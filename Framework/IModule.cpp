/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   IModule.cpp
 *  @brief  Interface for Naali modules.
 *          See @ref ModuleArchitecture for details.
 */
 
#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MemoryLeakCheck.h"
#include "Framework.h"
#include "IModule.h"
#include "ServiceManager.h"
#include "ModuleManager.h"
#include "LoggingFunctions.h"

using namespace Foundation;

IModule::IModule(const std::string &name) :
    name_(name), state_(MS_Unloaded), framework_(0)
{
}

IModule::~IModule()
{
}

Framework *IModule::GetFramework() const
{
    return framework_;
}

void IModule::InitializeInternal()
{
    assert(framework_ != 0);
    assert(state_ == MS_Loaded);

    /// Register components
    for(size_t n = 0; n < component_registrars_.size(); ++n)
        component_registrars_[n]->Register(framework_, this);

    Initialize();
}

void IModule::UninitializeInternal()
{
    assert(framework_ != 0);
    if (state_ != MS_Initialized)
    {
        LogError("Uninitialize called on non-initialized module");
        // Initialization failed somehow, can't do proper uninit
        return;
    }

    for(size_t n=0 ; n<component_registrars_.size() ; ++n)
        component_registrars_[n]->Unregister(framework_);

    Uninitialize();

    // The module is now uninitialized, but it is still loaded in memory.
    // The module can now be initialized again, and InitializeInternal()
    // expects the state to be MS_Loaded.
    state_ = MS_Loaded;
}
