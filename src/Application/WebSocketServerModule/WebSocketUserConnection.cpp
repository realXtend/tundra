// For conditions of distribution and use, see copyright notice in LICENSE

#include "WebSocketUserConnection.h"
#include "LoggingFunctions.h"

#include <kNet/DataDeserializer.h>
#include <kNet/DataSerializer.h>

#include <websocketpp/frame.hpp>

#include <QTimer>

namespace WebSocket
{

UserConnection::UserConnection(uint connectionId_, ConnectionPtr connection_) :
    connectionId(connectionId_),
    connection(connection_)
{
}

UserConnection::~UserConnection()
{
    connection.reset();
    syncState.reset();
}

uint UserConnection::ConnectionId()
{
    return connectionId;
}

ConnectionPtr UserConnection::Connection() const
{
    return connection.lock();
}

void UserConnection::Send(const kNet::DataSerializer &data)
{
    if (connection.expired())
        return;
    if (data.BytesFilled() == 0)
        return;
    
    connection.lock()->send(static_cast<void*>(data.GetData()), static_cast<uint64_t>(data.BytesFilled()));
}

void UserConnection::Exec(Entity *entity, const QString &action, const QStringList &params)
{
    if (entity)
        emit ActionTriggered(this, entity, action, params);
    else
        LogWarning("WebSocket::UserConnection::Exec: null entity passed!");
}

void UserConnection::Exec(Entity *entity, const QString &action, const QString &p1, const QString &p2, const QString &p3)
{
    Exec(entity, action, QStringList(QStringList() << p1 << p2 << p3));
}

QString UserConnection::Property(const QString &key)
{
    return properties.value(key, "").toString();
}

void UserConnection::DenyConnection(const QString &reason)
{
    properties["authenticated"] = false;
    properties["reason"] = reason;
}

void UserConnection::Disconnect()
{
    if (!connection.expired())
        connection.lock()->close(websocketpp::close::status::normal, "ok");
}

void UserConnection::DisconnectDelayed(int msec)
{
    if (msec < 0)
        msec = 1;
    QTimer::singleShot(msec, this, SLOT(Disconnect()));
}

}
