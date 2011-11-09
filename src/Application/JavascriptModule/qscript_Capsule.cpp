#include "QtScriptBindingsHelpers.h"

void ToExistingScriptValue_Capsule(QScriptEngine *engine, const Capsule &value, QScriptValue obj)
{
    obj.setProperty("l", qScriptValueFromValue(engine, value.l), QScriptValue::Undeletable);
    obj.setProperty("r", qScriptValueFromValue(engine, value.r), QScriptValue::Undeletable);
}

static QScriptValue Capsule_Capsule(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Capsule_Capsule in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule ret;
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_Capsule_LineSegment_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Capsule_Capsule_LineSegment_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LineSegment endPoints = qscriptvalue_cast<LineSegment>(context->argument(0));
    float radius = qscriptvalue_cast<float>(context->argument(1));
    Capsule ret(endPoints, radius);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_Capsule_float3_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Capsule_Capsule_float3_float3_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 bottomPoint = qscriptvalue_cast<float3>(context->argument(0));
    float3 topPoint = qscriptvalue_cast<float3>(context->argument(1));
    float radius = qscriptvalue_cast<float>(context->argument(2));
    Capsule ret(bottomPoint, topPoint, radius);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_SetFrom_Sphere(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_SetFrom_Sphere in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    Sphere s = qscriptvalue_cast<Sphere>(context->argument(0));
    This.SetFrom(s);
    ToExistingScriptValue_Capsule(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Capsule_LineLength_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Capsule_LineLength_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    float ret = This.LineLength();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_Height_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Capsule_Height_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    float ret = This.Height();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_Diameter_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Capsule_Diameter_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    float ret = This.Diameter();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_Bottom_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Capsule_Bottom_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    float3 ret = This.Bottom();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_Center_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Capsule_Center_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    float3 ret = This.Center();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_Centroid_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Capsule_Centroid_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    float3 ret = This.Centroid();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_ExtremePoint_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_ExtremePoint_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    float3 direction = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.ExtremePoint(direction);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_Top_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Capsule_Top_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    float3 ret = This.Top();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_UpDirection_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Capsule_UpDirection_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    float3 ret = This.UpDirection();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_Volume_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Capsule_Volume_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    float ret = This.Volume();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_SurfaceArea_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Capsule_SurfaceArea_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    float ret = This.SurfaceArea();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_CrossSection_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_CrossSection_float_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    float l = qscriptvalue_cast<float>(context->argument(0));
    Circle ret = This.CrossSection(l);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_HeightLineSegment_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Capsule_HeightLineSegment_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    LineSegment ret = This.HeightLineSegment();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_IsFinite_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Capsule_IsFinite_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    bool ret = This.IsFinite();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_PointInside_float_float_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Capsule_PointInside_float_float_float_const in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    float l = qscriptvalue_cast<float>(context->argument(0));
    float a = qscriptvalue_cast<float>(context->argument(1));
    float d = qscriptvalue_cast<float>(context->argument(2));
    float3 ret = This.PointInside(l, a, d);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_UniformPointPerhapsInside_float_float_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Capsule_UniformPointPerhapsInside_float_float_float_const in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    float l = qscriptvalue_cast<float>(context->argument(0));
    float x = qscriptvalue_cast<float>(context->argument(1));
    float y = qscriptvalue_cast<float>(context->argument(2));
    float3 ret = This.UniformPointPerhapsInside(l, x, y);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_MinimalEnclosingAABB_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Capsule_MinimalEnclosingAABB_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    AABB ret = This.MinimalEnclosingAABB();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_MinimalEnclosingOBB_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Capsule_MinimalEnclosingOBB_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    OBB ret = This.MinimalEnclosingOBB();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_RandomPointInside_LCG_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_RandomPointInside_LCG_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    LCG rng = qscriptvalue_cast<LCG>(context->argument(0));
    float3 ret = This.RandomPointInside(rng);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_RandomPointOnSurface_LCG_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_RandomPointOnSurface_LCG_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    LCG rng = qscriptvalue_cast<LCG>(context->argument(0));
    float3 ret = This.RandomPointOnSurface(rng);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_Translate_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_Translate_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    float3 offset = qscriptvalue_cast<float3>(context->argument(0));
    This.Translate(offset);
    ToExistingScriptValue_Capsule(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Capsule_Scale_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Capsule_Scale_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    float3 centerPoint = qscriptvalue_cast<float3>(context->argument(0));
    float scaleFactor = qscriptvalue_cast<float>(context->argument(1));
    This.Scale(centerPoint, scaleFactor);
    ToExistingScriptValue_Capsule(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Capsule_Transform_float3x3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_Transform_float3x3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    float3x3 transform = qscriptvalue_cast<float3x3>(context->argument(0));
    This.Transform(transform);
    ToExistingScriptValue_Capsule(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Capsule_Transform_float3x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_Transform_float3x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    float3x4 transform = qscriptvalue_cast<float3x4>(context->argument(0));
    This.Transform(transform);
    ToExistingScriptValue_Capsule(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Capsule_Transform_float4x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_Transform_float4x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    float4x4 transform = qscriptvalue_cast<float4x4>(context->argument(0));
    This.Transform(transform);
    ToExistingScriptValue_Capsule(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Capsule_Transform_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_Transform_Quat in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    Quat transform = qscriptvalue_cast<Quat>(context->argument(0));
    This.Transform(transform);
    ToExistingScriptValue_Capsule(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Capsule_ClosestPoint_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_ClosestPoint_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    float3 targetPoint = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.ClosestPoint(targetPoint);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_Distance_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_Distance_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float ret = This.Distance(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_Distance_Plane_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_Distance_Plane_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    Plane plane = qscriptvalue_cast<Plane>(context->argument(0));
    float ret = This.Distance(plane);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_Distance_Sphere_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_Distance_Sphere_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    Sphere sphere = qscriptvalue_cast<Sphere>(context->argument(0));
    float ret = This.Distance(sphere);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_Distance_Ray_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_Distance_Ray_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    Ray ray = qscriptvalue_cast<Ray>(context->argument(0));
    float ret = This.Distance(ray);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_Distance_Line_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_Distance_Line_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    Line line = qscriptvalue_cast<Line>(context->argument(0));
    float ret = This.Distance(line);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_Distance_LineSegment_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_Distance_LineSegment_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    LineSegment lineSegment = qscriptvalue_cast<LineSegment>(context->argument(0));
    float ret = This.Distance(lineSegment);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_Distance_Capsule_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_Distance_Capsule_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    Capsule capsule = qscriptvalue_cast<Capsule>(context->argument(0));
    float ret = This.Distance(capsule);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_Contains_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_Contains_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    bool ret = This.Contains(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_Contains_LineSegment_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_Contains_LineSegment_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    LineSegment lineSegment = qscriptvalue_cast<LineSegment>(context->argument(0));
    bool ret = This.Contains(lineSegment);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_Contains_Triangle_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_Contains_Triangle_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    Triangle triangle = qscriptvalue_cast<Triangle>(context->argument(0));
    bool ret = This.Contains(triangle);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_Contains_Polygon_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_Contains_Polygon_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    Polygon polygon = qscriptvalue_cast<Polygon>(context->argument(0));
    bool ret = This.Contains(polygon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_Contains_AABB_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_Contains_AABB_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    AABB aabb = qscriptvalue_cast<AABB>(context->argument(0));
    bool ret = This.Contains(aabb);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_Contains_OBB_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_Contains_OBB_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    OBB obb = qscriptvalue_cast<OBB>(context->argument(0));
    bool ret = This.Contains(obb);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_Contains_Frustum_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_Contains_Frustum_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    Frustum frustum = qscriptvalue_cast<Frustum>(context->argument(0));
    bool ret = This.Contains(frustum);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_Contains_Polyhedron_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_Contains_Polyhedron_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    Polyhedron polyhedron = qscriptvalue_cast<Polyhedron>(context->argument(0));
    bool ret = This.Contains(polyhedron);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_Intersects_Ray_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_Intersects_Ray_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    Ray ray = qscriptvalue_cast<Ray>(context->argument(0));
    bool ret = This.Intersects(ray);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_Intersects_Line_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_Intersects_Line_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    Line line = qscriptvalue_cast<Line>(context->argument(0));
    bool ret = This.Intersects(line);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_Intersects_LineSegment_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_Intersects_LineSegment_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    LineSegment lineSegment = qscriptvalue_cast<LineSegment>(context->argument(0));
    bool ret = This.Intersects(lineSegment);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_Intersects_Plane_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_Intersects_Plane_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    Plane plane = qscriptvalue_cast<Plane>(context->argument(0));
    bool ret = This.Intersects(plane);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_Intersects_Sphere_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_Intersects_Sphere_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    Sphere sphere = qscriptvalue_cast<Sphere>(context->argument(0));
    bool ret = This.Intersects(sphere);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_Intersects_Capsule_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_Intersects_Capsule_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    Capsule capsule = qscriptvalue_cast<Capsule>(context->argument(0));
    bool ret = This.Intersects(capsule);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_Intersects_AABB_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_Intersects_AABB_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    AABB aabb = qscriptvalue_cast<AABB>(context->argument(0));
    bool ret = This.Intersects(aabb);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_Intersects_OBB_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_Intersects_OBB_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    OBB obb = qscriptvalue_cast<OBB>(context->argument(0));
    bool ret = This.Intersects(obb);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_Intersects_Triangle_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_Intersects_Triangle_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    Triangle triangle = qscriptvalue_cast<Triangle>(context->argument(0));
    bool ret = This.Intersects(triangle);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_Intersects_Polygon_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_Intersects_Polygon_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    Polygon polygon = qscriptvalue_cast<Polygon>(context->argument(0));
    bool ret = This.Intersects(polygon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_Intersects_Frustum_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_Intersects_Frustum_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    Frustum frustum = qscriptvalue_cast<Frustum>(context->argument(0));
    bool ret = This.Intersects(frustum);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_Intersects_Polyhedron_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Capsule_Intersects_Polyhedron_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Capsule This = qscriptvalue_cast<Capsule>(context->thisObject());
    Polyhedron polyhedron = qscriptvalue_cast<Polyhedron>(context->argument(0));
    bool ret = This.Intersects(polyhedron);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_toString_const(QScriptContext *context, QScriptEngine *engine)
{
    Capsule This;
    if (context->argumentCount() > 0) This = qscriptvalue_cast<Capsule>(context->argument(0)); // Qt oddity (bug?): Sometimes the built-in toString() function doesn't give us this from thisObject, but as the first argument.
    else This = qscriptvalue_cast<Capsule>(context->thisObject());
    QString ret = This.toString();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Capsule_ctor(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return Capsule_Capsule(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<LineSegment>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return Capsule_Capsule_LineSegment_float(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return Capsule_Capsule_float3_float3_float(context, engine);
    printf("Capsule_ctor failed to choose the right function to call! Did you use 'var x = Capsule();' instead of 'var x = new Capsule();'?\n"); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Capsule_Transform_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3x3>(context->argument(0)))
        return Capsule_Transform_float3x3(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3x4>(context->argument(0)))
        return Capsule_Transform_float3x4(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float4x4>(context->argument(0)))
        return Capsule_Transform_float4x4(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Quat>(context->argument(0)))
        return Capsule_Transform_Quat(context, engine);
    printf("Capsule_Transform_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Capsule_Distance_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return Capsule_Distance_float3_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Plane>(context->argument(0)))
        return Capsule_Distance_Plane_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Sphere>(context->argument(0)))
        return Capsule_Distance_Sphere_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Ray>(context->argument(0)))
        return Capsule_Distance_Ray_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Line>(context->argument(0)))
        return Capsule_Distance_Line_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<LineSegment>(context->argument(0)))
        return Capsule_Distance_LineSegment_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Capsule>(context->argument(0)))
        return Capsule_Distance_Capsule_const(context, engine);
    printf("Capsule_Distance_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Capsule_Contains_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return Capsule_Contains_float3_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<LineSegment>(context->argument(0)))
        return Capsule_Contains_LineSegment_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Triangle>(context->argument(0)))
        return Capsule_Contains_Triangle_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Polygon>(context->argument(0)))
        return Capsule_Contains_Polygon_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<AABB>(context->argument(0)))
        return Capsule_Contains_AABB_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<OBB>(context->argument(0)))
        return Capsule_Contains_OBB_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Frustum>(context->argument(0)))
        return Capsule_Contains_Frustum_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Polyhedron>(context->argument(0)))
        return Capsule_Contains_Polyhedron_const(context, engine);
    printf("Capsule_Contains_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Capsule_Intersects_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<Ray>(context->argument(0)))
        return Capsule_Intersects_Ray_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Line>(context->argument(0)))
        return Capsule_Intersects_Line_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<LineSegment>(context->argument(0)))
        return Capsule_Intersects_LineSegment_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Plane>(context->argument(0)))
        return Capsule_Intersects_Plane_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Sphere>(context->argument(0)))
        return Capsule_Intersects_Sphere_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Capsule>(context->argument(0)))
        return Capsule_Intersects_Capsule_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<AABB>(context->argument(0)))
        return Capsule_Intersects_AABB_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<OBB>(context->argument(0)))
        return Capsule_Intersects_OBB_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Triangle>(context->argument(0)))
        return Capsule_Intersects_Triangle_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Polygon>(context->argument(0)))
        return Capsule_Intersects_Polygon_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Frustum>(context->argument(0)))
        return Capsule_Intersects_Frustum_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Polyhedron>(context->argument(0)))
        return Capsule_Intersects_Polyhedron_const(context, engine);
    printf("Capsule_Intersects_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

void FromScriptValue_Capsule(const QScriptValue &obj, Capsule &value)
{
    value.l = qScriptValueToValue<LineSegment>(obj.property("l"));
    value.r = qScriptValueToValue<float>(obj.property("r"));
}

QScriptValue ToScriptValue_Capsule(QScriptEngine *engine, const Capsule &value)
{
    QScriptValue obj = engine->newVariant(QVariant::fromValue(value)); // The contents of this variant are NOT used. The real data lies in the data() pointer of this QScriptValue. This only exists to enable overload resolution to work for QObject slots.
    ToExistingScriptValue_Capsule(engine, value, obj);
    return obj;
}

QScriptValue ToScriptValue_const_Capsule(QScriptEngine *engine, const Capsule &value)
{
    QScriptValue obj = engine->newVariant(QVariant::fromValue(value)); // The contents of this variant are NOT used. The real data lies in the data() pointer of this QScriptValue. This only exists to enable overload resolution to work for QObject slots.
    obj.setPrototype(engine->defaultPrototype(qMetaTypeId<Capsule>()));
    obj.setProperty("l", ToScriptValue_const_LineSegment(engine, value.l), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("r", qScriptValueFromValue(engine, value.r), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    return obj;
}

QScriptValue register_Capsule_prototype(QScriptEngine *engine)
{
    QScriptValue proto = engine->newObject();
    proto.setProperty("SetFrom", engine->newFunction(Capsule_SetFrom_Sphere, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("LineLength", engine->newFunction(Capsule_LineLength_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Height", engine->newFunction(Capsule_Height_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Diameter", engine->newFunction(Capsule_Diameter_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Bottom", engine->newFunction(Capsule_Bottom_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Center", engine->newFunction(Capsule_Center_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Centroid", engine->newFunction(Capsule_Centroid_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ExtremePoint", engine->newFunction(Capsule_ExtremePoint_float3_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Top", engine->newFunction(Capsule_Top_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("UpDirection", engine->newFunction(Capsule_UpDirection_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Volume", engine->newFunction(Capsule_Volume_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SurfaceArea", engine->newFunction(Capsule_SurfaceArea_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("CrossSection", engine->newFunction(Capsule_CrossSection_float_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("HeightLineSegment", engine->newFunction(Capsule_HeightLineSegment_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsFinite", engine->newFunction(Capsule_IsFinite_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("PointInside", engine->newFunction(Capsule_PointInside_float_float_float_const, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("UniformPointPerhapsInside", engine->newFunction(Capsule_UniformPointPerhapsInside_float_float_float_const, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MinimalEnclosingAABB", engine->newFunction(Capsule_MinimalEnclosingAABB_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MinimalEnclosingOBB", engine->newFunction(Capsule_MinimalEnclosingOBB_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("RandomPointInside", engine->newFunction(Capsule_RandomPointInside_LCG_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("RandomPointOnSurface", engine->newFunction(Capsule_RandomPointOnSurface_LCG_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Translate", engine->newFunction(Capsule_Translate_float3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Scale", engine->newFunction(Capsule_Scale_float3_float, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Transform", engine->newFunction(Capsule_Transform_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ClosestPoint", engine->newFunction(Capsule_ClosestPoint_float3_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Distance", engine->newFunction(Capsule_Distance_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Contains", engine->newFunction(Capsule_Contains_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Intersects", engine->newFunction(Capsule_Intersects_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("toString", engine->newFunction(Capsule_toString_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("metaTypeId", engine->toScriptValue<qint32>((qint32)qMetaTypeId<Capsule>()));
    engine->setDefaultPrototype(qMetaTypeId<Capsule>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<Capsule*>(), proto);
    qScriptRegisterMetaType(engine, ToScriptValue_Capsule, FromScriptValue_Capsule, proto);

    QScriptValue ctor = engine->newFunction(Capsule_ctor, proto, 3);
    engine->globalObject().setProperty("Capsule", ctor, QScriptValue::Undeletable | QScriptValue::ReadOnly);

    return ctor;
}

