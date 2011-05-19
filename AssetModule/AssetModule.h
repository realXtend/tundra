// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_AssetModule_h
#define incl_AssetModule_h

#include "IModule.h"

#include <QObject>

#include "IAssetProvider.h"
#include "AssetModuleApi.h"

struct MsgAssetDiscovery;
struct MsgAssetDeleted;
struct UserConnectedResponseData;
struct ConsoleCommandResult;
class UserConnection;

namespace kNet
{
    class MessageConnection;
    typedef unsigned long message_id_t;
}

namespace Asset
{
    /** \defgroup AssetModuleClient AssetModule Client Interface
        This page lists the public interface of the AssetModule,
        which consists of implementing AssetServiceInterface and
        AssetInterface

        For details on how to use the public interface, see \ref AssetModule "Using the asset module".
    */

    /// Asset module.
    class ASSET_MODULE_API AssetModule : public QObject, public IModule
    {
        Q_OBJECT

    public:
        AssetModule();
        virtual ~AssetModule();

        virtual void Initialize();
        virtual void PostInitialize();

    public slots:
        void ConsoleRequestAsset(const QString &assetRef, const QString &assetType);

        void AddAssetStorage(const QString &storageString);

        void ListAssetStorages();

        void ConsoleRefreshHttpStorages();

        /// Loads from all the registered local storages all assets that have the given suffix.
        /// Type can also be optionally specified
        /// \todo Will be replaced with AssetStorage's GetAllAssetsRefs / GetAllAssets functionality
        void LoadAllLocalAssetsWithSuffix(const QString &suffix, const QString &assetType = "");
        
        /// Refreshes asset refs of all http storages
        void RefreshHttpStorages();
        
        /// If we are the server, this function gets called whenever a new connection is received. Populates the response data with the known asset storages in this server.
        void ServerNewUserConnected(int connectionID, UserConnection *connection, UserConnectedResponseData *responseData);
        /// If we are the client, this function gets called when we connect to a server. Adds all storages received from the server to our storage list.
        void ClientConnectedToServer(UserConnectedResponseData *responseData);
        /// If we are the client, this function gets called when we disconnected. Removes all storages received from the server from our storage list.
        void ClientDisconnectedFromServer();

    private slots:
        /// Handles a Kristalli protocol message. Used for AssetDiscovery & AssetDeleted messages
        void HandleKristalliMessage(kNet::MessageConnection* source, kNet::message_id_t id, const char* data, size_t numBytes);
        
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

        /// When the client connects to the server, it adds to its list of known storages all the storages on the server side.
        /// To be able to also remove these storages from being used after we disconnect, we track all the server-originated storages here.
        std::vector<AssetStorageWeakPtr> storagesReceivedFromServer;
    };
}

#endif
