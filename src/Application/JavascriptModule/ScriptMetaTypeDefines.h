/**
 *  For conditions of distribution and use, see copyright notice in LICENSE
 *
 *  @file   ScriptMetaTypeDefines.h
 *  @brief  Registration of the Core API objects to Javascript.
 */

#pragma once

#include <QtScript>

/// Dereferences a boost::shared_ptr<T> and converts it to a QScriptValue appropriate for the QtScript engine to access.
template<typename T>
QScriptValue qScriptValueFromBoostSharedPtr(QScriptEngine *engine, const boost::shared_ptr<T> &ptr)
{
    QScriptValue v = engine->newQObject(ptr.get());
    return v;
}

/// Recovers the boost::shared_ptr<T> of the given QScriptValue of an QObject that's stored in a boost::shared_ptr.
/// For this to be safe, T must derive from boost::enable_shared_from_this<T>.
template<typename T>
void qScriptValueToBoostSharedPtr(const QScriptValue &value, boost::shared_ptr<T> &ptr)
{
    if (!value.isQObject())
    {
        ptr.reset();
        return;
    }

    T *obj = dynamic_cast<T*>(value.toQObject());
    if (!obj)
    {
        ptr.reset();
        return;
    }
    ptr = obj->shared_from_this();
}

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
    QScriptValue ret = engine->newVariant(num);
    return ret;
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

