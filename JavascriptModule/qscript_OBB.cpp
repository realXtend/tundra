#include "QtScriptBindingsHelpers.h"

void ToExistingScriptValue_OBB(QScriptEngine *engine, const OBB &value, QScriptValue obj)
{
    obj.setProperty("pos", qScriptValueFromValue(engine, value.pos), QScriptValue::Undeletable);
    obj.setProperty("r", qScriptValueFromValue(engine, value.r), QScriptValue::Undeletable);
}

static QScriptValue OBB_OBB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function OBB_OBB in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB ret;
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue OBB_OBB_AABB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_OBB_AABB in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB aabb = qscriptvalue_cast<AABB>(context->argument(0));
    OBB ret(aabb);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue OBB_SetNegativeInfinity(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function OBB_SetNegativeInfinity in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    This.SetNegativeInfinity();
    ToExistingScriptValue_OBB(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue OBB_SetFrom_AABB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_SetFrom_AABB in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    AABB aabb = qscriptvalue_cast<AABB>(context->argument(0));
    This.SetFrom(aabb);
    ToExistingScriptValue_OBB(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue OBB_SetFrom_AABB_float3x3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function OBB_SetFrom_AABB_float3x3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    AABB aabb = qscriptvalue_cast<AABB>(context->argument(0));
    float3x3 transform = qscriptvalue_cast<float3x3>(context->argument(1));
    This.SetFrom(aabb, transform);
    ToExistingScriptValue_OBB(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue OBB_SetFrom_AABB_float3x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function OBB_SetFrom_AABB_float3x4 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    AABB aabb = qscriptvalue_cast<AABB>(context->argument(0));
    float3x4 transform = qscriptvalue_cast<float3x4>(context->argument(1));
    This.SetFrom(aabb, transform);
    ToExistingScriptValue_OBB(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue OBB_SetFrom_AABB_float4x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function OBB_SetFrom_AABB_float4x4 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    AABB aabb = qscriptvalue_cast<AABB>(context->argument(0));
    float4x4 transform = qscriptvalue_cast<float4x4>(context->argument(1));
    This.SetFrom(aabb, transform);
    ToExistingScriptValue_OBB(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue OBB_SetFrom_AABB_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function OBB_SetFrom_AABB_Quat in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    AABB aabb = qscriptvalue_cast<AABB>(context->argument(0));
    Quat transform = qscriptvalue_cast<Quat>(context->argument(1));
    This.SetFrom(aabb, transform);
    ToExistingScriptValue_OBB(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue OBB_SetFrom_Sphere(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_SetFrom_Sphere in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    Sphere sphere = qscriptvalue_cast<Sphere>(context->argument(0));
    This.SetFrom(sphere);
    ToExistingScriptValue_OBB(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue OBB_MinimalEnclosingAABB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function OBB_MinimalEnclosingAABB in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    AABB ret = This.MinimalEnclosingAABB();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue OBB_MaximalContainedAABB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function OBB_MaximalContainedAABB in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    AABB ret = This.MaximalContainedAABB();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue OBB_Size(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function OBB_Size in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    float3 ret = This.Size();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue OBB_HalfSize(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function OBB_HalfSize in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    float3 ret = This.HalfSize();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue OBB_Diagonal(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function OBB_Diagonal in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    float3 ret = This.Diagonal();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue OBB_HalfDiagonal(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function OBB_HalfDiagonal in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    float3 ret = This.HalfDiagonal();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue OBB_WorldToLocal(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function OBB_WorldToLocal in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    float3x4 ret = This.WorldToLocal();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue OBB_LocalToWorld(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function OBB_LocalToWorld in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    float3x4 ret = This.LocalToWorld();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue OBB_MinimalEnclosingSphere(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function OBB_MinimalEnclosingSphere in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    Sphere ret = This.MinimalEnclosingSphere();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue OBB_MaximalContainedSphere(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function OBB_MaximalContainedSphere in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    Sphere ret = This.MaximalContainedSphere();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue OBB_IsFinite(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function OBB_IsFinite in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    bool ret = This.IsFinite();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue OBB_IsDegenerate(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function OBB_IsDegenerate in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    bool ret = This.IsDegenerate();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue OBB_CenterPoint(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function OBB_CenterPoint in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    float3 ret = This.CenterPoint();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue OBB_GetPointInside_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function OBB_GetPointInside_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    float x = qscriptvalue_cast<float>(context->argument(0));
    float y = qscriptvalue_cast<float>(context->argument(1));
    float z = qscriptvalue_cast<float>(context->argument(2));
    float3 ret = This.GetPointInside(x, y, z);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue OBB_CornerPoint_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_CornerPoint_int in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    int cornerIndex = qscriptvalue_cast<int>(context->argument(0));
    float3 ret = This.CornerPoint(cornerIndex);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue OBB_PointOnEdge_int_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function OBB_PointOnEdge_int_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    int edgeIndex = qscriptvalue_cast<int>(context->argument(0));
    float u = qscriptvalue_cast<float>(context->argument(1));
    float3 ret = This.PointOnEdge(edgeIndex, u);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue OBB_FaceCenterPoint_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_FaceCenterPoint_int in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    int faceIndex = qscriptvalue_cast<int>(context->argument(0));
    float3 ret = This.FaceCenterPoint(faceIndex);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue OBB_FacePoint_int_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function OBB_FacePoint_int_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    int faceIndex = qscriptvalue_cast<int>(context->argument(0));
    float u = qscriptvalue_cast<float>(context->argument(1));
    float v = qscriptvalue_cast<float>(context->argument(2));
    float3 ret = This.FacePoint(faceIndex, u, v);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue OBB_FacePlane_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_FacePlane_int in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    int faceIndex = qscriptvalue_cast<int>(context->argument(0));
    Plane ret = This.FacePlane(faceIndex);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue OBB_GetFacePlanes_Plane_ptr(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_GetFacePlanes_Plane_ptr in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    Plane * outPlaneArray = qscriptvalue_cast<Plane *>(context->argument(0));
    This.GetFacePlanes(outPlaneArray);
    return QScriptValue();
}

static QScriptValue OBB_Volume(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function OBB_Volume in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    float ret = This.Volume();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue OBB_SurfaceArea(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function OBB_SurfaceArea in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    float ret = This.SurfaceArea();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue OBB_RandomPointInside_LCG(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_RandomPointInside_LCG in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    LCG rng = qscriptvalue_cast<LCG>(context->argument(0));
    float3 ret = This.RandomPointInside(rng);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue OBB_RandomPointOnSurface_LCG(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_RandomPointOnSurface_LCG in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    LCG rng = qscriptvalue_cast<LCG>(context->argument(0));
    float3 ret = This.RandomPointOnSurface(rng);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue OBB_RandomPointOnEdge_LCG(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_RandomPointOnEdge_LCG in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    LCG rng = qscriptvalue_cast<LCG>(context->argument(0));
    float3 ret = This.RandomPointOnEdge(rng);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue OBB_RandomCornerPoint_LCG(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_RandomCornerPoint_LCG in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    LCG rng = qscriptvalue_cast<LCG>(context->argument(0));
    float3 ret = This.RandomCornerPoint(rng);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue OBB_Translate_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_Translate_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    float3 offset = qscriptvalue_cast<float3>(context->argument(0));
    This.Translate(offset);
    ToExistingScriptValue_OBB(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue OBB_Scale_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function OBB_Scale_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    float3 centerPoint = qscriptvalue_cast<float3>(context->argument(0));
    float scaleFactor = qscriptvalue_cast<float>(context->argument(1));
    This.Scale(centerPoint, scaleFactor);
    ToExistingScriptValue_OBB(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue OBB_Scale_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function OBB_Scale_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    float3 centerPoint = qscriptvalue_cast<float3>(context->argument(0));
    float3 scaleFactor = qscriptvalue_cast<float3>(context->argument(1));
    This.Scale(centerPoint, scaleFactor);
    ToExistingScriptValue_OBB(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue OBB_Transform_float3x3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_Transform_float3x3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    float3x3 transform = qscriptvalue_cast<float3x3>(context->argument(0));
    This.Transform(transform);
    ToExistingScriptValue_OBB(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue OBB_Transform_float3x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_Transform_float3x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    float3x4 transform = qscriptvalue_cast<float3x4>(context->argument(0));
    This.Transform(transform);
    ToExistingScriptValue_OBB(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue OBB_Transform_float4x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_Transform_float4x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    float4x4 transform = qscriptvalue_cast<float4x4>(context->argument(0));
    This.Transform(transform);
    ToExistingScriptValue_OBB(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue OBB_Transform_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_Transform_Quat in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    Quat transform = qscriptvalue_cast<Quat>(context->argument(0));
    This.Transform(transform);
    ToExistingScriptValue_OBB(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue OBB_ClosestPoint_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_ClosestPoint_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    float3 targetPoint = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.ClosestPoint(targetPoint);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue OBB_Distance_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_Distance_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float ret = This.Distance(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue OBB_Contains_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_Contains_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    bool ret = This.Contains(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue OBB_Contains_LineSegment(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_Contains_LineSegment in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    LineSegment lineSegment = qscriptvalue_cast<LineSegment>(context->argument(0));
    bool ret = This.Contains(lineSegment);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue OBB_Contains_AABB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_Contains_AABB in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    AABB aabb = qscriptvalue_cast<AABB>(context->argument(0));
    bool ret = This.Contains(aabb);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue OBB_Contains_OBB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_Contains_OBB in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    OBB obb = qscriptvalue_cast<OBB>(context->argument(0));
    bool ret = This.Contains(obb);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue OBB_Intersects_AABB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_Intersects_AABB in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    AABB aabb = qscriptvalue_cast<AABB>(context->argument(0));
    bool ret = This.Intersects(aabb);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue OBB_Intersects_OBB_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function OBB_Intersects_OBB_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    OBB b = qscriptvalue_cast<OBB>(context->argument(0));
    float epsilon = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.Intersects(b, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue OBB_Intersects_Plane(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_Intersects_Plane in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    Plane plane = qscriptvalue_cast<Plane>(context->argument(0));
    bool ret = This.Intersects(plane);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue OBB_Intersects_Triangle(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_Intersects_Triangle in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB This = qscriptvalue_cast<OBB>(context->thisObject());
    Triangle triangle = qscriptvalue_cast<Triangle>(context->argument(0));
    bool ret = This.Intersects(triangle);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue OBB_ctor(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return OBB_OBB(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<AABB>(context->argument(0)))
        return OBB_OBB_AABB(context, engine);
    printf("OBB_ctor failed to choose the right function to call! Did you use 'var x = OBB();' instead of 'var x = new OBB();'?\n"); return QScriptValue();
}

static QScriptValue OBB_SetFrom_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<AABB>(context->argument(0)))
        return OBB_SetFrom_AABB(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<AABB>(context->argument(0)) && QSVIsOfType<float3x3>(context->argument(1)))
        return OBB_SetFrom_AABB_float3x3(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<AABB>(context->argument(0)) && QSVIsOfType<float3x4>(context->argument(1)))
        return OBB_SetFrom_AABB_float3x4(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<AABB>(context->argument(0)) && QSVIsOfType<float4x4>(context->argument(1)))
        return OBB_SetFrom_AABB_float4x4(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<AABB>(context->argument(0)) && QSVIsOfType<Quat>(context->argument(1)))
        return OBB_SetFrom_AABB_Quat(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Sphere>(context->argument(0)))
        return OBB_SetFrom_Sphere(context, engine);
    printf("OBB_SetFrom_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue OBB_Scale_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return OBB_Scale_float3_float(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return OBB_Scale_float3_float3(context, engine);
    printf("OBB_Scale_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue OBB_Transform_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3x3>(context->argument(0)))
        return OBB_Transform_float3x3(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3x4>(context->argument(0)))
        return OBB_Transform_float3x4(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float4x4>(context->argument(0)))
        return OBB_Transform_float4x4(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Quat>(context->argument(0)))
        return OBB_Transform_Quat(context, engine);
    printf("OBB_Transform_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue OBB_Contains_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return OBB_Contains_float3(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<LineSegment>(context->argument(0)))
        return OBB_Contains_LineSegment(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<AABB>(context->argument(0)))
        return OBB_Contains_AABB(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<OBB>(context->argument(0)))
        return OBB_Contains_OBB(context, engine);
    printf("OBB_Contains_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue OBB_Intersects_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<AABB>(context->argument(0)))
        return OBB_Intersects_AABB(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<OBB>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return OBB_Intersects_OBB_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Plane>(context->argument(0)))
        return OBB_Intersects_Plane(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Triangle>(context->argument(0)))
        return OBB_Intersects_Triangle(context, engine);
    printf("OBB_Intersects_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

void FromScriptValue_OBB(const QScriptValue &obj, OBB &value)
{
    value.pos = qScriptValueToValue<float3>(obj.property("pos"));
    value.r = qScriptValueToValue<float3>(obj.property("r"));
}

QScriptValue ToScriptValue_OBB(QScriptEngine *engine, const OBB &value)
{
    QScriptValue obj = engine->newObject();
    ToExistingScriptValue_OBB(engine, value, obj);
    return obj;
}

QScriptValue ToScriptValue_const_OBB(QScriptEngine *engine, const OBB &value)
{
    QScriptValue obj = engine->newObject();
    obj.setPrototype(engine->defaultPrototype(qMetaTypeId<OBB>()));
    obj.setProperty("pos", ToScriptValue_const_float3(engine, value.pos), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("r", ToScriptValue_const_float3(engine, value.r), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    return obj;
}

QScriptValue register_OBB_prototype(QScriptEngine *engine)
{
    QScriptValue proto = engine->newObject();
    proto.setProperty("SetNegativeInfinity", engine->newFunction(OBB_SetNegativeInfinity, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetFrom", engine->newFunction(OBB_SetFrom_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetFrom", engine->newFunction(OBB_SetFrom_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MinimalEnclosingAABB", engine->newFunction(OBB_MinimalEnclosingAABB, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MaximalContainedAABB", engine->newFunction(OBB_MaximalContainedAABB, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Size", engine->newFunction(OBB_Size, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("HalfSize", engine->newFunction(OBB_HalfSize, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Diagonal", engine->newFunction(OBB_Diagonal, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("HalfDiagonal", engine->newFunction(OBB_HalfDiagonal, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("WorldToLocal", engine->newFunction(OBB_WorldToLocal, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("LocalToWorld", engine->newFunction(OBB_LocalToWorld, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MinimalEnclosingSphere", engine->newFunction(OBB_MinimalEnclosingSphere, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MaximalContainedSphere", engine->newFunction(OBB_MaximalContainedSphere, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsFinite", engine->newFunction(OBB_IsFinite, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsDegenerate", engine->newFunction(OBB_IsDegenerate, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("CenterPoint", engine->newFunction(OBB_CenterPoint, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("GetPointInside", engine->newFunction(OBB_GetPointInside_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("CornerPoint", engine->newFunction(OBB_CornerPoint_int, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("PointOnEdge", engine->newFunction(OBB_PointOnEdge_int_float, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("FaceCenterPoint", engine->newFunction(OBB_FaceCenterPoint_int, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("FacePoint", engine->newFunction(OBB_FacePoint_int_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("FacePlane", engine->newFunction(OBB_FacePlane_int, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("GetFacePlanes", engine->newFunction(OBB_GetFacePlanes_Plane_ptr, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Volume", engine->newFunction(OBB_Volume, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SurfaceArea", engine->newFunction(OBB_SurfaceArea, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("RandomPointInside", engine->newFunction(OBB_RandomPointInside_LCG, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("RandomPointOnSurface", engine->newFunction(OBB_RandomPointOnSurface_LCG, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("RandomPointOnEdge", engine->newFunction(OBB_RandomPointOnEdge_LCG, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("RandomCornerPoint", engine->newFunction(OBB_RandomCornerPoint_LCG, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Translate", engine->newFunction(OBB_Translate_float3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Scale", engine->newFunction(OBB_Scale_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Transform", engine->newFunction(OBB_Transform_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ClosestPoint", engine->newFunction(OBB_ClosestPoint_float3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Distance", engine->newFunction(OBB_Distance_float3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Contains", engine->newFunction(OBB_Contains_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Intersects", engine->newFunction(OBB_Intersects_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Intersects", engine->newFunction(OBB_Intersects_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("metaTypeId", engine->toScriptValue<qint32>((qint32)qMetaTypeId<OBB>()));
    engine->setDefaultPrototype(qMetaTypeId<OBB>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<OBB*>(), proto);
    qScriptRegisterMetaType(engine, ToScriptValue_OBB, FromScriptValue_OBB, proto);

    QScriptValue ctor = engine->newFunction(OBB_ctor, proto, 1);
    engine->globalObject().setProperty("OBB", ctor, QScriptValue::Undeletable | QScriptValue::ReadOnly);

    return ctor;
}

