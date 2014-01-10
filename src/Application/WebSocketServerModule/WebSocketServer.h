
#pragma once

#include "WebSocketServerModuleApi.h"
#include "Win.h"

#include "FrameworkFwd.h"
#include "WebSocketFwd.h"
#include "kNetFwd.h"
#include "AssetFwd.h"
#include "AssetReference.h"

#include "SyncState.h"
#include "MsgEntityAction.h"
#include "EntityAction.h"

#include <QObject>
#include <QThread>
#include <QString>
#include <QStringList>
#include <QFileInfo>
#include <QDateTime>
#include <QMutex>

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include "kNet/DataSerializer.h"
#include "boost/weak_ptr.hpp"

class QScriptEngine;

namespace WebSocket
{
    typedef shared_ptr<websocketpp::server<websocketpp::config::asio> > ServerPtr;
    typedef websocketpp::server<websocketpp::config::asio>::connection_ptr ConnectionPtr;
    typedef boost::weak_ptr<websocketpp::server<websocketpp::config::asio>::connection_type> ConnectionWeakPtr;
    typedef websocketpp::connection_hdl ConnectionHandle;
    typedef websocketpp::server<websocketpp::config::asio>::message_ptr MessagePtr;
    typedef shared_ptr<kNet::DataSerializer> DataSerializerPtr;
    
    // WebSocket events
    struct SocketEvent
    {
        enum EventType
        {
            None = 0,
            Connected,
            Disconnected,
            Data
        };

        WebSocket::ConnectionPtr connection;
        DataSerializerPtr data;
        EventType type;

        SocketEvent() : type(None) {}
        SocketEvent(WebSocket::ConnectionPtr connection_, EventType type_) : connection(connection_), type(type_) {}
    };

    /// Server run thread
    class ServerThread : public QThread
    {
    public:
        virtual void run();

        WebSocket::ServerPtr server_;
    };

    /// WebSocket server. 
    /** Manages user requestedConnections and receiving/sending out data with them.
        All signals emitted by this object will be in the main thread. */
    class WEBSOCKET_SERVER_MODULE_API Server : public QObject, public enable_shared_from_this<Server>
    {
    Q_OBJECT

    public:
        Server(Framework *framework);
        ~Server();
        
        bool Start();
        void Stop();
        void Update(float frametime);
        
        friend class Handler;
        
    public slots:
        /// Returns client with id, null if not found.
        WebSocket::UserConnectionPtr UserConnection(uint connectionId);

        /// Returns client with websocket connection ptr, null if not found.
        WebSocket::UserConnectionPtr UserConnection(WebSocket::ConnectionPtr connection);
        
        /// Mirror the Server object API.
        WebSocket::UserConnectionPtr GetUserConnection(uint connectionId) { return UserConnection(connectionId); }

        /// Returns all user connections
        WebSocket::UserConnectionList UserConnections() { return connections_; }
        
    private slots:
        void OnScriptEngineCreated(QScriptEngine *engine);

    signals:
        /// The server has been started
        void ServerStarted();

        /// The server has been stopped
        void ServerStopped();
        
        /// Network message received from client
        void NetworkMessageReceived(WebSocket::UserConnection *source, kNet::message_id_t id, const char* data, size_t numBytes);

    protected:
        void Reset();

        void OnConnected(WebSocket::ConnectionHandle connection);
        void OnDisconnected(WebSocket::ConnectionHandle connection);
        void OnMessage(WebSocket::ConnectionHandle connection, WebSocket::MessagePtr data);
        void OnHttpRequest(WebSocket::ConnectionHandle connection);
        
    private:
        QString LC;
        ushort port_;
        
        Framework *framework_;
        
        WebSocket::ServerPtr server_;

        // Websocket connections. Once login is finalized, they are also added to TundraProtocolModule's connection list
        WebSocket::UserConnectionList connections_;

        ServerThread thread_;

        QMutex mutexEvents_;
        QList<SocketEvent*> events_;
    };
}
