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
    /// @todo Rename to UserConnectionForMessageConnection or similar.
    UserConnection* GetUserConnection(kNet::MessageConnection* source) const;

    /// Get all connected users
    /// @todo Rename to UserConnections
    UserConnectionList& GetUserConnections() const;

    /// Get all authenticated users
    /// @todo Rename to AuthenticatedUsers
    UserConnectionList GetAuthenticatedUsers() const;

    /// Set current action sender. Called by SyncManager
    void SetActionSender(UserConnection* user);

    /// Returns the backend server object.
    /** Use this object to Broadcast messages to all currently connected clients.
        @todo Rename to (KNet)NetworkServer or similar. */
    kNet::NetworkServer *GetServer() const;

    /// Returns server's port.
    /** @return Server port number, or -1 if server is not running. */
    int Port() const;

    /** Returns server's protocol.
        @return 'udp', tcp', or an empty string if server is not running. */
    QString Protocol() const;

signals:
    /// A user is connecting. This is your chance to deny access.
    /** Call user->Disconnect() to deny access and kick the user out */ 
    void UserAboutToConnect(int connectionID, UserConnection* connection);

    /// A user has connected (and authenticated)
    /** @param responseData The handler of this signal can add his own application-specific data to this structure. This data is sent to the
        client and the applications on the client computer can read them as needed. */
    void UserConnected(int connectionID, UserConnection* connection, UserConnectedResponseData *responseData);

    void MessageReceived(UserConnection *connection, kNet::packet_id_t, kNet::message_id_t id, const char* data, size_t numBytes);

    /// A user has disconnected
    void UserDisconnected(int connectionID, UserConnection* connection);

    /// The server has been started
    void ServerStarted();

    /// The server has been stopped
    void ServerStopped();

public slots:
    /// Create server scene & start server
    /** @param protocol The server protocol to use, either "tcp" or "udp". If not specified, the default UDP will be used.
        @return True if successful, false otherwise. No scene will be created if starting the server fails. */
    bool Start(unsigned short port, QString protocol = "");

    /// Stop server & delete server scene
    void Stop();

    /// Get whether server is running
    bool IsRunning() const;

    /// Get whether server is about to start.
    bool IsAboutToStart() const;

    /// @todo Add deprecation warning print, and instructions to use 'port' property, and and remove this function.
    int GetPort() const { return Port(); }

    /// @todo Add deprecation warning print, and instructions to use 'protocol' property, and and remove this function.
    QString GetProtocol() { return Protocol(); }

    /// Get connected users' connection ID's
    /** @todo This script-hack function is same as GetAuthenticatedUsers, remove this and expose UserConnectionList to QtScript. */
    QVariantList GetConnectionIDs() const;

    /// Get userconnection structure corresponding to connection ID
    /** @todo Rename to UserConnectionById. */
    UserConnection* GetUserConnection(int connectionID) const;

    /// Get current sender of an action.
    /** Valid (non-null) only while an action packet is being handled. Null if it was invoked by server
        @todo Rename to ActionSender. */
    UserConnection* GetActionSender() const;

    /// Initialize server datatypes for a script engine
    void OnScriptEngineCreated(QScriptEngine* engine);

private slots:
    /// Handle a Kristalli protocol message
    void HandleKristalliMessage(kNet::MessageConnection* source, kNet::packet_id_t, kNet::message_id_t id, const char* data, size_t numBytes);

    /// Handle a user disconnecting
    void HandleUserDisconnected(UserConnection* user);

private:
    /// Handle a login message
    void HandleLogin(kNet::MessageConnection* source, const MsgLogin& msg);

    UserConnection* actionsender_;
    TundraLogicModule* owner_;
    Framework* framework_;
    int current_port_;
    QString current_protocol_;
};

}
