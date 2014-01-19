/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   QtScriptBindingsHelpers.h
    @brief  QtScript bindings utilities. */

#pragma once

#include "Win.h"

#include <QStringList>
#include <QScriptValue>
#include <QScriptContext>
#include <QScriptEngine>
#include <QVariant>
#include <QScriptClass>
Q_DECLARE_METATYPE(QScriptClass*)

class float3;
class LineSegment;

template<typename T>
bool QSVIsOfType(const QScriptValue &value)
{
    // For the math classes, we expose the actual type as a member property, since we are not using
    // the opaque QVariant-based interop.
    // For basic types, like float and int, the latter value.toVariant().canConvert<T> is used.
    return (value.prototype().property("metaTypeId").toInt32() == qMetaTypeId<T>() || value.toVariant().canConvert<T>());
}

QScriptValue ToScriptValue_const_float3(QScriptEngine *engine, const float3 &value);
QScriptValue ToScriptValue_const_LineSegment(QScriptEngine *engine, const LineSegment &value);

// A function to help the automatically generated code produce cleaner error reporting.
inline std::string Capitalize(QString str)
{
    return (str.left(1).toUpper() + str.mid(1)).toStdString();
}

inline void PrintCallStack(const QStringList &callStack)
{
    foreach(QString i, callStack)
    {
        printf("Call stack:\n");
        printf("   %s\n", i.toStdString().c_str());
    }
}
