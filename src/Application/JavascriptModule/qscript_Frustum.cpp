#include "QtScriptBindingsHelpers.h"

void ToExistingScriptValue_Frustum(QScriptEngine *engine, const Frustum &value, QScriptValue obj)
{
    obj.setData(engine->newVariant(QVariant::fromValue(value)));
    obj.setProperty("pos", qScriptValueFromValue(engine, value.pos), QScriptValue::Undeletable);
    obj.setProperty("front", qScriptValueFromValue(engine, value.front), QScriptValue::Undeletable);
    obj.setProperty("up", qScriptValueFromValue(engine, value.up), QScriptValue::Undeletable);
    obj.setProperty("nearPlaneDistance", qScriptValueFromValue(engine, value.nearPlaneDistance), QScriptValue::Undeletable);
    obj.setProperty("farPlaneDistance", qScriptValueFromValue(engine, value.farPlaneDistance), QScriptValue::Undeletable);
    obj.setProperty("horizontalFov", qScriptValueFromValue(engine, value.horizontalFov), QScriptValue::Undeletable);
    obj.setProperty("orthographicWidth", qScriptValueFromValue(engine, value.orthographicWidth), QScriptValue::Undeletable);
    obj.setProperty("verticalFov", qScriptValueFromValue(engine, value.verticalFov), QScriptValue::Undeletable);
    obj.setProperty("orthographicHeight", qScriptValueFromValue(engine, value.orthographicHeight), QScriptValue::Undeletable);
}

static QScriptValue Frustum_ctor(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Frustum_ctor in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum ret;
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_AspectRatio_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Frustum_AspectRatio_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    float ret = This.AspectRatio();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_NearPlane_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Frustum_NearPlane_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    Plane ret = This.NearPlane();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_FarPlane_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Frustum_FarPlane_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    Plane ret = This.FarPlane();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_LeftPlane_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Frustum_LeftPlane_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    Plane ret = This.LeftPlane();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_RightPlane_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Frustum_RightPlane_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    Plane ret = This.RightPlane();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_TopPlane_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Frustum_TopPlane_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    Plane ret = This.TopPlane();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_BottomPlane_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Frustum_BottomPlane_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    Plane ret = This.BottomPlane();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_GetPlane_int_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Frustum_GetPlane_int_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    int faceIndex = qscriptvalue_cast<int>(context->argument(0));
    Plane ret = This.GetPlane(faceIndex);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_GetPlanes_Plane_ptr_const(QScriptContext *context, QScriptEngine * /*engine*/)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Frustum_GetPlanes_Plane_ptr_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    Plane * outArray = qscriptvalue_cast<Plane *>(context->argument(0));
    This.GetPlanes(outArray);
    return QScriptValue();
}

