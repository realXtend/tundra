// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AssetModule.h"
#include "LocalAssetProvider.h"
#include "HttpAssetProvider.h"
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

    void AssetModule::Initialize()
    {
        boost::shared_ptr<HttpAssetProvider> http = boost::shared_ptr<HttpAssetProvider>(new HttpAssetProvider(framework_));
        framework_->Asset()->RegisterAssetProvider(boost::dynamic_pointer_cast<IAssetProvider>(http));

        boost::shared_ptr<LocalAssetProvider> local = boost::shared_ptr<LocalAssetProvider>(new LocalAssetProvider(framework_));
        framework_->Asset()->RegisterAssetProvider(boost::dynamic_pointer_cast<IAssetProvider>(local));

        QDir dir((GuaranteeTrailingSlash(GetFramework()->GetPlatform()->GetInstallDirectory().c_str()) + "data/assets").toStdString().c_str());
        local->AddStorageDirectory(dir.absolutePath().toStdString(), "System", true);
        // Set asset dir as also as AssetAPI property
        framework_->Asset()->setProperty("assetdir", QVariant(GuaranteeTrailingSlash(dir.absolutePath())));
        framework_->Asset()->setProperty("inbuiltassetdir", QVariant(GuaranteeTrailingSlash(dir.absolutePath())));
        
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

        RegisterConsoleCommand(Console::CreateCommand(
            "AddHttpStorage", "Adds a new Http asset storage to the known storages. Usage: AddHttpStorage(url, name)", 
            Console::Bind(this, &AssetModule::AddHttpStorage)));

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
                {
                    framework_->Asset()->GetAssetProvider<LocalAssetProvider>()->AddStorageDirectory(dirname, "Scene", true);
                    framework_->Asset()->SetDefaultAssetStorage(framework_->Asset()->GetAssetStorage("Scene"));
                    
                    // Set asset dir as also as AssetAPI property
                    framework_->Asset()->setProperty("assetdir", QVariant(GuaranteeTrailingSlash(QString::fromStdString(dirname))));
                }
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
                {
                    framework_->Asset()->GetAssetProvider<LocalAssetProvider>()->AddStorageDirectory(dirname, "Scene", true);
                    framework_->Asset()->SetDefaultAssetStorage(framework_->Asset()->GetAssetStorage("Scene"));
                    
                    // Set asset dir as also as AssetAPI property
                    framework_->Asset()->setProperty("assetdir", QVariant(GuaranteeTrailingSlash(QString::fromStdString(dirname))));
                }
            }
        }
    }

    Console::CommandResult AssetModule::ConsoleRequestAsset(const StringVector &params)
    {
        if (params.size() != 2)
            return Console::ResultFailure("Usage: RequestAsset(uuid,assettype)");

        AssetTransferPtr transfer = framework_->Asset()->RequestAsset(params[0].c_str(), params[1].c_str());
        if (transfer)
            return Console::ResultSuccess();
        else
            return Console::ResultFailure();
    }

    Console::CommandResult AssetModule::AddHttpStorage(const StringVector &params)
    {
        if (params.size() != 2)
            return Console::ResultFailure("Usage: AddHttpStorage(url, name). For example: AddHttpStorage(http://www.google.com/, google)");

        if (!framework_->Asset()->GetAssetProvider<HttpAssetProvider>())
            return Console::ResultFailure();

        framework_->Asset()->GetAssetProvider<HttpAssetProvider>()->AddStorageAddress(params[0].c_str(), params[1].c_str());
        framework_->Asset()->SetDefaultAssetStorage(framework_->Asset()->GetAssetStorage(params[1].c_str()));
       
        return Console::ResultSuccess();
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

