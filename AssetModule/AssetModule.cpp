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
#include "ServiceManager.h"
#include "CoreException.h"
#include "AssetAPI.h"
#include "LocalAssetStorage.h"
#include "ConsoleAPI.h"
#include "Application.h"

#include "TundraLogicModule.h"
#include "Client.h"
#include "Server.h"
#include "UserConnectedResponseData.h"
#include "UserConnection.h"

#include "kNet/MessageConnection.h"

#include <QDir>

namespace Asset
{
    AssetModule::AssetModule()
    :IModule("Asset")
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
        local->AddStorageDirectory(systemAssetDir, "System", true);
        // Set asset dir as also as AssetAPI property
        framework_->Asset()->setProperty("assetdir", systemAssetDir);
        framework_->Asset()->setProperty("inbuiltassetdir", systemAssetDir);
        
        QString jsAssetDir = Application::InstallationDirectory() + "jsmodules";
        local->AddStorageDirectory(jsAssetDir, "Javascript", true);

        QString ogreAssetDir = Application::InstallationDirectory() + "media";
        local->AddStorageDirectory(ogreAssetDir, "Ogre Media", true);

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

    void AssetModule::AddStorageDirectory(const QString &storageDir)
    {
        QString path;
        QString protocolPath;
        AssetAPI::AssetRefType refType = AssetAPI::ParseAssetRef(storageDir, 0, 0, &protocolPath, 0, 0, &path);

        if (refType == AssetAPI::AssetRefRelativePath)
        {
            path = GuaranteeTrailingSlash(QDir::currentPath()) + path;
            refType = AssetAPI::AssetRefLocalPath;
        }

        AssetStoragePtr storage;

        if (refType == AssetAPI::AssetRefLocalPath)
            storage = framework_->Asset()->GetAssetProvider<LocalAssetProvider>()->AddStorageDirectory(path, "Scene", true);
        else if (refType == AssetAPI::AssetRefExternalUrl)
        {
            storage = framework_->Asset()->GetAssetProvider<HttpAssetProvider>()->AddStorageAddress(protocolPath, "Scene");
            path = protocolPath;
        }
        else
            return; ///\todo Log error.

        framework_->Asset()->SetDefaultAssetStorage(storage);

        // Set asset dir as also as AssetAPI property
        framework_->Asset()->setProperty("assetdir", QVariant(path));
    }

    void AssetModule::ProcessCommandLineOptions()
    {
        assert(framework_);

        const boost::program_options::variables_map &options = framework_->ProgramOptions();

        if (options.count("file") > 0)
            AddStorageDirectory(QString(options["file"].as<std::string>().c_str()).trimmed());
        if (options.count("storage") > 0)
            AddStorageDirectory(QString(options["storage"].as<std::string>().c_str()).trimmed());
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

    ConsoleCommandResult AssetModule::AddHttpStorage(const StringVector &params)
    {
        if (params.size() != 2)
            return ConsoleResultFailure("Usage: AddHttpStorage(url, name). For example: AddHttpStorage(http://www.google.com/, google)");

        if (!framework_->Asset()->GetAssetProvider<HttpAssetProvider>())
            return ConsoleResultFailure();

        HttpAssetStoragePtr storage = framework_->Asset()->GetAssetProvider<HttpAssetProvider>()->AddStorageAddress(params[0].c_str(), params[1].c_str());
        framework_->Asset()->SetDefaultAssetStorage(storage);
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

void SetProfiler(Profiler *profiler)
{
    ProfilerSection::SetProfiler(profiler);
}

using namespace Asset;

extern "C"
{
__declspec(dllexport) void TundraPluginMain(Framework *fw)
{
    IModule *module = new Asset::AssetModule();
    fw->GetModuleManager()->DeclareStaticModule(module);
}
}
