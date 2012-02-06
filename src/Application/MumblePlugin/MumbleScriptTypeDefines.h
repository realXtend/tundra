// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "MumbleNetwork.h"
#include "MumbleDefines.h"
#include "MumbleData.h"

#include "ScriptMetaTypeDefines.h"
#include "QScriptEngineHelpers.h"

#include <QMetaType>
#include <QList>
#include <QScriptEngine>
#include <QScriptValue>
#include <QScriptValueIterator>

Q_DECLARE_METATYPE(MumbleChannel*)
Q_DECLARE_METATYPE(MumbleUser*)
Q_DECLARE_METATYPE(QList<MumbleChannel*>)
Q_DECLARE_METATYPE(QList<MumbleUser*>)
Q_DECLARE_METATYPE(ByteArrayList)
Q_DECLARE_METATYPE(MumbleNetwork::ConnectionState)
Q_DECLARE_METATYPE(MumbleNetwork::RejectReason)
Q_DECLARE_METATYPE(MumbleNetwork::PermissionDeniedType)
Q_DECLARE_METATYPE(MumbleNetwork::ACLPermission)
Q_DECLARE_METATYPE(MumbleNetwork::NetworkMode)

QScriptValue toScriptValueMumbleChannelList(QScriptEngine *engine, const QList<MumbleChannel*> &channels)
{
    QScriptValue obj = engine->newArray(channels.size());
    for(int i=0; i<channels.size(); ++i)
    {
        MumbleChannel *channel = channels.at(i);
        if (channel)
            obj.setProperty(i, engine->newQObject(channel));
    }
    return obj;
}

void fromScriptValueMumbleChannelList(const QScriptValue &obj, QList<MumbleChannel*> &channels)
{
    channels.clear();
    QScriptValueIterator it(obj);
    while(it.hasNext())
    {
        it.next();
        MumbleChannel *qobj = qobject_cast<MumbleChannel*>(it.value().toQObject());
        if (qobj)
            channels.append(qobj);
    }
}

QScriptValue toScriptValueMumbleUserList(QScriptEngine *engine, const QList<MumbleUser*> &users)
{
    QScriptValue obj = engine->newArray(users.size());
    for(int i=0; i<users.size(); ++i)
    {
        MumbleUser *user = users.at(i);
        if (user)
            obj.setProperty(i, engine->newQObject(user));
    }
    return obj;
}

void fromScriptValueMumbleUserList(const QScriptValue &obj, QList<MumbleUser*> &users)
{
    users.clear();
    QScriptValueIterator it(obj);
    while(it.hasNext())
    {
        it.next();
        MumbleUser *qobj = qobject_cast<MumbleUser*>(it.value().toQObject());
        if (qobj)
            users.append(qobj);
    }
}

void RegisterMumblePluginMetaTypes()
{
    qRegisterMetaType<MumbleNetwork::ConnectionState>("MumbleNetwork::ConnectionState");
    qRegisterMetaType<MumbleNetwork::RejectReason>("MumbleNetwork::RejectReason");
    qRegisterMetaType<MumbleNetwork::PermissionDeniedType>("MumbleNetwork::PermissionDeniedType");
    qRegisterMetaType<MumbleNetwork::ACLPermission>("MumbleNetwork::ACLPermission");
    qRegisterMetaType<MumbleNetwork::NetworkMode>("MumbleNetwork::NetworkMode");
    qRegisterMetaType<QList<MumbleChannel*> >("QList<MumbleChannel*>");
    qRegisterMetaType<QList<MumbleUser*> >("QList<MumbleUser*>");
    qRegisterMetaType<QList<QByteArray> >("QList<QByteArray>");
    qRegisterMetaType<QList<uint> >("QList<uint>");
    qRegisterMetaType<ByteArrayList >("ByteArrayList");
}

void RegisterMumblePluginMetaTypes(QScriptEngine *engine)
{
    // Register custom QObjects
    qScriptRegisterQObjectMetaType<MumbleChannel*>(engine);
    qScriptRegisterQObjectMetaType<MumbleUser*>(engine);

    // Register custom QLists
    qScriptRegisterMetaType<QList<MumbleChannel*> >(engine, toScriptValueMumbleChannelList, 
        fromScriptValueMumbleChannelList);
    qScriptRegisterMetaType<QList<MumbleUser*> >(engine, toScriptValueMumbleUserList, 
        fromScriptValueMumbleUserList);

    // Register enums
    qScriptRegisterMetaType(engine, toScriptValueEnum<MumbleNetwork::ConnectionState>, 
        fromScriptValueEnum<MumbleNetwork::ConnectionState>);
    qScriptRegisterMetaType(engine, toScriptValueEnum<MumbleNetwork::RejectReason>, 
        fromScriptValueEnum<MumbleNetwork::RejectReason>);
    qScriptRegisterMetaType(engine, toScriptValueEnum<MumbleNetwork::PermissionDeniedType>, 
        fromScriptValueEnum<MumbleNetwork::PermissionDeniedType>);
    qScriptRegisterMetaType(engine, toScriptValueEnum<MumbleNetwork::ACLPermission>, 
        fromScriptValueEnum<MumbleNetwork::ACLPermission>);
    qScriptRegisterMetaType(engine, toScriptValueEnum<MumbleNetwork::NetworkMode>, 
        fromScriptValueEnum<MumbleNetwork::NetworkMode>);
}
