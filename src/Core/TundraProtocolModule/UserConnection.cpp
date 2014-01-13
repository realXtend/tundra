// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "UserConnection.h"

#include "Entity.h"
#include "LoggingFunctions.h"

#include "MemoryLeakCheck.h"

void UserConnection::Send(kNet::message_id_t id, bool reliable, bool inOrder, kNet::DataSerializer& ds, unsigned long priority, unsigned long contentID)
{
    Send(id, ds.GetData(), ds.BytesFilled(), reliable, inOrder, priority, contentID);
}

void UserConnection::EmitNetworkMessageReceived(kNet::packet_id_t packetId, kNet::message_id_t messageId, const char* data, size_t numBytes)
{
    emit NetworkMessageReceived(packetId, messageId, data, numBytes);
}

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

QVariant UserConnection::Property(const QString& key) const
{
    static QString empty;
    
    LoginPropertyMap::const_iterator i = properties.find(key);
    if (i != properties.end())
        return i.value();
    else
        return empty;
}

bool UserConnection::HasProperty(const QString& key) const
{
    LoginPropertyMap::const_iterator i = properties.find(key);
    if (i == properties.end())
        return false;
    else
        return i->isValid();
}

void UserConnection::DenyConnection(const QString &reason)
{
    properties["authenticated"] = false;
    properties["reason"] = reason;
}

void KNetUserConnection::Send(kNet::message_id_t id, const char* data, size_t numBytes, bool reliable, bool inOrder, unsigned long priority, unsigned long contentID)
{
    if (!data && numBytes)
    {
        LogError("KNetUserConnection::Send: can not queue message, null data pointer with nonzero data size specified");
        return;
    }

    if (!connection)
    {
        LogError("KNetUserConnection::Send: can not queue message as MessageConnection is null");
        return;
    }

    kNet::NetworkMessage* msg = connection->StartNewMessage(id, numBytes);
    if (numBytes)
        memcpy(msg->data, data, numBytes); /// \todo Copy should be optimized out
    msg->reliable = reliable;
    msg->inOrder = inOrder;
    msg->priority = priority;
    msg->contentID = contentID;
    connection->EndAndQueueMessage(msg);
}

void KNetUserConnection::Disconnect()
{
    if (connection)
        connection->Disconnect(0);
}


void KNetUserConnection::Close()
{
    if (connection)
        connection->Close(0);
}
