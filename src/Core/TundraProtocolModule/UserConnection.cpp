// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "UserConnection.h"

#include "Entity.h"
#include "LoggingFunctions.h"

#include "MemoryLeakCheck.h"

void UserConnection::Exec(Entity *entity, const QString &action, const QStringList &params)
{
    if (entity)
        emit ActionTriggered(this, entity, action, params);
    else
        LogWarning("UserConnection::Exec: null entity passed!");
}

void UserConnection::Exec(Entity *entity, const QString &action, const QString &p1, const QString &p2, const QString &p3)
{
    Exec(entity, action, QStringList(QStringList() << p1 << p2 << p3));
}

void UserConnection::SetProperty(const QString& key, const QString& value)
{
    properties[key] = value;
}

QString UserConnection::Property(const QString& key) const
{
    static QString empty;
    
    LoginPropertyMap::const_iterator i = properties.find(key);
    if (i != properties.end())
        return i->second;
    else
        return empty;
}

void UserConnection::DenyConnection(const QString &reason)
{
    properties["authenticated"] = "false";
    properties["reason"] = reason;
}

void UserConnection::Disconnect()
{
    if (connection)
        connection->Disconnect(0);
}

void UserConnection::Close()
{
    if (connection)
        connection->Close(0);
}
