
#include "WebSocketServer.h"
#include "WebSocketUserConnection.h"

#include "Framework.h"
#include "CoreDefines.h"
#include "CoreJsonUtils.h"
#include "LoggingFunctions.h"
#include "Profiler.h"
#include "UniqueIdGenerator.h"
#include "OgreMaterialUtils.h"

#include "TundraMessages.h"
#include "UserConnectedResponseData.h"
#include "MsgLoginReply.h"

#include "kNet/DataDeserializer.h"

#include "websocket_frame.hpp"

#include "AssetAPI.h"
#include "IAssetStorage.h"
#include "IAsset.h"

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

/// @todo Temporarily here, move to core string utils or similar.

void WriteUTF8String(kNet::DataSerializer& ds, const QString& str)
{
    QByteArray utf8bytes = str.toUtf8();
    ds.Add<u16>(utf8bytes.size());
    if (utf8bytes.size())
        ds.AddArray<u8>((const u8*)utf8bytes.data(), utf8bytes.size());
}

QString ReadUTF8String(kNet::DataDeserializer& dd)
{
    QByteArray utf8bytes;
    utf8bytes.resize(dd.Read<u16>());
    if (utf8bytes.size())
    {
        dd.ReadArray<u8>((u8*)utf8bytes.data(), utf8bytes.size());
        return QString::fromUtf8(utf8bytes.data(), utf8bytes.size());
    }
    else
        return QString();
}

// Server

Server::Server(Framework *framework) :
    LC("[WebSocketServer]: "),
    framework_(framework),
    port_(2345),
    updatePeriod_(1.0f / 20.0f),
    updateAcc_(0.0)
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
}

