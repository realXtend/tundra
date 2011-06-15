#pragma once

#include <QScriptValue>
#include <QScriptContext>
#include <QScriptEngine>
#include <QVariant>

#include <QScriptClass>
Q_DECLARE_METATYPE(QScriptClass*)

///\todo Remove these from here and move them to the programmatically generated files.
#include "Math/AABB.h"
#include "Math/Circle.h"
#include "Math/Cone.h"
#include "Math/Cylinder.h"
#include "Math/Ellipsoid.h"
#include "Math/float2.h"
#include "Math/float3.h"
#include "Math/float3x3.h"
#include "Math/float3x4.h"
#include "Math/float4.h"
#include "Math/float4x4.h"
#include "Math/Frustum.h"
#include "Math/HitInfo.h"
#include "Math/LCG.h"
#include "Math/Line.h"
#include "Math/LineSegment.h"
#include "Math/OBB.h"
#include "Math/Plane.h"
#include "Math/Quat.h"
#include "Math/Ray.h"
#include "Math/Sphere.h"
#include "Math/TransformOps.h"
#include "Math/Triangle.h"
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

// A function to help the automatically generated code produce cleaner error reporting.
inline std::string Capitalize(QString str)
{
    return (str.left(1).toUpper() + str.mid(1)).toStdString();
}
