// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TundraLogicModule_TundraLogicModule_h
#define incl_TundraLogicModule_TundraLogicModule_h

#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"

struct MsgLogin;
struct MsgLoginReply;
struct MsgClientJoined;
struct MsgClientLeft;
class MessageConnection;

typedef unsigned long message_id_t;

namespace KristalliProtocol
{
    struct UserConnection;
    class KristalliProtocolModule;
    typedef std::list<UserConnection> UserConnectionList;
}

namespace TundraLogic
{

class SyncManager;

class TundraLogicModule : public Foundation::ModuleInterface
{
    enum ClientLoginState
    {
        NotConnected = 0,
        ConnectionPending,
        Connected,
        LoggedIn
    };
    
public:
    /// Default constructor.
    TundraLogicModule();

    /// Destructor.
    ~TundraLogicModule();

    /// ModuleInterface override.
    void PreInitialize();

    /// ModuleInterface override.
    void Initialize();

    /// ModuleInterface override.
    void PostInitialize();

    /// ModuleInterface override.
    void Uninitialize();

    /// ModuleInterface override.
    void Update(f64 frametime);

    /// ModuleInterface override.
    bool HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data);

    MODULE_LOGGING_FUNCTIONS

    /// Returns name of this module. Needed for logging.
    static const std::string &NameStatic() { return type_name_static_; }

    /// Starts a server (console command)
    Console::CommandResult ConsoleStartServer(const StringVector &params);
    
    /// Stops a server (console command)
    Console::CommandResult ConsoleStopServer(const StringVector &params);
    
    /// Connects to server (console command)
    Console::CommandResult ConsoleConnect(const StringVector &params);
    
    /// Disconnects from server (console command)
    Console::CommandResult ConsoleDisconnect(const StringVector &params);
    
    /// Check whether we are a server (from KristalliProtocolModule)
    bool IsServer() const;
    
    /// Create server scene & start server
    void ServerStart(unsigned short port, bool use_udp);
    
    /// Stop server & delete server scene
    void ServerStop();
    
    /// Get connected users from KristalliProtocolModule
    KristalliProtocol::UserConnectionList& ServerGetUserConnections();
    
    /// Connect and login
    void ClientLogin(const std::string& address, unsigned short port, bool use_udp, const std::string& username, const std::string& password);
    
    /// Disconnect and delete client scene
    /// \param fail True if logout was due to connection/login failure
    void ClientLogout(bool fail = false);
    
    /// Get connection/login state
    ClientLoginState ClientGetLoginState() { return loginstate_; }
    
    /// Get client message connection from KristalliProtocolModule
    MessageConnection* ClientGetConnection();
    
    /// Get client connection ID (from loginreply message)
    u8 ClientGetConnectionID() { return client_id_; }
    
private:
    /// Handle a Kristalli protocol message
    void HandleKristalliMessage(MessageConnection* source, message_id_t id, const char* data, size_t numBytes);
    
    /// Server: Handle a user disconnecting
    void ServerHandleUserDisconnected(KristalliProtocol::UserConnection* user);
    
    /// Server: Handle a login message
    void ServerHandleLogin(MessageConnection* source, const MsgLogin& msg);
    
    /// Client: handle pending login to server
    void ClientCheckLogin();
    
    /// Client: Handle a loginreply message
    void ClientHandleLoginReply(MessageConnection* source, const MsgLoginReply& msg);
    
    /// Client: Handle a client joined message
    void ClientHandleClientJoined(MessageConnection* source, const MsgClientJoined& msg);
    
    /// Client: Handle a client left message
    void ClientHandleClientLeft(MessageConnection* source, const MsgClientLeft& msg);
    
    /// Client's connection/login state
    ClientLoginState loginstate_;
    /// Whether the connect attempt is a reconnect because of dropped connection
    bool reconnect_;
    
    /// Stored username for login
    std::string username_;
    /// Stored password for login
    std::string password_;
    /// User ID, once known
    u8 client_id_;
    
    /// Sync manager
    boost::shared_ptr<SyncManager> syncManager_;
    
    /// Kristalli event category
    event_category_id_t kristalliEventCategory_;
    /// Tundra event category
    event_category_id_t tundraEventCategory_;
    
    /// KristalliProtocolModule pointer
    boost::shared_ptr<KristalliProtocol::KristalliProtocolModule> kristalliModule_;
    
    //! Type name of the module.
    static std::string type_name_static_;
};

}

#endif
