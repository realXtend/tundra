// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TundraLogicModule_Client_h
#define incl_TundraLogicModule_Client_h

#include "ForwardDefines.h"

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

class TundraLogicModule;

class Client
{
    enum ClientLoginState
    {
        NotConnected = 0,
        ConnectionPending,
        Connected,
        LoggedIn
    };
    
public:
    //! Constructor
    Client(TundraLogicModule* owner, Foundation::Framework* fw);
    
    //! Destructor
    ~Client();
    
    //! Perform any per-frame processing
    void Update(f64 frametime);
    
    /// Connect and login
    void Login(const std::string& address, unsigned short port, const std::string& username, const std::string& password);
    
    /// Disconnect and delete client scene
    /// \param fail True if logout was due to connection/login failure
    void Logout(bool fail = false);
    
    /// Get connection/login state
    ClientLoginState GetLoginState() { return loginstate_; }
    
    /// Get client message connection from KristalliProtocolModule
    MessageConnection* GetConnection();
    
    /// Get client connection ID (from loginreply message)
    u8 GetConnectionID() { return client_id_; }
    
    //! Handle Kristalli event
    void HandleKristalliEvent(event_id_t event_id, IEventData* data);
    
private:
    /// Handle a Kristalli protocol message
    void HandleKristalliMessage(MessageConnection* source, message_id_t id, const char* data, size_t numBytes);
    
    /// Handle pending login to server
    void CheckLogin();
    
    /// Handle a loginreply message
    void HandleLoginReply(MessageConnection* source, const MsgLoginReply& msg);
    
    /// Handle a client joined message
    void HandleClientJoined(MessageConnection* source, const MsgClientJoined& msg);
    
    /// Client: Handle a client left message
    void HandleClientLeft(MessageConnection* source, const MsgClientLeft& msg);
    
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
