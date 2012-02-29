// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "ScriptMetaTypeDefines.h"
#include "QScriptEngineHelpers.h"

#include <QMetaType>
#include <QList>
#include <QScriptEngine>
#include <QScriptValue>
#include <QScriptValueIterator>

Q_DECLARE_METATYPE(QextSerialPort*)
Q_DECLARE_METATYPE(QSerialPortInfo*)
Q_DECLARE_METATYPE(QList<QSerialPortInfo*>)
Q_DECLARE_METATYPE(QextSerialPort::QueryMode)
Q_DECLARE_METATYPE(BaudRateType)
Q_DECLARE_METATYPE(DataBitsType)
Q_DECLARE_METATYPE(ParityType)
Q_DECLARE_METATYPE(StopBitsType)
Q_DECLARE_METATYPE(FlowType)

QScriptValue toScriptValueSerialPortList(QScriptEngine *engine, const QList<QSerialPortInfo*> &portInfos)
{
    QScriptValue obj = engine->newArray(portInfos.size());
    for(int i=0; i<portInfos.size(); ++i)
    {
        QSerialPortInfo *portInfo = portInfos.at(i);
        if (portInfo)
            obj.setProperty(i, engine->newQObject(portInfo));
    }
    return obj;
}

void fromScriptValueSerialPortList(const QScriptValue &obj, QList<QSerialPortInfo*> &portInfos)
{
    // no op
}

void RegisterSerialDevicePluginMetaTypes()
{
    qRegisterMetaType<QextSerialPort*>("QextSerialPort*");
    qRegisterMetaType<QSerialPortInfo*>("QSerialPortInfo*");
    qRegisterMetaType<QList<QextSerialPort*> >("QList<QextSerialPort*>");
    qRegisterMetaType<QextSerialPort::QueryMode>("QextSerialPort::QueryMode");
    qRegisterMetaType<BaudRateType>("BaudRateType");
    qRegisterMetaType<DataBitsType>("DataBitsType");
    qRegisterMetaType<ParityType>("ParityType");
    qRegisterMetaType<StopBitsType>("StopBitsType");
    qRegisterMetaType<FlowType>("FlowType");
}

void RegisterSerialDevicePluginMetaTypes(QScriptEngine *engine)
{
    // Register custom QObjects
    qScriptRegisterQObjectMetaType<QextSerialPort*>(engine);
    qScriptRegisterQObjectMetaType<QSerialPortInfo*>(engine);

    // Register custom QLists
    qScriptRegisterMetaType<QList<QSerialPortInfo*> >(engine, toScriptValueSerialPortList, fromScriptValueSerialPortList);

    // Register custom enums
    qScriptRegisterMetaType(engine, toScriptValueEnum<QextSerialPort::QueryMode>, fromScriptValueEnum<QextSerialPort::QueryMode>);
    qScriptRegisterMetaType(engine, toScriptValueEnum<BaudRateType>, fromScriptValueEnum<BaudRateType>);
    qScriptRegisterMetaType(engine, toScriptValueEnum<DataBitsType>, fromScriptValueEnum<DataBitsType>);
    qScriptRegisterMetaType(engine, toScriptValueEnum<ParityType>, fromScriptValueEnum<ParityType>);
    qScriptRegisterMetaType(engine, toScriptValueEnum<StopBitsType>, fromScriptValueEnum<StopBitsType>);
    qScriptRegisterMetaType(engine, toScriptValueEnum<FlowType>, fromScriptValueEnum<FlowType>);
}