void Server::Update(float frametime)
{
    // Check if it is yet time to perform a network update tick.
    updateAcc_ += (float)frametime;
    if (updateAcc_ < updatePeriod_)
        return;

    // If multiple updates passed, update still just once.
    updateAcc_ = fmod(updateAcc_, updatePeriod_);
    
    // Clean dead requestedConnections
    if (!connections_.empty())
    {
        WebSocket::UserConnectionList::iterator cleanupIter = connections_.begin();
        while (cleanupIter != connections_.end())
        {
            WebSocket::UserConnection *connection = (*cleanupIter);
            if (!connection)
            {
                cleanupIter = connections_.erase(cleanupIter);
            }
            else if (connection->connection.expired())
            {
                OnUserDisconnected(connection);
                delete connection;
                cleanupIter = connections_.erase(cleanupIter);
            }
            else
            {
                ++cleanupIter;
            }
        }
    }
    
    QMutexLocker lockEvents(&mutexEvents_);
    if (events_.size() == 0)
        return;

    // Process events pushed from the websocket thread(s)
    for (int i=0; i<events_.size(); ++i)
    {
        SocketEvent *event = events_[i];
        if (!event)
            continue;

        // User connected
        if (event->type == SocketEvent::Connected)
        {
            if (!UserConnection(event->connection))
            {
                WebSocket::UserConnection *userConnection = new WebSocket::UserConnection(NextFreeConnectionId(), event->connection);
                connections_.push_back(userConnection);

                LogDebug(LC + QString("New connection: id=%1 Connection count: %2").arg(userConnection->connectionId).arg(connections_.size()));
            }
        }
        // User disconnected
        else if (event->type == SocketEvent::Disconnected)
        {
            for(UserConnectionList::iterator iter = connections_.begin(); iter != connections_.end(); ++iter)
            {
                if (!(*iter) || (*iter)->Connection() != event->connection)
                    continue;

                WebSocket::UserConnection *userConnection = (*iter);

                // Tell everyone of the client leaving
                /// @todo At some point we might want to notify the native requestedConnections about the web client join/leave.
                kNet::DataSerializer ds;
                ds.Add<u16>(static_cast<u16>(cClientLeftMessage));
                ds.Add<u32>(userConnection->connectionId);

                for(UserConnectionList::const_iterator iterInner = connections_.begin(); iterInner != connections_.end(); ++iterInner)
                    if ((*iterInner)->connectionId != userConnection->connectionId)
                        (*iterInner)->Send(ds);

                emit UserDisconnected(userConnection);

                LogDebug(LC + QString("Removing connection: id=%1 Connection count: %2").arg(userConnection->connectionId).arg(connections_.size()-1));
                
                QString connectedUsername = userConnection->properties.value("username", "").toString();
                if (connectedUsername.isEmpty())
                    LogInfo(QString("[WEBSOCKET] ID %1 client disconnected").arg(userConnection->connectionId));
                else
                    LogInfo(QString("[WEBSOCKET] ID %1 client '%2' disconnected").arg(userConnection->connectionId).arg(connectedUsername));

                SAFE_DELETE(userConnection);
                connections_.erase(iter);
                break;
            }
        }
        // Data message
        else if (event->type == SocketEvent::Data && event->data.get())
        {
            WebSocket::UserConnection *userConnection = UserConnection(event->connection);
            if (userConnection)
            {
                kNet::DataDeserializer dd(event->data->GetData(), event->data->BytesFilled());
                u16 messageId = dd.Read<u16>();

                // LoginMessage
                if (messageId == cLoginMessage)
                {
                    bool ok = false;
                    QString loginDataString = ReadUTF8String(dd);
                    QVariantMap map = TundraJson::Parse(loginDataString.toUtf8(), &ok).toMap();
                    if (ok)
                    {
                        foreach(const QString &key, map.keys())
                            userConnection->properties[key] = map[key];
                        userConnection->properties["authenticated"] = true;
                        
                        QString connectedUsername = userConnection->properties.value("username", "").toString();
                        emit UserAboutToConnect(userConnection);

                        // Connection was granted
                        if (userConnection->properties["authenticated"].toBool())
                        {
                            if (connectedUsername.isEmpty())
                                LogInfo(QString("[WEBSOCKET] ID %1 client connected").arg(userConnection->connectionId));
                            else
                                LogInfo(QString("[WEBSOCKET] ID %1 client '%2' connected").arg(userConnection->connectionId).arg(connectedUsername));

                            kNet::DataSerializer dsJoined;
                            dsJoined.Add<u16>(static_cast<u16>(cClientJoinedMessage));
                            dsJoined.Add<u32>(userConnection->connectionId);

                            for(UserConnectionList::const_iterator iter = connections_.begin(); iter != connections_.end(); ++iter)
                            {
                                // Tell everyone of the client joining (also the user who joined)
                                (*iter)->Send(dsJoined);

                                // Advertise the users who already are in the world, to the new user
                                if ((*iter)->connectionId != userConnection->connectionId)
                                {
                                    kNet::DataSerializer dsAlreadyIn;
                                    dsAlreadyIn.Add<u16>(static_cast<u16>(cClientJoinedMessage));
                                    dsAlreadyIn.Add<u32>((*iter)->connectionId);

                                    userConnection->Send(dsAlreadyIn);
                                }
                            }

                            // Send login reply           
                            QVariantMap replyData;
                            emit UserConnected(userConnection, &replyData);
                            
                            // Add storage data into the map (until AssetAPI does this for us
                            AssetStoragePtr defaultStorage = framework_->Asset()->GetDefaultAssetStorage();
                            if (defaultStorage.get())
                            {
                                QVariantMap storageData;
                                storageData["default"] = true;
                                storageData["name"] = defaultStorage->Name();
                                storageData["type"] = defaultStorage->Type();
                                storageData["src"] = defaultStorage->BaseURL();
                                replyData["storage"] = storageData;
                            }
                            
                            QByteArray responseByteData = TundraJson::Serialize(replyData, TundraJson::IndentNone);
                            
                            std::vector<s8> loginReplyData;
                            loginReplyData.insert(loginReplyData.end(), responseByteData.data(), responseByteData.data() + responseByteData.size());

                            kNet::DataSerializer ds;
                            ds.Add<u16>(static_cast<u16>(cLoginReplyMessage));
                            ds.Add<u8>(1); // success
                            ds.Add<u32>(userConnection->connectionId);
                            ds.Add<u16>(responseByteData.size());
                            if (responseByteData.size() > 0)
                                ds.AddArray<s8>(&loginReplyData[0], loginReplyData.size());

                            userConnection->Send(ds);
                        }
                        // Connection was denied
                        else
                        {
                            QByteArray reason = userConnection->properties.value("reason", "").toString().toUtf8();
                            QString reasonLogString = reason.isEmpty() ? "Authentication failed" : reason;

                            if (connectedUsername.isEmpty())
                                LogInfo(QString("[WEBSOCKET] ID %1 client was denied access: ").arg(userConnection->connectionId) + reasonLogString);
                            else
                                LogInfo(QString("[WEBSOCKET] ID %1 client '%2' was denied access: ").arg(userConnection->connectionId).arg(connectedUsername) + reasonLogString);

                            std::vector<s8> loginReplyData;
                            loginReplyData.insert(loginReplyData.end(), reason.data(), reason.data() + reason.size());

                            kNet::DataSerializer ds;
                            ds.Add<u16>(static_cast<u16>(cLoginReplyMessage));
                            ds.Add<u8>(0); // failure
                            ds.Add<u32>(0);
                            ds.Add<u16>(loginReplyData.size());
                            if (loginReplyData.size() > 0)
                                ds.AddArray<s8>(&loginReplyData[0], loginReplyData.size());

                            userConnection->Send(ds);
                            userConnection->DisconnectDelayed();
                        }
                    }
                }
                // EntityActionMessage
                else if (messageId == cEntityActionMessage)
                {
                    MsgEntityAction msg;
                    
                    // Read data
                    msg.entityId = dd.Read<u32>();
                    u8 nameLen = dd.Read<u8>();
                    if (nameLen > 0)
                    {
                        msg.name.resize(nameLen);
                        dd.ReadArray<s8>(&msg.name[0], msg.name.size());
                    }
                    msg.executionType = dd.Read<u8>();
                    u8 paramCount = dd.Read<u8>();
                    for (int i=0; i<paramCount; ++i)
                    {
                        /// @note This is not VLE from web clients... yet
                        u32 paramLen = dd.Read<u32>(); 
                        if (paramLen > 0)
                        {
                            MsgEntityAction::S_parameters p;
                            p.parameter.resize(paramLen);
                            dd.ReadArray<s8>(&p.parameter[0], p.parameter.size());
                            msg.parameters.push_back(p);
                        }
                    }

                    emit ClientEntityAction(userConnection, msg);
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
    events_.clear();
}

void Server::OnUserDisconnected(WebSocket::UserConnection *userConnection)
{
    if (!userConnection)
        return;

    // Tell everyone of the client leaving
    /// @todo At some point we might want to notify the native requestedConnections about the web client join/leave.
    kNet::DataSerializer ds;
    ds.Add<u16>(static_cast<u16>(cClientLeftMessage));
    ds.Add<u32>(userConnection->connectionId);

    for(UserConnectionList::const_iterator iterInner = connections_.begin(); iterInner != connections_.end(); ++iterInner)
        if ((*iterInner)->connectionId != userConnection->connectionId)
            (*iterInner)->Send(ds);

    emit UserDisconnected(userConnection);

    LogDebug(LC + QString("Removing connection: id=%1 Connection count: %2").arg(userConnection->connectionId).arg(connections_.size()-1));
    
    QString connectedUsername = userConnection->properties.value("username", "").toString();
    if (connectedUsername.isEmpty())
        LogInfo(QString("[WEBSOCKET] ID %1 client disconnected").arg(userConnection->connectionId));
    else
        LogInfo(QString("[WEBSOCKET] ID %1 client '%2' disconnected").arg(userConnection->connectionId).arg(connectedUsername));
}

uint Server::NextFreeConnectionId() const
{
    UniqueIdGenerator g;
    while (true)
    {
        uint id = g.AllocateLocal();
        for(UserConnectionList::const_iterator iter = connections_.begin(); iter != connections_.end(); ++iter)
        {
            if ((*iter)->connectionId == id)
            {
                id = 0;
                break;
            }
        }
        if (id > 0)
            return id;
    }
    
    LogError(LC + "Failed to generate a sensible connection id, returning 0!");
    return 0;
}

WebSocket::UserConnectionList Server::AuthenticatedUsers() const
{
    WebSocket::UserConnectionList authenticated;
    for(WebSocket::UserConnectionList::const_iterator iter = connections_.begin(); iter != connections_.end(); ++iter)
        if ((*iter)->properties.value("authenticated", false).toBool())
            authenticated.push_back((*iter));
    return authenticated;
}

WebSocket::UserConnectionList &Server::UserConnections()
{
    return connections_;
}

WebSocket::UserConnection *Server::UserConnection(uint connectionId)
{
    for(UserConnectionList::iterator iter = connections_.begin(); iter != connections_.end(); ++iter)
        if ((*iter)->connectionId == connectionId)
            return (*iter);
    return 0;
}

WebSocket::UserConnection *Server::UserConnection(ConnectionPtr connection)
{
    if (!connection.get())
        return 0;

    for(UserConnectionList::iterator iter = connections_.begin(); iter != connections_.end(); ++iter)
        if ((*iter)->Connection().get() == connection.get())
            return (*iter);
    return 0;
}

void Server::SetActionSender(WebSocket::UserConnection *user)
{
    actionSender_ = user;
}

WebSocket::UserConnection *Server::ActionSender() const
{
    return actionSender_;
}

bool Server::Start()
{
    Reset();
    
    try
    {
        //  Create server with a handler. We'll get callbacks in the websocketpp thread context.
        handler_ = WebSocket::HandlerPtr(new Handler(this));
        server_ = WebSocket::ServerPtr(new websocketpp::server(handler_));

        // Setup logging
        server_->alog().unset_level(websocketpp::log::alevel::ALL);
        server_->elog().unset_level(websocketpp::log::elevel::ALL);
        server_->elog().set_level(websocketpp::log::elevel::RERROR);
        server_->elog().set_level(websocketpp::log::elevel::FATAL);

        // Start listening to port.
        server_->start_listen(port_, QThread::idealThreadCount() > 0 ? QThread::idealThreadCount() : 1);
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
            server_->stop(false);
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
    foreach(WebSocket::UserConnection *connection, connections_)
        SAFE_DELETE(connection);
    connections_.clear();

    handler_.reset();
    server_.reset();
}

void Server::OnConnected(ConnectionPtr connection)
{
    if (mutexEvents_.tryLock(100))
    {   
        // Find existing events and remove them if we got duplicates 
        // that were not synced to main thread yet.
        QList<SocketEvent*> removeItems;
        for (int i=0; i<events_.size(); ++i)
        {
            // Remove any and all messages from this connection, 
            // if there are any data messages for this connection.
            // Which is not possible in theory.
            SocketEvent *existing = events_[i];
            if (existing->connection == connection)
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

        events_ << new SocketEvent(connection, SocketEvent::Connected);
        
        mutexEvents_.unlock();
    }
    else
    {
        LogError(LC + "Event mutex too busy, denying new connection.");
        if (connection)
            connection->close(websocketpp::close::status::NORMAL);
    }
}

void Server::OnDisconnected(ConnectionPtr connection)
{
    if (mutexEvents_.tryLock(100))
    {
        // Find existing events and remove them if we got duplicates 
        // that were not synced to main thread yet.
        QList<SocketEvent*> removeItems;
        for (int i=0; i<events_.size(); ++i)
        {
            // Remove any and all messages from this connection, 
            // no need to process he is disconnecting
            SocketEvent *existing = events_[i];
            if (existing->connection == connection)
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

        events_ << new SocketEvent(connection, SocketEvent::Disconnected);
        
        mutexEvents_.unlock();
    }
    else
        LogError(LC + "Event mutex too busy, cannot handle disconnect event.");
}

void Server::OnMessage(ConnectionPtr connection, const char *data, size_t size)
{   
    QMutexLocker lock(&mutexEvents_);

    SocketEvent *event = new SocketEvent(connection, SocketEvent::Data);
    event->data = DataSerializerPtr(new kNet::DataSerializer(size));
    event->data->AddAlignedByteArray(data, size);

    events_ << event;
}

void Server::OnHttpRequest(WebSocket::ConnectionPtr connection)
{    
    QByteArray resourcePath = QString::fromStdString(connection->get_resource()).toUtf8();

    qDebug() << "OnHttpRequest" << resourcePath;
        
    QByteArray data("Hello World to " + resourcePath);
    std::string payload;
    payload.resize(data.size());
    memcpy((void*)payload.data(), (void*)data.data(), data.size());
    
    connection->set_status(websocketpp::http::status_code::OK);
    connection->set_body(payload);
    connection->replace_response_header("Content-Length", QString::number(data.size()).toStdString());
}

// Handler

Handler::Handler(Server *server) :
    server_(server)
{
}

Handler::~Handler()
{
    server_ = 0;
}

void Handler::validate(ConnectionPtr con)
{
    /// @todo Validate origin header here!  
}

void Handler::on_handshake_init(ConnectionPtr con)
{
}

void Handler::on_open(ConnectionPtr con)
{
    if (server_)
        server_->OnConnected(con);
}

void Handler::on_close(ConnectionPtr con)
{
    if (server_)
        server_->OnDisconnected(con);
}

void Handler::on_fail(ConnectionPtr con)
{
    if (server_)
        server_->OnDisconnected(con);
}

void Handler::on_message(ConnectionPtr con, websocketpp::message::data_ptr data)
{   
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
        if (server_)
            server_->OnMessage(con, &payload[0], payload.size());
    }
}

bool Handler::on_ping(ConnectionPtr con, std::string msg)
{
    LogInfo("on_ping");  
    return true;
}

void Handler::on_pong(ConnectionPtr con, std::string msg)
{
    LogInfo("on_pong");  
}

void Handler::on_pong_timeout(ConnectionPtr con, std::string msg)
{
    LogInfo("on_pong_timeout");  
}

void Handler::http(ConnectionPtr con)
{
    if (con->get_version() == -1 && server_)
        server_->OnHttpRequest(con);
}

}