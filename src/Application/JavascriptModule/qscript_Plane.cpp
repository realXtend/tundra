#include "QtScriptBindingsHelpers.h"

void ToExistingScriptValue_Plane(QScriptEngine *engine, const Plane &value, QScriptValue obj)
{
    obj.setProperty("normal", qScriptValueFromValue(engine, value.normal), QScriptValue::Undeletable);
    obj.setProperty("d", qScriptValueFromValue(engine, value.d), QScriptValue::Undeletable);
}

static QScriptValue Plane_Plane(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Plane_Plane in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane ret;
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Plane_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Plane_Plane_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 normal = qscriptvalue_cast<float3>(context->argument(0));
    float d = qscriptvalue_cast<float>(context->argument(1));
    Plane ret(normal, d);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Plane_float3_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Plane_Plane_float3_float3_float3 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 v1 = qscriptvalue_cast<float3>(context->argument(0));
    float3 v2 = qscriptvalue_cast<float3>(context->argument(1));
    float3 v3 = qscriptvalue_cast<float3>(context->argument(2));
    Plane ret(v1, v2, v3);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Plane_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Plane_Plane_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float3 normal = qscriptvalue_cast<float3>(context->argument(1));
    Plane ret(point, normal);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Plane_Ray_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Plane_Plane_Ray_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Ray line = qscriptvalue_cast<Ray>(context->argument(0));
    float3 normal = qscriptvalue_cast<float3>(context->argument(1));
    Plane ret(line, normal);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Plane_Line_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Plane_Plane_Line_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Line line = qscriptvalue_cast<Line>(context->argument(0));
    float3 normal = qscriptvalue_cast<float3>(context->argument(1));
    Plane ret(line, normal);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Plane_LineSegment_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Plane_Plane_LineSegment_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LineSegment line = qscriptvalue_cast<LineSegment>(context->argument(0));
    float3 normal = qscriptvalue_cast<float3>(context->argument(1));
    Plane ret(line, normal);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Set_float3_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Plane_Set_float3_float3_float3 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    float3 v1 = qscriptvalue_cast<float3>(context->argument(0));
    float3 v2 = qscriptvalue_cast<float3>(context->argument(1));
    float3 v3 = qscriptvalue_cast<float3>(context->argument(2));
    This.Set(v1, v2, v3);
    ToExistingScriptValue_Plane(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Plane_Set_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Plane_Set_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float3 normal = qscriptvalue_cast<float3>(context->argument(1));
    This.Set(point, normal);
    ToExistingScriptValue_Plane(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Plane_ReverseNormal(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Plane_ReverseNormal in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    This.ReverseNormal();
    ToExistingScriptValue_Plane(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Plane_PointOnPlane_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Plane_PointOnPlane_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    float3 ret = This.PointOnPlane();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Point_float_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Plane_Point_float_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    float u = qscriptvalue_cast<float>(context->argument(0));
    float v = qscriptvalue_cast<float>(context->argument(1));
    float3 ret = This.Point(u, v);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Point_float_float_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Plane_Point_float_float_float3_const in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    float u = qscriptvalue_cast<float>(context->argument(0));
    float v = qscriptvalue_cast<float>(context->argument(1));
    float3 referenceOrigin = qscriptvalue_cast<float3>(context->argument(2));
    float3 ret = This.Point(u, v, referenceOrigin);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Transform_float3x3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Plane_Transform_float3x3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    float3x3 transform = qscriptvalue_cast<float3x3>(context->argument(0));
    This.Transform(transform);
    ToExistingScriptValue_Plane(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Plane_Transform_float3x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Plane_Transform_float3x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    float3x4 transform = qscriptvalue_cast<float3x4>(context->argument(0));
    This.Transform(transform);
    ToExistingScriptValue_Plane(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Plane_Transform_float4x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Plane_Transform_float4x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    float4x4 transform = qscriptvalue_cast<float4x4>(context->argument(0));
    This.Transform(transform);
    ToExistingScriptValue_Plane(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Plane_Transform_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Plane_Transform_Quat in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    Quat transform = qscriptvalue_cast<Quat>(context->argument(0));
    This.Transform(transform);
    ToExistingScriptValue_Plane(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Plane_IsInPositiveDirection_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Plane_IsInPositiveDirection_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    float3 directionVector = qscriptvalue_cast<float3>(context->argument(0));
    bool ret = This.IsInPositiveDirection(directionVector);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_IsOnPositiveSide_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Plane_IsOnPositiveSide_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    bool ret = This.IsOnPositiveSide(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_ExamineSide_Triangle_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Plane_ExamineSide_Triangle_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    Triangle triangle = qscriptvalue_cast<Triangle>(context->argument(0));
    int ret = This.ExamineSide(triangle);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_AreOnSameSide_float3_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Plane_AreOnSameSide_float3_float3_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    float3 p1 = qscriptvalue_cast<float3>(context->argument(0));
    float3 p2 = qscriptvalue_cast<float3>(context->argument(1));
    bool ret = This.AreOnSameSide(p1, p2);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Distance_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Plane_Distance_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float ret = This.Distance(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Distance_LineSegment_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Plane_Distance_LineSegment_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    LineSegment lineSegment = qscriptvalue_cast<LineSegment>(context->argument(0));
    float ret = This.Distance(lineSegment);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Distance_Sphere_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Plane_Distance_Sphere_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    Sphere sphere = qscriptvalue_cast<Sphere>(context->argument(0));
    float ret = This.Distance(sphere);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Distance_Capsule_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Plane_Distance_Capsule_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    Capsule capsule = qscriptvalue_cast<Capsule>(context->argument(0));
    float ret = This.Distance(capsule);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_SignedDistance_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Plane_SignedDistance_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float ret = This.SignedDistance(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_OrthoProjection_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Plane_OrthoProjection_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    float3x4 ret = This.OrthoProjection();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Project_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Plane_Project_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.Project(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Project_LineSegment_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Plane_Project_LineSegment_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    LineSegment lineSegment = qscriptvalue_cast<LineSegment>(context->argument(0));
    LineSegment ret = This.Project(lineSegment);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Project_Triangle_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Plane_Project_Triangle_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    Triangle triangle = qscriptvalue_cast<Triangle>(context->argument(0));
    Triangle ret = This.Project(triangle);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Project_Polygon_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Plane_Project_Polygon_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    Polygon polygon = qscriptvalue_cast<Polygon>(context->argument(0));
    Polygon ret = This.Project(polygon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_ObliqueProjection_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Plane_ObliqueProjection_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    float3 obliqueProjectionDir = qscriptvalue_cast<float3>(context->argument(0));
    float3x4 ret = This.ObliqueProjection(obliqueProjectionDir);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_ObliqueProject_float3_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Plane_ObliqueProject_float3_float3_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float3 obliqueProjectionDir = qscriptvalue_cast<float3>(context->argument(1));
    float3 ret = This.ObliqueProject(point, obliqueProjectionDir);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_MirrorMatrix_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Plane_MirrorMatrix_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    float3x4 ret = This.MirrorMatrix();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Mirror_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Plane_Mirror_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.Mirror(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Refract_float3_float_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Plane_Refract_float3_float_float_const in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    float3 vec = qscriptvalue_cast<float3>(context->argument(0));
    float negativeSideRefractionIndex = qscriptvalue_cast<float>(context->argument(1));
    float positiveSideRefractionIndex = qscriptvalue_cast<float>(context->argument(2));
    float3 ret = This.Refract(vec, negativeSideRefractionIndex, positiveSideRefractionIndex);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_ClosestPoint_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Plane_ClosestPoint_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.ClosestPoint(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_ClosestPoint_Ray_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Plane_ClosestPoint_Ray_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    Ray ray = qscriptvalue_cast<Ray>(context->argument(0));
    float3 ret = This.ClosestPoint(ray);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_ClosestPoint_LineSegment_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Plane_ClosestPoint_LineSegment_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    LineSegment lineSegment = qscriptvalue_cast<LineSegment>(context->argument(0));
    float3 ret = This.ClosestPoint(lineSegment);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Contains_float3_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Plane_Contains_float3_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float epsilon = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.Contains(point, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Contains_Line_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Plane_Contains_Line_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    Line line = qscriptvalue_cast<Line>(context->argument(0));
    float epsilon = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.Contains(line, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Contains_Ray_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Plane_Contains_Ray_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    Ray ray = qscriptvalue_cast<Ray>(context->argument(0));
    float epsilon = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.Contains(ray, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Contains_LineSegment_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Plane_Contains_LineSegment_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    LineSegment lineSegment = qscriptvalue_cast<LineSegment>(context->argument(0));
    float epsilon = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.Contains(lineSegment, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Contains_Triangle_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Plane_Contains_Triangle_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    Triangle triangle = qscriptvalue_cast<Triangle>(context->argument(0));
    float epsilon = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.Contains(triangle, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Contains_Circle_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Plane_Contains_Circle_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    Circle circle = qscriptvalue_cast<Circle>(context->argument(0));
    float epsilon = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.Contains(circle, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Contains_Polygon_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Plane_Contains_Polygon_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    Polygon polygon = qscriptvalue_cast<Polygon>(context->argument(0));
    float epsilon = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.Contains(polygon, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_SetEquals_Plane_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Plane_SetEquals_Plane_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    Plane plane = qscriptvalue_cast<Plane>(context->argument(0));
    float epsilon = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.SetEquals(plane, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Equals_Plane_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Plane_Equals_Plane_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    Plane other = qscriptvalue_cast<Plane>(context->argument(0));
    float epsilon = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.Equals(other, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_IsParallel_Plane_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Plane_IsParallel_Plane_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    Plane plane = qscriptvalue_cast<Plane>(context->argument(0));
    float epsilon = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.IsParallel(plane, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_DihedralAngle_Plane_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Plane_DihedralAngle_Plane_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    Plane plane = qscriptvalue_cast<Plane>(context->argument(0));
    float ret = This.DihedralAngle(plane);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_IntersectsPlane_Plane_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Plane_IntersectsPlane_Plane_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    Plane plane = qscriptvalue_cast<Plane>(context->argument(0));
    Line ret = This.IntersectsPlane(plane);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Intersects_Plane_Line_ptr_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Plane_Intersects_Plane_Line_ptr_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    Plane plane = qscriptvalue_cast<Plane>(context->argument(0));
    Line * outLine = qscriptvalue_cast<Line *>(context->argument(1));
    bool ret = This.Intersects(plane, outLine);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Intersects_Sphere_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Plane_Intersects_Sphere_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    Sphere sphere = qscriptvalue_cast<Sphere>(context->argument(0));
    bool ret = This.Intersects(sphere);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Intersects_AABB_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Plane_Intersects_AABB_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    AABB aabb = qscriptvalue_cast<AABB>(context->argument(0));
    bool ret = This.Intersects(aabb);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Intersects_OBB_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Plane_Intersects_OBB_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    OBB obb = qscriptvalue_cast<OBB>(context->argument(0));
    bool ret = This.Intersects(obb);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Intersects_Polygon_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Plane_Intersects_Polygon_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    Polygon polygon = qscriptvalue_cast<Polygon>(context->argument(0));
    bool ret = This.Intersects(polygon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Intersects_Polyhedron_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Plane_Intersects_Polyhedron_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    Polyhedron polyhedron = qscriptvalue_cast<Polyhedron>(context->argument(0));
    bool ret = This.Intersects(polyhedron);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Intersects_Triangle_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Plane_Intersects_Triangle_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    Triangle triangle = qscriptvalue_cast<Triangle>(context->argument(0));
    bool ret = This.Intersects(triangle);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Intersects_Frustum_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Plane_Intersects_Frustum_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    Frustum frustum = qscriptvalue_cast<Frustum>(context->argument(0));
    bool ret = This.Intersects(frustum);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Intersects_Capsule_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Plane_Intersects_Capsule_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    Capsule capsule = qscriptvalue_cast<Capsule>(context->argument(0));
    bool ret = This.Intersects(capsule);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Intersects_Circle_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Plane_Intersects_Circle_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    Circle circle = qscriptvalue_cast<Circle>(context->argument(0));
    int ret = This.Intersects(circle);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Clip_LineSegment_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Plane_Clip_LineSegment_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    LineSegment line = qscriptvalue_cast<LineSegment>(context->argument(0));
    bool ret = This.Clip(line);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Clip_float3_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Plane_Clip_float3_float3_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    float3 a = qscriptvalue_cast<float3>(context->argument(0));
    float3 b = qscriptvalue_cast<float3>(context->argument(1));
    bool ret = This.Clip(a, b);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Clip_Line_Ray_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Plane_Clip_Line_Ray_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    Line line = qscriptvalue_cast<Line>(context->argument(0));
    Ray outRay = qscriptvalue_cast<Ray>(context->argument(1));
    int ret = This.Clip(line, outRay);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_Clip_Triangle_Triangle_Triangle_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Plane_Clip_Triangle_Triangle_Triangle_const in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    Triangle triangle = qscriptvalue_cast<Triangle>(context->argument(0));
    Triangle t1 = qscriptvalue_cast<Triangle>(context->argument(1));
    Triangle t2 = qscriptvalue_cast<Triangle>(context->argument(2));
    int ret = This.Clip(triangle, t1, t2);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_PassesThroughOrigin_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Plane_PassesThroughOrigin_float_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    float epsilon = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.PassesThroughOrigin(epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_GenerateCircle_float3_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Plane_GenerateCircle_float3_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane This = qscriptvalue_cast<Plane>(context->thisObject());
    float3 circleCenter = qscriptvalue_cast<float3>(context->argument(0));
    float radius = qscriptvalue_cast<float>(context->argument(1));
    Circle ret = This.GenerateCircle(circleCenter, radius);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_toString_const(QScriptContext *context, QScriptEngine *engine)
{
    Plane This;
    if (context->argumentCount() > 0) This = qscriptvalue_cast<Plane>(context->argument(0)); // Qt oddity (bug?): Sometimes the built-in toString() function doesn't give us this from thisObject, but as the first argument.
    else This = qscriptvalue_cast<Plane>(context->thisObject());
    QString ret = This.toString();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Plane_ctor(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return Plane_Plane(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return Plane_Plane_float3_float(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)))
        return Plane_Plane_float3_float3_float3(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return Plane_Plane_float3_float3(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<Ray>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return Plane_Plane_Ray_float3(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<Line>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return Plane_Plane_Line_float3(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<LineSegment>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return Plane_Plane_LineSegment_float3(context, engine);
    printf("Plane_ctor failed to choose the right function to call! Did you use 'var x = Plane();' instead of 'var x = new Plane();'?\n"); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Plane_Set_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 3 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)))
        return Plane_Set_float3_float3_float3(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return Plane_Set_float3_float3(context, engine);
    printf("Plane_Set_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Plane_Point_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return Plane_Point_float_float_const(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)))
        return Plane_Point_float_float_float3_const(context, engine);
    printf("Plane_Point_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Plane_Transform_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3x3>(context->argument(0)))
        return Plane_Transform_float3x3(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3x4>(context->argument(0)))
        return Plane_Transform_float3x4(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float4x4>(context->argument(0)))
        return Plane_Transform_float4x4(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Quat>(context->argument(0)))
        return Plane_Transform_Quat(context, engine);
    printf("Plane_Transform_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Plane_Distance_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return Plane_Distance_float3_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<LineSegment>(context->argument(0)))
        return Plane_Distance_LineSegment_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Sphere>(context->argument(0)))
        return Plane_Distance_Sphere_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Capsule>(context->argument(0)))
        return Plane_Distance_Capsule_const(context, engine);
    printf("Plane_Distance_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Plane_Project_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return Plane_Project_float3_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<LineSegment>(context->argument(0)))
        return Plane_Project_LineSegment_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Triangle>(context->argument(0)))
        return Plane_Project_Triangle_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Polygon>(context->argument(0)))
        return Plane_Project_Polygon_const(context, engine);
    printf("Plane_Project_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Plane_ClosestPoint_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return Plane_ClosestPoint_float3_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Ray>(context->argument(0)))
        return Plane_ClosestPoint_Ray_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<LineSegment>(context->argument(0)))
        return Plane_ClosestPoint_LineSegment_const(context, engine);
    printf("Plane_ClosestPoint_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Plane_Contains_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return Plane_Contains_float3_float_const(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<Line>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return Plane_Contains_Line_float_const(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<Ray>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return Plane_Contains_Ray_float_const(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<LineSegment>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return Plane_Contains_LineSegment_float_const(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<Triangle>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return Plane_Contains_Triangle_float_const(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<Circle>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return Plane_Contains_Circle_float_const(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<Polygon>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return Plane_Contains_Polygon_float_const(context, engine);
    printf("Plane_Contains_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Plane_Intersects_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<Plane>(context->argument(0)) && QSVIsOfType<Line *>(context->argument(1)))
        return Plane_Intersects_Plane_Line_ptr_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Sphere>(context->argument(0)))
        return Plane_Intersects_Sphere_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<AABB>(context->argument(0)))
        return Plane_Intersects_AABB_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<OBB>(context->argument(0)))
        return Plane_Intersects_OBB_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Polygon>(context->argument(0)))
        return Plane_Intersects_Polygon_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Polyhedron>(context->argument(0)))
        return Plane_Intersects_Polyhedron_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Triangle>(context->argument(0)))
        return Plane_Intersects_Triangle_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Frustum>(context->argument(0)))
        return Plane_Intersects_Frustum_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Capsule>(context->argument(0)))
        return Plane_Intersects_Capsule_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Circle>(context->argument(0)))
        return Plane_Intersects_Circle_const(context, engine);
    printf("Plane_Intersects_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Plane_Clip_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<LineSegment>(context->argument(0)))
        return Plane_Clip_LineSegment_const(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return Plane_Clip_float3_float3_const(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<Line>(context->argument(0)) && QSVIsOfType<Ray>(context->argument(1)))
        return Plane_Clip_Line_Ray_const(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<Triangle>(context->argument(0)) && QSVIsOfType<Triangle>(context->argument(1)) && QSVIsOfType<Triangle>(context->argument(2)))
        return Plane_Clip_Triangle_Triangle_Triangle_const(context, engine);
    printf("Plane_Clip_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

void FromScriptValue_Plane(const QScriptValue &obj, Plane &value)
{
    value.normal = qScriptValueToValue<float3>(obj.property("normal"));
    value.d = qScriptValueToValue<float>(obj.property("d"));
}

QScriptValue ToScriptValue_Plane(QScriptEngine *engine, const Plane &value)
{
    QScriptValue obj = engine->newVariant(QVariant::fromValue(value)); // The contents of this variant are NOT used. The real data lies in the data() pointer of this QScriptValue. This only exists to enable overload resolution to work for QObject slots.
    ToExistingScriptValue_Plane(engine, value, obj);
    return obj;
}

QScriptValue ToScriptValue_const_Plane(QScriptEngine *engine, const Plane &value)
{
    QScriptValue obj = engine->newVariant(QVariant::fromValue(value)); // The contents of this variant are NOT used. The real data lies in the data() pointer of this QScriptValue. This only exists to enable overload resolution to work for QObject slots.
    obj.setPrototype(engine->defaultPrototype(qMetaTypeId<Plane>()));
    obj.setProperty("normal", ToScriptValue_const_float3(engine, value.normal), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("d", qScriptValueFromValue(engine, value.d), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    return obj;
}

QScriptValue register_Plane_prototype(QScriptEngine *engine)
{
    QScriptValue proto = engine->newObject();
    proto.setProperty("Set", engine->newFunction(Plane_Set_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Set", engine->newFunction(Plane_Set_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ReverseNormal", engine->newFunction(Plane_ReverseNormal, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("PointOnPlane", engine->newFunction(Plane_PointOnPlane_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Point", engine->newFunction(Plane_Point_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Point", engine->newFunction(Plane_Point_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Transform", engine->newFunction(Plane_Transform_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsInPositiveDirection", engine->newFunction(Plane_IsInPositiveDirection_float3_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsOnPositiveSide", engine->newFunction(Plane_IsOnPositiveSide_float3_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ExamineSide", engine->newFunction(Plane_ExamineSide_Triangle_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("AreOnSameSide", engine->newFunction(Plane_AreOnSameSide_float3_float3_const, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Distance", engine->newFunction(Plane_Distance_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SignedDistance", engine->newFunction(Plane_SignedDistance_float3_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("OrthoProjection", engine->newFunction(Plane_OrthoProjection_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Project", engine->newFunction(Plane_Project_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ObliqueProjection", engine->newFunction(Plane_ObliqueProjection_float3_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ObliqueProject", engine->newFunction(Plane_ObliqueProject_float3_float3_const, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MirrorMatrix", engine->newFunction(Plane_MirrorMatrix_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Mirror", engine->newFunction(Plane_Mirror_float3_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Refract", engine->newFunction(Plane_Refract_float3_float_float_const, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ClosestPoint", engine->newFunction(Plane_ClosestPoint_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Contains", engine->newFunction(Plane_Contains_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetEquals", engine->newFunction(Plane_SetEquals_Plane_float_const, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Equals", engine->newFunction(Plane_Equals_Plane_float_const, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsParallel", engine->newFunction(Plane_IsParallel_Plane_float_const, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("DihedralAngle", engine->newFunction(Plane_DihedralAngle_Plane_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IntersectsPlane", engine->newFunction(Plane_IntersectsPlane_Plane_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Intersects", engine->newFunction(Plane_Intersects_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Intersects", engine->newFunction(Plane_Intersects_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Clip", engine->newFunction(Plane_Clip_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Clip", engine->newFunction(Plane_Clip_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Clip", engine->newFunction(Plane_Clip_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("PassesThroughOrigin", engine->newFunction(Plane_PassesThroughOrigin_float_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("GenerateCircle", engine->newFunction(Plane_GenerateCircle_float3_float_const, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("toString", engine->newFunction(Plane_toString_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("metaTypeId", engine->toScriptValue<qint32>((qint32)qMetaTypeId<Plane>()));
    engine->setDefaultPrototype(qMetaTypeId<Plane>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<Plane*>(), proto);
    qScriptRegisterMetaType(engine, ToScriptValue_Plane, FromScriptValue_Plane, proto);

    QScriptValue ctor = engine->newFunction(Plane_ctor, proto, 3);
    engine->globalObject().setProperty("Plane", ctor, QScriptValue::Undeletable | QScriptValue::ReadOnly);

    return ctor;
}

