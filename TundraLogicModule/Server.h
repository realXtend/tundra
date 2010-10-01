// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TundraLogicModule_Server_h
#define incl_TundraLogicModule_Server_h

#include "ForwardDefines.h"

#include "kNet.h"

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

class Server
{
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
    
    //! Get whether server is running
    bool IsRunning() const;
    
    //! Get matching userconnection from a messageconnection, or null if unknown
    KristalliProtocol::UserConnection* GetUserConnection(kNet::MessageConnection* source);
    
    //! Get all connected users
    KristalliProtocol::UserConnectionList& GetUserConnections();
    
    //! Handle Kristalli event
    void HandleKristalliEvent(event_id_t event_id, IEventData* data);
    
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
