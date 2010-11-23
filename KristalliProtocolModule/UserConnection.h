// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_KristalliProtocolModule_UserConnection_h
#define incl_KristalliProtocolModule_UserConnection_h

#include "Foundation.h"
#include "KristalliProtocolModuleApi.h"
#include "kNet.h"

#include <QObject>

namespace kNet
{
class MessageConnection;
}

namespace Scene
{
class Entity;
}

//! Interface class for user's currently replicated scene state
struct ISyncState
{
    virtual ~ISyncState() {}
};

//! Connection on the Kristalli server
class KRISTALLIPROTOCOL_MODULE_API UserConnection : public QObject
{
    Q_OBJECT
    
public:
    UserConnection() :
        userID(0)
    {
    }
    
    /// Message connection
    kNet::MessageConnection* connection;
    /// Connection ID
    u8 userID;
    /// User's name
    QString userName;
    /// Any extra properties
    std::map<QString, QString> properties;
    /// Scene sync state
    boost::shared_ptr<ISyncState> syncState;
    
public slots:
    /// Execute an action on an entity, sent only to the specific user
    void Exec(QObject* entity, const QString &action, const QString &p1 = "", const QString &p2 = "", const QString &p3 = "");
    
    /// Execute an action on an entity, sent only to the specific user
    void Exec(QObject* entity, const QString &action, const QStringList &params);
    
    /// Get username
    QString GetName() const;
    
    /// Set a property
    void SetProperty(const QString& key, const QString& value);
    
    /// Get a property
    QString GetProperty(const QString& key) const;
    
signals:
    void ActionTriggered(UserConnection* connection, Scene::Entity* entity, const QString& action, const QStringList& params);
};

typedef std::list<UserConnection*> UserConnectionList;

#endif

