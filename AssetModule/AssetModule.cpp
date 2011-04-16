// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MemoryLeakCheck.h"
#include "AssetModule.h"
#include "LocalAssetProvider.h"
#include "HttpAssetProvider.h"
#include "Framework.h"
#include "Profiler.h"
#include "EventManager.h"
#include "ServiceManager.h"
#include "CoreException.h"
#include "AssetAPI.h"
#include "LocalAssetStorage.h"
#include "ConsoleAPI.h"
#include "Application.h"

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

        QString systemAssetDir = Application::InstallationDirectory() + "data/assets";
        local->AddStorageDirectory(systemAssetDir.toStdString(), "System", true);
        // Set asset dir as also as AssetAPI property
        framework_->Asset()->setProperty("assetdir", systemAssetDir);
        framework_->Asset()->setProperty("inbuiltassetdir", systemAssetDir);
        
        QString jsAssetDir = Application::InstallationDirectory() + "jsmodules";
        local->AddStorageDirectory(jsAssetDir.toStdString(), "Javascript", true);

        QString ogreAssetDir = Application::InstallationDirectory() + "media";
        local->AddStorageDirectory(ogreAssetDir.toStdString(), "Ogre Media", true);

        framework_->RegisterDynamicObject("assetModule", this);
    }

    void AssetModule::PostInitialize()
    {
        framework_->Console()->RegisterCommand(CreateConsoleCommand(
            "RequestAsset", "Request asset from server. Usage: RequestAsset(uuid,assettype)", 
            ConsoleBind(this, &AssetModule::ConsoleRequestAsset)));

        framework_->Console()->RegisterCommand(CreateConsoleCommand(
            "AddHttpStorage", "Adds a new Http asset storage to the known storages. Usage: AddHttpStorage(url, name)", 
            ConsoleBind(this, &AssetModule::AddHttpStorage)));

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

    ConsoleCommandResult AssetModule::ConsoleRequestAsset(const StringVector &params)
    {
        if (params.size() != 2)
            return ConsoleResultFailure("Usage: RequestAsset(uuid,assettype)");

        AssetTransferPtr transfer = framework_->Asset()->RequestAsset(params[0].c_str(), params[1].c_str());
        if (transfer)
            return ConsoleResultSuccess();
        else
            return ConsoleResultFailure();
    }

    ConsoleCommandResult AssetModule::AddHttpStorage(const StringVector &params)
    {
        if (params.size() != 2)
            return ConsoleResultFailure("Usage: AddHttpStorage(url, name). For example: AddHttpStorage(http://www.google.com/, google)");

        if (!framework_->Asset()->GetAssetProvider<HttpAssetProvider>())
            return ConsoleResultFailure();
        framework_->Asset()->AddAssetStorage(params[0].c_str(), params[1].c_str(), true);       
        return ConsoleResultSuccess();
    }

    void AssetModule::LoadAllLocalAssetsWithSuffix(const QString &suffix)
    {
        std::vector<AssetStoragePtr> storages = framework_->Asset()->GetAssetStorages();
        for(size_t i = 0; i < storages.size(); ++i)
        {
            LocalAssetStorage *storage = dynamic_cast<LocalAssetStorage*>(storages[i].get());
            if (storage)
                storage->LoadAllAssetsOfType(framework_->Asset(), suffix);
        }
    }
}

void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace Asset;

extern "C"
{
__declspec(dllexport) void TundraPluginMain(Foundation::Framework *fw)
{
    IModule *module = new Asset::AssetModule();
    fw->GetModuleManager()->DeclareStaticModule(module);
}
}
