// For conditions of distribution and use, see copyright notice in LICENSE

#include "LoggingFunctions.h"

#include <QScriptEngine>
#include <QMetaType>

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
    qobject = dynamic_cast<Tp>(value.toQObject());
    if (!qobject)
    {
        // qobject_cast has been observed to fail for some metatypes, such as Entity*, so prefer dynamic_cast.
        // However, to see that there are no regressions from that, check that if dynamic_cast fails, so does qobject_cast
        Tp ptr = qobject_cast<Tp>(value.toQObject());
        assert(!ptr);
        if (ptr)
            ::LogError("qScriptValueToQObject: dynamic_cast was null, but qobject_cast was not!");
    }
}

template <typename Tp>
int qScriptRegisterQObjectMetaType(QScriptEngine *engine, const QScriptValue &prototype = QScriptValue()
#ifndef qdoc
    , Tp * = 0
#endif
    )
{
    return qScriptRegisterMetaType<Tp>(engine, qScriptValueFromQObject, qScriptValueToQObject, prototype);
}

/// Dereferences a shared_ptr<T> and converts it to a QScriptValue appropriate for the QtScript engine to access.
template<typename T>
QScriptValue qScriptValueFromBoostSharedPtr(QScriptEngine *engine, const shared_ptr<T> &ptr)
{
    QScriptValue v = engine->newQObject(ptr.get());
    return v;
}

/// Recovers the shared_ptr<T> of the given QScriptValue of an QObject that's stored in a shared_ptr.
/// For this to be safe, T must derive from enable_shared_from_this<T>.
template<typename T>
void qScriptValueToBoostSharedPtr(const QScriptValue &value, shared_ptr<T> &ptr)
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
    ptr = dynamic_pointer_cast<T>(obj->shared_from_this());
}
