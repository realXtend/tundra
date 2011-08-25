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

static QScriptValue AABB_MinX(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_MinX in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float ret = This.MinX();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_MinY(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_MinY in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float ret = This.MinY();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_MinZ(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_MinZ in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float ret = This.MinZ();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_MaxX(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_MaxX in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float ret = This.MaxX();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_MaxY(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_MaxY in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float ret = This.MaxY();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_MaxZ(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_MaxZ in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
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

static QScriptValue AABB_SetCenter_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function AABB_SetCenter_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float3 center = qscriptvalue_cast<float3>(context->argument(0));
    float3 halfSize = qscriptvalue_cast<float3>(context->argument(1));
    This.SetCenter(center, halfSize);
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

static QScriptValue AABB_ToOBB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_ToOBB in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    OBB ret = This.ToOBB();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_MinimalEnclosingSphere(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_MinimalEnclosingSphere in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    Sphere ret = This.MinimalEnclosingSphere();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_MaximalContainedSphere(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_MaximalContainedSphere in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    Sphere ret = This.MaximalContainedSphere();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_IsFinite(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_IsFinite in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    bool ret = This.IsFinite();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_IsDegenerate(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_IsDegenerate in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    bool ret = This.IsDegenerate();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_CenterPoint(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_CenterPoint in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float3 ret = This.CenterPoint();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_PointInside_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function AABB_PointInside_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float x = qscriptvalue_cast<float>(context->argument(0));
    float y = qscriptvalue_cast<float>(context->argument(1));
    float z = qscriptvalue_cast<float>(context->argument(2));
    float3 ret = This.PointInside(x, y, z);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Edge_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Edge_int in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    int edgeIndex = qscriptvalue_cast<int>(context->argument(0));
    LineSegment ret = This.Edge(edgeIndex);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_CornerPoint_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_CornerPoint_int in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    int cornerIndex = qscriptvalue_cast<int>(context->argument(0));
    float3 ret = This.CornerPoint(cornerIndex);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_PointOnEdge_int_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function AABB_PointOnEdge_int_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    int edgeIndex = qscriptvalue_cast<int>(context->argument(0));
    float u = qscriptvalue_cast<float>(context->argument(1));
    float3 ret = This.PointOnEdge(edgeIndex, u);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_FaceCenterPoint_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_FaceCenterPoint_int in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    int faceIndex = qscriptvalue_cast<int>(context->argument(0));
    float3 ret = This.FaceCenterPoint(faceIndex);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_FacePoint_int_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function AABB_FacePoint_int_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    int faceIndex = qscriptvalue_cast<int>(context->argument(0));
    float u = qscriptvalue_cast<float>(context->argument(1));
    float v = qscriptvalue_cast<float>(context->argument(2));
    float3 ret = This.FacePoint(faceIndex, u, v);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_FacePlane_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_FacePlane_int in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    int faceIndex = qscriptvalue_cast<int>(context->argument(0));
    Plane ret = This.FacePlane(faceIndex);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_GetFacePlanes_Plane_ptr(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_GetFacePlanes_Plane_ptr in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    Plane * outPlaneArray = qscriptvalue_cast<Plane *>(context->argument(0));
    This.GetFacePlanes(outPlaneArray);
    return QScriptValue();
}

static QScriptValue AABB_Size(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_Size in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float3 ret = This.Size();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_HalfSize(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_HalfSize in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float3 ret = This.HalfSize();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Diagonal(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_Diagonal in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float3 ret = This.Diagonal();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_HalfDiagonal(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_HalfDiagonal in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float3 ret = This.HalfDiagonal();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Volume(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_Volume in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float ret = This.Volume();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_SurfaceArea(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_SurfaceArea in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float ret = This.SurfaceArea();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_RandomPointInside_LCG(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_RandomPointInside_LCG in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    LCG rng = qscriptvalue_cast<LCG>(context->argument(0));
    float3 ret = This.RandomPointInside(rng);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_RandomPointOnSurface_LCG(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_RandomPointOnSurface_LCG in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    LCG rng = qscriptvalue_cast<LCG>(context->argument(0));
    float3 ret = This.RandomPointOnSurface(rng);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_RandomPointOnEdge_LCG(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_RandomPointOnEdge_LCG in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    LCG rng = qscriptvalue_cast<LCG>(context->argument(0));
    float3 ret = This.RandomPointOnEdge(rng);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_RandomCornerPoint_LCG(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_RandomCornerPoint_LCG in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
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

static QScriptValue AABB_Transform_float3x3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Transform_float3x3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float3x3 transform = qscriptvalue_cast<float3x3>(context->argument(0));
    OBB ret = This.Transform(transform);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Transform_float3x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Transform_float3x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float3x4 transform = qscriptvalue_cast<float3x4>(context->argument(0));
    OBB ret = This.Transform(transform);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Transform_float4x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Transform_float4x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float4x4 transform = qscriptvalue_cast<float4x4>(context->argument(0));
    OBB ret = This.Transform(transform);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Transform_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Transform_Quat in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    Quat transform = qscriptvalue_cast<Quat>(context->argument(0));
    OBB ret = This.Transform(transform);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_ClosestPoint_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_ClosestPoint_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float3 targetPoint = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.ClosestPoint(targetPoint);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Distance_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Distance_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float ret = This.Distance(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Contains_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Contains_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    bool ret = This.Contains(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Contains_LineSegment(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Contains_LineSegment in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    LineSegment lineSegment = qscriptvalue_cast<LineSegment>(context->argument(0));
    bool ret = This.Contains(lineSegment);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Contains_AABB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Contains_AABB in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    AABB aabb = qscriptvalue_cast<AABB>(context->argument(0));
    bool ret = This.Contains(aabb);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Intersects_Plane(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Intersects_Plane in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    Plane plane = qscriptvalue_cast<Plane>(context->argument(0));
    bool ret = This.Intersects(plane);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Intersects_OBB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Intersects_OBB in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    OBB obb = qscriptvalue_cast<OBB>(context->argument(0));
    bool ret = This.Intersects(obb);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Intersects_Triangle(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Intersects_Triangle in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    Triangle triangle = qscriptvalue_cast<Triangle>(context->argument(0));
    bool ret = This.Intersects(triangle);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_ProjectToAxis_float3_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function AABB_ProjectToAxis_float3_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
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

static QScriptValue AABB_toString(QScriptContext *context, QScriptEngine *engine)
{
    AABB This;
    if (context->argumentCount() > 0) This = qscriptvalue_cast<AABB>(context->argument(0)); // Qt oddity (bug?): Sometimes the built-in toString() function doesn't give us this from thisObject, but as the first argument.
    else This = qscriptvalue_cast<AABB>(context->thisObject());
    QString ret = This.toString();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue AABB_Intersection_AABB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Intersection_AABB in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    AABB This = qscriptvalue_cast<AABB>(context->thisObject());
    AABB aabb = qscriptvalue_cast<AABB>(context->argument(0));
    AABB ret = This.Intersection(aabb);
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
        return AABB_Transform_float3x3(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3x4>(context->argument(0)))
        return AABB_Transform_float3x4(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float4x4>(context->argument(0)))
        return AABB_Transform_float4x4(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Quat>(context->argument(0)))
        return AABB_Transform_Quat(context, engine);
    printf("AABB_Transform_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue AABB_Contains_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return AABB_Contains_float3(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<LineSegment>(context->argument(0)))
        return AABB_Contains_LineSegment(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<AABB>(context->argument(0)))
        return AABB_Contains_AABB(context, engine);
    printf("AABB_Contains_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue AABB_Intersects_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<Plane>(context->argument(0)))
        return AABB_Intersects_Plane(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<OBB>(context->argument(0)))
        return AABB_Intersects_OBB(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Triangle>(context->argument(0)))
        return AABB_Intersects_Triangle(context, engine);
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
    proto.setProperty("MinX", engine->newFunction(AABB_MinX, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MinY", engine->newFunction(AABB_MinY, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MinZ", engine->newFunction(AABB_MinZ, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MaxX", engine->newFunction(AABB_MaxX, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MaxY", engine->newFunction(AABB_MaxY, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MaxZ", engine->newFunction(AABB_MaxZ, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetNegativeInfinity", engine->newFunction(AABB_SetNegativeInfinity, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetCenter", engine->newFunction(AABB_SetCenter_float3_float3, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetFrom", engine->newFunction(AABB_SetFrom_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToOBB", engine->newFunction(AABB_ToOBB, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MinimalEnclosingSphere", engine->newFunction(AABB_MinimalEnclosingSphere, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MaximalContainedSphere", engine->newFunction(AABB_MaximalContainedSphere, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsFinite", engine->newFunction(AABB_IsFinite, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsDegenerate", engine->newFunction(AABB_IsDegenerate, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("CenterPoint", engine->newFunction(AABB_CenterPoint, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("PointInside", engine->newFunction(AABB_PointInside_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Edge", engine->newFunction(AABB_Edge_int, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("CornerPoint", engine->newFunction(AABB_CornerPoint_int, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("PointOnEdge", engine->newFunction(AABB_PointOnEdge_int_float, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("FaceCenterPoint", engine->newFunction(AABB_FaceCenterPoint_int, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("FacePoint", engine->newFunction(AABB_FacePoint_int_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("FacePlane", engine->newFunction(AABB_FacePlane_int, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("GetFacePlanes", engine->newFunction(AABB_GetFacePlanes_Plane_ptr, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Size", engine->newFunction(AABB_Size, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("HalfSize", engine->newFunction(AABB_HalfSize, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Diagonal", engine->newFunction(AABB_Diagonal, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("HalfDiagonal", engine->newFunction(AABB_HalfDiagonal, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Volume", engine->newFunction(AABB_Volume, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SurfaceArea", engine->newFunction(AABB_SurfaceArea, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("RandomPointInside", engine->newFunction(AABB_RandomPointInside_LCG, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("RandomPointOnSurface", engine->newFunction(AABB_RandomPointOnSurface_LCG, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("RandomPointOnEdge", engine->newFunction(AABB_RandomPointOnEdge_LCG, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("RandomCornerPoint", engine->newFunction(AABB_RandomCornerPoint_LCG, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Translate", engine->newFunction(AABB_Translate_float3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Scale", engine->newFunction(AABB_Scale_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("TransformAsAABB", engine->newFunction(AABB_TransformAsAABB_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Transform", engine->newFunction(AABB_Transform_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ClosestPoint", engine->newFunction(AABB_ClosestPoint_float3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Distance", engine->newFunction(AABB_Distance_float3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Contains", engine->newFunction(AABB_Contains_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Intersects", engine->newFunction(AABB_Intersects_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ProjectToAxis", engine->newFunction(AABB_ProjectToAxis_float3_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Enclose", engine->newFunction(AABB_Enclose_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("toString", engine->newFunction(AABB_toString, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Intersection", engine->newFunction(AABB_Intersection_AABB, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("metaTypeId", engine->toScriptValue<qint32>((qint32)qMetaTypeId<AABB>()));
    engine->setDefaultPrototype(qMetaTypeId<AABB>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<AABB*>(), proto);
    qScriptRegisterMetaType(engine, ToScriptValue_AABB, FromScriptValue_AABB, proto);

    QScriptValue ctor = engine->newFunction(AABB_ctor, proto, 2);
    engine->globalObject().setProperty("AABB", ctor, QScriptValue::Undeletable | QScriptValue::ReadOnly);

    return ctor;
}

