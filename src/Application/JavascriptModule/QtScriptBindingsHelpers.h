#pragma once

#include "Win.h"

#include <QScriptValue>
#include <QScriptContext>
#include <QScriptEngine>
#include <QVariant>

#include <QScriptClass>
Q_DECLARE_METATYPE(QScriptClass*)

///\todo Remove these from here and move them to the programmatically generated files.
#include "Geometry/AABB.h"
#include "Geometry/Circle.h"
#include "Geometry/Capsule.h"
#include "Math/float2.h"
#include "Math/float3.h"
#include "Math/float3x3.h"
#include "Math/float3x4.h"
#include "Math/float4.h"
#include "Math/float4x4.h"
#include "Geometry/Frustum.h"
#include "Geometry/HitInfo.h"
#include "Algorithm/Random/LCG.h"
#include "Geometry/Line.h"
#include "Geometry/LineSegment.h"
#include "Geometry/OBB.h"
#include "Geometry/Plane.h"
#include "Geometry/Polygon.h"
#include "Geometry/Polyhedron.h"
#include "Math/Quat.h"
#include "Geometry/Ray.h"
#include "Geometry/Sphere.h"
#include "Math/TransformOps.h"
#include "Geometry/Triangle.h"
#include "Transform.h"

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

