// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AssetManager.h"
#include "AssetModule.h"
#include "RexUUID.h"

using namespace OpenSimProtocol;
using namespace RexTypes;

namespace Asset
{
    AssetModule::AssetModule() : ModuleInterface_Impl(type_static_),
        inboundcategory_id_(0)
    {
    }

    AssetModule::~AssetModule()
    {
    }

    // virtual
    void AssetModule::Load()
    {
        LogInfo("Module " + Name() + " loaded.");
        
        AutoRegisterConsoleCommand(Console::CreateCommand(
            "RequestAsset", "Request asset from server (testing only so far). Usage: RequestAsset(uuid,assettype)", 
            Console::Bind(this, &AssetModule::ConsoleRequestAsset)));
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
    
    void AssetModule::PostInitialize()
    {
        inboundcategory_id_ = framework_->GetEventManager()->QueryEventCategory("OpenSimNetworkIn");
        
        if (inboundcategory_id_ == 0 )
            LogWarning("Unable to find event category for OpenSimNetwork events!");
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
    
    Console::CommandResult AssetModule::ConsoleRequestAsset(const Core::StringVector &params)
    {
        if (params.size() != 2)
        {
            return Console::ResultFailure("Usage: RequestAsset(uuid,assettype)");
        }

        try
        {
            RexUUID asset_id(params[0]);
            int asset_type = Core::ParseString<int>(params[1]);

            manager_->RequestAsset(asset_id, asset_type);
        } catch (std::exception)
        {
            return Console::ResultInvalidParameters();
        }

        return Console::ResultSuccess();
    }
}

using namespace Asset;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
    POCO_EXPORT_CLASS(AssetModule)
POCO_END_MANIFEST

