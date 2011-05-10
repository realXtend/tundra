/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   IModule.cpp
 *  @brief  Interface for module objects.
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

IModule::IModule(const std::string &name_)
:name(name_), 
state_(MS_Unloaded), 
framework_(0)
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

    Uninitialize();

    // The module is now uninitialized, but it is still loaded in memory.
    // The module can now be initialized again, and InitializeInternal()
    // expects the state to be MS_Loaded.
    state_ = MS_Loaded;
}
