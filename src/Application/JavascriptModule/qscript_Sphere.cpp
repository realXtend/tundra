#include "QtScriptBindingsHelpers.h"

void ToExistingScriptValue_Sphere(QScriptEngine *engine, const Sphere &value, QScriptValue obj)
{
    obj.setProperty("pos", qScriptValueFromValue(engine, value.pos), QScriptValue::Undeletable);
    obj.setProperty("r", qScriptValueFromValue(engine, value.r), QScriptValue::Undeletable);
}

static QScriptValue Sphere_Sphere(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Sphere_Sphere in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere ret;
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Sphere_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Sphere_Sphere_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 center = qscriptvalue_cast<float3>(context->argument(0));
    float radius = qscriptvalue_cast<float>(context->argument(1));
    Sphere ret(center, radius);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Sphere_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Sphere_Sphere_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 pointA = qscriptvalue_cast<float3>(context->argument(0));
    float3 pointB = qscriptvalue_cast<float3>(context->argument(1));
    Sphere ret(pointA, pointB);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Sphere_float3_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Sphere_Sphere_float3_float3_float3 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 pointA = qscriptvalue_cast<float3>(context->argument(0));
    float3 pointB = qscriptvalue_cast<float3>(context->argument(1));
    float3 pointC = qscriptvalue_cast<float3>(context->argument(2));
    Sphere ret(pointA, pointB, pointC);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Sphere_float3_float3_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function Sphere_Sphere_float3_float3_float3_float3 in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 pointA = qscriptvalue_cast<float3>(context->argument(0));
    float3 pointB = qscriptvalue_cast<float3>(context->argument(1));
    float3 pointC = qscriptvalue_cast<float3>(context->argument(2));
    float3 pointD = qscriptvalue_cast<float3>(context->argument(3));
    Sphere ret(pointA, pointB, pointC, pointD);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_MinimalEnclosingAABB_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Sphere_MinimalEnclosingAABB_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    AABB ret = This.MinimalEnclosingAABB();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_MaximalContainedAABB_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Sphere_MaximalContainedAABB_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    AABB ret = This.MaximalContainedAABB();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_SetNegativeInfinity(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Sphere_SetNegativeInfinity in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    This.SetNegativeInfinity();
    ToExistingScriptValue_Sphere(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Sphere_Volume_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Sphere_Volume_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    float ret = This.Volume();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_SurfaceArea_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Sphere_SurfaceArea_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    float ret = This.SurfaceArea();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Centroid_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Sphere_Centroid_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    float3 ret = This.Centroid();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_ExtremePoint_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_ExtremePoint_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    float3 direction = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.ExtremePoint(direction);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_IsFinite_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Sphere_IsFinite_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    bool ret = This.IsFinite();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_IsDegenerate_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Sphere_IsDegenerate_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    bool ret = This.IsDegenerate();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Contains_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Contains_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    bool ret = This.Contains(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Contains_LineSegment_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Contains_LineSegment_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    LineSegment lineSegment = qscriptvalue_cast<LineSegment>(context->argument(0));
    bool ret = This.Contains(lineSegment);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Contains_Triangle_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Contains_Triangle_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    Triangle triangle = qscriptvalue_cast<Triangle>(context->argument(0));
    bool ret = This.Contains(triangle);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Contains_Polygon_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Contains_Polygon_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    Polygon polygon = qscriptvalue_cast<Polygon>(context->argument(0));
    bool ret = This.Contains(polygon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Contains_AABB_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Contains_AABB_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    AABB aabb = qscriptvalue_cast<AABB>(context->argument(0));
    bool ret = This.Contains(aabb);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Contains_OBB_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Contains_OBB_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    OBB obb = qscriptvalue_cast<OBB>(context->argument(0));
    bool ret = This.Contains(obb);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Contains_Frustum_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Contains_Frustum_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    Frustum frustum = qscriptvalue_cast<Frustum>(context->argument(0));
    bool ret = This.Contains(frustum);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Contains_Polyhedron_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Contains_Polyhedron_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    Polyhedron polyhedron = qscriptvalue_cast<Polyhedron>(context->argument(0));
    bool ret = This.Contains(polyhedron);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Contains_Sphere_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Contains_Sphere_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    Sphere sphere = qscriptvalue_cast<Sphere>(context->argument(0));
    bool ret = This.Contains(sphere);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Contains_Capsule_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Contains_Capsule_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    Capsule capsule = qscriptvalue_cast<Capsule>(context->argument(0));
    bool ret = This.Contains(capsule);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Distance_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Distance_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float ret = This.Distance(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Distance_Sphere_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Distance_Sphere_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    Sphere sphere = qscriptvalue_cast<Sphere>(context->argument(0));
    float ret = This.Distance(sphere);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Distance_Capsule_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Distance_Capsule_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    Capsule capsule = qscriptvalue_cast<Capsule>(context->argument(0));
    float ret = This.Distance(capsule);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Distance_AABB_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Distance_AABB_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    AABB aabb = qscriptvalue_cast<AABB>(context->argument(0));
    float ret = This.Distance(aabb);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Distance_OBB_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Distance_OBB_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    OBB obb = qscriptvalue_cast<OBB>(context->argument(0));
    float ret = This.Distance(obb);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Distance_Plane_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Distance_Plane_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    Plane plane = qscriptvalue_cast<Plane>(context->argument(0));
    float ret = This.Distance(plane);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Distance_Triangle_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Distance_Triangle_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    Triangle triangle = qscriptvalue_cast<Triangle>(context->argument(0));
    float ret = This.Distance(triangle);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Distance_Ray_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Distance_Ray_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    Ray ray = qscriptvalue_cast<Ray>(context->argument(0));
    float ret = This.Distance(ray);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Distance_Line_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Distance_Line_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    Line line = qscriptvalue_cast<Line>(context->argument(0));
    float ret = This.Distance(line);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Distance_LineSegment_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Distance_LineSegment_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    LineSegment lineSegment = qscriptvalue_cast<LineSegment>(context->argument(0));
    float ret = This.Distance(lineSegment);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_ClosestPoint_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_ClosestPoint_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.ClosestPoint(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Intersects_Plane_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Intersects_Plane_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    Plane plane = qscriptvalue_cast<Plane>(context->argument(0));
    bool ret = This.Intersects(plane);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Intersects_Capsule_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Intersects_Capsule_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    Capsule capsule = qscriptvalue_cast<Capsule>(context->argument(0));
    bool ret = This.Intersects(capsule);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Intersects_Polygon_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Intersects_Polygon_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    Polygon polygon = qscriptvalue_cast<Polygon>(context->argument(0));
    bool ret = This.Intersects(polygon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Intersects_Frustum_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Intersects_Frustum_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    Frustum frustum = qscriptvalue_cast<Frustum>(context->argument(0));
    bool ret = This.Intersects(frustum);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Intersects_Polyhedron_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Intersects_Polyhedron_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    Polyhedron polyhedron = qscriptvalue_cast<Polyhedron>(context->argument(0));
    bool ret = This.Intersects(polyhedron);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Intersects_Sphere_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Intersects_Sphere_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    Sphere sphere = qscriptvalue_cast<Sphere>(context->argument(0));
    bool ret = This.Intersects(sphere);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Enclose_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Enclose_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    This.Enclose(point);
    ToExistingScriptValue_Sphere(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Sphere_Enclose_LineSegment(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Enclose_LineSegment in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    LineSegment lineSegment = qscriptvalue_cast<LineSegment>(context->argument(0));
    This.Enclose(lineSegment);
    ToExistingScriptValue_Sphere(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Sphere_Enclose_AABB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Enclose_AABB in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    AABB aabb = qscriptvalue_cast<AABB>(context->argument(0));
    This.Enclose(aabb);
    ToExistingScriptValue_Sphere(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Sphere_Enclose_OBB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Enclose_OBB in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    OBB obb = qscriptvalue_cast<OBB>(context->argument(0));
    This.Enclose(obb);
    ToExistingScriptValue_Sphere(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Sphere_Enclose_Sphere(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Enclose_Sphere in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    Sphere sphere = qscriptvalue_cast<Sphere>(context->argument(0));
    This.Enclose(sphere);
    ToExistingScriptValue_Sphere(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Sphere_RandomPointInside_LCG(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_RandomPointInside_LCG in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    LCG lcg = qscriptvalue_cast<LCG>(context->argument(0));
    float3 ret = This.RandomPointInside(lcg);
    ToExistingScriptValue_Sphere(engine, This, context->thisObject());
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_RandomPointOnSurface_LCG(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_RandomPointOnSurface_LCG in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    LCG lcg = qscriptvalue_cast<LCG>(context->argument(0));
    float3 ret = This.RandomPointOnSurface(lcg);
    ToExistingScriptValue_Sphere(engine, This, context->thisObject());
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_toString_const(QScriptContext *context, QScriptEngine *engine)
{
    Sphere This;
    if (context->argumentCount() > 0) This = qscriptvalue_cast<Sphere>(context->argument(0)); // Qt oddity (bug?): Sometimes the built-in toString() function doesn't give us this from thisObject, but as the first argument.
    else This = qscriptvalue_cast<Sphere>(context->thisObject());
    QString ret = This.toString();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_RandomPointInside_LCG_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Sphere_RandomPointInside_LCG_float3_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LCG lcg = qscriptvalue_cast<LCG>(context->argument(0));
    float3 center = qscriptvalue_cast<float3>(context->argument(1));
    float radius = qscriptvalue_cast<float>(context->argument(2));
    float3 ret = Sphere::RandomPointInside(lcg, center, radius);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_RandomPointOnSurface_LCG_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Sphere_RandomPointOnSurface_LCG_float3_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LCG lcg = qscriptvalue_cast<LCG>(context->argument(0));
    float3 center = qscriptvalue_cast<float3>(context->argument(1));
    float radius = qscriptvalue_cast<float>(context->argument(2));
    float3 ret = Sphere::RandomPointOnSurface(lcg, center, radius);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_RandomUnitaryFloat3_LCG(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_RandomUnitaryFloat3_LCG in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LCG lcg = qscriptvalue_cast<LCG>(context->argument(0));
    float3 ret = Sphere::RandomUnitaryFloat3(lcg);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_ctor(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return Sphere_Sphere(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return Sphere_Sphere_float3_float(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return Sphere_Sphere_float3_float3(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)))
        return Sphere_Sphere_float3_float3_float3(context, engine);
    if (context->argumentCount() == 4 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)) && QSVIsOfType<float3>(context->argument(3)))
        return Sphere_Sphere_float3_float3_float3_float3(context, engine);
    printf("Sphere_ctor failed to choose the right function to call! Did you use 'var x = Sphere();' instead of 'var x = new Sphere();'?\n"); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Sphere_Contains_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return Sphere_Contains_float3_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<LineSegment>(context->argument(0)))
        return Sphere_Contains_LineSegment_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Triangle>(context->argument(0)))
        return Sphere_Contains_Triangle_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Polygon>(context->argument(0)))
        return Sphere_Contains_Polygon_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<AABB>(context->argument(0)))
        return Sphere_Contains_AABB_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<OBB>(context->argument(0)))
        return Sphere_Contains_OBB_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Frustum>(context->argument(0)))
        return Sphere_Contains_Frustum_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Polyhedron>(context->argument(0)))
        return Sphere_Contains_Polyhedron_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Sphere>(context->argument(0)))
        return Sphere_Contains_Sphere_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Capsule>(context->argument(0)))
        return Sphere_Contains_Capsule_const(context, engine);
    printf("Sphere_Contains_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Sphere_Distance_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return Sphere_Distance_float3_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Sphere>(context->argument(0)))
        return Sphere_Distance_Sphere_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Capsule>(context->argument(0)))
        return Sphere_Distance_Capsule_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<AABB>(context->argument(0)))
        return Sphere_Distance_AABB_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<OBB>(context->argument(0)))
        return Sphere_Distance_OBB_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Plane>(context->argument(0)))
        return Sphere_Distance_Plane_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Triangle>(context->argument(0)))
        return Sphere_Distance_Triangle_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Ray>(context->argument(0)))
        return Sphere_Distance_Ray_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Line>(context->argument(0)))
        return Sphere_Distance_Line_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<LineSegment>(context->argument(0)))
        return Sphere_Distance_LineSegment_const(context, engine);
    printf("Sphere_Distance_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Sphere_Intersects_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<Plane>(context->argument(0)))
        return Sphere_Intersects_Plane_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Capsule>(context->argument(0)))
        return Sphere_Intersects_Capsule_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Polygon>(context->argument(0)))
        return Sphere_Intersects_Polygon_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Frustum>(context->argument(0)))
        return Sphere_Intersects_Frustum_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Polyhedron>(context->argument(0)))
        return Sphere_Intersects_Polyhedron_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Sphere>(context->argument(0)))
        return Sphere_Intersects_Sphere_const(context, engine);
    printf("Sphere_Intersects_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Sphere_Enclose_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return Sphere_Enclose_float3(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<LineSegment>(context->argument(0)))
        return Sphere_Enclose_LineSegment(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<AABB>(context->argument(0)))
        return Sphere_Enclose_AABB(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<OBB>(context->argument(0)))
        return Sphere_Enclose_OBB(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Sphere>(context->argument(0)))
        return Sphere_Enclose_Sphere(context, engine);
    printf("Sphere_Enclose_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Sphere_RandomPointInside_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<LCG>(context->argument(0)))
        return Sphere_RandomPointInside_LCG(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<LCG>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return Sphere_RandomPointInside_LCG_float3_float(context, engine);
    printf("Sphere_RandomPointInside_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Sphere_RandomPointOnSurface_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<LCG>(context->argument(0)))
        return Sphere_RandomPointOnSurface_LCG(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<LCG>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return Sphere_RandomPointOnSurface_LCG_float3_float(context, engine);
    printf("Sphere_RandomPointOnSurface_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

void FromScriptValue_Sphere(const QScriptValue &obj, Sphere &value)
{
    value.pos = qScriptValueToValue<float3>(obj.property("pos"));
    value.r = qScriptValueToValue<float>(obj.property("r"));
}

QScriptValue ToScriptValue_Sphere(QScriptEngine *engine, const Sphere &value)
{
    QScriptValue obj = engine->newVariant(QVariant::fromValue(value)); // The contents of this variant are NOT used. The real data lies in the data() pointer of this QScriptValue. This only exists to enable overload resolution to work for QObject slots.
    ToExistingScriptValue_Sphere(engine, value, obj);
    return obj;
}

QScriptValue ToScriptValue_const_Sphere(QScriptEngine *engine, const Sphere &value)
{
    QScriptValue obj = engine->newVariant(QVariant::fromValue(value)); // The contents of this variant are NOT used. The real data lies in the data() pointer of this QScriptValue. This only exists to enable overload resolution to work for QObject slots.
    obj.setPrototype(engine->defaultPrototype(qMetaTypeId<Sphere>()));
    obj.setProperty("pos", ToScriptValue_const_float3(engine, value.pos), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("r", qScriptValueFromValue(engine, value.r), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    return obj;
}

QScriptValue register_Sphere_prototype(QScriptEngine *engine)
{
    QScriptValue proto = engine->newObject();
    proto.setProperty("MinimalEnclosingAABB", engine->newFunction(Sphere_MinimalEnclosingAABB_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MaximalContainedAABB", engine->newFunction(Sphere_MaximalContainedAABB_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetNegativeInfinity", engine->newFunction(Sphere_SetNegativeInfinity, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Volume", engine->newFunction(Sphere_Volume_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SurfaceArea", engine->newFunction(Sphere_SurfaceArea_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Centroid", engine->newFunction(Sphere_Centroid_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ExtremePoint", engine->newFunction(Sphere_ExtremePoint_float3_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsFinite", engine->newFunction(Sphere_IsFinite_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsDegenerate", engine->newFunction(Sphere_IsDegenerate_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Contains", engine->newFunction(Sphere_Contains_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Distance", engine->newFunction(Sphere_Distance_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ClosestPoint", engine->newFunction(Sphere_ClosestPoint_float3_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Intersects", engine->newFunction(Sphere_Intersects_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Enclose", engine->newFunction(Sphere_Enclose_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("RandomPointInside", engine->newFunction(Sphere_RandomPointInside_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("RandomPointOnSurface", engine->newFunction(Sphere_RandomPointOnSurface_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("toString", engine->newFunction(Sphere_toString_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("metaTypeId", engine->toScriptValue<qint32>((qint32)qMetaTypeId<Sphere>()));
    engine->setDefaultPrototype(qMetaTypeId<Sphere>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<Sphere*>(), proto);
    qScriptRegisterMetaType(engine, ToScriptValue_Sphere, FromScriptValue_Sphere, proto);

    QScriptValue ctor = engine->newFunction(Sphere_ctor, proto, 4);
    ctor.setProperty("RandomPointInside", engine->newFunction(Sphere_RandomPointInside_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("RandomPointOnSurface", engine->newFunction(Sphere_RandomPointOnSurface_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("RandomUnitaryFloat3", engine->newFunction(Sphere_RandomUnitaryFloat3_LCG, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    engine->globalObject().setProperty("Sphere", ctor, QScriptValue::Undeletable | QScriptValue::ReadOnly);

    return ctor;
}

