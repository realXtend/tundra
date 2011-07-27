// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TundraLogicModule_TundraLogicModule_h
#define incl_TundraLogicModule_TundraLogicModule_h

#include "IModule.h"
#include "TundraLogicModuleApi.h"
#include "ModuleLoggingFunctions.h"

#include "AssetFwd.h"

#include <QMap>

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
    /// Default constructor.
    TundraLogicModule();

    /// Destructor.
    ~TundraLogicModule();

    /// IModule override.
    void PreInitialize();

    /// IModule override.
    void Initialize();

    /// IModule override.
    void PostInitialize();

    /// IModule override.
    void Uninitialize();

    /// IModule override.
    void Update(f64 frametime);

    /// IModule override.
    bool HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data);

    MODULE_LOGGING_FUNCTIONS

    /// Returns name of this module. Needed for logging.
    static const std::string &NameStatic() { return type_name_static_; }

    /// Starts a server (console command)
    ConsoleCommandResult ConsoleStartServer(const StringVector &params);
    
    /// Stops a server (console command)
    ConsoleCommandResult ConsoleStopServer(const StringVector &params);
    
    /// Connects to server (console command)
    ConsoleCommandResult ConsoleConnect(const StringVector &params);
    
    /// Disconnects from server (console command)
    ConsoleCommandResult ConsoleDisconnect(const StringVector &params);

    /// Saves scene to an XML file
    ConsoleCommandResult ConsoleSaveScene(const StringVector &params);

    /// Loads scene from an XML file.
    ConsoleCommandResult ConsoleLoadScene(const StringVector &params);
    
    /// Imports a dotscene
    ConsoleCommandResult ConsoleImportScene(const StringVector& params);
    
    /// Imports one mesh as a new entity
    ConsoleCommandResult ConsoleImportMesh(const StringVector& params);

    /// Lists all established connections
    ConsoleCommandResult ConsoleListConnections(const StringVector& params);
    
    /// Check whether we are a server
    bool IsServer() const;
    
    /// Return pointer to KristalliProtocolModule for convenience
    const boost::shared_ptr<KristalliProtocol::KristalliProtocolModule>& GetKristalliModule() const { return kristalliModule_; }
    
    /// Return client
    const boost::shared_ptr<Client>& GetClient() const { return client_; }
    
    /// Return server
    const boost::shared_ptr<Server>& GetServer() const { return server_; }

public slots:
    //Returns syncManager. Used only for server.
    SyncManager* GetSyncManager();
    // changeScene
    void changeScene(const QString&);

    // Grep number out of connection name
    unsigned short Grep(const QString);
    
private slots:
    void StartupSceneLoaded(AssetPtr asset);
    void StartupSceneTransferFailed(IAssetTransfer *transfer, QString reason);

    // Connects new syncManager object to newly created scene
    void AttachSyncManagerToScene(const QString&);
    // Removes syncManager object from QMap when scene removed.
    void RemoveSyncManagerFromScene(const QString&);

signals:
    // Signals which activate clients slots which again emit signals. This is because we want to catch these signals in javascript.
    void createOgre(const QString&);
    void deleteOgre(const QString&);
    void setOgre(const QString&);
    void setClientActiveConnection(const QString&, unsigned short);

private:

    /// Handle a Kristalli protocol message
    void HandleKristalliMessage(kNet::MessageConnection* source, kNet::message_id_t id, const char* data, size_t numBytes);
    
    /// Load the startup scene
    void LoadStartupScene();
    
    // Syncmanager array
    QMap<QString, SyncManager*> syncManagers_;

    /// Client
    boost::shared_ptr<Client> client_;
    /// Server
    boost::shared_ptr<Server> server_;
    
    /// Kristalli event category
    event_category_id_t kristalliEventCategory_;
    /// Tundra event category
    event_category_id_t tundraEventCategory_;
    /// Framework event category
    event_category_id_t frameworkEventCategory_;
    
    /// KristalliProtocolModule pointer
    boost::shared_ptr<KristalliProtocol::KristalliProtocolModule> kristalliModule_;
    
    //! Type name of the module.
    static std::string type_name_static_;
    
    //! Whether to autostart the server
    bool autostartserver_;
    //! Autostart server port
    short autostartserver_port_;

    // Active syncManager
    QString activeSyncManager;

};

}

#endif
