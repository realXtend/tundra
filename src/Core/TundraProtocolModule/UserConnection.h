// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "CoreTypes.h"
#include "TundraProtocolModuleApi.h"
#include "TundraProtocolModuleFwd.h"

#include <kNet/SharedPtr.h>
#include <kNet/MessageConnection.h>

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <QObject>

class Entity;
class SceneSyncState;

/// Represents a client conncetion on the server side.
class TUNDRAPROTOCOL_MODULE_API UserConnection : public QObject, public boost::enable_shared_from_this<UserConnection>
{
    Q_OBJECT
    
public:
    /// Connection ID property
    Q_PROPERTY (int id READ GetConnectionID)
    
    UserConnection() :
        userID(0)
    {
    }
    
    /// Message connection
    Ptr(kNet::MessageConnection) connection;
    /// Connection ID
    u8 userID;
    /// Raw xml login data
    QString loginData;
    /// Property map
    std::map<QString, QString> properties;
    /// Scene sync state, created and used by the SyncManager
    boost::shared_ptr<SceneSyncState> syncState;
    
public slots:
    /// Execute an action on an entity, sent only to the specific user
    void Exec(QObject* entity, const QString &action, const QString &p1 = "", const QString &p2 = "", const QString &p3 = "");
    
    /// Execute an action on an entity, sent only to the specific user
    void Exec(QObject* entity, const QString &action, const QStringList &params);
    
    /// Get connection id
    int GetConnectionID() const;
    
    /// Get raw login data
    QString GetLoginData() const;
     
    /// Set a property
    void SetProperty(const QString& key, const QString& value);
    
    /// Get a property
    QString GetProperty(const QString& key) const;
    
    /// Deny connection. Call as a response to server.UserAboutToConnect() if necessary
    void DenyConnection(const QString& reason);
    
    /// Starts a benign disconnect procedure (one which waits for the peer acknowledge procedure).
    void Disconnect();

    /// Forcibly kills this connection without notifying the peer.
    void Close();

signals:
    void ActionTriggered(UserConnection* connection, Entity* entity, const QString& action, const QStringList& params);
};
