// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "CoreTypes.h"
#include "TundraProtocolModuleApi.h"
#include "TundraProtocolModuleFwd.h"

#include <kNet/SharedPtr.h>
#include <kNet/MessageConnection.h>

#include <QObject>

class Entity;
class SceneSyncState;

/// Represents a client connection on the server side.
class TUNDRAPROTOCOL_MODULE_API UserConnection : public QObject, public enable_shared_from_this<UserConnection>
{
    Q_OBJECT
    Q_PROPERTY(int id READ ConnectionId)

public:
    UserConnection() : userID(0) {}

    /// Returns the connection ID.
    int ConnectionId() const { return userID; }

    /// Message connection
    Ptr(kNet::MessageConnection) connection;
    /// Connection ID
    u32 userID;
    /// Raw xml login data
    QString loginData;
    /// Property map
    LoginPropertyMap properties;
    /// Scene sync state, created and used by the SyncManager
    shared_ptr<SceneSyncState> syncState;

public slots:
    /// Execute an action on an entity, sent only to the specific user
    void Exec(Entity *entity, const QString &action, const QStringList &params);
    void Exec(Entity *entity, const QString &action, const QString &p1 = "", const QString &p2 = "", const QString &p3 = "");  ///< @overload

    /// Returns raw login data
    QString LoginData() const { return loginData; }

    /// Sets a property
    void SetProperty(const QString& key, const QString& value);

    /// Returns a property
    QString Property(const QString& key) const;

    /// Returns all the login properties that were used to login to the server.
    LoginPropertyMap LoginProperties() const { return properties; }

    /// Deny connection. Call as a response to server.UserAboutToConnect() if necessary
    void DenyConnection(const QString& reason);

    /// Starts a benign disconnect procedure (one which waits for the peer acknowledge procedure).
    void Disconnect();

    /// Forcibly kills this connection without notifying the peer.
    void Close();

    int GetConnectionID() const { return ConnectionId(); }  /**< @deprecated Use ConnectionId or 'id' @todo Add warning print */
    QString GetLoginData() const { return LoginData(); }  /**< @deprecated Use LoginData @todo Add warning print */
    QString GetProperty(const QString& key) const { return Property(key); } /**< @deprecated Use Property @todo Add warning print */

signals:
    /// Emitted when action has been triggered for this specific user connection.
    void ActionTriggered(UserConnection* connection, Entity* entity, const QString& action, const QStringList& params);
};
