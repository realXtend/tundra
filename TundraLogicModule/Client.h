// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TundraLogicModule_Client_h
#define incl_TundraLogicModule_Client_h

#include "Core.h"
#include "ForwardDefines.h"

#include <QObject>

struct MsgLogin;
struct MsgLoginReply;
struct MsgClientJoined;
struct MsgClientLeft;

namespace kNet
{
class MessageConnection;
typedef unsigned long message_id_t;
}

namespace KristalliProtocol
{
    class KristalliProtocolModule;
}

class UserConnection;
typedef std::list<UserConnection*> UserConnectionList;

namespace TundraLogic
{

class TundraLogicModule;

class Client : public QObject
{
    Q_OBJECT
    
    enum ClientLoginState
    {
        NotConnected = 0,
        ConnectionPending,
        ConnectionEstablished,
        LoggedIn
    };
    
public:
    //! Constructor
    Client(TundraLogicModule* owner, Foundation::Framework* fw);
    
    //! Destructor
    ~Client();
    
    //! Perform any per-frame processing
    void Update(f64 frametime);
    
    /// Get connection/login state
    ClientLoginState GetLoginState() { return loginstate_; }
    
    /// Get client message connection from KristalliProtocolModule
    kNet::MessageConnection* GetConnection();
    
    //! Handle Kristalli event
    void HandleKristalliEvent(event_id_t event_id, IEventData* data);
    
signals:
    //! Connected (& logged in) to server
    void Connected(int connectionID);
    
    //! Disconnected from server
    void Disconnected();
    
public slots:
    /// Connect and login. Username and password will be encoded to xml key-value data
    void Login(const QString& address, unsigned short port, const QString& username, const QString& password);
    /// Connect and login using freeform logindata
    void Login(const QString& address, unsigned short port, const QString& loginData);
    
    /// Disconnect and delete client scene
    /// \param fail True if logout was due to connection/login failure
    void Logout(bool fail = false);
    
    /// Get client connection ID (from loginreply message). Is zero if not connected
    int GetConnectionID() { return client_id_; }
    
    //! See if connected & authenticated
    bool IsConnected() const;
    
    //! Get current raw logindata that is used to connect
    QString GetLoginData() const { return logindata_; }
    
private:
    /// Handle a Kristalli protocol message
    void HandleKristalliMessage(kNet::MessageConnection* source, kNet::message_id_t id, const char* data, size_t numBytes);
    
    /// Handle pending login to server
    void CheckLogin();
    
    /// Handle a loginreply message
    void HandleLoginReply(kNet::MessageConnection* source, const MsgLoginReply& msg);
    
    /// Handle a client joined message
    void HandleClientJoined(kNet::MessageConnection* source, const MsgClientJoined& msg);
    
    /// Client: Handle a client left message
    void HandleClientLeft(kNet::MessageConnection* source, const MsgClientLeft& msg);
    
    /// Client's connection/login state
    ClientLoginState loginstate_;
    /// Whether the connect attempt is a reconnect because of dropped connection
    bool reconnect_;
    /// Stored logindata for login
    QString logindata_;
    /// User ID, once known
    u8 client_id_;
    
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
