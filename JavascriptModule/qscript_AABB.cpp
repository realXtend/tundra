#include "QtScriptBindingsHelpers.h"

static QScriptValue AABB_AABB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_AABB in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB ret;
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue AABB_AABB_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function AABB_AABB_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 minPoint = TypeFromQScriptValue<float3>(context->argument(0));
    float3 maxPoint = TypeFromQScriptValue<float3>(context->argument(1));
    AABB ret(minPoint, maxPoint);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue AABB_AABB_OBB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_AABB_OBB in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    OBB obb = TypeFromQScriptValue<OBB>(context->argument(0));
    AABB ret(obb);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue AABB_AABB_Sphere(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_AABB_Sphere in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Sphere s = TypeFromQScriptValue<Sphere>(context->argument(0));
    AABB ret(s);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue AABB_SetNegativeInfinity(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_SetNegativeInfinity in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_SetNegativeInfinity in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    This->SetNegativeInfinity();
    return QScriptValue();
}

static QScriptValue AABB_SetCenter_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function AABB_SetCenter_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_SetCenter_float3_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 center = TypeFromQScriptValue<float3>(context->argument(0));
    float3 halfSize = TypeFromQScriptValue<float3>(context->argument(1));
    This->SetCenter(center, halfSize);
    return QScriptValue();
}

static QScriptValue AABB_SetFrom_OBB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_SetFrom_OBB in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_SetFrom_OBB in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    OBB obb = TypeFromQScriptValue<OBB>(context->argument(0));
    This->SetFrom(obb);
    return QScriptValue();
}

static QScriptValue AABB_SetFrom_Sphere(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_SetFrom_Sphere in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_SetFrom_Sphere in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    Sphere s = TypeFromQScriptValue<Sphere>(context->argument(0));
    This->SetFrom(s);
    return QScriptValue();
}

