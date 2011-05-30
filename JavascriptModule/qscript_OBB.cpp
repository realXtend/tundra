#include "QtScriptBindingsHelpers.h"

static QScriptValue OBB_OBB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function OBB_OBB in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB ret;
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue OBB_OBB_AABB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_OBB_AABB in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB aabb = TypeFromQScriptValue<AABB>(context->argument(0));
    OBB ret(aabb);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue OBB_SetNegativeInfinity(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function OBB_SetNegativeInfinity in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_SetNegativeInfinity in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    This->SetNegativeInfinity();
    return QScriptValue();
}

static QScriptValue OBB_SetFrom_AABB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_SetFrom_AABB in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_SetFrom_AABB in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    AABB aabb = TypeFromQScriptValue<AABB>(context->argument(0));
    This->SetFrom(aabb);
    return QScriptValue();
}

static QScriptValue OBB_SetFrom_AABB_float3x3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function OBB_SetFrom_AABB_float3x3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_SetFrom_AABB_float3x3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    AABB aabb = TypeFromQScriptValue<AABB>(context->argument(0));
    float3x3 transform = TypeFromQScriptValue<float3x3>(context->argument(1));
    This->SetFrom(aabb, transform);
    return QScriptValue();
}

static QScriptValue OBB_SetFrom_AABB_float3x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function OBB_SetFrom_AABB_float3x4 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_SetFrom_AABB_float3x4 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    AABB aabb = TypeFromQScriptValue<AABB>(context->argument(0));
    float3x4 transform = TypeFromQScriptValue<float3x4>(context->argument(1));
    This->SetFrom(aabb, transform);
    return QScriptValue();
}

static QScriptValue OBB_SetFrom_AABB_float4x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function OBB_SetFrom_AABB_float4x4 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_SetFrom_AABB_float4x4 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    AABB aabb = TypeFromQScriptValue<AABB>(context->argument(0));
    float4x4 transform = TypeFromQScriptValue<float4x4>(context->argument(1));
    This->SetFrom(aabb, transform);
    return QScriptValue();
}

static QScriptValue OBB_SetFrom_AABB_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function OBB_SetFrom_AABB_Quat in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_SetFrom_AABB_Quat in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    AABB aabb = TypeFromQScriptValue<AABB>(context->argument(0));
    Quat transform = TypeFromQScriptValue<Quat>(context->argument(1));
    This->SetFrom(aabb, transform);
    return QScriptValue();
}

static QScriptValue OBB_SetFrom_Sphere(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_SetFrom_Sphere in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_SetFrom_Sphere in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    Sphere sphere = TypeFromQScriptValue<Sphere>(context->argument(0));
    This->SetFrom(sphere);
    return QScriptValue();
}

