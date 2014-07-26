// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "WebSocketServerModuleApi.h"
#include "FrameworkFwd.h"
#include "WebSocketFwd.h"

#include <kNetFwd.h>

#include <QObject>
#include <QThread>
#include <QMutex>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100)
#pragma warning(disable : 4267)
#pragma warning(disable : 4701)
#pragma warning(disable : 4996)
#endif
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <kNet/DataSerializer.h>

/** @todo Try building on windows without boost includes and ConnectionWeakPtr typedef.
    This should really not be needed, as the other typedefs prove. Still did not want
    to break the build before someone gives it a go on windows. */
#ifndef Q_OS_LINUX
#include <boost/weak_ptr.hpp>
#endif

class QScriptEngine;

namespace WebSocket
{
    typedef shared_ptr<websocketpp::server<websocketpp::config::asio> > ServerPtr;
    typedef websocketpp::server<websocketpp::config::asio>::connection_ptr ConnectionPtr;
#ifndef Q_OS_LINUX
    typedef boost::weak_ptr<websocketpp::server<websocketpp::config::asio>::connection_type> ConnectionWeakPtr;
#else
    typedef weak_ptr<websocketpp::server<websocketpp::config::asio>::connection_type> ConnectionWeakPtr;
#endif
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
        Q_OBJECT

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
        explicit Server(Framework *framework);
        ~Server();
        
        bool Start();
        void Stop();
        void Update(float frametime);
        
        friend class Handler;
        
    public slots:
        /// Returns client with id, null if not found.
        WebSocket::UserConnectionPtr UserConnection(uint connectionId) const;

        /// Returns client with websocket connection ptr, null if not found.
        WebSocket::UserConnectionPtr UserConnection(WebSocket::ConnectionPtr connection) const;
        
        /// Mirror the Server object API.
        WebSocket::UserConnectionPtr GetUserConnection(uint connectionId) const { return UserConnection(connectionId); }

        /// Returns all user connections
        WebSocket::UserConnectionList UserConnections() const { return connections_; }
        
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
        void OnSocketInit(WebSocket::ConnectionHandle connection, boost::asio::ip::tcp::socket& s);
        
    private:
        const QString LC;
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
