// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "CoreDefines.h"
#include "CoreTypes.h"
#include "TundraProtocolModuleApi.h"
#include "TundraProtocolModuleFwd.h"

#include <kNet/Types.h>

#include <QObject>
#include <QVariant>

class QScriptEngine;

class Framework;

namespace TundraLogic
{
/// Implements Tundra server functionality.
class TUNDRAPROTOCOL_MODULE_API Server : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int port READ Port)
    Q_PROPERTY(QString protocol READ Protocol)

public:
    explicit Server(TundraLogicModule* owner);
    ~Server();

    /// Perform any per-frame processing
    void Update(f64 frametime);

    /// Get matching userconnection from a messageconnection, or null if unknown
    /// @todo Rename to UserConnection(ForMessageConnection) or similar.
    UserConnectionPtr GetUserConnection(kNet::MessageConnection* source) const;

    /// Get all connected users
    UserConnectionList& UserConnections() const;

    /// Set current action sender. Called by SyncManager
    void SetActionSender(const UserConnectionPtr &user);

    /// Returns the backend server object.
    /** Use this object to Broadcast messages to all currently connected clients.
        @todo Rename to (KNet)NetworkServer or similar. */
    kNet::NetworkServer *GetServer() const;

    /// Returns server's port.
    /** @return Server port number, or -1 if server is not running. */
    int Port() const;

    /// Returns server's protocol.
    /** @return 'udp', tcp', or an empty string if server is not running. */
    QString Protocol() const;

public slots:
    /// Create server scene & start server
    /** @param protocol The server protocol to use, either "tcp" or "udp". If not specified, the default UDP will be used.
        @return True if successful, false otherwise. No scene will be created if starting the server fails. */
    bool Start(unsigned short port, QString protocol = "");

    /// Stop server & delete server scene
    void Stop();

    /// Returns whether server is running
    bool IsRunning() const;

    /// Returns whether server is about to start.
    bool IsAboutToStart() const;

    /// Returns all authenticated users.
    UserConnectionList AuthenticatedUsers() const;

    /// Returns connection corresponding to a connection ID.
    /** @todo Rename to UserConnection or UserConnectionById. */
    UserConnectionPtr GetUserConnection(unsigned int connectionID) const;

    /// Returns current sender of an action.
    /** Valid (non-null) only while an action packet is being handled. Null if it was invoked by server */
    UserConnectionPtr ActionSender() const;

    QVariantList GetConnectionIDs() const; /**< @deprecated Use AuthenticatedUsers. */
    int GetPort() const; /**< @deprecated Use Port or 'port' property. */
    QString GetProtocol() const; /**< @deprecated Use Protocol or 'protocol' property. */
    UserConnectionPtr GetActionSender() const; /**< @deprecated Use ActionSender. */

signals:
    /// A user is connecting. This is your chance to deny access.
    /** Call user->Disconnect() to deny access and kick the user out.
        @todo the connectionID parameter is unnecessary as it can be retrieved from connection. */
    void UserAboutToConnect(unsigned int connectionID, UserConnection* connection);

    /// A user has connected (and authenticated)
    /** @param responseData The handler of this signal can add his own application-specific data to this structure.
        This data is sent to the client and the applications on the client computer can read them as needed.
        @todo the connectionID parameter is unnecessary as it can be retrieved from connection. */
    void UserConnected(unsigned int connectionID, UserConnection* connection, UserConnectedResponseData *responseData);

    void MessageReceived(UserConnection *connection, kNet::packet_id_t, kNet::message_id_t id, const char* data, size_t numBytes);

    /// A user has disconnected
    /** @todo the connectionID parameter is unnecessary as it can be retrieved from connection. */
    void UserDisconnected(unsigned int connectionID, UserConnection* connection);

    /// The server has been started
    void ServerStarted();

    /// The server has been stopped
    void ServerStopped();

private slots:
    /// Handle a Kristalli protocol message
    void HandleKristalliMessage(kNet::MessageConnection* source, kNet::packet_id_t, kNet::message_id_t id, const char* data, size_t numBytes);

    /// Handle a user disconnecting
    void HandleUserDisconnected(UserConnection* user);

    /// Initialize server datatypes for a script engine
    void OnScriptEngineCreated(QScriptEngine* engine);

private:
    /// Handle a login message
    void HandleLogin(kNet::MessageConnection* source, const MsgLogin& msg);

    UserConnectionWeakPtr actionSender;
    TundraLogicModule* owner_;
    Framework* framework_;
    int current_port_;
    QString current_protocol_;
};

}
