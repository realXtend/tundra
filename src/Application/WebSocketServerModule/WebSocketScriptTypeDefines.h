// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "WebSocketServer.h"
#include "WebSocketUserConnection.h"
#include "WebSocketSyncManager.h"

#include "QScriptEngineHelpers.h"

#include <QScriptEngine>
#include <QScriptValue>
#include <QScriptValueIterator>
#include <QVariantMap>

Q_DECLARE_METATYPE(WebSocket::Server*)
Q_DECLARE_METATYPE(WebSocket::UserConnection*)
Q_DECLARE_METATYPE(WebSocket::SyncManager*)
Q_DECLARE_METATYPE(WebSocketServerPtr)
Q_DECLARE_METATYPE(WebSocketSyncManagerPtr)
Q_DECLARE_METATYPE(WebSocketUserConnectionPtr)
Q_DECLARE_METATYPE(WebSocket::UserConnectionList)
Q_DECLARE_METATYPE(QVariantMap*)

void fromScriptValueWebSocketUserConnectionList(const QScriptValue &obj, WebSocket::UserConnectionList &cons)
{
    cons.clear();
    QScriptValueIterator it(obj);
    while(it.hasNext())
    {
        it.next();
        WebSocket::UserConnection *u = qobject_cast<WebSocket::UserConnection*>(it.value().toQObject());
        if (u)
            cons.push_back(u);
    }
}

QScriptValue toScriptValueWebSocketUserConnectionList(QScriptEngine *engine, const WebSocket::UserConnectionList &cons)
{
    QScriptValue scriptValue = engine->newArray();
    int i = 0;
    for(WebSocket::UserConnectionList::const_iterator iter = cons.begin(); iter != cons.end(); ++iter)
    {
        scriptValue.setProperty(i, engine->newQObject((*iter)));
        ++i;
    }
    return scriptValue;
}

template<typename T>
QScriptValue qScriptValueFromNull(QScriptEngine *engine, const T& /*v*/)
{
    return engine->newObject();
}

template<typename T>
void qScriptValueToNull(const QScriptValue& /*value*/, T& /*v*/)
{
}

void RegisterWebSocketPluginMetaTypes(QScriptEngine *engine)
{
    // Custom QObjects
    qScriptRegisterQObjectMetaType<WebSocket::Server*>(engine);
    qScriptRegisterQObjectMetaType<WebSocket::UserConnection*>(engine);
    qScriptRegisterQObjectMetaType<WebSocket::SyncManager*>(engine);
    
    // Custom boost ptrs
    qScriptRegisterMetaType<WebSocketServerPtr>(engine, qScriptValueFromBoostSharedPtr, qScriptValueToBoostSharedPtr);
    qScriptRegisterMetaType<WebSocketSyncManagerPtr>(engine, qScriptValueFromBoostSharedPtr, qScriptValueToBoostSharedPtr);
    qScriptRegisterMetaType<WebSocketUserConnectionPtr>(engine, qScriptValueFromBoostSharedPtr, qScriptValueToBoostSharedPtr);
    
    // Custom std::vector of user connections
    qScriptRegisterMetaType<WebSocket::UserConnectionList>(engine, toScriptValueWebSocketUserConnectionList, fromScriptValueWebSocketUserConnectionList);
    qScriptRegisterMetaType<QVariantMap*>(engine, qScriptValueFromNull<QVariantMap*>, qScriptValueToNull<QVariantMap*>);
}
