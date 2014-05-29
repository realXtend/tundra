// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "CoreTypes.h"
#include "TundraProtocolModuleApi.h"
#include "TundraProtocolModuleFwd.h"

#include <kNet/SharedPtr.h>
#include <kNet/MessageConnection.h>

#include <QObject>

class Entity;

/// Protocol versioning for client connections.
enum NetworkProtocolVersion
{
    ProtocolOriginal = 0x1
};

/// Highest supported protocol version in the build. Update this when a new protocol version is added
const NetworkProtocolVersion cHighestSupportedProtocolVersion = ProtocolOriginal;

/// Represents a client connection on the server side. Subclassed by networking implementations.
class TUNDRAPROTOCOL_MODULE_API UserConnection : public QObject, public enable_shared_from_this<UserConnection>
{
    Q_OBJECT
    Q_PROPERTY(u32 id READ ConnectionId)
    Q_PROPERTY(int protocolVersion READ ProtocolVersion)
    Q_PROPERTY(QString connectionType READ ConnectionType)

public:
    UserConnection() : 
        userID(0),
        protocolVersion(ProtocolOriginal)
    {}

    /// Returns the connection ID.
    u32 ConnectionId() const { return userID; }
    /// Returns the protocol version.
    NetworkProtocolVersion ProtocolVersion() const { return protocolVersion; }
    /// Returns connection type.
    virtual QString ConnectionType() const = 0;

    /// Connection ID
    u32 userID;
    /// Raw xml login data
    QString loginData;
    /// Property map
    LoginPropertyMap properties;
    /// Scene sync state, created and used by the SyncManager
    shared_ptr<SceneSyncState> syncState;
    /// Network protocol version in use
    NetworkProtocolVersion protocolVersion;

    /// Queue a network message to be sent to the client. All implementations may not use the reliable, inOrder, priority and contentID parameters.
    virtual void Send(kNet::message_id_t id, const char* data, size_t numBytes, bool reliable, bool inOrder, unsigned long priority = 100, unsigned long contentID = 0) = 0;

    /// Queue a network message to be sent to the client, with the data to be sent in a DataSerializer. All implementations may not use the reliable, inOrder, priority and contentID parameters.
    void Send(kNet::message_id_t id, bool reliable, bool inOrder, kNet::DataSerializer& ds, unsigned long priority = 100, unsigned long contentID = 0);

    /// Queue a typed network message to be sent to the client.
    template<typename SerializableMessage> void Send(const SerializableMessage &data)
    {
        kNet::DataSerializer ds(data.Size());
        data.SerializeTo(ds);
        Send(SerializableMessage::messageID, data.reliable, data.inOrder, ds);
    }

    /// Trigger a network message signal. Called by the networking implementation.
    void EmitNetworkMessageReceived(kNet::packet_id_t packetId, kNet::message_id_t messageId, const char* data, size_t numBytes);

public slots:
    /// Execute an action on an entity, sent only to the specific user
    void Exec(Entity *entity, const QString &action, const QStringList &params);
    void Exec(Entity *entity, const QString &action, const QString &p1 = "", const QString &p2 = "", const QString &p3 = "");  /**< @overload */

    /// Returns raw login data
    QString LoginData() const { return loginData; }

    /// Sets a property
    void SetProperty(const QString& key, const QString& value);

    /// Returns a property
    QVariant Property(const QString& key) const;

    /// Check whether has a property
    bool HasProperty(const QString& key) const;

    /// Returns all the login properties that were used to login to the server.
    LoginPropertyMap LoginProperties() const { return properties; }

    /// Deny connection. Call as a response to server.UserAboutToConnect() if necessary
    void DenyConnection(const QString& reason);

    /// Starts a benign disconnect procedure (one which waits for the peer acknowledge procedure).
    virtual void Disconnect() = 0;

    /// Forcibly kills this connection without notifying the peer.
    virtual void Close() = 0;

    u32 GetConnectionID() const { return ConnectionId(); }  /**< @deprecated Use ConnectionId or 'id' @todo Add warning print */
    QString GetLoginData() const { return LoginData(); }  /**< @deprecated Use LoginData @todo Add warning print */
    QString GetProperty(const QString& key) const { return Property(key).toString(); } /**< @deprecated Use Property @todo Add warning print */

signals:
    /// Emitted when action has been triggered for this specific user connection.
    void ActionTriggered(UserConnection* connection, Entity* entity, const QString& action, const QStringList& params);
    /// Emitted when the client has sent a network message. PacketId will be 0 if not supported by the networking implementation.
    void NetworkMessageReceived(UserConnection* connection, kNet::packet_id_t packetId, kNet::message_id_t messageId, const char* data, size_t numBytes);
};

/// A kNet user connection.
class TUNDRAPROTOCOL_MODULE_API KNetUserConnection : public UserConnection
{
    Q_OBJECT

public:
    virtual QString ConnectionType() const { return "knet"; }

    /// Message connection.
    Ptr(kNet::MessageConnection) connection;

    /// Queue a network message to be sent to the client. 
    virtual void Send(kNet::message_id_t id, const char* data, size_t numBytes, bool reliable, bool inOrder, unsigned long priority = 100, unsigned long contentID = 0);

public slots:
    /// Starts a benign disconnect procedure (one which waits for the peer acknowledge procedure).
    virtual void Disconnect();

    /// Forcibly kills this connection without notifying the peer.
    virtual void Close();
};
Q_DECLARE_METATYPE(UserConnection*)
