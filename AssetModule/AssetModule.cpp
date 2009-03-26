// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AssetManager.h"
#include "AssetModule.h"

namespace Asset
{
    AssetModule::AssetModule() : ModuleInterface_Impl(type_static_)
    {
    }

    AssetModule::~AssetModule()
    {
    }

    // virtual
    void AssetModule::Load()
    {
        LogInfo("Module " + Name() + " loaded.");
    }

    // virtual
    void AssetModule::Unload()
    {
        LogInfo("Module " + Name() + " unloaded.");
    }

    // virtual
    void AssetModule::Initialize()
    {
        manager_ = AssetManagerPtr(new AssetManager(framework_));
        manager_->Initialize();
        framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Asset, manager_.get());
        
        LogInfo("Module " + Name() + " initialized.");
    }

    void AssetModule::Update()
    {
    }

    // virtual 
    void AssetModule::Uninitialize()
    {
        framework_->GetServiceManager()->UnregisterService(manager_.get());
        manager_.reset();
        
        LogInfo("Module " + Name() + " uninitialized.");
    }
}

using namespace Asset;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
    POCO_EXPORT_CLASS(AssetModule)
POCO_END_MANIFEST

