// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "IModule.h"
#include "TundraLogicModuleApi.h"
#include "AssetFwd.h"

namespace kNet
{
    class MessageConnection;
    typedef unsigned long message_id_t;
}

namespace KristalliProtocol
{
    class KristalliProtocolModule;
}

namespace TundraLogic
{

class Client;
class Server;
class SyncManager;

class TUNDRALOGIC_MODULE_API TundraLogicModule : public IModule
{
    Q_OBJECT

public:
    TundraLogicModule();
    ~TundraLogicModule();

    void PreInitialize();
    void Load();
    void Initialize();
    void PostInitialize();
    void Uninitialize();
    void Update(f64 frametime);

    /// Check whether we are a server
    bool IsServer() const;

    /// Return pointer to KristalliProtocolModule for convenience
    KristalliProtocol::KristalliProtocolModule *GetKristalliModule() const { return kristalliModule_; }

    /// Return syncmanager
    const boost::shared_ptr<SyncManager>& GetSyncManager() const { return syncManager_; }

    /// Return client
    const boost::shared_ptr<Client>& GetClient() const { return client_; }

    /// Return server
    const boost::shared_ptr<Server>& GetServer() const { return server_; }

public slots:
    /// Starts the server.
    /** @param port Port.
        @todo Add protocol option */
    void StartServer(int port);

    /// Stops the server.
    void StopServer();

    /// Connects to server.
    void Connect(QString address, int port, QString username, QString password);

    /// Disconnects from server.
    void Disconnect();

    /// Saves scene to an XML file
    /// @param asBinary If true, saves as .tbin. Otherwise saves as .txml.
    void SaveScene(QString filename, bool asBinary = false, bool saveTemporaryEntities = false, bool saveLocalEntities = true);

    /// Loads scene from an XML file.
    void LoadScene(QString filename, bool clearScene = true, bool useEntityIDsFromFile = true);

    /// Imports a dotscene.
    void ImportScene(QString filename, bool clearScene = true, bool replace = true);

    /// Imports one mesh as a new entity.
    void ImportMesh(QString filename, float tx = 0.f, float ty = 0.f, float tz = 0.f, float rx = 0.f, float ry = 0.f,
        float rz = 0.f, float sx = 1.f, float sy = 1.f, float sz = 1.f, bool inspect = true);

private slots:
    void StartupSceneLoaded(AssetPtr asset);
    void StartupSceneTransferFailed(IAssetTransfer *transfer, QString reason);

private:
    /// Handle a Kristalli protocol message
    void HandleKristalliMessage(kNet::MessageConnection* source, kNet::message_id_t id, const char* data, size_t numBytes);

    /// Load the startup scene
    void LoadStartupScene();

    boost::shared_ptr<SyncManager> syncManager_; ///< Sync manager
    boost::shared_ptr<Client> client_; ///< Client
    boost::shared_ptr<Server> server_; ///< Server
    KristalliProtocol::KristalliProtocolModule *kristalliModule_; ///< KristalliProtocolModule pointer
    bool autostartserver_; ///< Whether to autostart the server
    short autostartserver_port_; ///< Autostart server port
};

}

