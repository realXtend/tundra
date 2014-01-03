
#pragma once

#include "WebSocketServerModuleApi.h"
#include "Win.h"

#include "FrameworkFwd.h"
#include "WebSocketFwd.h"
#include "kNetFwd.h"

#include "WebSocketServer.h"
#include "SyncState.h"
#include "UserConnection.h"

#include <QObject>
#include <QHash>
#include <QString>
#include <QVariant>

namespace WebSocket
{
    class WEBSOCKET_SERVER_MODULE_API UserConnection : public ::UserConnection
    {
    Q_OBJECT

    public:
        UserConnection(ConnectionPtr connection_);
        ~UserConnection();

        ConnectionPtr WebSocketConnection() const;

        void Send(const kNet::DataSerializer &data);

        /// Queue a network message to be sent to the client. All implementations may not use the reliable, inOrder, priority and contentID parameters.
        virtual void Send(kNet::message_id_t id, const char* data, size_t numBytes, bool reliable, bool inOrder, unsigned long priority = 100, unsigned long contentID = 0);

        ConnectionWeakPtr webSocketConnection;

    public slots:
        virtual void Disconnect();
        virtual void Close();

        void DisconnectDelayed(int msec = 1000);
        
        
    };
}
