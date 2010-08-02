#pragma once

#include "clb/Network/KristalliProtocol.h"
#include "RexUUID.h"

/// Connection info
struct ServerConnection
{
    MessageConnection *connection;
    u8 userID; // Per-session ID, necessary for efficient sending of realtime data
    RexUUID userUUID; // Identifies user uniquely
    std::string userName;
};

typedef std::list<ServerConnection> ServerConnectionList;
typedef std::list<ServerConnection>::iterator ServerConnectionListIter;
typedef std::list<ServerConnection>::const_iterator ServerConnectionListConstIter;

class KristalliServer;

/// Service interface for sub-servers
class IService
{
public:
    virtual ~IService() {}
    
    /// A new user connects
    virtual void HandleConnect(ServerConnection* connection) {}
    /// A user disconnects
    virtual void HandleDisconnect(ServerConnection* connection) {}
    /// A network message is received from a user
    virtual void HandleMessage(ServerConnection* source, message_id_t id, const char *data, size_t numBytes) = 0;

    /// If a service needs to do periodic updates of some sort, it can override this function for doing so.
    virtual void Update() {}

    /// Called on server startup. Load anything persistent
    virtual void LoadScene() {}
    /// Initiated by a console command. Save anything persistent that should be there next time server starts up
    virtual void SaveScene() {}
    /// All users have left, reset scene as necessary (for security)
    virtual void ResetScene() {}
};

typedef std::list<IService*> ServiceList;
