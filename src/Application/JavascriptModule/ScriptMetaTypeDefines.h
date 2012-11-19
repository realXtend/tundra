/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   ScriptMetaTypeDefines.h
    @brief  Registration of the Core API objects to QtScript. */

#pragma once

#include <QtScript>

template<typename T> QScriptValue toScriptValueEnum(QScriptEngine *engine, const T &s)
{
    QScriptValue obj = engine->newObject();
    obj.setProperty("value", QScriptValue(engine, s));
    return obj;
}

template<typename T> void fromScriptValueEnum(const QScriptValue &obj, T &s)
{
    s = static_cast<T>(obj.property("value").toInt32());
}

template<typename T>
QScriptValue toScriptUInt(QScriptEngine *engine, const T &num)
{
    return engine->newVariant(num);
}

template<typename T> 
void fromScriptUInt(const QScriptValue &obj, T &s)
{
    s = obj.toUInt32();
}

/// Register Qt related stuff to QScriptEngine.
/// @todo replace this with qscriptgenerator.
void ExposeQtMetaTypes(QScriptEngine *engine);

/// Will register all meta data types that are needed to use the Core API objects.
void ExposeCoreApiMetaTypes(QScriptEngine *engine);
