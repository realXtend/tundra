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

#include "KristalliProtocolModule.h"
#include "TundraLogicModule.h"
#include "TundraMessages.h"
#include "Client.h"
#include "Server.h"
#include "UserConnectedResponseData.h"
#include "UserConnection.h"
#include "MsgAssetDeleted.h"
#include "MsgAssetDiscovery.h"

#include "kNetBuildConfig.h"
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
        
        KristalliProtocol::KristalliProtocolModule *kristalli = framework_->GetModule<KristalliProtocol::KristalliProtocolModule>();
        connect(kristalli, SIGNAL(NetworkMessageReceived(kNet::MessageConnection *, kNet::message_id_t, const char *, size_t)), 
            this, SLOT(HandleKristalliMessage(kNet::MessageConnection*, kNet::message_id_t, const char*, size_t)), Qt::UniqueConnection);

        // Connect to asset uploads & deletions from storage to be able to broadcast asset discovery & deletion messages
        connect(framework_->Asset(), SIGNAL(AssetUploaded(const QString &)), this, SLOT(OnAssetUploaded(const QString &)));
        connect(framework_->Asset(), SIGNAL(AssetDeletedFromStorage(const QString&)), this, SLOT(OnAssetDeleted(const QString&)));
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
                AssetStoragePtr defaultStoragePtr = framework_->Asset()->GetAssetStorageByName(defaultStorageName);
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

    void AssetModule::HandleKristalliMessage(kNet::MessageConnection* source, kNet::message_id_t id, const char* data, size_t numBytes)
    {
        switch (id)
        {
        case cAssetDiscoveryMessage:
            {
                MsgAssetDiscovery msg(data, numBytes);
                HandleAssetDiscovery(source, msg);
            }
            break;
        case cAssetDeletedMessage:
            {
                MsgAssetDeleted msg(data, numBytes);
                HandleAssetDeleted(source, msg);
            }
            break;
        }
    }

    void AssetModule::HandleAssetDiscovery(kNet::MessageConnection* source, MsgAssetDiscovery& msg)
    {
        QString assetRef = QString::fromStdString(BufferToString(msg.assetRef));
        QString assetType = QString::fromStdString(BufferToString(msg.assetType));
        
        // Check for possible malicious discovery message and ignore it. Otherwise let AssetAPI handle
        if (!framework_->Asset()->ShouldReplicateAssetDiscovery(assetRef))
            return;
        
        // If we are server, the message had to come from a client, and we replicate it to everyone except the sender
        TundraLogic::TundraLogicModule* tundra = framework_->GetModule<TundraLogic::TundraLogicModule>();
        KristalliProtocol::KristalliProtocolModule *kristalli = framework_->GetModule<KristalliProtocol::KristalliProtocolModule>();
        bool isServer = tundra->IsServer();
        
        if (isServer)
        {
            foreach(UserConnection* userConn, kristalli->GetUserConnections())
            {
                if (userConn->connection != source)
                    userConn->connection->Send(msg);
            }
        }
        
        // Then let assetAPI handle locally
        framework_->Asset()->HandleAssetDiscovery(assetRef, assetType);
    }

    void AssetModule::HandleAssetDeleted(kNet::MessageConnection* source, MsgAssetDeleted& msg)
    {
        QString assetRef = QString::fromStdString(BufferToString(msg.assetRef));
        
        // Check for possible malicious delete message and ignore it. Otherwise let AssetAPI handle
        if (!framework_->Asset()->ShouldReplicateAssetDiscovery(assetRef))
            return;
        
        // If we are server, the message had to come from a client, and we replicate it to everyone except the sender
        TundraLogic::TundraLogicModule* tundra = framework_->GetModule<TundraLogic::TundraLogicModule>();
        KristalliProtocol::KristalliProtocolModule *kristalli = framework_->GetModule<KristalliProtocol::KristalliProtocolModule>();
        bool isServer = tundra->IsServer();
        
        if (isServer)
        {
            foreach(UserConnection* userConn, kristalli->GetUserConnections())
            {
                if (userConn->connection != source)
                    userConn->connection->Send(msg);
            }
        }
        
        // Then let assetAPI handle locally
        framework_->Asset()->HandleAssetDeleted(assetRef);
    }

    void AssetModule::OnAssetUploaded(const QString& assetRef)
    {
        // Check whether the asset upload needs to be replicated
        if (!framework_->Asset()->ShouldReplicateAssetDiscovery(assetRef))
            return;
        
        TundraLogic::TundraLogicModule* tundra = framework_->GetModule<TundraLogic::TundraLogicModule>();
        KristalliProtocol::KristalliProtocolModule *kristalli = framework_->GetModule<KristalliProtocol::KristalliProtocolModule>();
        
        bool isServer = tundra->IsServer();
        
        MsgAssetDiscovery msg;
        msg.assetRef = StringToBuffer(assetRef.toStdString());
        /// \todo Would preferably need the assettype as well
        
        // If we are server, send to everyone
        if (isServer)
        {
            foreach(UserConnection* userConn, kristalli->GetUserConnections())
                userConn->connection->Send(msg);
        }
        // If we are client, send to server
        else
        {
            kNet::MessageConnection* connection = tundra->GetClient()->GetConnection();
            if (connection)
                connection->Send(msg);
        }
    }

    void AssetModule::OnAssetDeleted(const QString& assetRef)
    {
        // Check whether the asset delete needs to be replicated
        if (!framework_->Asset()->ShouldReplicateAssetDiscovery(assetRef))
            return;
        
        TundraLogic::TundraLogicModule* tundra = framework_->GetModule<TundraLogic::TundraLogicModule>();
        KristalliProtocol::KristalliProtocolModule *kristalli = framework_->GetModule<KristalliProtocol::KristalliProtocolModule>();
        
        bool isServer = tundra->IsServer();
        
        MsgAssetDeleted msg;
        msg.assetRef = StringToBuffer(assetRef.toStdString());
        
        // If we are server, send to everyone
        if (isServer)
        {
            foreach(UserConnection* userConn, kristalli->GetUserConnections())
                userConn->connection->Send(msg);
        }
        // If we are client, send to server
        else
        {
            kNet::MessageConnection* connection = tundra->GetClient()->GetConnection();
            if (connection)
                connection->Send(msg);
        }
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