static QScriptValue Frustum_CornerPoint_int_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Frustum_CornerPoint_int_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    int cornerIndex = qscriptvalue_cast<int>(context->argument(0));
    float3 ret = This.CornerPoint(cornerIndex);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_ExtremePoint_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Frustum_ExtremePoint_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    float3 direction = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.ExtremePoint(direction);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_WorldMatrix_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Frustum_WorldMatrix_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    float3x4 ret = This.WorldMatrix();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_ViewMatrix_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Frustum_ViewMatrix_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    float3x4 ret = This.ViewMatrix();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_ProjectionMatrix_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Frustum_ProjectionMatrix_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    float4x4 ret = This.ProjectionMatrix();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_ViewProjMatrix_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Frustum_ViewProjMatrix_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    float4x4 ret = This.ViewProjMatrix();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_LookAt_float_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Frustum_LookAt_float_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    float x = qscriptvalue_cast<float>(context->argument(0));
    float y = qscriptvalue_cast<float>(context->argument(1));
    Ray ret = This.LookAt(x, y);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_LookAtFromNearPlane_float_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Frustum_LookAtFromNearPlane_float_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    float x = qscriptvalue_cast<float>(context->argument(0));
    float y = qscriptvalue_cast<float>(context->argument(1));
    Ray ret = This.LookAtFromNearPlane(x, y);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_Project_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Frustum_Project_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.Project(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_NearPlanePos_float_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Frustum_NearPlanePos_float_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    float x = qscriptvalue_cast<float>(context->argument(0));
    float y = qscriptvalue_cast<float>(context->argument(1));
    float3 ret = This.NearPlanePos(x, y);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_NearPlanePos_float2_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Frustum_NearPlanePos_float2_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    float2 point = qscriptvalue_cast<float2>(context->argument(0));
    float3 ret = This.NearPlanePos(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_FarPlanePos_float_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Frustum_FarPlanePos_float_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    float x = qscriptvalue_cast<float>(context->argument(0));
    float y = qscriptvalue_cast<float>(context->argument(1));
    float3 ret = This.FarPlanePos(x, y);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_FarPlanePos_float2_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Frustum_FarPlanePos_float2_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    float2 point = qscriptvalue_cast<float2>(context->argument(0));
    float3 ret = This.FarPlanePos(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_IsFinite_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Frustum_IsFinite_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    bool ret = This.IsFinite();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_Volume_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Frustum_Volume_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    float ret = This.Volume();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_RandomPointInside_LCG_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Frustum_RandomPointInside_LCG_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    LCG rng = qscriptvalue_cast<LCG>(context->argument(0));
    float3 ret = This.RandomPointInside(rng);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_Translate_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Frustum_Translate_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    float3 offset = qscriptvalue_cast<float3>(context->argument(0));
    This.Translate(offset);
    ToExistingScriptValue_Frustum(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Frustum_Transform_float3x3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Frustum_Transform_float3x3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    float3x3 transform = qscriptvalue_cast<float3x3>(context->argument(0));
    This.Transform(transform);
    ToExistingScriptValue_Frustum(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Frustum_Transform_float3x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Frustum_Transform_float3x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    float3x4 transform = qscriptvalue_cast<float3x4>(context->argument(0));
    This.Transform(transform);
    ToExistingScriptValue_Frustum(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Frustum_Transform_float4x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Frustum_Transform_float4x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    float4x4 transform = qscriptvalue_cast<float4x4>(context->argument(0));
    This.Transform(transform);
    ToExistingScriptValue_Frustum(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Frustum_Transform_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Frustum_Transform_Quat in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    Quat transform = qscriptvalue_cast<Quat>(context->argument(0));
    This.Transform(transform);
    ToExistingScriptValue_Frustum(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Frustum_MinimalEnclosingAABB_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Frustum_MinimalEnclosingAABB_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    AABB ret = This.MinimalEnclosingAABB();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_MinimalEnclosingOBB_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Frustum_MinimalEnclosingOBB_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    OBB ret = This.MinimalEnclosingOBB();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_ToPolyhedron_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Frustum_ToPolyhedron_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    Polyhedron ret = This.ToPolyhedron();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_Contains_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Frustum_Contains_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    bool ret = This.Contains(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_Contains_LineSegment_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Frustum_Contains_LineSegment_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    LineSegment lineSegment = qscriptvalue_cast<LineSegment>(context->argument(0));
    bool ret = This.Contains(lineSegment);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_Contains_Triangle_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Frustum_Contains_Triangle_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    Triangle triangle = qscriptvalue_cast<Triangle>(context->argument(0));
    bool ret = This.Contains(triangle);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_Contains_Polygon_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Frustum_Contains_Polygon_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    Polygon polygon = qscriptvalue_cast<Polygon>(context->argument(0));
    bool ret = This.Contains(polygon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_Contains_AABB_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Frustum_Contains_AABB_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    AABB aabb = qscriptvalue_cast<AABB>(context->argument(0));
    bool ret = This.Contains(aabb);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_Contains_OBB_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Frustum_Contains_OBB_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    OBB obb = qscriptvalue_cast<OBB>(context->argument(0));
    bool ret = This.Contains(obb);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_Contains_Frustum_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Frustum_Contains_Frustum_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    Frustum frustum = qscriptvalue_cast<Frustum>(context->argument(0));
    bool ret = This.Contains(frustum);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_Contains_Polyhedron_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Frustum_Contains_Polyhedron_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    Polyhedron polyhedron = qscriptvalue_cast<Polyhedron>(context->argument(0));
    bool ret = This.Contains(polyhedron);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_ClosestPoint_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Frustum_ClosestPoint_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.ClosestPoint(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_Distance_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Frustum_Distance_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float ret = This.Distance(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_Intersects_Ray_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Frustum_Intersects_Ray_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    Ray ray = qscriptvalue_cast<Ray>(context->argument(0));
    bool ret = This.Intersects(ray);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_Intersects_Line_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Frustum_Intersects_Line_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    Line line = qscriptvalue_cast<Line>(context->argument(0));
    bool ret = This.Intersects(line);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_Intersects_LineSegment_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Frustum_Intersects_LineSegment_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    LineSegment lineSegment = qscriptvalue_cast<LineSegment>(context->argument(0));
    bool ret = This.Intersects(lineSegment);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_Intersects_AABB_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Frustum_Intersects_AABB_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    AABB aabb = qscriptvalue_cast<AABB>(context->argument(0));
    bool ret = This.Intersects(aabb);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_Intersects_OBB_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Frustum_Intersects_OBB_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    OBB obb = qscriptvalue_cast<OBB>(context->argument(0));
    bool ret = This.Intersects(obb);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_Intersects_Plane_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Frustum_Intersects_Plane_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    Plane plane = qscriptvalue_cast<Plane>(context->argument(0));
    bool ret = This.Intersects(plane);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_Intersects_Triangle_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Frustum_Intersects_Triangle_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    Triangle triangle = qscriptvalue_cast<Triangle>(context->argument(0));
    bool ret = This.Intersects(triangle);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_Intersects_Polygon_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Frustum_Intersects_Polygon_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    Polygon polygon = qscriptvalue_cast<Polygon>(context->argument(0));
    bool ret = This.Intersects(polygon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_Intersects_Sphere_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Frustum_Intersects_Sphere_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    Sphere sphere = qscriptvalue_cast<Sphere>(context->argument(0));
    bool ret = This.Intersects(sphere);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_Intersects_Capsule_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Frustum_Intersects_Capsule_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    Capsule capsule = qscriptvalue_cast<Capsule>(context->argument(0));
    bool ret = This.Intersects(capsule);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_Intersects_Frustum_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Frustum_Intersects_Frustum_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    Frustum frustum = qscriptvalue_cast<Frustum>(context->argument(0));
    bool ret = This.Intersects(frustum);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_Intersects_Polyhedron_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Frustum_Intersects_Polyhedron_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Frustum This = qscriptvalue_cast<Frustum>(context->thisObject());
    Polyhedron polyhedron = qscriptvalue_cast<Polyhedron>(context->argument(0));
    bool ret = This.Intersects(polyhedron);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_toString_const(QScriptContext *context, QScriptEngine *engine)
{
    Frustum This;
    if (context->argumentCount() > 0) This = qscriptvalue_cast<Frustum>(context->argument(0)); // Qt oddity (bug?): Sometimes the built-in toString() function doesn't give us this from thisObject, but as the first argument.
    else This = qscriptvalue_cast<Frustum>(context->thisObject());
    QString ret = This.toString();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_ViewportToScreenSpace_float_float_int_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function Frustum_ViewportToScreenSpace_float_float_int_int in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float x = qscriptvalue_cast<float>(context->argument(0));
    float y = qscriptvalue_cast<float>(context->argument(1));
    int screenWidth = qscriptvalue_cast<int>(context->argument(2));
    int screenHeight = qscriptvalue_cast<int>(context->argument(3));
    float2 ret = Frustum::ViewportToScreenSpace(x, y, screenWidth, screenHeight);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_ViewportToScreenSpace_float2_int_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Frustum_ViewportToScreenSpace_float2_int_int in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 point = qscriptvalue_cast<float2>(context->argument(0));
    int screenWidth = qscriptvalue_cast<int>(context->argument(1));
    int screenHeight = qscriptvalue_cast<int>(context->argument(2));
    float2 ret = Frustum::ViewportToScreenSpace(point, screenWidth, screenHeight);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_ScreenToViewportSpace_float_float_int_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function Frustum_ScreenToViewportSpace_float_float_int_int in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float x = qscriptvalue_cast<float>(context->argument(0));
    float y = qscriptvalue_cast<float>(context->argument(1));
    int screenWidth = qscriptvalue_cast<int>(context->argument(2));
    int screenHeight = qscriptvalue_cast<int>(context->argument(3));
    float2 ret = Frustum::ScreenToViewportSpace(x, y, screenWidth, screenHeight);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_ScreenToViewportSpace_float2_int_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Frustum_ScreenToViewportSpace_float2_int_int in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 point = qscriptvalue_cast<float2>(context->argument(0));
    int screenWidth = qscriptvalue_cast<int>(context->argument(1));
    int screenHeight = qscriptvalue_cast<int>(context->argument(2));
    float2 ret = Frustum::ScreenToViewportSpace(point, screenWidth, screenHeight);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Frustum_NearPlanePos_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return Frustum_NearPlanePos_float_float_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float2>(context->argument(0)))
        return Frustum_NearPlanePos_float2_const(context, engine);
    printf("Frustum_NearPlanePos_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Frustum_FarPlanePos_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return Frustum_FarPlanePos_float_float_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float2>(context->argument(0)))
        return Frustum_FarPlanePos_float2_const(context, engine);
    printf("Frustum_FarPlanePos_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Frustum_Transform_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3x3>(context->argument(0)))
        return Frustum_Transform_float3x3(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3x4>(context->argument(0)))
        return Frustum_Transform_float3x4(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float4x4>(context->argument(0)))
        return Frustum_Transform_float4x4(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Quat>(context->argument(0)))
        return Frustum_Transform_Quat(context, engine);
    printf("Frustum_Transform_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Frustum_Contains_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return Frustum_Contains_float3_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<LineSegment>(context->argument(0)))
        return Frustum_Contains_LineSegment_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Triangle>(context->argument(0)))
        return Frustum_Contains_Triangle_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Polygon>(context->argument(0)))
        return Frustum_Contains_Polygon_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<AABB>(context->argument(0)))
        return Frustum_Contains_AABB_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<OBB>(context->argument(0)))
        return Frustum_Contains_OBB_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Frustum>(context->argument(0)))
        return Frustum_Contains_Frustum_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Polyhedron>(context->argument(0)))
        return Frustum_Contains_Polyhedron_const(context, engine);
    printf("Frustum_Contains_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Frustum_Intersects_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<Ray>(context->argument(0)))
        return Frustum_Intersects_Ray_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Line>(context->argument(0)))
        return Frustum_Intersects_Line_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<LineSegment>(context->argument(0)))
        return Frustum_Intersects_LineSegment_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<AABB>(context->argument(0)))
        return Frustum_Intersects_AABB_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<OBB>(context->argument(0)))
        return Frustum_Intersects_OBB_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Plane>(context->argument(0)))
        return Frustum_Intersects_Plane_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Triangle>(context->argument(0)))
        return Frustum_Intersects_Triangle_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Polygon>(context->argument(0)))
        return Frustum_Intersects_Polygon_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Sphere>(context->argument(0)))
        return Frustum_Intersects_Sphere_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Capsule>(context->argument(0)))
        return Frustum_Intersects_Capsule_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Frustum>(context->argument(0)))
        return Frustum_Intersects_Frustum_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Polyhedron>(context->argument(0)))
        return Frustum_Intersects_Polyhedron_const(context, engine);
    printf("Frustum_Intersects_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Frustum_ViewportToScreenSpace_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 4 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<int>(context->argument(2)) && QSVIsOfType<int>(context->argument(3)))
        return Frustum_ViewportToScreenSpace_float_float_int_int(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float2>(context->argument(0)) && QSVIsOfType<int>(context->argument(1)) && QSVIsOfType<int>(context->argument(2)))
        return Frustum_ViewportToScreenSpace_float2_int_int(context, engine);
    printf("Frustum_ViewportToScreenSpace_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Frustum_ScreenToViewportSpace_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 4 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<int>(context->argument(2)) && QSVIsOfType<int>(context->argument(3)))
        return Frustum_ScreenToViewportSpace_float_float_int_int(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float2>(context->argument(0)) && QSVIsOfType<int>(context->argument(1)) && QSVIsOfType<int>(context->argument(2)))
        return Frustum_ScreenToViewportSpace_float2_int_int(context, engine);
    printf("Frustum_ScreenToViewportSpace_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

void FromScriptValue_Frustum(const QScriptValue &obj, Frustum &value)
{
    value = obj.data().toVariant().value<Frustum>();
    value.pos = qScriptValueToValue<float3>(obj.property("pos"));
    value.front = qScriptValueToValue<float3>(obj.property("front"));
    value.up = qScriptValueToValue<float3>(obj.property("up"));
    value.nearPlaneDistance = qScriptValueToValue<float>(obj.property("nearPlaneDistance"));
    value.farPlaneDistance = qScriptValueToValue<float>(obj.property("farPlaneDistance"));
    value.horizontalFov = qScriptValueToValue<float>(obj.property("horizontalFov"));
    value.orthographicWidth = qScriptValueToValue<float>(obj.property("orthographicWidth"));
    value.verticalFov = qScriptValueToValue<float>(obj.property("verticalFov"));
    value.orthographicHeight = qScriptValueToValue<float>(obj.property("orthographicHeight"));
}

QScriptValue ToScriptValue_Frustum(QScriptEngine *engine, const Frustum &value)
{
    QScriptValue obj = engine->newVariant(QVariant::fromValue(value)); // The contents of this variant are NOT used. The real data lies in the data() pointer of this QScriptValue. This only exists to enable overload resolution to work for QObject slots.
    ToExistingScriptValue_Frustum(engine, value, obj);
    return obj;
}

QScriptValue ToScriptValue_const_Frustum(QScriptEngine *engine, const Frustum &value)
{
    QScriptValue obj = engine->newVariant(QVariant::fromValue(value)); // The contents of this variant are NOT used. The real data lies in the data() pointer of this QScriptValue. This only exists to enable overload resolution to work for QObject slots.
    obj.setPrototype(engine->defaultPrototype(qMetaTypeId<Frustum>()));
    obj.setData(engine->newVariant(QVariant::fromValue(value)));
    obj.setProperty("pos", ToScriptValue_const_float3(engine, value.pos), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("front", ToScriptValue_const_float3(engine, value.front), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("up", ToScriptValue_const_float3(engine, value.up), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("nearPlaneDistance", qScriptValueFromValue(engine, value.nearPlaneDistance), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("farPlaneDistance", qScriptValueFromValue(engine, value.farPlaneDistance), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("horizontalFov", qScriptValueFromValue(engine, value.horizontalFov), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("orthographicWidth", qScriptValueFromValue(engine, value.orthographicWidth), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("verticalFov", qScriptValueFromValue(engine, value.verticalFov), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("orthographicHeight", qScriptValueFromValue(engine, value.orthographicHeight), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    return obj;
}

QScriptValue register_Frustum_prototype(QScriptEngine *engine)
{
    QScriptValue proto = engine->newObject();
    proto.setProperty("AspectRatio", engine->newFunction(Frustum_AspectRatio_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("NearPlane", engine->newFunction(Frustum_NearPlane_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("FarPlane", engine->newFunction(Frustum_FarPlane_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("LeftPlane", engine->newFunction(Frustum_LeftPlane_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("RightPlane", engine->newFunction(Frustum_RightPlane_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("TopPlane", engine->newFunction(Frustum_TopPlane_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("BottomPlane", engine->newFunction(Frustum_BottomPlane_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("GetPlane", engine->newFunction(Frustum_GetPlane_int_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("GetPlanes", engine->newFunction(Frustum_GetPlanes_Plane_ptr_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("CornerPoint", engine->newFunction(Frustum_CornerPoint_int_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ExtremePoint", engine->newFunction(Frustum_ExtremePoint_float3_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("WorldMatrix", engine->newFunction(Frustum_WorldMatrix_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ViewMatrix", engine->newFunction(Frustum_ViewMatrix_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ProjectionMatrix", engine->newFunction(Frustum_ProjectionMatrix_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ViewProjMatrix", engine->newFunction(Frustum_ViewProjMatrix_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("LookAt", engine->newFunction(Frustum_LookAt_float_float_const, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("LookAtFromNearPlane", engine->newFunction(Frustum_LookAtFromNearPlane_float_float_const, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Project", engine->newFunction(Frustum_Project_float3_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("NearPlanePos", engine->newFunction(Frustum_NearPlanePos_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("NearPlanePos", engine->newFunction(Frustum_NearPlanePos_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("FarPlanePos", engine->newFunction(Frustum_FarPlanePos_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("FarPlanePos", engine->newFunction(Frustum_FarPlanePos_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsFinite", engine->newFunction(Frustum_IsFinite_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Volume", engine->newFunction(Frustum_Volume_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("RandomPointInside", engine->newFunction(Frustum_RandomPointInside_LCG_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Translate", engine->newFunction(Frustum_Translate_float3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Transform", engine->newFunction(Frustum_Transform_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MinimalEnclosingAABB", engine->newFunction(Frustum_MinimalEnclosingAABB_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MinimalEnclosingOBB", engine->newFunction(Frustum_MinimalEnclosingOBB_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToPolyhedron", engine->newFunction(Frustum_ToPolyhedron_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Contains", engine->newFunction(Frustum_Contains_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ClosestPoint", engine->newFunction(Frustum_ClosestPoint_float3_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Distance", engine->newFunction(Frustum_Distance_float3_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Intersects", engine->newFunction(Frustum_Intersects_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("toString", engine->newFunction(Frustum_toString_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("metaTypeId", engine->toScriptValue<qint32>((qint32)qMetaTypeId<Frustum>()));
    engine->setDefaultPrototype(qMetaTypeId<Frustum>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<Frustum*>(), proto);
    qScriptRegisterMetaType(engine, ToScriptValue_Frustum, FromScriptValue_Frustum, proto);

    QScriptValue ctor = engine->newFunction(Frustum_ctor, proto, 0);
    ctor.setProperty("ViewportToScreenSpace", engine->newFunction(Frustum_ViewportToScreenSpace_selector, 4), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("ViewportToScreenSpace", engine->newFunction(Frustum_ViewportToScreenSpace_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("ScreenToViewportSpace", engine->newFunction(Frustum_ScreenToViewportSpace_selector, 4), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("ScreenToViewportSpace", engine->newFunction(Frustum_ScreenToViewportSpace_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    engine->globalObject().setProperty("Frustum", ctor, QScriptValue::Undeletable | QScriptValue::ReadOnly);

    return ctor;
}

