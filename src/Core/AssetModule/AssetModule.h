// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "IModule.h"

#include "IAssetProvider.h"
#include "AssetModuleApi.h"
#include "kNetFwd.h"
#include "kNet/Types.h"
#include "TundraProtocolModuleFwd.h"

/// Implements asset providers and storages for local disk assets and HTTP assets.
class ASSET_MODULE_API AssetModule : public IModule
{
    Q_OBJECT

public:
    AssetModule();
    virtual ~AssetModule();

    virtual void Initialize();

public slots:
    void ConsoleRequestAsset(const QString &assetRef, const QString &assetType);

    /// Adds a new storage to the system.
    /// \important Call this function only for locally originating storages! (Not ones deserialized from network)
    void AddAssetStorage(const QString &storageString);

    void ListAssetStorages();

    void ConsoleRefreshHttpStorages();

    void ConsoleDumpAssetTransfers();

    void ConsoleDumpAssets();

    /// Loads from all the registered local storages all assets that have the given suffix.
    /// Type can also be optionally specified
    /// \todo Will be replaced with AssetStorage's GetAllAssetsRefs / GetAllAssets functionality
    void LoadAllLocalAssetsWithSuffix(const QString &suffix, const QString &assetType = "");
    
    /// Refreshes asset refs of all http storages
    void RefreshHttpStorages();
    
    /// If we are the server, this function gets called whenever a new connection is received. Populates the response data with the known asset storages in this server.
    void ServerNewUserConnected(unsigned int connectionID, UserConnection *connection, UserConnectedResponseData *responseData);
    /// If we are the client, this function gets called when we connect to a server. Adds all storages received from the server to our storage list.
    void ClientConnectedToServer(UserConnectedResponseData *responseData);
    /// If we are the client, this function gets called when we disconnected. Removes all storages received from the server from our storage list.
    void ClientDisconnectedFromServer();

private slots:
    /// Handles a Kristalli protocol message. Used for AssetDiscovery & AssetDeleted messages
    void HandleKristalliMessage(kNet::MessageConnection* source, kNet::packet_id_t packetId, kNet::message_id_t messageId, const char* data, size_t numBytes);
    
    /// Handle incoming asset discovery message.
    void HandleAssetDiscovery(kNet::MessageConnection* source, MsgAssetDiscovery& msg);
    
    /// Handle incoming asset deleted message.
    void HandleAssetDeleted(kNet::MessageConnection* source, MsgAssetDeleted& msg);
    
    /// Asset uploaded. Send AssetDiscovery network message
    void OnAssetUploaded(const QString& assetRef);

    /// Asset deleted from a storage. Send AssetDeleted network message
    void OnAssetDeleted(const QString& assetRef);
    
private:
    void ProcessCommandLineOptions();

    /// Check from an assetref whether it should be replicated when a modify or a delete to it is detected.
    bool ShouldReplicateAssetDiscovery(const QString& assetRef);

    /// Whenever we receive a new asset storage from the server, this function is called to determine if the storage is to be trusted.
    void DetermineStorageTrustStatus(AssetStoragePtr storage);

    /// When the client connects to the server, it adds to its list of known storages all the storages on the server side.
    /// To be able to also remove these storages from being used after we disconnect, we track all the server-originated storages here.
    std::vector<AssetStorageWeakPtr> storagesReceivedFromServer;
};
