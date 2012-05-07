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
Q_DECLARE_METATYPE(MumbleAudio::AudioSettings)
Q_DECLARE_METATYPE(QList<MumbleChannel*>)
Q_DECLARE_METATYPE(QList<MumbleUser*>)
Q_DECLARE_METATYPE(ByteArrayVector)
Q_DECLARE_METATYPE(MumbleNetwork::MumbleUserState)
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

QScriptValue toMumbleSettings(QScriptEngine *engine, const MumbleAudio::AudioSettings &settings)
{
    QScriptValue scriptSettings = engine->newObject();
    scriptSettings.setProperty("quality", (int)settings.quality);
    scriptSettings.setProperty("transmitMode", (int)settings.transmitMode);
    scriptSettings.setProperty("suppression", settings.suppression);
    scriptSettings.setProperty("amplification", settings.amplification);
    scriptSettings.setProperty("VADmin", settings.VADmin);
    scriptSettings.setProperty("VADmax", settings.VADmax);
    scriptSettings.setProperty("innerRange", settings.innerRange);
    scriptSettings.setProperty("outerRange", settings.outerRange);
    scriptSettings.setProperty("allowSendingPositional", settings.allowSendingPositional);
    scriptSettings.setProperty("allowReceivingPositional", settings.allowReceivingPositional);
    scriptSettings.setProperty("recordingDevice", settings.recordingDevice);
    return scriptSettings;
}

void fromMumbleSettings(const QScriptValue &obj, MumbleAudio::AudioSettings &settings)
{
    if (obj.property("quality").isValid() && obj.property("quality").isNumber())
    {
        int quality = obj.property("quality").toInt32();
        if (quality < 1 || quality > 3)
            LogError("fromMumbleSettings(): quality property has to be 1-3.");
        else
            settings.quality = (MumbleAudio::AudioQuality)quality;
    }
    
    if (obj.property("transmitMode").isValid() && obj.property("transmitMode").isNumber())
    {
        int transmitMode = obj.property("transmitMode").toInt32();
        if (transmitMode < 0 || transmitMode > 1)
            LogError("fromMumbleSettings(): transmitMode property has to be 0-1.");
        else
            settings.transmitMode = (MumbleAudio::TransmitMode)transmitMode;
    }
    
    if (obj.property("suppression").isValid() && obj.property("suppression").isNumber())
    {
        int suppression = obj.property("suppression").toInt32();
        if (suppression > -15)
            suppression = 0;
        settings.suppression = suppression;
    }
        
    if (obj.property("amplification").isValid() && obj.property("amplification").isNumber())
        settings.amplification = obj.property("amplification").toInt32();
        
    if (obj.property("VADmin").isValid() && obj.property("VADmin").isNumber())
        settings.VADmin = (float)obj.property("VADmin").toNumber();
        
    if (obj.property("VADmax").isValid() && obj.property("VADmax").isNumber())
        settings.VADmax = (float)obj.property("VADmax").toNumber();
        
    if (obj.property("innerRange").isValid() && obj.property("innerRange").isNumber())
        settings.innerRange = obj.property("innerRange").toInt32();
        
    if (obj.property("outerRange").isValid() && obj.property("outerRange").isNumber())
        settings.outerRange = obj.property("outerRange").toInt32();
        
    if (obj.property("allowSendingPositional").isValid() && obj.property("allowSendingPositional").isBool())
        settings.allowSendingPositional = obj.property("allowSendingPositional").toBool();
        
    if (obj.property("allowReceivingPositional").isValid() && obj.property("allowReceivingPositional").isBool())
        settings.allowReceivingPositional = obj.property("allowReceivingPositional").toBool();
        
    if (obj.property("recordingDevice").isValid() && obj.property("recordingDevice").isString())
        settings.recordingDevice = obj.property("recordingDevice").toString();
}

void RegisterMumblePluginMetaTypes()
{
    qRegisterMetaType<MumbleNetwork::MumbleUserState>("MumbleNetwork::MumbleUserState");
    qRegisterMetaType<MumbleNetwork::ConnectionState>("MumbleNetwork::ConnectionState");
    qRegisterMetaType<MumbleNetwork::RejectReason>("MumbleNetwork::RejectReason");
    qRegisterMetaType<MumbleNetwork::PermissionDeniedType>("MumbleNetwork::PermissionDeniedType");
    qRegisterMetaType<MumbleNetwork::ACLPermission>("MumbleNetwork::ACLPermission");
    qRegisterMetaType<MumbleNetwork::NetworkMode>("MumbleNetwork::NetworkMode");
    qRegisterMetaType<QList<MumbleChannel*> >("QList<MumbleChannel*>");
    qRegisterMetaType<QList<MumbleUser*> >("QList<MumbleUser*>");
    qRegisterMetaType<QList<QByteArray> >("QList<QByteArray>");
    qRegisterMetaType<QList<uint> >("QList<uint>");
    qRegisterMetaType<ByteArrayVector >("ByteArrayList");
    qRegisterMetaType<MumbleAudio::AudioSettings>("MumbleAudio::AudioSettings");
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
        
    // Register mumble settings handlers.
    qScriptRegisterMetaType<MumbleAudio::AudioSettings>(engine, toMumbleSettings, fromMumbleSettings);

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
