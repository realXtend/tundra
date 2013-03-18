// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "IModule.h"
#include "TundraProtocolModuleApi.h"
#include "UserConnection.h"

#include <kNet/IMessageHandler.h>
#include <kNet/INetworkServerListener.h>
#include <kNet/Network.h>

#ifdef KNET_USE_QT
#include <QPointer>
namespace kNet { class NetworkDialog; }
#endif

/// Implements kNet protocol -based server and client functionality.
class TUNDRAPROTOCOL_MODULE_API KristalliProtocolModule : public IModule, public kNet::IMessageHandler, public kNet::INetworkServerListener
{
    Q_OBJECT

public:
    KristalliProtocolModule();
    ~KristalliProtocolModule();

    void Load();
    void Unload();
    void Initialize();
    void Uninitialize();
    void Update(f64 frametime);

    /// Connects to the Kristalli server at the given address.
    void Connect(const char *ip, unsigned short port, kNet::SocketTransportLayer transport);

    void Disconnect();

    /// Starts a Kristalli server at the given port/transport.
    /// @return true if successful
    bool StartServer(unsigned short port, kNet::SocketTransportLayer transport);
    
    /// Stops Kristalli server
    void StopServer();
    
    /// Invoked by the Network library for each received network message.
    void HandleMessage(kNet::MessageConnection *source, kNet::packet_id_t packetId, kNet::message_id_t id, const char *data, size_t numBytes);

    /// Invoked by the Network library for each new connection
    void NewConnectionEstablished(kNet::MessageConnection* source);
    
    /// Invoked by the Network library for disconnected client
    void ClientDisconnected(kNet::MessageConnection* source);

    bool Connected() const { return serverConnection != 0; }

    void SubscribeToNetworkEvents();

    /// Return message connection, for use by other modules (null if no connection made)
    kNet::MessageConnection *GetMessageConnection() { return serverConnection.ptr(); }
    
    /// Return server, for use by other modules (null if not running)
    kNet::NetworkServer* GetServer() const { return server; }
    
    kNet::Network *GetNetwork() { return &network; }

    /// Return whether we are a server
    bool IsServer() const { return server != 0; }
    
    /// Returns all user connections for a server
    UserConnectionList& GetUserConnections() { return connections; }
    
    /// Gets user by message connection. Returns null if no such connection
    UserConnectionPtr GetUserConnection(kNet::MessageConnection* source) const;
    UserConnectionPtr GetUserConnection(u32 id) const; /**< @overload @param id Connection ID. */

    /// What trasport layer to use. Read on startup from "--protocol <udp|tcp>". Defaults to UDP if no start param was given.
    kNet::SocketTransportLayer defaultTransport;

public slots:
    void OpenKNetLogWindow();

signals:
    /// Triggered whenever a new message is received rom the network.
    void NetworkMessageReceived(kNet::MessageConnection *source, kNet::packet_id_t packetId, kNet::message_id_t messageId, const char *data, size_t numBytes);

    /// Triggered on the server side when a new user connects.
    void ClientConnectedEvent(UserConnection *connection);

    /// Triggered on the server side when a user disconnects.
    void ClientDisconnectedEvent(UserConnection *connection);

    /// Triggered on the client side when a server connection attempt has failed.
    void ConnectionAttemptFailed();

private:
    /// This timer tracks when we perform the next reconnection attempt when the connection is lost.
    kNet::PolledTimer reconnectTimer;

    /// Amount of retries remaining for reconnection. Is low for the initial connection, higher for reconnection
    int reconnectAttempts;

    void PerformConnection();

    /// Allocate a  connection ID for new connection
    u32 AllocateNewConnectionID() const;
    
    /// If true, the connection attempt we've started has not yet been established, but is waiting
    /// for a transition to OK state. When this happens, the MsgLogin message is sent.
    bool connectionPending;
    
    /// This variable stores the server ip address we are desiring to connect to.
    /// This is used to remember where we need to reconnect in case the connection goes down.
    std::string serverIp;
    /// Store the port number we are desiring to connect to. Used for reconnecting
    unsigned short serverPort;
    /// Store the transport type. Used for reconnecting
    kNet::SocketTransportLayer serverTransport;
    
    kNet::Network network;
    Ptr(kNet::MessageConnection) serverConnection;
    kNet::NetworkServer *server;
    
    /// Users that are connected to server
    UserConnectionList connections;
#ifdef KNET_USE_QT
    QPointer<kNet::NetworkDialog> networkDialog;
#endif
};