static QScriptValue OBB_MinimalEnclosingAABB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function OBB_MinimalEnclosingAABB in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_MinimalEnclosingAABB in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    AABB ret = This->MinimalEnclosingAABB();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue OBB_MaximalContainedAABB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function OBB_MaximalContainedAABB in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_MaximalContainedAABB in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    AABB ret = This->MaximalContainedAABB();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue OBB_Size(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function OBB_Size in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_Size in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->Size();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue OBB_HalfSize(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function OBB_HalfSize in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_HalfSize in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->HalfSize();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue OBB_Diagonal(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function OBB_Diagonal in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_Diagonal in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->Diagonal();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue OBB_HalfDiagonal(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function OBB_HalfDiagonal in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_HalfDiagonal in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->HalfDiagonal();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue OBB_MinimalEnclosingSphere(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function OBB_MinimalEnclosingSphere in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_MinimalEnclosingSphere in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    Sphere ret = This->MinimalEnclosingSphere();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue OBB_MaximalContainedSphere(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function OBB_MaximalContainedSphere in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_MaximalContainedSphere in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    Sphere ret = This->MaximalContainedSphere();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue OBB_IsFinite(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function OBB_IsFinite in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_IsFinite in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    bool ret = This->IsFinite();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue OBB_IsDegenerate(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function OBB_IsDegenerate in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_IsDegenerate in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    bool ret = This->IsDegenerate();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue OBB_CenterPoint(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function OBB_CenterPoint in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_CenterPoint in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->CenterPoint();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue OBB_GetPointInside_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function OBB_GetPointInside_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_GetPointInside_float_float_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float x = TypeFromQScriptValue<float>(context->argument(0));
    float y = TypeFromQScriptValue<float>(context->argument(1));
    float z = TypeFromQScriptValue<float>(context->argument(2));
    float3 ret = This->GetPointInside(x, y, z);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue OBB_CornerPoint_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_CornerPoint_int in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_CornerPoint_int in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int cornerIndex = TypeFromQScriptValue<int>(context->argument(0));
    float3 ret = This->CornerPoint(cornerIndex);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue OBB_PointOnEdge_int_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function OBB_PointOnEdge_int_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_PointOnEdge_int_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int edgeIndex = TypeFromQScriptValue<int>(context->argument(0));
    float u = TypeFromQScriptValue<float>(context->argument(1));
    float3 ret = This->PointOnEdge(edgeIndex, u);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue OBB_FaceCenterPoint_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_FaceCenterPoint_int in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_FaceCenterPoint_int in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int faceIndex = TypeFromQScriptValue<int>(context->argument(0));
    float3 ret = This->FaceCenterPoint(faceIndex);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue OBB_FacePoint_int_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function OBB_FacePoint_int_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_FacePoint_int_float_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int faceIndex = TypeFromQScriptValue<int>(context->argument(0));
    float u = TypeFromQScriptValue<float>(context->argument(1));
    float v = TypeFromQScriptValue<float>(context->argument(2));
    float3 ret = This->FacePoint(faceIndex, u, v);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue OBB_FacePlane_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_FacePlane_int in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_FacePlane_int in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int faceIndex = TypeFromQScriptValue<int>(context->argument(0));
    Plane ret = This->FacePlane(faceIndex);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue OBB_GetFacePlanes_Plane_ptr(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_GetFacePlanes_Plane_ptr in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_GetFacePlanes_Plane_ptr in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    Plane * outPlaneArray = TypeFromQScriptValue<Plane *>(context->argument(0));
    This->GetFacePlanes(outPlaneArray);
    return QScriptValue();
}

static QScriptValue OBB_Volume(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function OBB_Volume in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_Volume in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->Volume();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue OBB_SurfaceArea(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function OBB_SurfaceArea in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_SurfaceArea in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->SurfaceArea();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue OBB_RandomPointInside_LCG(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_RandomPointInside_LCG in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_RandomPointInside_LCG in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    LCG rng = TypeFromQScriptValue<LCG>(context->argument(0));
    float3 ret = This->RandomPointInside(rng);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue OBB_RandomPointOnSurface_LCG(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_RandomPointOnSurface_LCG in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_RandomPointOnSurface_LCG in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    LCG rng = TypeFromQScriptValue<LCG>(context->argument(0));
    float3 ret = This->RandomPointOnSurface(rng);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue OBB_RandomPointOnEdge_LCG(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_RandomPointOnEdge_LCG in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_RandomPointOnEdge_LCG in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    LCG rng = TypeFromQScriptValue<LCG>(context->argument(0));
    float3 ret = This->RandomPointOnEdge(rng);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue OBB_RandomCornerPoint_LCG(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_RandomCornerPoint_LCG in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_RandomCornerPoint_LCG in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    LCG rng = TypeFromQScriptValue<LCG>(context->argument(0));
    float3 ret = This->RandomCornerPoint(rng);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue OBB_Translate_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_Translate_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_Translate_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 offset = TypeFromQScriptValue<float3>(context->argument(0));
    This->Translate(offset);
    return QScriptValue();
}

static QScriptValue OBB_Scale_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function OBB_Scale_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_Scale_float3_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 centerPoint = TypeFromQScriptValue<float3>(context->argument(0));
    float scaleFactor = TypeFromQScriptValue<float>(context->argument(1));
    This->Scale(centerPoint, scaleFactor);
    return QScriptValue();
}

static QScriptValue OBB_Scale_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function OBB_Scale_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_Scale_float3_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 centerPoint = TypeFromQScriptValue<float3>(context->argument(0));
    float3 scaleFactor = TypeFromQScriptValue<float3>(context->argument(1));
    This->Scale(centerPoint, scaleFactor);
    return QScriptValue();
}

static QScriptValue OBB_Transform_float3x3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_Transform_float3x3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_Transform_float3x3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3x3 transform = TypeFromQScriptValue<float3x3>(context->argument(0));
    This->Transform(transform);
    return QScriptValue();
}

static QScriptValue OBB_Transform_float3x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_Transform_float3x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_Transform_float3x4 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3x4 transform = TypeFromQScriptValue<float3x4>(context->argument(0));
    This->Transform(transform);
    return QScriptValue();
}

static QScriptValue OBB_Transform_float4x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_Transform_float4x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_Transform_float4x4 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4x4 transform = TypeFromQScriptValue<float4x4>(context->argument(0));
    This->Transform(transform);
    return QScriptValue();
}

static QScriptValue OBB_Transform_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_Transform_Quat in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_Transform_Quat in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    Quat transform = TypeFromQScriptValue<Quat>(context->argument(0));
    This->Transform(transform);
    return QScriptValue();
}

static QScriptValue OBB_Contains_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_Contains_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_Contains_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 point = TypeFromQScriptValue<float3>(context->argument(0));
    bool ret = This->Contains(point);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue OBB_Contains_LineSegment(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_Contains_LineSegment in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_Contains_LineSegment in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    LineSegment lineSegment = TypeFromQScriptValue<LineSegment>(context->argument(0));
    bool ret = This->Contains(lineSegment);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue OBB_Contains_AABB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_Contains_AABB in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_Contains_AABB in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    AABB aabb = TypeFromQScriptValue<AABB>(context->argument(0));
    bool ret = This->Contains(aabb);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue OBB_Contains_OBB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_Contains_OBB in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_Contains_OBB in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    OBB obb = TypeFromQScriptValue<OBB>(context->argument(0));
    bool ret = This->Contains(obb);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue OBB_Intersects_AABB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_Intersects_AABB in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_Intersects_AABB in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    AABB aabb = TypeFromQScriptValue<AABB>(context->argument(0));
    bool ret = This->Intersects(aabb);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue OBB_Intersects_OBB_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function OBB_Intersects_OBB_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function OBB_Intersects_OBB_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    OBB b = TypeFromQScriptValue<OBB>(context->argument(0));
    float epsilon = TypeFromQScriptValue<float>(context->argument(1));
    bool ret = This->Intersects(b, epsilon);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue OBB_pos_get(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function OBB_pos_get in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    return qScriptValueFromValue(context->engine(), This->pos);
}

static QScriptValue OBB_pos_set(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_pos_set in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 pos = qscriptvalue_cast<float3>(context->argument(0));
    This->pos = pos;
    return QScriptValue();
}

static QScriptValue OBB_r_get(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function OBB_r_get in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    return qScriptValueFromValue(context->engine(), This->r);
}

static QScriptValue OBB_r_set(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function OBB_r_set in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB *This = TypeFromQScriptValue<OBB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 r = qscriptvalue_cast<float3>(context->argument(0));
    This->r = r;
    return QScriptValue();
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
    printf("OBB_Intersects_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

class OBB_scriptclass : public QScriptClass
{
public:
    QScriptValue objectPrototype;
    OBB_scriptclass(QScriptEngine *engine):QScriptClass(engine){}
    QScriptValue property(const QScriptValue &object, const QScriptString &name, uint id)
    {
        OBB *This = TypeFromQScriptValue<OBB*>(object);
        if (!This) { printf("Error! Cannot convert QScriptValue to type OBB in file %s, line %d!\nTry using OBB.get%s() and OBB.set%s() to query the member variable '%s'!\n", __FILE__, __LINE__, Capitalize((QString)name).c_str(), Capitalize((QString)name).c_str(), ((QString)name).toStdString().c_str()); return QScriptValue(); }
        QString name_ = (QString)name;
        if (name_ == "pos_") return TypeToQScriptValue(engine(), This->pos);
        if (name_ == "pos_ptr") return TypeToQScriptValue(engine(), &This->pos);
        if (name_ == "r_") return TypeToQScriptValue(engine(), This->r);
        if (name_ == "r_ptr") return TypeToQScriptValue(engine(), &This->r);
        return QScriptValue();
    }
    void setProperty(QScriptValue &object, const QScriptString &name, uint id, const QScriptValue &value)
    {
        OBB *This = TypeFromQScriptValue<OBB*>(object);
        if (!This) { printf("Error! Cannot convert QScriptValue to type OBB in file %s, line %d!\nTry using OBB.get%s() and OBB.set%s() to query the member variable '%s'!\n", __FILE__, __LINE__, Capitalize((QString)name).c_str(), Capitalize((QString)name).c_str(), ((QString)name).toStdString().c_str()); return; }
        QString name_ = (QString)name;
        if (name_ == "pos_") This->pos = TypeFromQScriptValue<float3>(value);
        if (name_ == "pos_ptr") This->pos = *TypeFromQScriptValue<float3*>(value);
        if (name_ == "r_") This->r = TypeFromQScriptValue<float3>(value);
        if (name_ == "r_ptr") This->r = *TypeFromQScriptValue<float3*>(value);
    }
    QueryFlags queryProperty(const QScriptValue &object, const QScriptString &name, QueryFlags flags, uint *id)
    {
        QString name_ = (QString)name;
        if (name_ == "pos_" || name_ == "pos_ptr") return flags;
        if (name_ == "r_" || name_ == "r_ptr") return flags;
        return 0;
    }
    QScriptValue prototype() const { return objectPrototype; }
};
QScriptValue register_OBB_prototype(QScriptEngine *engine)
{
    engine->setDefaultPrototype(qMetaTypeId<OBB*>(), QScriptValue());
    QScriptValue proto = engine->newVariant(qVariantFromValue((OBB*)0));
    proto.setProperty("SetNegativeInfinity", engine->newFunction(OBB_SetNegativeInfinity, 0));
    proto.setProperty("SetFrom", engine->newFunction(OBB_SetFrom_selector, 1));
    proto.setProperty("SetFrom", engine->newFunction(OBB_SetFrom_selector, 2));
    proto.setProperty("MinimalEnclosingAABB", engine->newFunction(OBB_MinimalEnclosingAABB, 0));
    proto.setProperty("MaximalContainedAABB", engine->newFunction(OBB_MaximalContainedAABB, 0));
    proto.setProperty("Size", engine->newFunction(OBB_Size, 0));
    proto.setProperty("HalfSize", engine->newFunction(OBB_HalfSize, 0));
    proto.setProperty("Diagonal", engine->newFunction(OBB_Diagonal, 0));
    proto.setProperty("HalfDiagonal", engine->newFunction(OBB_HalfDiagonal, 0));
    proto.setProperty("MinimalEnclosingSphere", engine->newFunction(OBB_MinimalEnclosingSphere, 0));
    proto.setProperty("MaximalContainedSphere", engine->newFunction(OBB_MaximalContainedSphere, 0));
    proto.setProperty("IsFinite", engine->newFunction(OBB_IsFinite, 0));
    proto.setProperty("IsDegenerate", engine->newFunction(OBB_IsDegenerate, 0));
    proto.setProperty("CenterPoint", engine->newFunction(OBB_CenterPoint, 0));
    proto.setProperty("GetPointInside", engine->newFunction(OBB_GetPointInside_float_float_float, 3));
    proto.setProperty("CornerPoint", engine->newFunction(OBB_CornerPoint_int, 1));
    proto.setProperty("PointOnEdge", engine->newFunction(OBB_PointOnEdge_int_float, 2));
    proto.setProperty("FaceCenterPoint", engine->newFunction(OBB_FaceCenterPoint_int, 1));
    proto.setProperty("FacePoint", engine->newFunction(OBB_FacePoint_int_float_float, 3));
    proto.setProperty("FacePlane", engine->newFunction(OBB_FacePlane_int, 1));
    proto.setProperty("GetFacePlanes", engine->newFunction(OBB_GetFacePlanes_Plane_ptr, 1));
    proto.setProperty("Volume", engine->newFunction(OBB_Volume, 0));
    proto.setProperty("SurfaceArea", engine->newFunction(OBB_SurfaceArea, 0));
    proto.setProperty("RandomPointInside", engine->newFunction(OBB_RandomPointInside_LCG, 1));
    proto.setProperty("RandomPointOnSurface", engine->newFunction(OBB_RandomPointOnSurface_LCG, 1));
    proto.setProperty("RandomPointOnEdge", engine->newFunction(OBB_RandomPointOnEdge_LCG, 1));
    proto.setProperty("RandomCornerPoint", engine->newFunction(OBB_RandomCornerPoint_LCG, 1));
    proto.setProperty("Translate", engine->newFunction(OBB_Translate_float3, 1));
    proto.setProperty("Scale", engine->newFunction(OBB_Scale_selector, 2));
    proto.setProperty("Transform", engine->newFunction(OBB_Transform_selector, 1));
    proto.setProperty("Contains", engine->newFunction(OBB_Contains_selector, 1));
    proto.setProperty("Intersects", engine->newFunction(OBB_Intersects_selector, 1));
    proto.setProperty("Intersects", engine->newFunction(OBB_Intersects_selector, 2));
    proto.setProperty("pos", engine->newFunction(OBB_pos_get, 1));
    proto.setProperty("setPos", engine->newFunction(OBB_pos_set, 1));
    proto.setProperty("r", engine->newFunction(OBB_r_get, 1));
    proto.setProperty("setR", engine->newFunction(OBB_r_set, 1));
    OBB_scriptclass *sc = new OBB_scriptclass(engine);
    engine->setProperty("OBB_scriptclass", QVariant::fromValue<QScriptClass*>(sc));
    proto.setScriptClass(sc);
    sc->objectPrototype = proto;
    engine->setDefaultPrototype(qMetaTypeId<OBB>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<OBB*>(), proto);
    QScriptValue ctor = engine->newFunction(OBB_ctor, proto, 1);
    engine->globalObject().setProperty("OBB", ctor);
    return ctor;
}

