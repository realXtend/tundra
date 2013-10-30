// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "WebSocketServerModuleApi.h"
#include "WebSocketFwd.h"
#include "WebSocketServer.h"

#include "FrameworkFwd.h"
#include "SyncState.h"

#include <kNetFwd.h>

#include <QObject>
#include <QHash>
#include <QString>
#include <QVariant>

namespace WebSocket
{
    class WEBSOCKET_SERVER_MODULE_API UserConnection : public QObject, public enable_shared_from_this<UserConnection>
    {
    Q_OBJECT

    Q_PROPERTY(uint connectionId READ ConnectionId)
    Q_PROPERTY(uint id READ ConnectionId)

    public:
        UserConnection(uint connectionId_, ConnectionPtr connection_);
        ~UserConnection();

        uint ConnectionId();
        ConnectionPtr Connection() const;

        void Send(const kNet::DataSerializer &data);

        uint connectionId;

        ConnectionWeakPtr connection;
        LoginPropertyMap properties;
        shared_ptr<SceneSyncState> syncState;

    public slots:
        /// Execute an action on an entity, sent only to the specific user
        void Exec(Entity *entity, const QString &action, const QStringList &params);
        void Exec(Entity *entity, const QString &action, const QString &p1 = "", const QString &p2 = "", const QString &p3 = "");  ///< @overload
            
        /// Gets a string property. If you want other variant supported types use the properties map.
        QString Property(const QString &key);
        
        /// Denies user connection with reason.
        void DenyConnection(const QString &reason);
        
        void Disconnect();
        void DisconnectDelayed(int msec = 1000);
        
    signals:
        /// Emitted when action has been triggered for this specific user connection.
        void ActionTriggered(WebSocket::UserConnection* connection, Entity* entity, const QString& action, const QStringList& params);
    };
}
