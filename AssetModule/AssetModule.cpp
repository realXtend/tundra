// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AssetModule.h"
#include "AssetManager.h"
#include "QtHttpAssetProvider.h"
#include "LocalAssetProvider.h"
#include "NetworkEvents.h"
#include "Framework.h"
#include "Profiler.h"
#include "EventManager.h"
#include "ServiceManager.h"
#include "CoreException.h"
#include "AssetAPI.h"

#include <QDir>

#include "Interfaces/ProtocolModuleInterface.h"

namespace Asset
{
    std::string AssetModule::type_name_static_ = "Asset";

    AssetModule::AssetModule() : IModule(type_name_static_), inboundcategory_id_(0)
    {
    }

    AssetModule::~AssetModule()
    {
    }

    // virtual
    void AssetModule::Load()
    {
    }

    // virtual
    void AssetModule::Initialize()
    {
        manager_ = AssetManagerPtr(new AssetManager(framework_));
        framework_->GetServiceManager()->RegisterService(Service::ST_Asset, manager_);

        // Add HTTP handler before UDP so it can handle the texture http gets with UUID via GetTexture caps url
        http_asset_provider_ = AssetProviderPtr(new QtHttpAssetProvider(framework_));
        manager_->RegisterAssetProvider(http_asset_provider_);

        // Add localassethandler, with a hardcoded dir for now
        // Note: this directory is a different concept than the "pre-warmed assetcache"
        boost::shared_ptr<LocalAssetProvider> local = boost::shared_ptr<LocalAssetProvider>(new LocalAssetProvider(framework_));
        local_asset_provider_ = boost::dynamic_pointer_cast<IAssetProvider>(local);

        QDir dir((GuaranteeTrailingSlash(GetFramework()->GetPlatform()->GetInstallDirectory().c_str()) + "data/assets").toStdString().c_str());
        local->AddStorageDirectory(dir.absolutePath().toStdString(), "System", true);

        dir = QDir((GuaranteeTrailingSlash(GetFramework()->GetPlatform()->GetInstallDirectory().c_str()) + "jsmodules").toStdString().c_str());
        local->AddStorageDirectory(dir.absolutePath().toStdString(), "Javascript", true);

        dir = QDir((GuaranteeTrailingSlash(GetFramework()->GetPlatform()->GetInstallDirectory().c_str()) + "media").toStdString().c_str());
        local->AddStorageDirectory(dir.absolutePath().toStdString(), "Ogre Media", true);

        manager_->RegisterAssetProvider(local_asset_provider_);

        framework_category_id_ = framework_->GetEventManager()->QueryEventCategory("Framework");
    }

    void AssetModule::PostInitialize()
    {
        RegisterConsoleCommand(Console::CreateCommand(
            "RequestAsset", "Request asset from server. Usage: RequestAsset(uuid,assettype)", 
            Console::Bind(this, &AssetModule::ConsoleRequestAsset)));

        ProcessCommandLineOptions();
    }

    void AssetModule::ProcessCommandLineOptions()
    {
        assert(framework_);

        const boost::program_options::variables_map &options = framework_->ProgramOptions();

        if (options.count("file") > 0)
        {
            std::string startup_scene_ = QString(options["file"].as<std::string>().c_str()).trimmed().toStdString();
            if (!startup_scene_.empty())
            {
                // If scene name is expressed as a full path, add it as a recursive asset source for localassetprovider
                boost::filesystem::path scenepath(startup_scene_);
                std::string dirname = scenepath.branch_path().string();
                if (!dirname.empty())
                    boost::dynamic_pointer_cast<LocalAssetProvider>(local_asset_provider_)->AddStorageDirectory(dirname, "Scene Local", true);
            }
        }

        if (options.count("storage") > 0)
        {
            std::string startup_scene_ = QString(options["storage"].as<std::string>().c_str()).trimmed().toStdString();
            if (!startup_scene_.empty())
            {
                // If scene name is expressed as a full path, add it as a recursive asset source for localassetprovider
                boost::filesystem::path scenepath(startup_scene_);  
                std::string dirname = scenepath.branch_path().string();
                if (!dirname.empty())
                    boost::dynamic_pointer_cast<LocalAssetProvider>(local_asset_provider_)->AddStorageDirectory(dirname, "Scene Local", true);
            }
        }
    }

    void AssetModule::SubscribeToNetworkEvents(boost::weak_ptr<ProtocolUtilities::ProtocolModuleInterface> currentProtocolModule)
    {
        protocolModule_ = currentProtocolModule;
        network_state_category_id_ = framework_->GetEventManager()->QueryEventCategory("NetworkState");
        inboundcategory_id_ = framework_->GetEventManager()->QueryEventCategory("NetworkIn");
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
        manager_->UnregisterAssetProvider(local_asset_provider_);
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
        IEventData* data)
    {
        PROFILE(AssetModule_HandleEvent);
        if (category_id == framework_category_id_ && event_id == Foundation::NETWORKING_REGISTERED)
        {
            ProtocolUtilities::NetworkingRegisteredEvent *event_data = dynamic_cast<ProtocolUtilities::NetworkingRegisteredEvent *>(data);
            if (event_data)
                SubscribeToNetworkEvents(event_data->currentProtocolModule);
            return false;
        }
        if (category_id == network_state_category_id_ && event_id == ProtocolUtilities::Events::EVENT_SERVER_DISCONNECTED)
        {
            if (http_asset_provider_)
                checked_static_cast<QtHttpAssetProvider*>(http_asset_provider_.get())->ClearAllTransfers();
        }
        if (category_id == network_state_category_id_ && event_id == ProtocolUtilities::Events::EVENT_CAPS_FETCHED)
        {
            if (protocolModule_.lock().get() && http_asset_provider_)
            {
                std::string get_texture_cap = protocolModule_.lock()->GetCapability("GetTexture");
                checked_static_cast<QtHttpAssetProvider*>(http_asset_provider_.get())->SetGetTextureCap(get_texture_cap);
            }
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

POCO_BEGIN_MANIFEST(IModule)
    POCO_EXPORT_CLASS(AssetModule)
POCO_END_MANIFEST

