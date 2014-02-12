// For conditions of distribution and use, see copyright notice in LICENSE

#include "WebSocketServer.h"
#include "WebSocketUserConnection.h"
#include "WebSocketScriptTypeDefines.h"
#include "WebSocketScriptTypeDefines.h"

#include "Framework.h"
#include "CoreDefines.h"
#include "CoreJsonUtils.h"
#include "CoreStringUtils.h"
#include "LoggingFunctions.h"
#include "TundraMessages.h"
#include "TundraLogicModule.h"
#include "Server.h"
#include "MsgEntityAction.h"

#include <kNet/DataDeserializer.h>

#include <websocketpp/frame.hpp>

#include <QMutexLocker>
#include <QByteArray>
#include <QStringList>
#include <QVariant>
#include <QVariantMap>
#include <QVariantList>
#include <QProcess>
#include <QThreadPool>
#include <QFile>
#include <QTime>
#include <QLocale>
#include <QDebug>

#include <algorithm>

#ifdef Q_WS_WIN
#include "Win.h"
#else
#include <sys/stat.h>
#include <utime.h>
#endif

#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif

namespace WebSocket
{

// ServerThread

void ServerThread::run()
{
    if (!server_)
        return;
    
    try
    {
        server_->run();
    } 
    catch (const std::exception & e) 
    {
        LogError("Exception while running websocket server: " + QString(e.what()));
    } 
    catch (websocketpp::lib::error_code e) 
    {
        LogError("Exception while running websocket server: " + QString::fromStdString(e.message()));
    } 
    catch (...) 
    {
        LogError("Exception while running websocket server: other exception");
    }
}

// Server

Server::Server(Framework *framework) :
    LC("[WebSocketServer]: "),
    framework_(framework),
    port_(2345)
{
    // Port
    QStringList portParam = framework->CommandLineParameters("--port");
    if (!portParam.isEmpty())
    {
        bool ok = false;
        port_ = portParam.first().toUShort(&ok);
        if (!ok)
        {
            port_ = 2345;
            LogWarning(LC + "Failed to parse int from --port, using default port 2345.");
        }
    }
    
    qRegisterMetaType<MsgEntityAction>("MsgEntityAction");
}

Server::~Server()
{
    Reset();
    /// @todo 12.02.2014 Memory leak: dynamically allocated SocketEvents are not freed.
}

void Server::Update(float /*frametime*/)
{
    TundraLogic::TundraLogicModule* tundraLogic = framework_->Module<TundraLogicModule>();
    TundraLogic::Server* tundraServer = tundraLogic->GetServer().get();

    // Clean dead requestedConnections
    if (!connections_.empty())
    {
        WebSocket::UserConnectionList::iterator cleanupIter = connections_.begin();
        while (cleanupIter != connections_.end())
        {
            WebSocket::UserConnectionPtr connection = *cleanupIter;
            if (!connection)
            {
                cleanupIter = connections_.erase(cleanupIter);
            }
            else if (connection->webSocketConnection.expired())
            {
                // If user was already registered to the Tundra server, remove from there
                tundraServer->RemoveExternalUser(static_pointer_cast< ::UserConnection>(connection));
                if (!connection->userID)
                    LogDebug(LC + QString("Removing non-logged in WebSocket connection."));
                else
                    LogInfo(LC + QString("Removing expired WebSocket connection with ID %1").arg(connection->userID));
                cleanupIter = connections_.erase(cleanupIter);
            }
            else
            {
                ++cleanupIter;
            }
        }
    }
    
    QList<SocketEvent*> processEvents;
    {
        QMutexLocker lockEvents(&mutexEvents_);
        if (events_.size() == 0)
            return;
        // Make copy of current event queue for processing
        processEvents = events_;
        events_.clear();
    }

    // Process events pushed from the websocket thread(s)
    for (int i=0; i< processEvents.size(); ++i)
    {
        SocketEvent *event = processEvents[i];
        if (!event)
            continue;

        // User connected
        if (event->type == SocketEvent::Connected)
        {
            if (!UserConnection(event->connection))
            {
                WebSocket::UserConnectionPtr userConnection(new WebSocket::UserConnection(event->connection));
                connections_.push_back(userConnection);

                // The connection does not yet have an ID assigned. Tundra server will assign on login
                LogDebug(LC + QString("New WebSocket connection."));
            }
        }
        // User disconnected
        else if (event->type == SocketEvent::Disconnected)
        {
            for(UserConnectionList::iterator iter = connections_.begin(); iter != connections_.end(); ++iter)
            {
                if ((*iter) && (*iter)->WebSocketConnection() == event->connection)
                {
                    tundraServer->RemoveExternalUser(static_pointer_cast< ::UserConnection>(*iter));
                    if (!(*iter)->userID)
                        LogDebug(LC + QString("Removing non-logged in WebSocket connection."));
                    else
                        LogInfo(LC + QString("Removing WebSocket connection with ID %1").arg((*iter)->userID));
                    connections_.erase(iter);
                    break;
                }
            }
        }
        // Data message
        else if (event->type == SocketEvent::Data && event->data.get())
        {
            WebSocket::UserConnectionPtr userConnection = UserConnection(event->connection);
            if (userConnection)
            {
                kNet::DataDeserializer dd(event->data->GetData(), event->data->BytesFilled());
                u16 messageId = dd.Read<u16>();

                // LoginMessage
                if (messageId == cLoginMessage)
                {
                    bool ok = false;
                    QString loginDataString = ReadUtf8String(dd);
                    QVariantMap map = TundraJson::Parse(loginDataString.toUtf8(), &ok).toMap();
                    if (ok)
                    {
                        foreach(const QString &key, map.keys())
                            userConnection->properties[key] = map[key];
                        userConnection->properties["authenticated"] = true;
                        bool success = tundraServer->AddExternalUser(static_pointer_cast< ::UserConnection>(userConnection));
                        if (!success)
                        {
                            LogInfo(LC + QString("Connection ID %1 login refused").arg(userConnection->userID));
                            userConnection->DisconnectDelayed();
                        }
                        else
                            LogInfo(LC + QString("Connection ID %1 login successful").arg(userConnection->userID));
                    }
                }
                else
                {
                    // Only signal messages from authenticated users
                    if (userConnection->properties["authenticated"].toBool() == true)
                    {
                        // Signal network message. As per kNet tradition the message ID is given separately in addition with the rest of the data
                        emit NetworkMessageReceived(userConnection.get(), messageId, event->data->GetData() + sizeof(u16), event->data->BytesFilled() - sizeof(u16));
                        // Signal network message on the Tundra server so that it can be globally picked up
                        tundraServer->EmitNetworkMessageReceived(userConnection.get(), 0, messageId, event->data->GetData() + sizeof(u16), event->data->BytesFilled() - sizeof(u16));
                    }
                }
            }
            else
                LogError(LC + "Received message from unauthorized connection, ignoring.");

            event->data.reset();
        }
        else
            event->data.reset();
        
        SAFE_DELETE(event);
    }
}

WebSocket::UserConnectionPtr Server::UserConnection(uint connectionId) const
{
    for(UserConnectionList::const_iterator iter = connections_.begin(); iter != connections_.end(); ++iter)
        if ((*iter)->userID == connectionId)
            return (*iter);

    return WebSocket::UserConnectionPtr();
}

WebSocket::UserConnectionPtr Server::UserConnection(ConnectionPtr connection) const
{
    if (!connection.get())
        return WebSocket::UserConnectionPtr();

    for(UserConnectionList::const_iterator iter = connections_.begin(); iter != connections_.end(); ++iter)
        if ((*iter)->WebSocketConnection().get() == connection.get())
            return (*iter);
    return WebSocket::UserConnectionPtr();
}

bool Server::Start()
{
    Reset();
    
    try
    {
        server_ = WebSocket::ServerPtr(new websocketpp::server<websocketpp::config::asio>());

        // Initialize ASIO transport
        server_->init_asio();

        // Register handler callbacks
        server_->set_open_handler(boost::bind(&Server::OnConnected, this, ::_1));
        server_->set_close_handler(boost::bind(&Server::OnDisconnected, this, ::_1));
        server_->set_message_handler(boost::bind(&Server::OnMessage, this, ::_1, ::_2));
        server_->set_socket_init_handler(boost::bind(&Server::OnSocketInit, this, ::_1, ::_2));

        // Setup logging
        server_->get_alog().clear_channels(websocketpp::log::alevel::all);
        server_->get_elog().clear_channels(websocketpp::log::elevel::all);
        server_->get_elog().set_channels(websocketpp::log::elevel::rerror);
        server_->get_elog().set_channels(websocketpp::log::elevel::fatal);

        server_->listen(port_);

        // Start the server accept loop
        server_->start_accept();

        // Start the server polling thread
        thread_.server_ = server_;
        thread_.start();

    } 
    catch (std::exception &e) 
    {
        LogError(LC + QString::fromStdString(e.what()));
        return false;
    }
    
    qDebug() << QString(LC + "Started to port %1 with %2 listeners threads in main thread")
        .arg(port_).arg(QThread::idealThreadCount() > 0 ? QThread::idealThreadCount() : 1).toStdString().c_str() 
        << QThread::currentThreadId();

    emit ServerStarted();
    
    return true;
}

void Server::Stop()
{    
    try
    {
        if (server_)
        {
            server_->stop();
            thread_.wait();
            emit ServerStopped();
        }
    }
    catch (std::exception &e) 
    {
        LogError(LC + "Error while closing server: " + QString::fromStdString(e.what()));
        return;
    }
    
    LogDebug(LC + "Stopped"); 
    
    Reset();
}

void Server::Reset()
{
    connections_.clear();

    server_.reset();
}

void Server::OnConnected(ConnectionHandle connection)
{
    QMutexLocker lock(&mutexEvents_);

    ConnectionPtr connectionPtr = server_->get_con_from_hdl(connection);

    // Find existing events and remove them if we got duplicates 
    // that were not synced to main thread yet.
    QList<SocketEvent*> removeItems;
    for (int i=0; i<events_.size(); ++i)
    {
        // Remove any and all messages from this connection, 
        // if there are any data messages for this connection.
        // Which is not possible in theory.
        SocketEvent *existing = events_[i];
        if (existing->connection == connectionPtr)
            removeItems << existing;
    }
    if (!removeItems.isEmpty())
    {
        foreach(SocketEvent *existing, removeItems)
        {
            events_.removeAll(existing);
            SAFE_DELETE(existing);
        }
        removeItems.clear();
    }

    events_ << new SocketEvent(connectionPtr, SocketEvent::Connected); /**< @todo 12.02.2014 no need to dynamically allocate these */
        
    mutexEvents_.unlock();
}

void Server::OnDisconnected(ConnectionHandle connection)
{
    QMutexLocker lock(&mutexEvents_);

    ConnectionPtr connectionPtr = server_->get_con_from_hdl(connection);

    // Find existing events and remove them if we got duplicates 
    // that were not synced to main thread yet.
    QList<SocketEvent*> removeItems;
    for (int i=0; i<events_.size(); ++i)
    {
        // Remove any and all messages from this connection, 
        // no need to process he is disconnecting
        SocketEvent *existing = events_[i];
        if (existing->connection == connectionPtr)
            removeItems << existing;
    }
    if (!removeItems.isEmpty())
    {
        foreach(SocketEvent *existing, removeItems)
        {
            events_.removeAll(existing);
            SAFE_DELETE(existing);
        }
        removeItems.clear();
    }

    events_ << new SocketEvent(connectionPtr, SocketEvent::Disconnected); /**< @todo 12.02.2014 no need to dynamically allocate these */
}

void Server::OnMessage(ConnectionHandle connection, MessagePtr data)
{   
    QMutexLocker lock(&mutexEvents_);

    ConnectionPtr connectionPtr = server_->get_con_from_hdl(connection);

    if (data->get_opcode() == websocketpp::frame::opcode::TEXT)
    {
        QByteArray buffer = QString::fromStdString(data->get_payload()).toUtf8();
        LogInfo(QString("[WebSocketServer]: on_utf8_message: size=%1 msg=%2").arg(buffer.size()).arg(QString(buffer)));
    }
    else if (data->get_opcode() == websocketpp::frame::opcode::BINARY)
    {
        const std::string &payload = data->get_payload();
        if (payload.size() == 0)
        {
            LogError("[WebSocketServer]: Received 0 sized payload, ignoring");
            return;
        }
        SocketEvent *event = new SocketEvent(connectionPtr, SocketEvent::Data); /**< @todo 12.02.2014 no need to dynamically allocate these */
        event->data = DataSerializerPtr(new kNet::DataSerializer(payload.size()));
        event->data->AddAlignedByteArray(&payload[0], payload.size());

        events_ << event;
    }
}

void Server::OnScriptEngineCreated(QScriptEngine *engine)
{
    RegisterWebSocketPluginMetaTypes(engine);
}


/// \todo Implement actual registering of http handlers, for now disabled
/*
void Server::OnHttpRequest(WebSocket::ConnectionHandle connection)
{
    QByteArray resourcePath = QString::fromStdString(connection->get_resource()).toUtf8();

    qDebug() << "OnHttpRequest" << resourcePath;
        
    QByteArray data("Hello World to " + resourcePath);
    std::string payload;
    payload.resize(data.size());
    memcpy((void*)payload.data(), (void*)data.data(), data.size());
    
    connection->set_status(websocketpp::http::status_code::ok);
    connection->set_body(payload);
    connection->replace_header("Content-Length", QString::number(data.size()).toStdString());
}
*/

void Server::OnSocketInit(ConnectionHandle connection, boost::asio::ip::tcp::socket& s)
{
    // Disable Nagle's algorithm from each connection to avoid delays in sync
    boost::asio::ip::tcp::no_delay option(true);
    s.set_option(option);
}

}
