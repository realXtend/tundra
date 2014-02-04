// For conditions of distribution and use, see copyright notice in LICENSE

#include "WebSocketUserConnection.h"

#include <kNet/DataDeserializer.h>
#include <kNet/DataSerializer.h>

#include <websocketpp/frame.hpp>

#include <QTimer>

namespace WebSocket
{

UserConnection::UserConnection(const ConnectionPtr &connection) :
    webSocketConnection(connection)
{
}

UserConnection::~UserConnection()
{
    webSocketConnection.reset();
    syncState.reset();
}

void UserConnection::Send(kNet::message_id_t id, const char* data, size_t numBytes,
    bool /*reliable*/, bool /*inOrder*/, unsigned long /*priority*/, unsigned long /*contentID*/)
{
    kNet::DataSerializer ds(numBytes + 2);
    ds.Add<u16>(id);
    if (numBytes)
        ds.AddAlignedByteArray(data, numBytes);
    Send(ds);
}

ConnectionPtr UserConnection::WebSocketConnection() const
{
    return webSocketConnection.lock();
}

void UserConnection::Send(const kNet::DataSerializer &data)
{
    if (webSocketConnection.expired())
        return;
    if (data.BytesFilled() == 0)
        return;
    
    webSocketConnection.lock()->send(static_cast<void*>(data.GetData()), static_cast<uint64_t>(data.BytesFilled()));
}

void UserConnection::Disconnect()
{
    if (!webSocketConnection.expired())
        webSocketConnection.lock()->close(websocketpp::close::status::normal, "ok");
}

void UserConnection::Close()
{
    Disconnect();
}

void UserConnection::DisconnectDelayed(int msec)
{
    if (msec < 0)
        msec = 1;
    QTimer::singleShot(msec, this, SLOT(Disconnect()));
}

}
