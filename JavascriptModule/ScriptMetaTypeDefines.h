/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   ScriptMetaTypeDefines.h
 *  @brief  Registration of Naali Core API to Javascript.
 */

#ifndef incl_JavascriptModule_ScriptMetaTypeDefines_h
#define incl_JavascriptModule_ScriptMetaTypeDefines_h

#include <QtScript>

// The following functions help register a custom QObject-derived class to a QScriptEngine.
// See http://lists.trolltech.com/qt-interest/2007-12/thread00158-0.html .
template <typename Tp>
QScriptValue qScriptValueFromQObject(QScriptEngine *engine, Tp const &qobject)
{
    return engine->newQObject(qobject);
}

template <typename Tp>
void qScriptValueToQObject(const QScriptValue &value, Tp &qobject)
{   
    qobject = qobject_cast<Tp>(value.toQObject());
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
