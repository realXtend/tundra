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
    //! Constructor
    Server(TundraLogicModule* owner, Foundation::Framework* fw);
    
    //! Destructor
    ~Server();
    
    //! Perform any per-frame processing
    void Update(f64 frametime);
    
    //! Create server scene & start server
    /*! \return True if successful, false otherwise. No scene will be created if starting the server fails.
     */
    bool Start(unsigned short port);
    
    //! Stop server & delete server scene
    void Stop();
    
    //! Get matching userconnection from a messageconnection, or null if unknown
    UserConnection* GetUserConnection(kNet::MessageConnection* source) const;
    
    //! Get all connected users
    UserConnectionList& GetUserConnections() const;
    
    //! Get all authenticated users
    UserConnectionList GetAuthenticatedUsers() const;
    
    //! Handle Kristalli event
    void HandleKristalliEvent(event_id_t event_id, IEventData* data);
    
    //! Set current action sender. Called by SyncManager
    void SetActionSender(UserConnection* user);
    
signals:
    //! A user is connecting. This is your chance to deny access.
    /*! Call user->Disconnect() to deny access and kick the user out
     */ 
    void UserAboutToConnect(int connectionID, UserConnection* connection);
     
    //! A user has connected (and authenticated)
    void UserConnected(int connectionID, UserConnection* connection);
    
    //! A user has disconnected
    void UserDisconnected(int connectionID, UserConnection* connection);
    
    //! The server has been started
    void ServerStarted();
    
    //! The server has been stopped
    void ServerStopped();
    
public slots:
    //! Get whether server is running
    bool IsRunning() const;
    
    //! Get connected users' connection ID's
    QVariantList GetConnectionIDs() const;
    
    //! Get userconnection structure corresponding to connection ID
    UserConnection* GetUserConnection(int connectionID) const;
    
    //! Get current sender of an action. Valid (non-null) only while an action packet is being handled. Null if it was invoked by server
    UserConnection* GetActionSender() const;
    
    //! Initialize server datatypes for a script engine
    void OnScriptEngineCreated(QScriptEngine* engine);
    
private:
    /// Handle a Kristalli protocol message
    void HandleKristalliMessage(kNet::MessageConnection* source, kNet::message_id_t id, const char* data, size_t numBytes);
    
    /// Handle a user disconnecting
    void HandleUserDisconnected(UserConnection* user);
    
    /// Handle a login message
    void HandleLogin(kNet::MessageConnection* source, const MsgLogin& msg);
    
    /// Kristalli event category
    event_category_id_t kristalliEventCategory_;
    /// Tundra event category
    event_category_id_t tundraEventCategory_;
    
    //! Current action sender
    UserConnection* actionsender_;
    
    //! Owning module
    TundraLogicModule* owner_;
    //! Framework pointer
    Foundation::Framework* framework_;
};

}

#endif
