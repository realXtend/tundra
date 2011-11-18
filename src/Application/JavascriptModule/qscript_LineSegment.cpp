#include "QtScriptBindingsHelpers.h"

void ToExistingScriptValue_LineSegment(QScriptEngine *engine, const LineSegment &value, QScriptValue obj)
{
    obj.setProperty("a", qScriptValueFromValue(engine, value.a), QScriptValue::Undeletable);
    obj.setProperty("b", qScriptValueFromValue(engine, value.b), QScriptValue::Undeletable);
}

static QScriptValue LineSegment_LineSegment(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function LineSegment_LineSegment in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LineSegment ret;
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue LineSegment_LineSegment_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function LineSegment_LineSegment_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 a = qscriptvalue_cast<float3>(context->argument(0));
    float3 b = qscriptvalue_cast<float3>(context->argument(1));
    LineSegment ret(a, b);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue LineSegment_LineSegment_Ray_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function LineSegment_LineSegment_Ray_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Ray ray = qscriptvalue_cast<Ray>(context->argument(0));
    float d = qscriptvalue_cast<float>(context->argument(1));
    LineSegment ret(ray, d);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue LineSegment_LineSegment_Line_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function LineSegment_LineSegment_Line_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Line line = qscriptvalue_cast<Line>(context->argument(0));
    float d = qscriptvalue_cast<float>(context->argument(1));
    LineSegment ret(line, d);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue LineSegment_GetPoint_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function LineSegment_GetPoint_float_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LineSegment This = qscriptvalue_cast<LineSegment>(context->thisObject());
    float d = qscriptvalue_cast<float>(context->argument(0));
    float3 ret = This.GetPoint(d);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue LineSegment_CenterPoint_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function LineSegment_CenterPoint_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LineSegment This = qscriptvalue_cast<LineSegment>(context->thisObject());
    float3 ret = This.CenterPoint();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue LineSegment_Reverse(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function LineSegment_Reverse in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LineSegment This = qscriptvalue_cast<LineSegment>(context->thisObject());
    This.Reverse();
    ToExistingScriptValue_LineSegment(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue LineSegment_Dir_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function LineSegment_Dir_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LineSegment This = qscriptvalue_cast<LineSegment>(context->thisObject());
    float3 ret = This.Dir();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue LineSegment_ExtremePoint_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function LineSegment_ExtremePoint_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LineSegment This = qscriptvalue_cast<LineSegment>(context->thisObject());
    float3 direction = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.ExtremePoint(direction);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue LineSegment_Transform_float3x3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function LineSegment_Transform_float3x3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LineSegment This = qscriptvalue_cast<LineSegment>(context->thisObject());
    float3x3 transform = qscriptvalue_cast<float3x3>(context->argument(0));
    This.Transform(transform);
    ToExistingScriptValue_LineSegment(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue LineSegment_Transform_float3x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function LineSegment_Transform_float3x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LineSegment This = qscriptvalue_cast<LineSegment>(context->thisObject());
    float3x4 transform = qscriptvalue_cast<float3x4>(context->argument(0));
    This.Transform(transform);
    ToExistingScriptValue_LineSegment(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue LineSegment_Transform_float4x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function LineSegment_Transform_float4x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LineSegment This = qscriptvalue_cast<LineSegment>(context->thisObject());
    float4x4 transform = qscriptvalue_cast<float4x4>(context->argument(0));
    This.Transform(transform);
    ToExistingScriptValue_LineSegment(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue LineSegment_Transform_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function LineSegment_Transform_Quat in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LineSegment This = qscriptvalue_cast<LineSegment>(context->thisObject());
    Quat transform = qscriptvalue_cast<Quat>(context->argument(0));
    This.Transform(transform);
    ToExistingScriptValue_LineSegment(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue LineSegment_Length_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function LineSegment_Length_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LineSegment This = qscriptvalue_cast<LineSegment>(context->thisObject());
    float ret = This.Length();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue LineSegment_LengthSq_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function LineSegment_LengthSq_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LineSegment This = qscriptvalue_cast<LineSegment>(context->thisObject());
    float ret = This.LengthSq();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue LineSegment_IsFinite_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function LineSegment_IsFinite_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LineSegment This = qscriptvalue_cast<LineSegment>(context->thisObject());
    bool ret = This.IsFinite();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue LineSegment_Equals_LineSegment_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function LineSegment_Equals_LineSegment_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LineSegment This = qscriptvalue_cast<LineSegment>(context->thisObject());
    LineSegment rhs = qscriptvalue_cast<LineSegment>(context->argument(0));
    float distanceThreshold = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.Equals(rhs, distanceThreshold);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue LineSegment_Contains_float3_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function LineSegment_Contains_float3_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LineSegment This = qscriptvalue_cast<LineSegment>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float distanceThreshold = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.Contains(point, distanceThreshold);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue LineSegment_Contains_LineSegment_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function LineSegment_Contains_LineSegment_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LineSegment This = qscriptvalue_cast<LineSegment>(context->thisObject());
    LineSegment lineSegment = qscriptvalue_cast<LineSegment>(context->argument(0));
    float distanceThreshold = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.Contains(lineSegment, distanceThreshold);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue LineSegment_Distance_Plane_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function LineSegment_Distance_Plane_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LineSegment This = qscriptvalue_cast<LineSegment>(context->thisObject());
    Plane other = qscriptvalue_cast<Plane>(context->argument(0));
    float ret = This.Distance(other);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue LineSegment_Distance_Sphere_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function LineSegment_Distance_Sphere_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LineSegment This = qscriptvalue_cast<LineSegment>(context->thisObject());
    Sphere other = qscriptvalue_cast<Sphere>(context->argument(0));
    float ret = This.Distance(other);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue LineSegment_Distance_Capsule_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function LineSegment_Distance_Capsule_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LineSegment This = qscriptvalue_cast<LineSegment>(context->thisObject());
    Capsule other = qscriptvalue_cast<Capsule>(context->argument(0));
    float ret = This.Distance(other);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue LineSegment_Intersects_Plane_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function LineSegment_Intersects_Plane_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LineSegment This = qscriptvalue_cast<LineSegment>(context->thisObject());
    Plane plane = qscriptvalue_cast<Plane>(context->argument(0));
    bool ret = This.Intersects(plane);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue LineSegment_Intersects_Capsule_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function LineSegment_Intersects_Capsule_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LineSegment This = qscriptvalue_cast<LineSegment>(context->thisObject());
    Capsule capsule = qscriptvalue_cast<Capsule>(context->argument(0));
    bool ret = This.Intersects(capsule);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue LineSegment_Intersects_Polygon_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function LineSegment_Intersects_Polygon_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LineSegment This = qscriptvalue_cast<LineSegment>(context->thisObject());
    Polygon polygon = qscriptvalue_cast<Polygon>(context->argument(0));
    bool ret = This.Intersects(polygon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue LineSegment_Intersects_Frustum_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function LineSegment_Intersects_Frustum_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LineSegment This = qscriptvalue_cast<LineSegment>(context->thisObject());
    Frustum frustum = qscriptvalue_cast<Frustum>(context->argument(0));
    bool ret = This.Intersects(frustum);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue LineSegment_Intersects_Polyhedron_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function LineSegment_Intersects_Polyhedron_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LineSegment This = qscriptvalue_cast<LineSegment>(context->thisObject());
    Polyhedron polyhedron = qscriptvalue_cast<Polyhedron>(context->argument(0));
    bool ret = This.Intersects(polyhedron);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue LineSegment_Intersects_LineSegment_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function LineSegment_Intersects_LineSegment_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LineSegment This = qscriptvalue_cast<LineSegment>(context->thisObject());
    LineSegment lineSegment = qscriptvalue_cast<LineSegment>(context->argument(0));
    float epsilon = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.Intersects(lineSegment, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue LineSegment_IntersectsDisc_Circle_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function LineSegment_IntersectsDisc_Circle_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LineSegment This = qscriptvalue_cast<LineSegment>(context->thisObject());
    Circle disc = qscriptvalue_cast<Circle>(context->argument(0));
    bool ret = This.IntersectsDisc(disc);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue LineSegment_ToRay_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function LineSegment_ToRay_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LineSegment This = qscriptvalue_cast<LineSegment>(context->thisObject());
    Ray ret = This.ToRay();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue LineSegment_ToLine_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function LineSegment_ToLine_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LineSegment This = qscriptvalue_cast<LineSegment>(context->thisObject());
    Line ret = This.ToLine();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue LineSegment_toString_const(QScriptContext *context, QScriptEngine *engine)
{
    LineSegment This;
    if (context->argumentCount() > 0) This = qscriptvalue_cast<LineSegment>(context->argument(0)); // Qt oddity (bug?): Sometimes the built-in toString() function doesn't give us this from thisObject, but as the first argument.
    else This = qscriptvalue_cast<LineSegment>(context->thisObject());
    QString ret = This.toString();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue LineSegment_ctor(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return LineSegment_LineSegment(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return LineSegment_LineSegment_float3_float3(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<Ray>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return LineSegment_LineSegment_Ray_float(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<Line>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return LineSegment_LineSegment_Line_float(context, engine);
    printf("LineSegment_ctor failed to choose the right function to call! Did you use 'var x = LineSegment();' instead of 'var x = new LineSegment();'?\n"); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue LineSegment_Transform_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3x3>(context->argument(0)))
        return LineSegment_Transform_float3x3(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3x4>(context->argument(0)))
        return LineSegment_Transform_float3x4(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float4x4>(context->argument(0)))
        return LineSegment_Transform_float4x4(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Quat>(context->argument(0)))
        return LineSegment_Transform_Quat(context, engine);
    printf("LineSegment_Transform_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue LineSegment_Contains_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return LineSegment_Contains_float3_float_const(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<LineSegment>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return LineSegment_Contains_LineSegment_float_const(context, engine);
    printf("LineSegment_Contains_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue LineSegment_Distance_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<Plane>(context->argument(0)))
        return LineSegment_Distance_Plane_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Sphere>(context->argument(0)))
        return LineSegment_Distance_Sphere_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Capsule>(context->argument(0)))
        return LineSegment_Distance_Capsule_const(context, engine);
    printf("LineSegment_Distance_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue LineSegment_Intersects_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<Plane>(context->argument(0)))
        return LineSegment_Intersects_Plane_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Capsule>(context->argument(0)))
        return LineSegment_Intersects_Capsule_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Polygon>(context->argument(0)))
        return LineSegment_Intersects_Polygon_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Frustum>(context->argument(0)))
        return LineSegment_Intersects_Frustum_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Polyhedron>(context->argument(0)))
        return LineSegment_Intersects_Polyhedron_const(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<LineSegment>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return LineSegment_Intersects_LineSegment_float_const(context, engine);
    printf("LineSegment_Intersects_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

void FromScriptValue_LineSegment(const QScriptValue &obj, LineSegment &value)
{
    value.a = qScriptValueToValue<float3>(obj.property("a"));
    value.b = qScriptValueToValue<float3>(obj.property("b"));
}

QScriptValue ToScriptValue_LineSegment(QScriptEngine *engine, const LineSegment &value)
{
    QScriptValue obj = engine->newVariant(QVariant::fromValue(value)); // The contents of this variant are NOT used. The real data lies in the data() pointer of this QScriptValue. This only exists to enable overload resolution to work for QObject slots.
    ToExistingScriptValue_LineSegment(engine, value, obj);
    return obj;
}

QScriptValue ToScriptValue_const_LineSegment(QScriptEngine *engine, const LineSegment &value)
{
    QScriptValue obj = engine->newVariant(QVariant::fromValue(value)); // The contents of this variant are NOT used. The real data lies in the data() pointer of this QScriptValue. This only exists to enable overload resolution to work for QObject slots.
    obj.setPrototype(engine->defaultPrototype(qMetaTypeId<LineSegment>()));
    obj.setProperty("a", ToScriptValue_const_float3(engine, value.a), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("b", ToScriptValue_const_float3(engine, value.b), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    return obj;
}

QScriptValue register_LineSegment_prototype(QScriptEngine *engine)
{
    QScriptValue proto = engine->newObject();
    proto.setProperty("GetPoint", engine->newFunction(LineSegment_GetPoint_float_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("CenterPoint", engine->newFunction(LineSegment_CenterPoint_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Reverse", engine->newFunction(LineSegment_Reverse, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Dir", engine->newFunction(LineSegment_Dir_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ExtremePoint", engine->newFunction(LineSegment_ExtremePoint_float3_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Transform", engine->newFunction(LineSegment_Transform_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Length", engine->newFunction(LineSegment_Length_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("LengthSq", engine->newFunction(LineSegment_LengthSq_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsFinite", engine->newFunction(LineSegment_IsFinite_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Equals", engine->newFunction(LineSegment_Equals_LineSegment_float_const, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Contains", engine->newFunction(LineSegment_Contains_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Distance", engine->newFunction(LineSegment_Distance_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Intersects", engine->newFunction(LineSegment_Intersects_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Intersects", engine->newFunction(LineSegment_Intersects_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IntersectsDisc", engine->newFunction(LineSegment_IntersectsDisc_Circle_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToRay", engine->newFunction(LineSegment_ToRay_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToLine", engine->newFunction(LineSegment_ToLine_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("toString", engine->newFunction(LineSegment_toString_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("metaTypeId", engine->toScriptValue<qint32>((qint32)qMetaTypeId<LineSegment>()));
    engine->setDefaultPrototype(qMetaTypeId<LineSegment>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<LineSegment*>(), proto);
    qScriptRegisterMetaType(engine, ToScriptValue_LineSegment, FromScriptValue_LineSegment, proto);

    QScriptValue ctor = engine->newFunction(LineSegment_ctor, proto, 2);
    engine->globalObject().setProperty("LineSegment", ctor, QScriptValue::Undeletable | QScriptValue::ReadOnly);

    return ctor;
}

