// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MemoryLeakCheck.h"
#include "AssetModule.h"
#include "LocalAssetProvider.h"
#include "HttpAssetProvider.h"
#include "HttpAssetStorage.h"
#include "Framework.h"
#include "Profiler.h"
#include "EventManager.h"
#include "ServiceManager.h"
#include "CoreException.h"
#include "AssetAPI.h"
#include "LocalAssetStorage.h"
#include "ConsoleAPI.h"

#include "TundraLogicModule.h"
#include "Client.h"
#include "Server.h"
#include "UserConnectedResponseData.h"
#include "UserConnection.h"

#include "kNetBuildConfig.h"
#include "kNet/MessageConnection.h"

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
        local->AddStorageDirectory(dir.absolutePath(), "System", true);
        // Set asset dir as also as AssetAPI property
        framework_->Asset()->setProperty("assetdir", QVariant(GuaranteeTrailingSlash(dir.absolutePath())));
        framework_->Asset()->setProperty("inbuiltassetdir", QVariant(GuaranteeTrailingSlash(dir.absolutePath())));
        
        dir = QDir((GuaranteeTrailingSlash(GetFramework()->GetPlatform()->GetInstallDirectory().c_str()) + "jsmodules").toStdString().c_str());
        local->AddStorageDirectory(dir.absolutePath(), "Javascript", true);

        dir = QDir((GuaranteeTrailingSlash(GetFramework()->GetPlatform()->GetInstallDirectory().c_str()) + "media").toStdString().c_str());
        local->AddStorageDirectory(dir.absolutePath(), "Ogre Media", true);

        framework_->RegisterDynamicObject("assetModule", this);
    }

    static const std::string addAssetStorageHelp = "Usage: AddAssetStorage(storage string). For example: AddAssetStorage(name=MyAssets;type=HttpAssetStorage;src=http://www.myserver.com/;default;)";

    void AssetModule::PostInitialize()
    {
        framework_->Console()->RegisterCommand(CreateConsoleCommand(
            "RequestAsset", "Request asset from server. Usage: RequestAsset(uuid,assettype)", 
            ConsoleBind(this, &AssetModule::ConsoleRequestAsset)));

        framework_->Console()->RegisterCommand(CreateConsoleCommand(
            "AddAssetStorage", addAssetStorageHelp, 
            ConsoleBind(this, &AssetModule::AddAssetStorage)));

        framework_->Console()->RegisterCommand(CreateConsoleCommand(
            "ListAssetStorages", "Serializes all currently registered asset storages to the console output log.", 
            ConsoleBind(this, &AssetModule::ListAssetStorages)));

        framework_->Console()->RegisterCommand(CreateConsoleCommand(
            "RefreshHttpStorages", "Refreshes known assetrefs for all http asset storages", 
            ConsoleBind(this, &AssetModule::ConsoleRefreshHttpStorages)));
            
        ProcessCommandLineOptions();

        TundraLogic::Server *server = framework_->GetModule<TundraLogic::TundraLogicModule>()->GetServer().get();
        QObject::connect(server, SIGNAL(UserConnected(int, UserConnection *, UserConnectedResponseData *)), this, 
            SLOT(ServerNewUserConnected(int, UserConnection *, UserConnectedResponseData *)));

        TundraLogic::Client *client = framework_->GetModule<TundraLogic::TundraLogicModule>()->GetClient().get();
        QObject::connect(client, SIGNAL(Connected(UserConnectedResponseData *)), this, SLOT(ClientConnectedToServer(UserConnectedResponseData *)));
        QObject::connect(client, SIGNAL(Disconnected()), this, SLOT(ClientDisconnectedFromServer()));
    }

    void AssetModule::ProcessCommandLineOptions()
    {
        assert(framework_);

        const boost::program_options::variables_map &options = framework_->ProgramOptions();

        if (options.count("file") > 0)
        {
            AssetStoragePtr storage = framework_->Asset()->DeserializeAssetStorageFromString(QString(options["file"].as<std::string>().c_str()).trimmed());
            framework_->Asset()->SetDefaultAssetStorage(storage);
        }
        if (options.count("storage") > 0)
        {
            AssetStoragePtr storage = framework_->Asset()->DeserializeAssetStorageFromString(QString(options["storage"].as<std::string>().c_str()).trimmed());
            if (options.count("file") == 0) // If "--file" was not specified, then use "--storage" as the default. (If both are specified, "--file" takes precedence over "--storage").
                framework_->Asset()->SetDefaultAssetStorage(storage);
        }
    }

    ConsoleCommandResult AssetModule::ConsoleRefreshHttpStorages(const StringVector &params)
    {
        RefreshHttpStorages();
        return ConsoleResultSuccess();
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

    ConsoleCommandResult AssetModule::AddAssetStorage(const StringVector &params)
    {
        if (params.size() != 1)
            return ConsoleResultFailure("Invalid number of parameters! " + addAssetStorageHelp);
        
        AssetStoragePtr storage = framework_->Asset()->DeserializeAssetStorageFromString(params[0].c_str());
        if (storage)
            return ConsoleResultSuccess();
        else
            return ConsoleResultFailure();
    }

    ConsoleCommandResult AssetModule::ListAssetStorages(const StringVector &params)
    {
        AssetStorageVector storages = framework_->Asset()->GetAssetStorages();
        LogInfo("Registered storages: ");
        foreach(AssetStoragePtr storage, storages)
        {
            QString storageString = storage->SerializeToString();
            if (framework_->Asset()->GetDefaultAssetStorage() == storage)
                storageString += ";default";
            LogInfo(storageString.toStdString());
        }

        return ConsoleResultSuccess();
    }

    void AssetModule::LoadAllLocalAssetsWithSuffix(const QString &suffix, const QString &assetType)
    {
        std::vector<AssetStoragePtr> storages = framework_->Asset()->GetAssetStorages();
        for(size_t i = 0; i < storages.size(); ++i)
        {
            LocalAssetStorage *storage = dynamic_cast<LocalAssetStorage*>(storages[i].get());
            if (storage)
                storage->LoadAllAssetsOfType(framework_->Asset(), suffix, assetType);
        }
    }
    
    void AssetModule::RefreshHttpStorages()
    {
        std::vector<AssetStoragePtr> storages = framework_->Asset()->GetAssetStorages();
        for(size_t i = 0; i < storages.size(); ++i)
        {
            HttpAssetStorage *storage = dynamic_cast<HttpAssetStorage*>(storages[i].get());
            if (storage)
                storage->RefreshAssetRefs();
        }
    }

    /// If we are the server, this function gets called whenever a new connection is received. Populates the response data with the known asset storages in this server.
    void AssetModule::ServerNewUserConnected(int connectionID, UserConnection *connection, UserConnectedResponseData *responseData)
    {
        QDomDocument &doc = responseData->responseData;
        QDomElement assetRoot = doc.createElement("asset");
        doc.appendChild(assetRoot);
        
        bool isLocalhostConnection = (connection->connection->RemoteEndPoint().IPToString() == "127.0.0.1" || 
            connection->connection->LocalEndPoint().IPToString() == connection->connection->RemoteEndPoint().IPToString());

        std::vector<AssetStoragePtr> storages = framework_->Asset()->GetAssetStorages();
        for(size_t i = 0; i < storages.size(); ++i)
        {
            bool isLocalStorage = (dynamic_cast<LocalAssetStorage*>(storages[i].get()) != 0);
            if (!isLocalStorage || isLocalhostConnection)
            {
                QDomElement storage = doc.createElement("storage");
                storage.setAttribute("data", storages[i]->SerializeToString());
                assetRoot.appendChild(storage);
            }
        }
        AssetStoragePtr defaultStorage = framework_->Asset()->GetDefaultAssetStorage();
        bool defaultStorageIsLocal = (dynamic_cast<LocalAssetStorage*>(defaultStorage.get()) != 0);
        if (defaultStorage && (!defaultStorageIsLocal || isLocalhostConnection))
        {
            QDomElement storage = doc.createElement("defaultStorage");
            storage.setAttribute("name", defaultStorage->Name());
            assetRoot.appendChild(storage);
        }
    }

    /// If we are the client, this function gets called when we connect to a server.
    void AssetModule::ClientConnectedToServer(UserConnectedResponseData *responseData)
    {
        QDomDocument &doc = responseData->responseData;
        QDomElement assetRoot = doc.firstChildElement("asset");
        if (!assetRoot.isNull())
        {
            for (QDomElement storage = assetRoot.firstChildElement("storage"); !storage.isNull(); 
                storage = storage.nextSiblingElement("storage"))
            {
                QString storageData = storage.attribute("data");
                AssetStoragePtr assetStorage = framework_->Asset()->DeserializeAssetStorageFromString(storageData);

                // Remember that this storage was received from the server, so we can later stop using it when we disconnect (and possibly reconnect to another server).
                if (assetStorage)
                    storagesReceivedFromServer.push_back(assetStorage);
            }

            QDomElement defaultStorage = assetRoot.firstChildElement("defaultStorage");
            if (!defaultStorage.isNull())
            {
                QString defaultStorageName = defaultStorage.attribute("name");
                AssetStoragePtr defaultStoragePtr = framework_->Asset()->GetAssetStorage(defaultStorageName);
                if (defaultStoragePtr)
                    framework_->Asset()->SetDefaultAssetStorage(defaultStoragePtr);
            }
        }
    }

    void AssetModule::ClientDisconnectedFromServer()
    {
        for(size_t i = 0; i < storagesReceivedFromServer.size(); ++i)
        {
            AssetStoragePtr storage = storagesReceivedFromServer[i].lock();
            if (storage)
                framework_->Asset()->RemoveAssetStorage(storage->Name());
        }
        storagesReceivedFromServer.clear();
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

