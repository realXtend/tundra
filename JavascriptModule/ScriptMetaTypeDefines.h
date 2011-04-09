/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   ScriptMetaTypeDefines.h
 *  @brief  Registration of Naali Core API to Javascript.
 */

#ifndef incl_JavascriptModule_ScriptMetaTypeDefines_h
#define incl_JavascriptModule_ScriptMetaTypeDefines_h

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

// The following functions help register a custom QObject-derived class to a QScriptEngine.
// See http://lists.trolltech.com/qt-interest/2007-12/thread00158-0.html .
template <typename Tp>
QScriptValue qScriptValueFromQObject(QScriptEngine *engine, Tp const &qobject)
{
    return engine->newQObject(qobject);
}

/// @todo qobject_cast needed to replace with dynamic_cast in order to get IComponent type casted.
/// for some reason qobject_cast will return us a null pointer. Figure out why qobject_cast wont work with IComponent.
template <typename Tp>
void qScriptValueToQObject(const QScriptValue &value, Tp &qobject)
{
    qobject = dynamic_cast<Tp>(value.toQObject());
}

template <typename Tp>
int qScriptRegisterQObjectMetaType(QScriptEngine *engine, const QScriptValue &prototype = QScriptValue()
#ifndef qdoc
    , Tp * = 0
#endif
    )
{
    return qScriptRegisterMetaType<Tp>(engine, qScriptValueFromQObject,
                                       qScriptValueToQObject, prototype);
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

//! Register Qt related stuff to QScriptEngine.
//! @todo repalce this with qscriptgenerator.
void ExposeQtMetaTypes(QScriptEngine *engine);

//! Will register all meta data types that are needed to use Naali Core API objects.
void ExposeCoreApiMetaTypes(QScriptEngine *engine);

#endif
