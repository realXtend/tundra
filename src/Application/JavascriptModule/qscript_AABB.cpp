#include "QtScriptBindingsHelpers.h"

void ToExistingScriptValue_AABB(QScriptEngine *engine, const AABB &value, QScriptValue obj)
{
    obj.setProperty("minPoint", qScriptValueFromValue(engine, value.minPoint), QScriptValue::Undeletable);
    obj.setProperty("maxPoint", qScriptValueFromValue(engine, value.maxPoint), QScriptValue::Undeletable);
}

static QScriptValue AABB_AABB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_AABB in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB ret;
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_AABB_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function AABB_AABB_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 minPoint = qscriptvalue_cast<float3>(context->argument(0));
    float3 maxPoint = qscriptvalue_cast<float3>(context->argument(1));
    AABB ret(minPoint, maxPoint);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_AABB_OBB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_AABB_OBB in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    OBB obb = qscriptvalue_cast<OBB>(context->argument(0));
    AABB ret(obb);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_AABB_Sphere(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_AABB_Sphere in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere s = qscriptvalue_cast<Sphere>(context->argument(0));
    AABB ret(s);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_MinX_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_MinX_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float ret = This.MinX();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_MinY_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_MinY_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float ret = This.MinY();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_MinZ_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_MinZ_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float ret = This.MinZ();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_MaxX_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_MaxX_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float ret = This.MaxX();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_MaxY_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_MaxY_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float ret = This.MaxY();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_MaxZ_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_MaxZ_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float ret = This.MaxZ();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_SetNegativeInfinity(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_SetNegativeInfinity in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    This.SetNegativeInfinity();
    ToExistingScriptValue_AABB(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue AABB_SetFromCenterAndSize_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function AABB_SetFromCenterAndSize_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float3 center = qscriptvalue_cast<float3>(context->argument(0));
    float3 size = qscriptvalue_cast<float3>(context->argument(1));
    This.SetFromCenterAndSize(center, size);
    ToExistingScriptValue_AABB(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue AABB_SetFrom_OBB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_SetFrom_OBB in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    OBB obb = qscriptvalue_cast<OBB>(context->argument(0));
    This.SetFrom(obb);
    ToExistingScriptValue_AABB(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue AABB_SetFrom_Sphere(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_SetFrom_Sphere in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    Sphere s = qscriptvalue_cast<Sphere>(context->argument(0));
    This.SetFrom(s);
    ToExistingScriptValue_AABB(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue AABB_ToPolyhedron_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_ToPolyhedron_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    Polyhedron ret = This.ToPolyhedron();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_ToOBB_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_ToOBB_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    OBB ret = This.ToOBB();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_MinimalEnclosingSphere_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_MinimalEnclosingSphere_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    Sphere ret = This.MinimalEnclosingSphere();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_MaximalContainedSphere_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_MaximalContainedSphere_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    Sphere ret = This.MaximalContainedSphere();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_IsFinite_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_IsFinite_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    bool ret = This.IsFinite();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_IsDegenerate_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_IsDegenerate_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    bool ret = This.IsDegenerate();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_CenterPoint_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_CenterPoint_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float3 ret = This.CenterPoint();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Centroid_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_Centroid_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float3 ret = This.Centroid();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_PointInside_float_float_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function AABB_PointInside_float_float_float_const in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float x = qscriptvalue_cast<float>(context->argument(0));
    float y = qscriptvalue_cast<float>(context->argument(1));
    float z = qscriptvalue_cast<float>(context->argument(2));
    float3 ret = This.PointInside(x, y, z);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Edge_int_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Edge_int_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    int edgeIndex = qscriptvalue_cast<int>(context->argument(0));
    LineSegment ret = This.Edge(edgeIndex);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_CornerPoint_int_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_CornerPoint_int_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    int cornerIndex = qscriptvalue_cast<int>(context->argument(0));
    float3 ret = This.CornerPoint(cornerIndex);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_ExtremePoint_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_ExtremePoint_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float3 direction = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.ExtremePoint(direction);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_PointOnEdge_int_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function AABB_PointOnEdge_int_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    int edgeIndex = qscriptvalue_cast<int>(context->argument(0));
    float u = qscriptvalue_cast<float>(context->argument(1));
    float3 ret = This.PointOnEdge(edgeIndex, u);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_FaceCenterPoint_int_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_FaceCenterPoint_int_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    int faceIndex = qscriptvalue_cast<int>(context->argument(0));
    float3 ret = This.FaceCenterPoint(faceIndex);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_FacePoint_int_float_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function AABB_FacePoint_int_float_float_const in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    int faceIndex = qscriptvalue_cast<int>(context->argument(0));
    float u = qscriptvalue_cast<float>(context->argument(1));
    float v = qscriptvalue_cast<float>(context->argument(2));
    float3 ret = This.FacePoint(faceIndex, u, v);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_FaceNormal_int_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_FaceNormal_int_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    int faceIndex = qscriptvalue_cast<int>(context->argument(0));
    float3 ret = This.FaceNormal(faceIndex);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_FacePlane_int_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_FacePlane_int_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    int faceIndex = qscriptvalue_cast<int>(context->argument(0));
    Plane ret = This.FacePlane(faceIndex);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_GetFacePlanes_Plane_ptr_const(QScriptContext *context, QScriptEngine * /*engine*/)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_GetFacePlanes_Plane_ptr_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    Plane * outPlaneArray = qscriptvalue_cast<Plane *>(context->argument(0));
    This.GetFacePlanes(outPlaneArray);
    return QScriptValue();
}

static QScriptValue AABB_Size_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_Size_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float3 ret = This.Size();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_HalfSize_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_HalfSize_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float3 ret = This.HalfSize();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Diagonal_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_Diagonal_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float3 ret = This.Diagonal();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_HalfDiagonal_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_HalfDiagonal_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float3 ret = This.HalfDiagonal();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Volume_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_Volume_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float ret = This.Volume();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_SurfaceArea_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_SurfaceArea_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float ret = This.SurfaceArea();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_RandomPointInside_LCG_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_RandomPointInside_LCG_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    LCG rng = qscriptvalue_cast<LCG>(context->argument(0));
    float3 ret = This.RandomPointInside(rng);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_RandomPointOnSurface_LCG_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_RandomPointOnSurface_LCG_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    LCG rng = qscriptvalue_cast<LCG>(context->argument(0));
    float3 ret = This.RandomPointOnSurface(rng);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_RandomPointOnEdge_LCG_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_RandomPointOnEdge_LCG_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    LCG rng = qscriptvalue_cast<LCG>(context->argument(0));
    float3 ret = This.RandomPointOnEdge(rng);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_RandomCornerPoint_LCG_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_RandomCornerPoint_LCG_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    LCG rng = qscriptvalue_cast<LCG>(context->argument(0));
    float3 ret = This.RandomCornerPoint(rng);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Translate_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Translate_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float3 offset = qscriptvalue_cast<float3>(context->argument(0));
    This.Translate(offset);
    ToExistingScriptValue_AABB(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue AABB_Scale_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function AABB_Scale_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float3 centerPoint = qscriptvalue_cast<float3>(context->argument(0));
    float scaleFactor = qscriptvalue_cast<float>(context->argument(1));
    This.Scale(centerPoint, scaleFactor);
    ToExistingScriptValue_AABB(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue AABB_Scale_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function AABB_Scale_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float3 centerPoint = qscriptvalue_cast<float3>(context->argument(0));
    float3 scaleFactor = qscriptvalue_cast<float3>(context->argument(1));
    This.Scale(centerPoint, scaleFactor);
    ToExistingScriptValue_AABB(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue AABB_TransformAsAABB_float3x3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_TransformAsAABB_float3x3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float3x3 transform = qscriptvalue_cast<float3x3>(context->argument(0));
    This.TransformAsAABB(transform);
    ToExistingScriptValue_AABB(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue AABB_TransformAsAABB_float3x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_TransformAsAABB_float3x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float3x4 transform = qscriptvalue_cast<float3x4>(context->argument(0));
    This.TransformAsAABB(transform);
    ToExistingScriptValue_AABB(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue AABB_TransformAsAABB_float4x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_TransformAsAABB_float4x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float4x4 transform = qscriptvalue_cast<float4x4>(context->argument(0));
    This.TransformAsAABB(transform);
    ToExistingScriptValue_AABB(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue AABB_TransformAsAABB_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_TransformAsAABB_Quat in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    Quat transform = qscriptvalue_cast<Quat>(context->argument(0));
    This.TransformAsAABB(transform);
    ToExistingScriptValue_AABB(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue AABB_Transform_float3x3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Transform_float3x3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float3x3 transform = qscriptvalue_cast<float3x3>(context->argument(0));
    OBB ret = This.Transform(transform);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Transform_float3x4_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Transform_float3x4_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float3x4 transform = qscriptvalue_cast<float3x4>(context->argument(0));
    OBB ret = This.Transform(transform);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Transform_float4x4_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Transform_float4x4_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float4x4 transform = qscriptvalue_cast<float4x4>(context->argument(0));
    OBB ret = This.Transform(transform);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Transform_Quat_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Transform_Quat_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    Quat transform = qscriptvalue_cast<Quat>(context->argument(0));
    OBB ret = This.Transform(transform);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_ClosestPoint_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_ClosestPoint_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float3 targetPoint = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.ClosestPoint(targetPoint);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Distance_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Distance_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float ret = This.Distance(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Distance_Sphere_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Distance_Sphere_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    Sphere sphere = qscriptvalue_cast<Sphere>(context->argument(0));
    float ret = This.Distance(sphere);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Contains_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Contains_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    bool ret = This.Contains(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Contains_LineSegment_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Contains_LineSegment_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    LineSegment lineSegment = qscriptvalue_cast<LineSegment>(context->argument(0));
    bool ret = This.Contains(lineSegment);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Contains_AABB_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Contains_AABB_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    AABB aabb = qscriptvalue_cast<AABB>(context->argument(0));
    bool ret = This.Contains(aabb);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Contains_OBB_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Contains_OBB_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    OBB obb = qscriptvalue_cast<OBB>(context->argument(0));
    bool ret = This.Contains(obb);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Contains_Sphere_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Contains_Sphere_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    Sphere sphere = qscriptvalue_cast<Sphere>(context->argument(0));
    bool ret = This.Contains(sphere);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Contains_Triangle_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Contains_Triangle_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    Triangle triangle = qscriptvalue_cast<Triangle>(context->argument(0));
    bool ret = This.Contains(triangle);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Contains_Polygon_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Contains_Polygon_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    Polygon polygon = qscriptvalue_cast<Polygon>(context->argument(0));
    bool ret = This.Contains(polygon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Contains_Frustum_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Contains_Frustum_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    Frustum frustum = qscriptvalue_cast<Frustum>(context->argument(0));
    bool ret = This.Contains(frustum);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Contains_Polyhedron_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Contains_Polyhedron_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    Polyhedron polyhedron = qscriptvalue_cast<Polyhedron>(context->argument(0));
    bool ret = This.Contains(polyhedron);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Intersects_Plane_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Intersects_Plane_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    Plane plane = qscriptvalue_cast<Plane>(context->argument(0));
    bool ret = This.Intersects(plane);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Intersects_AABB_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Intersects_AABB_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    AABB aabb = qscriptvalue_cast<AABB>(context->argument(0));
    bool ret = This.Intersects(aabb);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Intersects_OBB_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Intersects_OBB_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    OBB obb = qscriptvalue_cast<OBB>(context->argument(0));
    bool ret = This.Intersects(obb);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Intersects_Capsule_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Intersects_Capsule_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    Capsule capsule = qscriptvalue_cast<Capsule>(context->argument(0));
    bool ret = This.Intersects(capsule);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Intersects_Triangle_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Intersects_Triangle_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    Triangle triangle = qscriptvalue_cast<Triangle>(context->argument(0));
    bool ret = This.Intersects(triangle);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Intersects_Polygon_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Intersects_Polygon_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    Polygon polygon = qscriptvalue_cast<Polygon>(context->argument(0));
    bool ret = This.Intersects(polygon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Intersects_Frustum_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Intersects_Frustum_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    Frustum frustum = qscriptvalue_cast<Frustum>(context->argument(0));
    bool ret = This.Intersects(frustum);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Intersects_Polyhedron_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Intersects_Polyhedron_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    Polyhedron polyhedron = qscriptvalue_cast<Polyhedron>(context->argument(0));
    bool ret = This.Intersects(polyhedron);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_ProjectToAxis_float3_float_float_const(QScriptContext *context, QScriptEngine * /*engine*/)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function AABB_ProjectToAxis_float3_float_float_const in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float3 axis = qscriptvalue_cast<float3>(context->argument(0));
    float dMin = qscriptvalue_cast<float>(context->argument(1));
    float dMax = qscriptvalue_cast<float>(context->argument(2));
    This.ProjectToAxis(axis, dMin, dMax);
    return QScriptValue();
}

static QScriptValue AABB_Enclose_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Enclose_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    This.Enclose(point);
    ToExistingScriptValue_AABB(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue AABB_Enclose_LineSegment(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Enclose_LineSegment in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    LineSegment lineSegment = qscriptvalue_cast<LineSegment>(context->argument(0));
    This.Enclose(lineSegment);
    ToExistingScriptValue_AABB(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue AABB_Enclose_AABB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Enclose_AABB in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    AABB aabb = qscriptvalue_cast<AABB>(context->argument(0));
    This.Enclose(aabb);
    ToExistingScriptValue_AABB(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue AABB_Enclose_OBB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Enclose_OBB in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    OBB obb = qscriptvalue_cast<OBB>(context->argument(0));
    This.Enclose(obb);
    ToExistingScriptValue_AABB(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue AABB_Enclose_Sphere(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Enclose_Sphere in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    Sphere sphere = qscriptvalue_cast<Sphere>(context->argument(0));
    This.Enclose(sphere);
    ToExistingScriptValue_AABB(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue AABB_Enclose_Triangle(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Enclose_Triangle in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    Triangle triangle = qscriptvalue_cast<Triangle>(context->argument(0));
    This.Enclose(triangle);
    ToExistingScriptValue_AABB(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue AABB_Enclose_Capsule(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Enclose_Capsule in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    Capsule capsule = qscriptvalue_cast<Capsule>(context->argument(0));
    This.Enclose(capsule);
    ToExistingScriptValue_AABB(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue AABB_Enclose_Frustum(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Enclose_Frustum in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    Frustum frustum = qscriptvalue_cast<Frustum>(context->argument(0));
    This.Enclose(frustum);
    ToExistingScriptValue_AABB(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue AABB_Enclose_Polygon(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Enclose_Polygon in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    Polygon polygon = qscriptvalue_cast<Polygon>(context->argument(0));
    This.Enclose(polygon);
    ToExistingScriptValue_AABB(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue AABB_Enclose_Polyhedron(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Enclose_Polyhedron in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    Polyhedron polyhedron = qscriptvalue_cast<Polyhedron>(context->argument(0));
    This.Enclose(polyhedron);
    ToExistingScriptValue_AABB(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue AABB_toString_const(QScriptContext *context, QScriptEngine *engine)
{
    AABB This;
    if (context->argumentCount() > 0) This = qscriptvalue_cast<AABB>(context->argument(0)); // Qt oddity (bug?): Sometimes the built-in toString() function doesn't give us this from thisObject, but as the first argument.
    else This = qscriptvalue_cast<AABB>(context->thisObject());
    QString ret = This.toString();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Intersection_AABB_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Intersection_AABB_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    AABB aabb = qscriptvalue_cast<AABB>(context->argument(0));
    AABB ret = This.Intersection(aabb);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_IntersectRayAABB_float3_float3_float_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function AABB_IntersectRayAABB_float3_float3_float_float_const in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float3 rayPos = qscriptvalue_cast<float3>(context->argument(0));
    float3 rayDir = qscriptvalue_cast<float3>(context->argument(1));
    float tNear = qscriptvalue_cast<float>(context->argument(2));
    float tFar = qscriptvalue_cast<float>(context->argument(3));
    bool ret = This.IntersectRayAABB(rayPos, rayDir, tNear, tFar);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_FromCenterAndSize_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function AABB_FromCenterAndSize_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 aabbCenterPos = qscriptvalue_cast<float3>(context->argument(0));
    float3 aabbSize = qscriptvalue_cast<float3>(context->argument(1));
    AABB ret = AABB::FromCenterAndSize(aabbCenterPos, aabbSize);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_NumVerticesInTriangulation_int_int_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function AABB_NumVerticesInTriangulation_int_int_int in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    int numFacesX = qscriptvalue_cast<int>(context->argument(0));
    int numFacesY = qscriptvalue_cast<int>(context->argument(1));
    int numFacesZ = qscriptvalue_cast<int>(context->argument(2));
    int ret = AABB::NumVerticesInTriangulation(numFacesX, numFacesY, numFacesZ);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_NumVerticesInEdgeList(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_NumVerticesInEdgeList in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    int ret = AABB::NumVerticesInEdgeList();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_ctor(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return AABB_AABB(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return AABB_AABB_float3_float3(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<OBB>(context->argument(0)))
        return AABB_AABB_OBB(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Sphere>(context->argument(0)))
        return AABB_AABB_Sphere(context, engine);
    printf("AABB_ctor failed to choose the right function to call! Did you use 'var x = AABB();' instead of 'var x = new AABB();'?\n"); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue AABB_SetFrom_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<OBB>(context->argument(0)))
        return AABB_SetFrom_OBB(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Sphere>(context->argument(0)))
        return AABB_SetFrom_Sphere(context, engine);
    printf("AABB_SetFrom_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue AABB_Scale_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return AABB_Scale_float3_float(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return AABB_Scale_float3_float3(context, engine);
    printf("AABB_Scale_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue AABB_TransformAsAABB_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3x3>(context->argument(0)))
        return AABB_TransformAsAABB_float3x3(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3x4>(context->argument(0)))
        return AABB_TransformAsAABB_float3x4(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float4x4>(context->argument(0)))
        return AABB_TransformAsAABB_float4x4(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Quat>(context->argument(0)))
        return AABB_TransformAsAABB_Quat(context, engine);
    printf("AABB_TransformAsAABB_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue AABB_Transform_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3x3>(context->argument(0)))
        return AABB_Transform_float3x3_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3x4>(context->argument(0)))
        return AABB_Transform_float3x4_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float4x4>(context->argument(0)))
        return AABB_Transform_float4x4_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Quat>(context->argument(0)))
        return AABB_Transform_Quat_const(context, engine);
    printf("AABB_Transform_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue AABB_Distance_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return AABB_Distance_float3_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Sphere>(context->argument(0)))
        return AABB_Distance_Sphere_const(context, engine);
    printf("AABB_Distance_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue AABB_Contains_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return AABB_Contains_float3_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<LineSegment>(context->argument(0)))
        return AABB_Contains_LineSegment_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<AABB>(context->argument(0)))
        return AABB_Contains_AABB_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<OBB>(context->argument(0)))
        return AABB_Contains_OBB_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Sphere>(context->argument(0)))
        return AABB_Contains_Sphere_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Triangle>(context->argument(0)))
        return AABB_Contains_Triangle_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Polygon>(context->argument(0)))
        return AABB_Contains_Polygon_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Frustum>(context->argument(0)))
        return AABB_Contains_Frustum_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Polyhedron>(context->argument(0)))
        return AABB_Contains_Polyhedron_const(context, engine);
    printf("AABB_Contains_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue AABB_Intersects_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<Plane>(context->argument(0)))
        return AABB_Intersects_Plane_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<AABB>(context->argument(0)))
        return AABB_Intersects_AABB_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<OBB>(context->argument(0)))
        return AABB_Intersects_OBB_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Capsule>(context->argument(0)))
        return AABB_Intersects_Capsule_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Triangle>(context->argument(0)))
        return AABB_Intersects_Triangle_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Polygon>(context->argument(0)))
        return AABB_Intersects_Polygon_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Frustum>(context->argument(0)))
        return AABB_Intersects_Frustum_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Polyhedron>(context->argument(0)))
        return AABB_Intersects_Polyhedron_const(context, engine);
    printf("AABB_Intersects_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue AABB_Enclose_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return AABB_Enclose_float3(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<LineSegment>(context->argument(0)))
        return AABB_Enclose_LineSegment(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<AABB>(context->argument(0)))
        return AABB_Enclose_AABB(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<OBB>(context->argument(0)))
        return AABB_Enclose_OBB(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Sphere>(context->argument(0)))
        return AABB_Enclose_Sphere(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Triangle>(context->argument(0)))
        return AABB_Enclose_Triangle(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Capsule>(context->argument(0)))
        return AABB_Enclose_Capsule(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Frustum>(context->argument(0)))
        return AABB_Enclose_Frustum(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Polygon>(context->argument(0)))
        return AABB_Enclose_Polygon(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Polyhedron>(context->argument(0)))
        return AABB_Enclose_Polyhedron(context, engine);
    printf("AABB_Enclose_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

void FromScriptValue_AABB(const QScriptValue &obj, AABB &value)
{
    value.minPoint = qScriptValueToValue<float3>(obj.property("minPoint"));
    value.maxPoint = qScriptValueToValue<float3>(obj.property("maxPoint"));
}

QScriptValue ToScriptValue_AABB(QScriptEngine *engine, const AABB &value)
{
    QScriptValue obj = engine->newVariant(QVariant::fromValue(value)); // The contents of this variant are NOT used. The real data lies in the data() pointer of this QScriptValue. This only exists to enable overload resolution to work for QObject slots.
    ToExistingScriptValue_AABB(engine, value, obj);
    return obj;
}

QScriptValue ToScriptValue_const_AABB(QScriptEngine *engine, const AABB &value)
{
    QScriptValue obj = engine->newVariant(QVariant::fromValue(value)); // The contents of this variant are NOT used. The real data lies in the data() pointer of this QScriptValue. This only exists to enable overload resolution to work for QObject slots.
    obj.setPrototype(engine->defaultPrototype(qMetaTypeId<AABB>()));
    obj.setProperty("minPoint", ToScriptValue_const_float3(engine, value.minPoint), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("maxPoint", ToScriptValue_const_float3(engine, value.maxPoint), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    return obj;
}

QScriptValue register_AABB_prototype(QScriptEngine *engine)
{
    QScriptValue proto = engine->newObject();
    proto.setProperty("MinX", engine->newFunction(AABB_MinX_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MinY", engine->newFunction(AABB_MinY_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MinZ", engine->newFunction(AABB_MinZ_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MaxX", engine->newFunction(AABB_MaxX_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MaxY", engine->newFunction(AABB_MaxY_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MaxZ", engine->newFunction(AABB_MaxZ_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetNegativeInfinity", engine->newFunction(AABB_SetNegativeInfinity, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetFromCenterAndSize", engine->newFunction(AABB_SetFromCenterAndSize_float3_float3, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetFrom", engine->newFunction(AABB_SetFrom_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToPolyhedron", engine->newFunction(AABB_ToPolyhedron_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToOBB", engine->newFunction(AABB_ToOBB_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MinimalEnclosingSphere", engine->newFunction(AABB_MinimalEnclosingSphere_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MaximalContainedSphere", engine->newFunction(AABB_MaximalContainedSphere_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsFinite", engine->newFunction(AABB_IsFinite_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsDegenerate", engine->newFunction(AABB_IsDegenerate_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("CenterPoint", engine->newFunction(AABB_CenterPoint_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Centroid", engine->newFunction(AABB_Centroid_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("PointInside", engine->newFunction(AABB_PointInside_float_float_float_const, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Edge", engine->newFunction(AABB_Edge_int_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("CornerPoint", engine->newFunction(AABB_CornerPoint_int_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ExtremePoint", engine->newFunction(AABB_ExtremePoint_float3_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("PointOnEdge", engine->newFunction(AABB_PointOnEdge_int_float_const, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("FaceCenterPoint", engine->newFunction(AABB_FaceCenterPoint_int_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("FacePoint", engine->newFunction(AABB_FacePoint_int_float_float_const, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("FaceNormal", engine->newFunction(AABB_FaceNormal_int_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("FacePlane", engine->newFunction(AABB_FacePlane_int_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("GetFacePlanes", engine->newFunction(AABB_GetFacePlanes_Plane_ptr_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Size", engine->newFunction(AABB_Size_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("HalfSize", engine->newFunction(AABB_HalfSize_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Diagonal", engine->newFunction(AABB_Diagonal_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("HalfDiagonal", engine->newFunction(AABB_HalfDiagonal_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Volume", engine->newFunction(AABB_Volume_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SurfaceArea", engine->newFunction(AABB_SurfaceArea_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("RandomPointInside", engine->newFunction(AABB_RandomPointInside_LCG_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("RandomPointOnSurface", engine->newFunction(AABB_RandomPointOnSurface_LCG_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("RandomPointOnEdge", engine->newFunction(AABB_RandomPointOnEdge_LCG_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("RandomCornerPoint", engine->newFunction(AABB_RandomCornerPoint_LCG_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Translate", engine->newFunction(AABB_Translate_float3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Scale", engine->newFunction(AABB_Scale_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("TransformAsAABB", engine->newFunction(AABB_TransformAsAABB_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Transform", engine->newFunction(AABB_Transform_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ClosestPoint", engine->newFunction(AABB_ClosestPoint_float3_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Distance", engine->newFunction(AABB_Distance_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Contains", engine->newFunction(AABB_Contains_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Intersects", engine->newFunction(AABB_Intersects_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ProjectToAxis", engine->newFunction(AABB_ProjectToAxis_float3_float_float_const, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Enclose", engine->newFunction(AABB_Enclose_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("toString", engine->newFunction(AABB_toString_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Intersection", engine->newFunction(AABB_Intersection_AABB_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IntersectRayAABB", engine->newFunction(AABB_IntersectRayAABB_float3_float3_float_float_const, 4), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("metaTypeId", engine->toScriptValue<qint32>((qint32)qMetaTypeId<AABB>()));
    engine->setDefaultPrototype(qMetaTypeId<AABB>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<AABB*>(), proto);
    qScriptRegisterMetaType(engine, ToScriptValue_AABB, FromScriptValue_AABB, proto);

    QScriptValue ctor = engine->newFunction(AABB_ctor, proto, 2);
    ctor.setProperty("FromCenterAndSize", engine->newFunction(AABB_FromCenterAndSize_float3_float3, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("NumVerticesInTriangulation", engine->newFunction(AABB_NumVerticesInTriangulation_int_int_int, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("NumVerticesInEdgeList", engine->newFunction(AABB_NumVerticesInEdgeList, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    engine->globalObject().setProperty("AABB", ctor, QScriptValue::Undeletable | QScriptValue::ReadOnly);

    return ctor;
}