static QScriptValue AABB_ToOBB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_ToOBB in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_ToOBB in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    OBB ret = This->ToOBB();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue AABB_MinimalEnclosingSphere(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_MinimalEnclosingSphere in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_MinimalEnclosingSphere in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    Sphere ret = This->MinimalEnclosingSphere();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue AABB_MaximalContainedSphere(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_MaximalContainedSphere in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_MaximalContainedSphere in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    Sphere ret = This->MaximalContainedSphere();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue AABB_IsFinite(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_IsFinite in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_IsFinite in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    bool ret = This->IsFinite();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue AABB_IsDegenerate(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_IsDegenerate in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_IsDegenerate in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    bool ret = This->IsDegenerate();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue AABB_CenterPoint(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_CenterPoint in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_CenterPoint in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->CenterPoint();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue AABB_GetPointInside_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function AABB_GetPointInside_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_GetPointInside_float_float_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float x = TypeFromQScriptValue<float>(context->argument(0));
    float y = TypeFromQScriptValue<float>(context->argument(1));
    float z = TypeFromQScriptValue<float>(context->argument(2));
    float3 ret = This->GetPointInside(x, y, z);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue AABB_CornerPoint_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_CornerPoint_int in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_CornerPoint_int in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int cornerIndex = TypeFromQScriptValue<int>(context->argument(0));
    float3 ret = This->CornerPoint(cornerIndex);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue AABB_PointOnEdge_int_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function AABB_PointOnEdge_int_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_PointOnEdge_int_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int edgeIndex = TypeFromQScriptValue<int>(context->argument(0));
    float u = TypeFromQScriptValue<float>(context->argument(1));
    float3 ret = This->PointOnEdge(edgeIndex, u);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue AABB_FaceCenterPoint_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_FaceCenterPoint_int in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_FaceCenterPoint_int in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int faceIndex = TypeFromQScriptValue<int>(context->argument(0));
    float3 ret = This->FaceCenterPoint(faceIndex);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue AABB_FacePoint_int_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function AABB_FacePoint_int_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_FacePoint_int_float_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int faceIndex = TypeFromQScriptValue<int>(context->argument(0));
    float u = TypeFromQScriptValue<float>(context->argument(1));
    float v = TypeFromQScriptValue<float>(context->argument(2));
    float3 ret = This->FacePoint(faceIndex, u, v);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue AABB_FacePlane_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_FacePlane_int in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_FacePlane_int in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int faceIndex = TypeFromQScriptValue<int>(context->argument(0));
    Plane ret = This->FacePlane(faceIndex);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue AABB_GetFacePlanes_Plane_ptr(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_GetFacePlanes_Plane_ptr in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_GetFacePlanes_Plane_ptr in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    Plane * outPlaneArray = TypeFromQScriptValue<Plane *>(context->argument(0));
    This->GetFacePlanes(outPlaneArray);
    return QScriptValue();
}

static QScriptValue AABB_Size(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_Size in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_Size in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->Size();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue AABB_HalfSize(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_HalfSize in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_HalfSize in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->HalfSize();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue AABB_Diagonal(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_Diagonal in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_Diagonal in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->Diagonal();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue AABB_HalfDiagonal(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_HalfDiagonal in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_HalfDiagonal in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->HalfDiagonal();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue AABB_Volume(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_Volume in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_Volume in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->Volume();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue AABB_SurfaceArea(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_SurfaceArea in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_SurfaceArea in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->SurfaceArea();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue AABB_RandomPointInside_LCG(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_RandomPointInside_LCG in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_RandomPointInside_LCG in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    LCG rng = TypeFromQScriptValue<LCG>(context->argument(0));
    float3 ret = This->RandomPointInside(rng);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue AABB_RandomPointOnSurface_LCG(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_RandomPointOnSurface_LCG in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_RandomPointOnSurface_LCG in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    LCG rng = TypeFromQScriptValue<LCG>(context->argument(0));
    float3 ret = This->RandomPointOnSurface(rng);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue AABB_RandomPointOnEdge_LCG(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_RandomPointOnEdge_LCG in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_RandomPointOnEdge_LCG in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    LCG rng = TypeFromQScriptValue<LCG>(context->argument(0));
    float3 ret = This->RandomPointOnEdge(rng);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue AABB_RandomCornerPoint_LCG(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_RandomCornerPoint_LCG in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_RandomCornerPoint_LCG in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    LCG rng = TypeFromQScriptValue<LCG>(context->argument(0));
    float3 ret = This->RandomCornerPoint(rng);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue AABB_Translate_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Translate_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_Translate_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 offset = TypeFromQScriptValue<float3>(context->argument(0));
    This->Translate(offset);
    return QScriptValue();
}

static QScriptValue AABB_Scale_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function AABB_Scale_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_Scale_float3_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 centerPoint = TypeFromQScriptValue<float3>(context->argument(0));
    float scaleFactor = TypeFromQScriptValue<float>(context->argument(1));
    This->Scale(centerPoint, scaleFactor);
    return QScriptValue();
}

static QScriptValue AABB_Scale_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function AABB_Scale_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_Scale_float3_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 centerPoint = TypeFromQScriptValue<float3>(context->argument(0));
    float3 scaleFactor = TypeFromQScriptValue<float3>(context->argument(1));
    This->Scale(centerPoint, scaleFactor);
    return QScriptValue();
}

static QScriptValue AABB_TransformAsAABB_float3x3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_TransformAsAABB_float3x3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_TransformAsAABB_float3x3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3x3 transform = TypeFromQScriptValue<float3x3>(context->argument(0));
    This->TransformAsAABB(transform);
    return QScriptValue();
}

static QScriptValue AABB_TransformAsAABB_float3x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_TransformAsAABB_float3x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_TransformAsAABB_float3x4 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3x4 transform = TypeFromQScriptValue<float3x4>(context->argument(0));
    This->TransformAsAABB(transform);
    return QScriptValue();
}

static QScriptValue AABB_TransformAsAABB_float4x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_TransformAsAABB_float4x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_TransformAsAABB_float4x4 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4x4 transform = TypeFromQScriptValue<float4x4>(context->argument(0));
    This->TransformAsAABB(transform);
    return QScriptValue();
}

static QScriptValue AABB_TransformAsAABB_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_TransformAsAABB_Quat in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_TransformAsAABB_Quat in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    Quat transform = TypeFromQScriptValue<Quat>(context->argument(0));
    This->TransformAsAABB(transform);
    return QScriptValue();
}

static QScriptValue AABB_Transform_float3x3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Transform_float3x3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_Transform_float3x3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3x3 transform = TypeFromQScriptValue<float3x3>(context->argument(0));
    OBB ret = This->Transform(transform);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue AABB_Transform_float3x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Transform_float3x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_Transform_float3x4 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3x4 transform = TypeFromQScriptValue<float3x4>(context->argument(0));
    OBB ret = This->Transform(transform);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue AABB_Transform_float4x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Transform_float4x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_Transform_float4x4 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4x4 transform = TypeFromQScriptValue<float4x4>(context->argument(0));
    OBB ret = This->Transform(transform);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue AABB_Transform_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Transform_Quat in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_Transform_Quat in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    Quat transform = TypeFromQScriptValue<Quat>(context->argument(0));
    OBB ret = This->Transform(transform);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue AABB_Contains_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Contains_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_Contains_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 point = TypeFromQScriptValue<float3>(context->argument(0));
    bool ret = This->Contains(point);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue AABB_Contains_LineSegment(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Contains_LineSegment in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_Contains_LineSegment in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    LineSegment lineSegment = TypeFromQScriptValue<LineSegment>(context->argument(0));
    bool ret = This->Contains(lineSegment);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue AABB_Contains_AABB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Contains_AABB in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_Contains_AABB in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    AABB aabb = TypeFromQScriptValue<AABB>(context->argument(0));
    bool ret = This->Contains(aabb);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue AABB_Enclose_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Enclose_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_Enclose_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 point = TypeFromQScriptValue<float3>(context->argument(0));
    This->Enclose(point);
    return QScriptValue();
}

static QScriptValue AABB_Enclose_LineSegment(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Enclose_LineSegment in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_Enclose_LineSegment in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    LineSegment lineSegment = TypeFromQScriptValue<LineSegment>(context->argument(0));
    This->Enclose(lineSegment);
    return QScriptValue();
}

static QScriptValue AABB_Enclose_AABB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Enclose_AABB in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_Enclose_AABB in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    AABB aabb = TypeFromQScriptValue<AABB>(context->argument(0));
    This->Enclose(aabb);
    return QScriptValue();
}

static QScriptValue AABB_Enclose_OBB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Enclose_OBB in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_Enclose_OBB in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    OBB obb = TypeFromQScriptValue<OBB>(context->argument(0));
    This->Enclose(obb);
    return QScriptValue();
}

static QScriptValue AABB_Enclose_Sphere(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Enclose_Sphere in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_Enclose_Sphere in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    Sphere sphere = TypeFromQScriptValue<Sphere>(context->argument(0));
    This->Enclose(sphere);
    return QScriptValue();
}

static QScriptValue AABB_Intersection_AABB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_Intersection_AABB in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function AABB_Intersection_AABB in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    AABB aabb = TypeFromQScriptValue<AABB>(context->argument(0));
    AABB ret = This->Intersection(aabb);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue AABB_minPoint_get(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_minPoint_get in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    return qScriptValueFromValue(context->engine(), This->minPoint);
}

static QScriptValue AABB_minPoint_set(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_minPoint_set in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 minPoint = qscriptvalue_cast<float3>(context->argument(0));
    This->minPoint = minPoint;
    return QScriptValue();
}

static QScriptValue AABB_maxPoint_get(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function AABB_maxPoint_get in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    return qScriptValueFromValue(context->engine(), This->maxPoint);
}

static QScriptValue AABB_maxPoint_set(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function AABB_maxPoint_set in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    AABB *This = TypeFromQScriptValue<AABB*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 maxPoint = qscriptvalue_cast<float3>(context->argument(0));
    This->maxPoint = maxPoint;
    return QScriptValue();
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
    printf("AABB_ctor failed to choose the right function to call! Did you use 'var x = AABB();' instead of 'var x = new AABB();'?\n"); return QScriptValue();
}

static QScriptValue AABB_SetFrom_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<OBB>(context->argument(0)))
        return AABB_SetFrom_OBB(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Sphere>(context->argument(0)))
        return AABB_SetFrom_Sphere(context, engine);
    printf("AABB_SetFrom_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue AABB_Scale_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return AABB_Scale_float3_float(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return AABB_Scale_float3_float3(context, engine);
    printf("AABB_Scale_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
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
    printf("AABB_TransformAsAABB_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
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
    printf("AABB_Transform_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue AABB_Contains_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return AABB_Contains_float3(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<LineSegment>(context->argument(0)))
        return AABB_Contains_LineSegment(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<AABB>(context->argument(0)))
        return AABB_Contains_AABB(context, engine);
    printf("AABB_Contains_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
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
    printf("AABB_Enclose_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

class AABB_scriptclass : public QScriptClass
{
public:
    QScriptValue objectPrototype;
    AABB_scriptclass(QScriptEngine *engine):QScriptClass(engine){}
    QScriptValue property(const QScriptValue &object, const QScriptString &name, uint id)
    {
        AABB *This = TypeFromQScriptValue<AABB*>(object);
        if (!This) { printf("Error! Cannot convert QScriptValue to type AABB in file %s, line %d!\nTry using AABB.get%s() and AABB.set%s() to query the member variable '%s'!\n", __FILE__, __LINE__, Capitalize((QString)name).c_str(), Capitalize((QString)name).c_str(), ((QString)name).toStdString().c_str()); return QScriptValue(); }
        if ((QString)name == (QString)"minPoint") return TypeToQScriptValue(engine(), This->minPoint);
        if ((QString)name == (QString)"maxPoint") return TypeToQScriptValue(engine(), This->maxPoint);
        return QScriptValue();
    }
    void setProperty(QScriptValue &object, const QScriptString &name, uint id, const QScriptValue &value)
    {
        AABB *This = TypeFromQScriptValue<AABB*>(object);
        if (!This) { printf("Error! Cannot convert QScriptValue to type AABB in file %s, line %d!\nTry using AABB.get%s() and AABB.set%s() to query the member variable '%s'!\n", __FILE__, __LINE__, Capitalize((QString)name).c_str(), Capitalize((QString)name).c_str(), ((QString)name).toStdString().c_str()); return; }
        if ((QString)name == (QString)"minPoint") This->minPoint = TypeFromQScriptValue<float3>(value);
        if ((QString)name == (QString)"maxPoint") This->maxPoint = TypeFromQScriptValue<float3>(value);
    }
    QueryFlags queryProperty(const QScriptValue &object, const QScriptString &name, QueryFlags flags, uint *id)
    {
        if ((QString)name == (QString)"minPoint") return flags;
        if ((QString)name == (QString)"maxPoint") return flags;
        return 0;
    }
    QScriptValue prototype() const { return objectPrototype; }
};
QScriptValue register_AABB_prototype(QScriptEngine *engine)
{
    engine->setDefaultPrototype(qMetaTypeId<AABB*>(), QScriptValue());
    QScriptValue proto = engine->newVariant(qVariantFromValue((AABB*)0));
    proto.setProperty("SetNegativeInfinity", engine->newFunction(AABB_SetNegativeInfinity, 0));
    proto.setProperty("SetCenter", engine->newFunction(AABB_SetCenter_float3_float3, 2));
    proto.setProperty("SetFrom", engine->newFunction(AABB_SetFrom_selector, 1));
    proto.setProperty("ToOBB", engine->newFunction(AABB_ToOBB, 0));
    proto.setProperty("MinimalEnclosingSphere", engine->newFunction(AABB_MinimalEnclosingSphere, 0));
    proto.setProperty("MaximalContainedSphere", engine->newFunction(AABB_MaximalContainedSphere, 0));
    proto.setProperty("IsFinite", engine->newFunction(AABB_IsFinite, 0));
    proto.setProperty("IsDegenerate", engine->newFunction(AABB_IsDegenerate, 0));
    proto.setProperty("CenterPoint", engine->newFunction(AABB_CenterPoint, 0));
    proto.setProperty("GetPointInside", engine->newFunction(AABB_GetPointInside_float_float_float, 3));
    proto.setProperty("CornerPoint", engine->newFunction(AABB_CornerPoint_int, 1));
    proto.setProperty("PointOnEdge", engine->newFunction(AABB_PointOnEdge_int_float, 2));
    proto.setProperty("FaceCenterPoint", engine->newFunction(AABB_FaceCenterPoint_int, 1));
    proto.setProperty("FacePoint", engine->newFunction(AABB_FacePoint_int_float_float, 3));
    proto.setProperty("FacePlane", engine->newFunction(AABB_FacePlane_int, 1));
    proto.setProperty("GetFacePlanes", engine->newFunction(AABB_GetFacePlanes_Plane_ptr, 1));
    proto.setProperty("Size", engine->newFunction(AABB_Size, 0));
    proto.setProperty("HalfSize", engine->newFunction(AABB_HalfSize, 0));
    proto.setProperty("Diagonal", engine->newFunction(AABB_Diagonal, 0));
    proto.setProperty("HalfDiagonal", engine->newFunction(AABB_HalfDiagonal, 0));
    proto.setProperty("Volume", engine->newFunction(AABB_Volume, 0));
    proto.setProperty("SurfaceArea", engine->newFunction(AABB_SurfaceArea, 0));
    proto.setProperty("RandomPointInside", engine->newFunction(AABB_RandomPointInside_LCG, 1));
    proto.setProperty("RandomPointOnSurface", engine->newFunction(AABB_RandomPointOnSurface_LCG, 1));
    proto.setProperty("RandomPointOnEdge", engine->newFunction(AABB_RandomPointOnEdge_LCG, 1));
    proto.setProperty("RandomCornerPoint", engine->newFunction(AABB_RandomCornerPoint_LCG, 1));
    proto.setProperty("Translate", engine->newFunction(AABB_Translate_float3, 1));
    proto.setProperty("Scale", engine->newFunction(AABB_Scale_selector, 2));
    proto.setProperty("TransformAsAABB", engine->newFunction(AABB_TransformAsAABB_selector, 1));
    proto.setProperty("Transform", engine->newFunction(AABB_Transform_selector, 1));
    proto.setProperty("Contains", engine->newFunction(AABB_Contains_selector, 1));
    proto.setProperty("Enclose", engine->newFunction(AABB_Enclose_selector, 1));
    proto.setProperty("Intersection", engine->newFunction(AABB_Intersection_AABB, 1));
    proto.setProperty("getMinPoint", engine->newFunction(AABB_minPoint_get, 1));
    proto.setProperty("setMinPoint", engine->newFunction(AABB_minPoint_set, 1));
    proto.setProperty("getMaxPoint", engine->newFunction(AABB_maxPoint_get, 1));
    proto.setProperty("setMaxPoint", engine->newFunction(AABB_maxPoint_set, 1));
    AABB_scriptclass *sc = new AABB_scriptclass(engine);
    engine->setProperty("AABB_scriptclass", QVariant::fromValue<QScriptClass*>(sc));
    proto.setScriptClass(sc);
    sc->objectPrototype = proto;
    engine->setDefaultPrototype(qMetaTypeId<AABB>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<AABB*>(), proto);
    QScriptValue ctor = engine->newFunction(AABB_ctor, proto, 2);
    engine->globalObject().setProperty("AABB", ctor);
    return ctor;
}

