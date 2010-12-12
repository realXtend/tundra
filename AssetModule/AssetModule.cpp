// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AssetModule.h"
#include "LocalAssetProvider.h"
#include "Framework.h"
#include "Profiler.h"
#include "EventManager.h"
#include "ServiceManager.h"
#include "CoreException.h"
#include "AssetAPI.h"

#include <QDir>

namespace Asset
{
    std::string AssetModule::type_name_static_ = "Asset";

    AssetModule::AssetModule() : IModule(type_name_static_)
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
        /*
        // Add HTTP handler before UDP so it can handle the texture http gets with UUID via GetTexture caps url
        http_asset_provider_ = AssetProviderPtr(new QtHttpAssetProvider(framework_));
        manager_->RegisterAssetProvider(http_asset_provider_);
        */

        // Add localassethandler, with a hardcoded dir for now
        // Note: this directory is a different concept than the "pre-warmed assetcache"
        boost::shared_ptr<LocalAssetProvider> local = boost::shared_ptr<LocalAssetProvider>(new LocalAssetProvider(framework_));
        framework_->Asset()->RegisterAssetProvider(boost::dynamic_pointer_cast<IAssetProvider>(local));

        QDir dir((GuaranteeTrailingSlash(GetFramework()->GetPlatform()->GetInstallDirectory().c_str()) + "data/assets").toStdString().c_str());
        local->AddStorageDirectory(dir.absolutePath().toStdString(), "System", true);

        dir = QDir((GuaranteeTrailingSlash(GetFramework()->GetPlatform()->GetInstallDirectory().c_str()) + "jsmodules").toStdString().c_str());
        local->AddStorageDirectory(dir.absolutePath().toStdString(), "Javascript", true);

        dir = QDir((GuaranteeTrailingSlash(GetFramework()->GetPlatform()->GetInstallDirectory().c_str()) + "media").toStdString().c_str());
        local->AddStorageDirectory(dir.absolutePath().toStdString(), "Ogre Media", true);
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
                    framework_->Asset()->GetAssetProvider<LocalAssetProvider>()->AddStorageDirectory(dirname, "Scene Local", true);
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
                    framework_->Asset()->GetAssetProvider<LocalAssetProvider>()->AddStorageDirectory(dirname, "Scene Local", true);
            }
        }
    }

    Console::CommandResult AssetModule::ConsoleRequestAsset(const StringVector &params)
    {
        if (params.size() != 2)
            return Console::ResultFailure("Usage: RequestAsset(uuid,assettype)");

        AssetTransferPtr transfer = framework_->Asset()->RequestAsset(params[0].c_str(), params[1].c_str());
        if (transfer.get())
            return Console::ResultSuccess();
        else
            return Console::ResultFailure();
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

