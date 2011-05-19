// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TundraLogicModule_TundraLogicModule_h
#define incl_TundraLogicModule_TundraLogicModule_h

#include "IModule.h"
#include "TundraLogicModuleApi.h"
#include "AssetFwd.h"

struct ConsoleCommandResult;

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

class TUNDRALOGIC_MODULE_API TundraLogicModule : public QObject, public IModule
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

    /// Starts the server (console command)
    ///\todo Add protocol choice, TCP or UDP.
    void ConsoleStartServer(int port);
    
    /// Stops the server (console command)
    void ConsoleStopServer();
    
    /// Connects to server (console command)
    void ConsoleConnect(QString address, int port, QString username, QString password);
    
    /// Disconnects from server (console command)
    void ConsoleDisconnect();

    /// Saves scene to an XML file
    /// @param asBinary If true, saves as .tbin. Otherwise saves as .txml.
    void ConsoleSaveScene(QString filename, bool asBinary = false, bool saveTemporaryEntities = false, bool saveLocalEntities = true);

    /// Loads scene from an XML file.
    void ConsoleLoadScene(QString filename, bool clearScene = true, bool useEntityIDsFromFile = true);
    
    /// Imports a dotscene
    void ConsoleImportScene(QString filename, bool clearScene = true, bool replace = true);
    
    /// Imports one mesh as a new entity
    void ConsoleImportMesh(QString filename, float tx = 0.f, float ty = 0.f, float tz = 0.f, float rx = 0.f, float ry = 0.f, float rz = 0.f, float sx = 1.f, float sy = 1.f, float sz = 1.f, bool inspect = true);

private slots:
    void StartupSceneLoaded(AssetPtr asset);
    void StartupSceneTransferFailed(IAssetTransfer *transfer, QString reason);

private:
    /// Handle a Kristalli protocol message
    void HandleKristalliMessage(kNet::MessageConnection* source, kNet::message_id_t id, const char* data, size_t numBytes);
    
    /// Load the startup scene
    void LoadStartupScene();
    
    /// Sync manager
    boost::shared_ptr<SyncManager> syncManager_;
    /// Client
    boost::shared_ptr<Client> client_;
    /// Server
    boost::shared_ptr<Server> server_;
        
    /// KristalliProtocolModule pointer
    KristalliProtocol::KristalliProtocolModule *kristalliModule_;
        
    /// Whether to autostart the server
    bool autostartserver_;
    /// Autostart server port
    short autostartserver_port_;
};

}

#endif
