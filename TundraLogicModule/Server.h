// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TundraLogicModule_Server_h
#define incl_TundraLogicModule_Server_h

#include "Core.h"
#include "TundraLogicModuleApi.h"
#include "ForwardDefines.h"

#include "kNet.h"

#include <QObject>
#include <QVariant>

struct MsgLogin;
class MessageConnection;

typedef unsigned long message_id_t;

namespace KristalliProtocol
{
    class KristalliProtocolModule;
}

class UserConnection;
typedef std::list<UserConnection*> UserConnectionList;

class QScriptEngine;

namespace TundraLogic
{

class TundraLogicModule;

class TUNDRALOGIC_MODULE_API Server : public QObject
{
    Q_OBJECT
    
public:
    /// Constructor
    explicit Server(TundraLogicModule* owner);

    /// Destructor
    ~Server();
    
    /// Perform any per-frame processing
    void Update(f64 frametime);
    
    /// Create server scene & start server
    /** \return True if successful, false otherwise. No scene will be created if starting the server fails.
     */
    bool Start(unsigned short port);
    
    /// Stop server & delete server scene
    void Stop();
    
    /// Get matching userconnection from a messageconnection, or null if unknown
    UserConnection* GetUserConnection(kNet::MessageConnection* source) const;
    
    /// Get all connected users
    UserConnectionList& GetUserConnections() const;
    
    /// Get all authenticated users
    UserConnectionList GetAuthenticatedUsers() const;
        
    /// Set current action sender. Called by SyncManager
    void SetActionSender(UserConnection* user);
    
signals:
    /// A user is connecting. This is your chance to deny access.
    /** Call user->Disconnect() to deny access and kick the user out
     */ 
    void UserAboutToConnect(int connectionID, UserConnection* connection);
     
    /// A user has connected (and authenticated)
    void UserConnected(int connectionID, UserConnection* connection);
    
    /// A user has disconnected
    void UserDisconnected(int connectionID, UserConnection* connection);
    
    /// The server has been started
    void ServerStarted();
    
    /// The server has been stopped
    void ServerStopped();
    
public slots:
    /// Get whether server is running
    bool IsRunning() const;

    /// Get whether server is about to start.
    bool IsAboutToStart() const;

    /// Get the running servers port.
    /// \return int Valid port if server is running. -1 if server is not running.
    int GetPort() const;

    /// Get the running servers protocol.
    /// \note This function returns QString due we dont want kNet::TransportLayer enum here. If the module creators feels its ok then change this.
    /// \return QString Will return 'udp' or 'tcp' if server is running. Otherwise an empty string.
    QString GetProtocol() const;
    
    /// Get connected users' connection ID's
    QVariantList GetConnectionIDs() const;
    
    /// Get userconnection structure corresponding to connection ID
    UserConnection* GetUserConnection(int connectionID) const;
    
    /// Get current sender of an action. Valid (non-null) only while an action packet is being handled. Null if it was invoked by server
    UserConnection* GetActionSender() const;
    
    /// Initialize server datatypes for a script engine
    void OnScriptEngineCreated(QScriptEngine* engine);
    
private:
    /// Handle a Kristalli protocol message
    void HandleKristalliMessage(kNet::MessageConnection* source, kNet::message_id_t id, const char* data, size_t numBytes);
    
    /// Handle a user disconnecting
    void HandleUserDisconnected(UserConnection* user);
    
    /// Handle a login message
    void HandleLogin(kNet::MessageConnection* source, const MsgLogin& msg);
        
    /// Current action sender
    UserConnection* actionsender_;
    
    /// Owning module
    TundraLogicModule* owner_;
    
    /// Framework pointer
    Foundation::Framework* framework_;

    /// Current running servers port.
    int current_port_;

    /// Current running servers protocol.
    QString current_protocol_;
};

}

#endif
