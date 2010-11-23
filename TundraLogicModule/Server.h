// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TundraLogicModule_Server_h
#define incl_TundraLogicModule_Server_h

#include "Core.h"
#include "ForwardDefines.h"

#include "kNet.h"

#include <QObject>
#include <QVariant>

struct MsgLogin;
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

class TundraLogicModule;

class Server : public QObject
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
    KristalliProtocol::UserConnection* GetUserConnection(kNet::MessageConnection* source);
    
    //! Get all connected users
    KristalliProtocol::UserConnectionList& GetUserConnections();
    
    //! Handle Kristalli event
    void HandleKristalliEvent(event_id_t event_id, IEventData* data);
    
signals:
    //! A user has connected (and authenticated)
    void UserConnected(int connectionId, const QString& userName);
    
    //! A user has disconnected
    void UserDisconnected(int connectionId, const QString& userName);
    
    //! The server has been started
    void ServerStarted();
    
    //! The server has been stopped
    void ServerStopped();
    
public slots:
    //! Get whether server is running
    bool IsRunning() const;
    
    //! Get connected users' connection ID's
    QVariantList GetConnectionIDs() const;
    
    //! Get username for connection ID, or empty if no such connection
    QString GetUsername(int connectionID);
    
    //! Get user property for connection ID
    /*! Note: this is not (yet) profile data ie. it does not persist over separate connections!
        \param connectionID connection id
        \param key Property key
     */
    QString GetUserProperty(int connectionID, const QString& key);
    
    //! Set user property for connection ID
    /*! \param connectionID connection id
        \param key Property key
        \param value Property value
     */
    void SetUserProperty(int connectionID, const QString& key, const QString& value);
    
private:
    /// Handle a Kristalli protocol message
    void HandleKristalliMessage(kNet::MessageConnection* source, kNet::message_id_t id, const char* data, size_t numBytes);
    
    /// Handle a user disconnecting
    void HandleUserDisconnected(KristalliProtocol::UserConnection* user);
    
    /// Handle a login message
    void HandleLogin(kNet::MessageConnection* source, const MsgLogin& msg);
    
    /// Kristalli event category
    event_category_id_t kristalliEventCategory_;
    /// Tundra event category
    event_category_id_t tundraEventCategory_;
    
    //! Owning module
    TundraLogicModule* owner_;
    //! Framework pointer
    Foundation::Framework* framework_;
};

}

#endif
