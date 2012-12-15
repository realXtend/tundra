#include "QtScriptBindingsHelpers.h"

void ToExistingScriptValue_Triangle(QScriptEngine *engine, const Triangle &value, QScriptValue obj)
{
    obj.setProperty("a", qScriptValueFromValue(engine, value.a), QScriptValue::Undeletable);
    obj.setProperty("b", qScriptValueFromValue(engine, value.b), QScriptValue::Undeletable);
    obj.setProperty("c", qScriptValueFromValue(engine, value.c), QScriptValue::Undeletable);
}

static QScriptValue Triangle_Triangle(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Triangle_Triangle in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle ret;
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_Triangle_float3_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Triangle_Triangle_float3_float3_float3 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 a = qscriptvalue_cast<float3>(context->argument(0));
    float3 b = qscriptvalue_cast<float3>(context->argument(1));
    float3 c = qscriptvalue_cast<float3>(context->argument(2));
    Triangle ret(a, b, c);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_BarycentricUVW_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_BarycentricUVW_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.BarycentricUVW(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_BarycentricUV_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_BarycentricUV_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float2 ret = This.BarycentricUV(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_Point_float_float_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Triangle_Point_float_float_float_const in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    float u = qscriptvalue_cast<float>(context->argument(0));
    float v = qscriptvalue_cast<float>(context->argument(1));
    float w = qscriptvalue_cast<float>(context->argument(2));
    float3 ret = This.Point(u, v, w);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_Point_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_Point_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    float3 uvw = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.Point(uvw);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_Point_float_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Triangle_Point_float_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    float u = qscriptvalue_cast<float>(context->argument(0));
    float v = qscriptvalue_cast<float>(context->argument(1));
    float3 ret = This.Point(u, v);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_Point_float2_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_Point_float2_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    float2 uv = qscriptvalue_cast<float2>(context->argument(0));
    float3 ret = This.Point(uv);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_Centroid_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Triangle_Centroid_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    float3 ret = This.Centroid();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_Area_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Triangle_Area_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    float ret = This.Area();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_Perimeter_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Triangle_Perimeter_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    float ret = This.Perimeter();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_Edge_int_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_Edge_int_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    int i = qscriptvalue_cast<int>(context->argument(0));
    LineSegment ret = This.Edge(i);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_Vertex_int_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_Vertex_int_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    int i = qscriptvalue_cast<int>(context->argument(0));
    float3 ret = This.Vertex(i);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_PlaneCCW_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Triangle_PlaneCCW_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    Plane ret = This.PlaneCCW();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_PlaneCW_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Triangle_PlaneCW_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    Plane ret = This.PlaneCW();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_NormalCCW_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Triangle_NormalCCW_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    float3 ret = This.NormalCCW();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_NormalCW_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Triangle_NormalCW_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    float3 ret = This.NormalCW();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_UnnormalizedNormalCCW_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Triangle_UnnormalizedNormalCCW_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    float3 ret = This.UnnormalizedNormalCCW();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_UnnormalizedNormalCW_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Triangle_UnnormalizedNormalCW_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    float3 ret = This.UnnormalizedNormalCW();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_ExtremePoint_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_ExtremePoint_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    float3 direction = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.ExtremePoint(direction);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_ToPolygon_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Triangle_ToPolygon_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    Polygon ret = This.ToPolygon();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_ToPolyhedron_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Triangle_ToPolyhedron_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    Polyhedron ret = This.ToPolyhedron();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_IsFinite_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Triangle_IsFinite_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    bool ret = This.IsFinite();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_IsDegenerate_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_IsDegenerate_float_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    float epsilon = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.IsDegenerate(epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_Contains_float3_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Triangle_Contains_float3_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float triangleThickness = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.Contains(point, triangleThickness);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_Contains_LineSegment_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Triangle_Contains_LineSegment_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    LineSegment lineSegment = qscriptvalue_cast<LineSegment>(context->argument(0));
    float triangleThickness = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.Contains(lineSegment, triangleThickness);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_Contains_Triangle_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Triangle_Contains_Triangle_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    Triangle triangle = qscriptvalue_cast<Triangle>(context->argument(0));
    float triangleThickness = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.Contains(triangle, triangleThickness);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_Distance_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_Distance_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float ret = This.Distance(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_Distance_Sphere_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_Distance_Sphere_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    Sphere sphere = qscriptvalue_cast<Sphere>(context->argument(0));
    float ret = This.Distance(sphere);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_Intersects_Plane_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_Intersects_Plane_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    Plane plane = qscriptvalue_cast<Plane>(context->argument(0));
    bool ret = This.Intersects(plane);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_Intersects_Sphere_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_Intersects_Sphere_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    Sphere sphere = qscriptvalue_cast<Sphere>(context->argument(0));
    bool ret = This.Intersects(sphere);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_Intersects_Triangle_LineSegment_ptr_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Triangle_Intersects_Triangle_LineSegment_ptr_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    Triangle triangle = qscriptvalue_cast<Triangle>(context->argument(0));
    LineSegment * outLine = qscriptvalue_cast<LineSegment *>(context->argument(1));
    bool ret = This.Intersects(triangle, outLine);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_Intersects_AABB_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_Intersects_AABB_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    AABB aabb = qscriptvalue_cast<AABB>(context->argument(0));
    bool ret = This.Intersects(aabb);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_Intersects_OBB_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_Intersects_OBB_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    OBB obb = qscriptvalue_cast<OBB>(context->argument(0));
    bool ret = This.Intersects(obb);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_Intersects_Polygon_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_Intersects_Polygon_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    Polygon polygon = qscriptvalue_cast<Polygon>(context->argument(0));
    bool ret = This.Intersects(polygon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_Intersects_Frustum_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_Intersects_Frustum_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    Frustum frustum = qscriptvalue_cast<Frustum>(context->argument(0));
    bool ret = This.Intersects(frustum);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_Intersects_Polyhedron_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_Intersects_Polyhedron_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    Polyhedron polyhedron = qscriptvalue_cast<Polyhedron>(context->argument(0));
    bool ret = This.Intersects(polyhedron);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_Intersects_Capsule_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_Intersects_Capsule_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    Capsule capsule = qscriptvalue_cast<Capsule>(context->argument(0));
    bool ret = This.Intersects(capsule);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_ProjectToAxis_float3_float_float_const(QScriptContext *context, QScriptEngine * /*engine*/)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Triangle_ProjectToAxis_float3_float_float_const in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    float3 axis = qscriptvalue_cast<float3>(context->argument(0));
    float dMin = qscriptvalue_cast<float>(context->argument(1));
    float dMax = qscriptvalue_cast<float>(context->argument(2));
    This.ProjectToAxis(axis, dMin, dMax);
    return QScriptValue();
}

static QScriptValue Triangle_ClosestPoint_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_ClosestPoint_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.ClosestPoint(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_RandomPointInside_LCG_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_RandomPointInside_LCG_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    LCG rng = qscriptvalue_cast<LCG>(context->argument(0));
    float3 ret = This.RandomPointInside(rng);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_RandomVertex_LCG_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_RandomVertex_LCG_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    LCG rng = qscriptvalue_cast<LCG>(context->argument(0));
    float3 ret = This.RandomVertex(rng);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_RandomPointOnEdge_LCG_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_RandomPointOnEdge_LCG_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    LCG rng = qscriptvalue_cast<LCG>(context->argument(0));
    float3 ret = This.RandomPointOnEdge(rng);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_toString_const(QScriptContext *context, QScriptEngine *engine)
{
    Triangle This;
    if (context->argumentCount() > 0) This = qscriptvalue_cast<Triangle>(context->argument(0)); // Qt oddity (bug?): Sometimes the built-in toString() function doesn't give us this from thisObject, but as the first argument.
    else This = qscriptvalue_cast<Triangle>(context->thisObject());
    QString ret = This.toString();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_BarycentricInsideTriangle_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_BarycentricInsideTriangle_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 uvw = qscriptvalue_cast<float3>(context->argument(0));
    bool ret = Triangle::BarycentricInsideTriangle(uvw);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_Area2D_float2_float2_float2(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Triangle_Area2D_float2_float2_float2 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 p1 = qscriptvalue_cast<float2>(context->argument(0));
    float2 p2 = qscriptvalue_cast<float2>(context->argument(1));
    float2 p3 = qscriptvalue_cast<float2>(context->argument(2));
    float ret = Triangle::Area2D(p1, p2, p3);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_SignedArea_float3_float3_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function Triangle_SignedArea_float3_float3_float3_float3 in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float3 a = qscriptvalue_cast<float3>(context->argument(1));
    float3 b = qscriptvalue_cast<float3>(context->argument(2));
    float3 c = qscriptvalue_cast<float3>(context->argument(3));
    float ret = Triangle::SignedArea(point, a, b, c);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_IsDegenerate_float3_float3_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function Triangle_IsDegenerate_float3_float3_float3_float in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 p1 = qscriptvalue_cast<float3>(context->argument(0));
    float3 p2 = qscriptvalue_cast<float3>(context->argument(1));
    float3 p3 = qscriptvalue_cast<float3>(context->argument(2));
    float epsilon = qscriptvalue_cast<float>(context->argument(3));
    bool ret = Triangle::IsDegenerate(p1, p2, p3, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_IntersectLineTri_float3_float3_float3_float3_float3_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 8) { printf("Error! Invalid number of arguments passed to function Triangle_IntersectLineTri_float3_float3_float3_float3_float3_float_float_float in file %s, line %d!\nExpected 8, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 linePos = qscriptvalue_cast<float3>(context->argument(0));
    float3 lineDir = qscriptvalue_cast<float3>(context->argument(1));
    float3 v0 = qscriptvalue_cast<float3>(context->argument(2));
    float3 v1 = qscriptvalue_cast<float3>(context->argument(3));
    float3 v2 = qscriptvalue_cast<float3>(context->argument(4));
    float u = qscriptvalue_cast<float>(context->argument(5));
    float v = qscriptvalue_cast<float>(context->argument(6));
    float t = qscriptvalue_cast<float>(context->argument(7));
    bool ret = Triangle::IntersectLineTri(linePos, lineDir, v0, v1, v2, u, v, t);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_ctor(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return Triangle_Triangle(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)))
        return Triangle_Triangle_float3_float3_float3(context, engine);
    printf("Triangle_ctor failed to choose the right function to call! Did you use 'var x = Triangle();' instead of 'var x = new Triangle();'?\n"); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Triangle_Point_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 3 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return Triangle_Point_float_float_float_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return Triangle_Point_float3_const(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return Triangle_Point_float_float_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float2>(context->argument(0)))
        return Triangle_Point_float2_const(context, engine);
    printf("Triangle_Point_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Triangle_IsDegenerate_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return Triangle_IsDegenerate_float_const(context, engine);
    if (context->argumentCount() == 4 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)) && QSVIsOfType<float>(context->argument(3)))
        return Triangle_IsDegenerate_float3_float3_float3_float(context, engine);
    printf("Triangle_IsDegenerate_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Triangle_Contains_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return Triangle_Contains_float3_float_const(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<LineSegment>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return Triangle_Contains_LineSegment_float_const(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<Triangle>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return Triangle_Contains_Triangle_float_const(context, engine);
    printf("Triangle_Contains_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Triangle_Distance_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return Triangle_Distance_float3_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Sphere>(context->argument(0)))
        return Triangle_Distance_Sphere_const(context, engine);
    printf("Triangle_Distance_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Triangle_Intersects_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<Plane>(context->argument(0)))
        return Triangle_Intersects_Plane_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Sphere>(context->argument(0)))
        return Triangle_Intersects_Sphere_const(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<Triangle>(context->argument(0)) && QSVIsOfType<LineSegment *>(context->argument(1)))
        return Triangle_Intersects_Triangle_LineSegment_ptr_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<AABB>(context->argument(0)))
        return Triangle_Intersects_AABB_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<OBB>(context->argument(0)))
        return Triangle_Intersects_OBB_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Polygon>(context->argument(0)))
        return Triangle_Intersects_Polygon_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Frustum>(context->argument(0)))
        return Triangle_Intersects_Frustum_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Polyhedron>(context->argument(0)))
        return Triangle_Intersects_Polyhedron_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Capsule>(context->argument(0)))
        return Triangle_Intersects_Capsule_const(context, engine);
    printf("Triangle_Intersects_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

void FromScriptValue_Triangle(const QScriptValue &obj, Triangle &value)
{
    value.a = qScriptValueToValue<float3>(obj.property("a"));
    value.b = qScriptValueToValue<float3>(obj.property("b"));
    value.c = qScriptValueToValue<float3>(obj.property("c"));
}

QScriptValue ToScriptValue_Triangle(QScriptEngine *engine, const Triangle &value)
{
    QScriptValue obj = engine->newVariant(QVariant::fromValue(value)); // The contents of this variant are NOT used. The real data lies in the data() pointer of this QScriptValue. This only exists to enable overload resolution to work for QObject slots.
    ToExistingScriptValue_Triangle(engine, value, obj);
    return obj;
}

QScriptValue ToScriptValue_const_Triangle(QScriptEngine *engine, const Triangle &value)
{
    QScriptValue obj = engine->newVariant(QVariant::fromValue(value)); // The contents of this variant are NOT used. The real data lies in the data() pointer of this QScriptValue. This only exists to enable overload resolution to work for QObject slots.
    obj.setPrototype(engine->defaultPrototype(qMetaTypeId<Triangle>()));
    obj.setProperty("a", ToScriptValue_const_float3(engine, value.a), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("b", ToScriptValue_const_float3(engine, value.b), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("c", ToScriptValue_const_float3(engine, value.c), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    return obj;
}

QScriptValue register_Triangle_prototype(QScriptEngine *engine)
{
    QScriptValue proto = engine->newObject();
    proto.setProperty("BarycentricUVW", engine->newFunction(Triangle_BarycentricUVW_float3_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("BarycentricUV", engine->newFunction(Triangle_BarycentricUV_float3_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Point", engine->newFunction(Triangle_Point_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Point", engine->newFunction(Triangle_Point_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Point", engine->newFunction(Triangle_Point_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Centroid", engine->newFunction(Triangle_Centroid_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Area", engine->newFunction(Triangle_Area_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Perimeter", engine->newFunction(Triangle_Perimeter_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Edge", engine->newFunction(Triangle_Edge_int_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Vertex", engine->newFunction(Triangle_Vertex_int_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("PlaneCCW", engine->newFunction(Triangle_PlaneCCW_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("PlaneCW", engine->newFunction(Triangle_PlaneCW_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("NormalCCW", engine->newFunction(Triangle_NormalCCW_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("NormalCW", engine->newFunction(Triangle_NormalCW_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("UnnormalizedNormalCCW", engine->newFunction(Triangle_UnnormalizedNormalCCW_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("UnnormalizedNormalCW", engine->newFunction(Triangle_UnnormalizedNormalCW_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ExtremePoint", engine->newFunction(Triangle_ExtremePoint_float3_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToPolygon", engine->newFunction(Triangle_ToPolygon_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToPolyhedron", engine->newFunction(Triangle_ToPolyhedron_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsFinite", engine->newFunction(Triangle_IsFinite_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsDegenerate", engine->newFunction(Triangle_IsDegenerate_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Contains", engine->newFunction(Triangle_Contains_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Distance", engine->newFunction(Triangle_Distance_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Intersects", engine->newFunction(Triangle_Intersects_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Intersects", engine->newFunction(Triangle_Intersects_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ProjectToAxis", engine->newFunction(Triangle_ProjectToAxis_float3_float_float_const, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ClosestPoint", engine->newFunction(Triangle_ClosestPoint_float3_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("RandomPointInside", engine->newFunction(Triangle_RandomPointInside_LCG_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("RandomVertex", engine->newFunction(Triangle_RandomVertex_LCG_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("RandomPointOnEdge", engine->newFunction(Triangle_RandomPointOnEdge_LCG_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("toString", engine->newFunction(Triangle_toString_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("metaTypeId", engine->toScriptValue<qint32>((qint32)qMetaTypeId<Triangle>()));
    engine->setDefaultPrototype(qMetaTypeId<Triangle>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<Triangle*>(), proto);
    qScriptRegisterMetaType(engine, ToScriptValue_Triangle, FromScriptValue_Triangle, proto);

    QScriptValue ctor = engine->newFunction(Triangle_ctor, proto, 3);
    ctor.setProperty("BarycentricInsideTriangle", engine->newFunction(Triangle_BarycentricInsideTriangle_float3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("Area2D", engine->newFunction(Triangle_Area2D_float2_float2_float2, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("SignedArea", engine->newFunction(Triangle_SignedArea_float3_float3_float3_float3, 4), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("IsDegenerate", engine->newFunction(Triangle_IsDegenerate_selector, 4), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("IntersectLineTri", engine->newFunction(Triangle_IntersectLineTri_float3_float3_float3_float3_float3_float_float_float, 8), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    engine->globalObject().setProperty("Triangle", ctor, QScriptValue::Undeletable | QScriptValue::ReadOnly);

    return ctor;
}

