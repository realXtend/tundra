// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AssetModule.h"
#include "AssetManager.h"
#include "UDPAssetProvider.h"
#include "XMLRPCAssetProvider.h"
#include "HttpAssetProvider.h"
#include "NetworkEvents.h"
#include "Framework.h"
#include "Profiler.h"
#include "EventManager.h"
#include "ServiceManager.h"
#include "CoreException.h"

namespace Asset
{
    AssetModule::AssetModule() : ModuleInterfaceImpl(type_static_), inboundcategory_id_(0)
    {
    }

    AssetModule::~AssetModule()
    {
    }

    // virtual
    void AssetModule::Load()
    {
        AutoRegisterConsoleCommand(Console::CreateCommand(
            "RequestAsset", "Request asset from server. Usage: RequestAsset(uuid,assettype)", 
            Console::Bind(this, &AssetModule::ConsoleRequestAsset)));
    }

    // virtual
    void AssetModule::Initialize()
    {
        manager_ = AssetManagerPtr(new AssetManager(framework_));
        framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Asset, manager_);

        udp_asset_provider_ = Foundation::AssetProviderPtr(new UDPAssetProvider(framework_));
        manager_->RegisterAssetProvider(udp_asset_provider_);

        // Add XMLRPC asset provider before http asset provider, so it will take requests it recognizes though both use http
        xmlrpc_asset_provider_ = Foundation::AssetProviderPtr(new XMLRPCAssetProvider(framework_));
        manager_->RegisterAssetProvider(xmlrpc_asset_provider_);

        try
        {
            http_asset_provider_ = Foundation::AssetProviderPtr(new HttpAssetProvider(framework_));
            manager_->RegisterAssetProvider(http_asset_provider_);
        }
        catch (Exception &/*e*/)
        {
            AssetModule::LogWarning("Failed to create HTTP asset provider.");
        }

        framework_category_id_ = framework_->GetEventManager()->QueryEventCategory("Framework");
    }

    void AssetModule::SubscribeToNetworkEvents(boost::weak_ptr<ProtocolUtilities::ProtocolModuleInterface> currentProtocolModule)
    {
        protocolModule_ = currentProtocolModule;
        udp_asset_provider_->SetCurrentProtocolModule(protocolModule_);

        network_state_category_id_ = framework_->GetEventManager()->QueryEventCategory("NetworkState");
        if (network_state_category_id_ == 0)
            LogWarning("Failed to query \"NetworkState\" event category");

        inboundcategory_id_ = framework_->GetEventManager()->QueryEventCategory("NetworkIn");
        if (inboundcategory_id_ == 0 )
            LogWarning("Unable to find event category for OpenSimNetwork events!");
        else
            LogInfo("System " + Name() + " subscribed to network events [NetworkIn]");
    }

    void AssetModule::UnsubscribeNetworkEvents()
    {
    }

    // virtual
    void AssetModule::Update(f64 frametime)
    {
        {
            PROFILE(AssetModule_Update);
            if (manager_)
                manager_->Update(frametime);
        }
        RESETPROFILER;
    }

    // virtual 
    void AssetModule::Uninitialize()
    {
        manager_->UnregisterAssetProvider(udp_asset_provider_);
        manager_->UnregisterAssetProvider(xmlrpc_asset_provider_);
        manager_->UnregisterAssetProvider(http_asset_provider_);

        framework_->GetServiceManager()->UnregisterService(manager_);
        manager_.reset();
    }

    Console::CommandResult AssetModule::ConsoleRequestAsset(const StringVector &params)
    {
        if (params.size() != 2)
            return Console::ResultFailure("Usage: RequestAsset(uuid,assettype)");

        manager_->RequestAsset(params[0], params[1]);
        return Console::ResultSuccess();
    }

    bool AssetModule::HandleEvent(
        event_category_id_t category_id,
        event_id_t event_id, 
        Foundation::EventDataInterface* data)
    {
        PROFILE(AssetModule_HandleEvent);
        if ((category_id == inboundcategory_id_))
        {
            if (udp_asset_provider_)
                return checked_static_cast<UDPAssetProvider*>(udp_asset_provider_.get())->HandleNetworkEvent(data);
        }
        else if (category_id == framework_category_id_ && event_id == Foundation::NETWORKING_REGISTERED)
        {
            ProtocolUtilities::NetworkingRegisteredEvent *event_data = dynamic_cast<ProtocolUtilities::NetworkingRegisteredEvent *>(data);
            if (event_data)
                SubscribeToNetworkEvents(event_data->currentProtocolModule);
            return false;
        }
        if (category_id == network_state_category_id_ && event_id == ProtocolUtilities::Events::EVENT_SERVER_DISCONNECTED)
        {
            if (udp_asset_provider_)
                checked_static_cast<UDPAssetProvider*>(udp_asset_provider_.get())->ClearAllTransfers();
        }

        return false;
    }
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace Asset;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
    POCO_EXPORT_CLASS(AssetModule)
POCO_END_MANIFEST

