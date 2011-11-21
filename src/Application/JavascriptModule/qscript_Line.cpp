#include "QtScriptBindingsHelpers.h"

void ToExistingScriptValue_Line(QScriptEngine *engine, const Line &value, QScriptValue obj)
{
    obj.setProperty("pos", qScriptValueFromValue(engine, value.pos), QScriptValue::Undeletable);
    obj.setProperty("dir", qScriptValueFromValue(engine, value.dir), QScriptValue::Undeletable);
}

static QScriptValue Line_Line(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Line_Line in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Line ret;
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Line_Line_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Line_Line_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 pos = qscriptvalue_cast<float3>(context->argument(0));
    float3 dir = qscriptvalue_cast<float3>(context->argument(1));
    Line ret(pos, dir);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Line_Line_Ray(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Line_Line_Ray in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Ray ray = qscriptvalue_cast<Ray>(context->argument(0));
    Line ret(ray);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Line_Line_LineSegment(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Line_Line_LineSegment in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LineSegment lineSegment = qscriptvalue_cast<LineSegment>(context->argument(0));
    Line ret(lineSegment);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Line_GetPoint_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Line_GetPoint_float_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Line This = qscriptvalue_cast<Line>(context->thisObject());
    float distance = qscriptvalue_cast<float>(context->argument(0));
    float3 ret = This.GetPoint(distance);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Line_Transform_float3x3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Line_Transform_float3x3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Line This = qscriptvalue_cast<Line>(context->thisObject());
    float3x3 transform = qscriptvalue_cast<float3x3>(context->argument(0));
    This.Transform(transform);
    ToExistingScriptValue_Line(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Line_Transform_float3x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Line_Transform_float3x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Line This = qscriptvalue_cast<Line>(context->thisObject());
    float3x4 transform = qscriptvalue_cast<float3x4>(context->argument(0));
    This.Transform(transform);
    ToExistingScriptValue_Line(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Line_Transform_float4x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Line_Transform_float4x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Line This = qscriptvalue_cast<Line>(context->thisObject());
    float4x4 transform = qscriptvalue_cast<float4x4>(context->argument(0));
    This.Transform(transform);
    ToExistingScriptValue_Line(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Line_Transform_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Line_Transform_Quat in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Line This = qscriptvalue_cast<Line>(context->thisObject());
    Quat transform = qscriptvalue_cast<Quat>(context->argument(0));
    This.Transform(transform);
    ToExistingScriptValue_Line(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Line_Contains_float3_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Line_Contains_float3_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Line This = qscriptvalue_cast<Line>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float distanceThreshold = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.Contains(point, distanceThreshold);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Line_Contains_Ray_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Line_Contains_Ray_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Line This = qscriptvalue_cast<Line>(context->thisObject());
    Ray ray = qscriptvalue_cast<Ray>(context->argument(0));
    float distanceThreshold = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.Contains(ray, distanceThreshold);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Line_Contains_LineSegment_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Line_Contains_LineSegment_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Line This = qscriptvalue_cast<Line>(context->thisObject());
    LineSegment lineSegment = qscriptvalue_cast<LineSegment>(context->argument(0));
    float distanceThreshold = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.Contains(lineSegment, distanceThreshold);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Line_Equals_Line_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Line_Equals_Line_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Line This = qscriptvalue_cast<Line>(context->thisObject());
    Line line = qscriptvalue_cast<Line>(context->argument(0));
    float epsilon = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.Equals(line, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Line_Distance_Ray_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Line_Distance_Ray_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Line This = qscriptvalue_cast<Line>(context->thisObject());
    Ray other = qscriptvalue_cast<Ray>(context->argument(0));
    float ret = This.Distance(other);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Line_Distance_Line_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Line_Distance_Line_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Line This = qscriptvalue_cast<Line>(context->thisObject());
    Line other = qscriptvalue_cast<Line>(context->argument(0));
    float ret = This.Distance(other);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Line_Distance_LineSegment_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Line_Distance_LineSegment_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Line This = qscriptvalue_cast<Line>(context->thisObject());
    LineSegment other = qscriptvalue_cast<LineSegment>(context->argument(0));
    float ret = This.Distance(other);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Line_Distance_Sphere_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Line_Distance_Sphere_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Line This = qscriptvalue_cast<Line>(context->thisObject());
    Sphere other = qscriptvalue_cast<Sphere>(context->argument(0));
    float ret = This.Distance(other);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Line_Distance_Capsule_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Line_Distance_Capsule_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Line This = qscriptvalue_cast<Line>(context->thisObject());
    Capsule other = qscriptvalue_cast<Capsule>(context->argument(0));
    float ret = This.Distance(other);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Line_Intersects_Capsule_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Line_Intersects_Capsule_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Line This = qscriptvalue_cast<Line>(context->thisObject());
    Capsule capsule = qscriptvalue_cast<Capsule>(context->argument(0));
    bool ret = This.Intersects(capsule);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Line_Intersects_Polygon_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Line_Intersects_Polygon_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Line This = qscriptvalue_cast<Line>(context->thisObject());
    Polygon polygon = qscriptvalue_cast<Polygon>(context->argument(0));
    bool ret = This.Intersects(polygon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Line_Intersects_Frustum_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Line_Intersects_Frustum_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Line This = qscriptvalue_cast<Line>(context->thisObject());
    Frustum frustum = qscriptvalue_cast<Frustum>(context->argument(0));
    bool ret = This.Intersects(frustum);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Line_Intersects_Polyhedron_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Line_Intersects_Polyhedron_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Line This = qscriptvalue_cast<Line>(context->thisObject());
    Polyhedron polyhedron = qscriptvalue_cast<Polyhedron>(context->argument(0));
    bool ret = This.Intersects(polyhedron);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Line_IntersectsDisc_Circle_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Line_IntersectsDisc_Circle_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Line This = qscriptvalue_cast<Line>(context->thisObject());
    Circle disc = qscriptvalue_cast<Circle>(context->argument(0));
    bool ret = This.IntersectsDisc(disc);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Line_ToRay_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Line_ToRay_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Line This = qscriptvalue_cast<Line>(context->thisObject());
    Ray ret = This.ToRay();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Line_ToLineSegment_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Line_ToLineSegment_float_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Line This = qscriptvalue_cast<Line>(context->thisObject());
    float d = qscriptvalue_cast<float>(context->argument(0));
    LineSegment ret = This.ToLineSegment(d);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Line_toString_const(QScriptContext *context, QScriptEngine *engine)
{
    Line This;
    if (context->argumentCount() > 0) This = qscriptvalue_cast<Line>(context->argument(0)); // Qt oddity (bug?): Sometimes the built-in toString() function doesn't give us this from thisObject, but as the first argument.
    else This = qscriptvalue_cast<Line>(context->thisObject());
    QString ret = This.toString();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Line_AreCollinear_float3_float3_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function Line_AreCollinear_float3_float3_float3_float in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 p1 = qscriptvalue_cast<float3>(context->argument(0));
    float3 p2 = qscriptvalue_cast<float3>(context->argument(1));
    float3 p3 = qscriptvalue_cast<float3>(context->argument(2));
    float epsilon = qscriptvalue_cast<float>(context->argument(3));
    bool ret = Line::AreCollinear(p1, p2, p3, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Line_ctor(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return Line_Line(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return Line_Line_float3_float3(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Ray>(context->argument(0)))
        return Line_Line_Ray(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<LineSegment>(context->argument(0)))
        return Line_Line_LineSegment(context, engine);
    printf("Line_ctor failed to choose the right function to call! Did you use 'var x = Line();' instead of 'var x = new Line();'?\n"); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Line_Transform_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3x3>(context->argument(0)))
        return Line_Transform_float3x3(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3x4>(context->argument(0)))
        return Line_Transform_float3x4(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float4x4>(context->argument(0)))
        return Line_Transform_float4x4(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Quat>(context->argument(0)))
        return Line_Transform_Quat(context, engine);
    printf("Line_Transform_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Line_Contains_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return Line_Contains_float3_float_const(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<Ray>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return Line_Contains_Ray_float_const(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<LineSegment>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return Line_Contains_LineSegment_float_const(context, engine);
    printf("Line_Contains_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Line_Distance_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<Ray>(context->argument(0)))
        return Line_Distance_Ray_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Line>(context->argument(0)))
        return Line_Distance_Line_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<LineSegment>(context->argument(0)))
        return Line_Distance_LineSegment_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Sphere>(context->argument(0)))
        return Line_Distance_Sphere_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Capsule>(context->argument(0)))
        return Line_Distance_Capsule_const(context, engine);
    printf("Line_Distance_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Line_Intersects_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<Capsule>(context->argument(0)))
        return Line_Intersects_Capsule_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Polygon>(context->argument(0)))
        return Line_Intersects_Polygon_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Frustum>(context->argument(0)))
        return Line_Intersects_Frustum_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Polyhedron>(context->argument(0)))
        return Line_Intersects_Polyhedron_const(context, engine);
    printf("Line_Intersects_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

void FromScriptValue_Line(const QScriptValue &obj, Line &value)
{
    value.pos = qScriptValueToValue<float3>(obj.property("pos"));
    value.dir = qScriptValueToValue<float3>(obj.property("dir"));
}

QScriptValue ToScriptValue_Line(QScriptEngine *engine, const Line &value)
{
    QScriptValue obj = engine->newVariant(QVariant::fromValue(value)); // The contents of this variant are NOT used. The real data lies in the data() pointer of this QScriptValue. This only exists to enable overload resolution to work for QObject slots.
    ToExistingScriptValue_Line(engine, value, obj);
    return obj;
}

QScriptValue ToScriptValue_const_Line(QScriptEngine *engine, const Line &value)
{
    QScriptValue obj = engine->newVariant(QVariant::fromValue(value)); // The contents of this variant are NOT used. The real data lies in the data() pointer of this QScriptValue. This only exists to enable overload resolution to work for QObject slots.
    obj.setPrototype(engine->defaultPrototype(qMetaTypeId<Line>()));
    obj.setProperty("pos", ToScriptValue_const_float3(engine, value.pos), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("dir", ToScriptValue_const_float3(engine, value.dir), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    return obj;
}

QScriptValue register_Line_prototype(QScriptEngine *engine)
{
    QScriptValue proto = engine->newObject();
    proto.setProperty("GetPoint", engine->newFunction(Line_GetPoint_float_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Transform", engine->newFunction(Line_Transform_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Contains", engine->newFunction(Line_Contains_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Equals", engine->newFunction(Line_Equals_Line_float_const, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Distance", engine->newFunction(Line_Distance_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Intersects", engine->newFunction(Line_Intersects_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IntersectsDisc", engine->newFunction(Line_IntersectsDisc_Circle_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToRay", engine->newFunction(Line_ToRay_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToLineSegment", engine->newFunction(Line_ToLineSegment_float_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("toString", engine->newFunction(Line_toString_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("metaTypeId", engine->toScriptValue<qint32>((qint32)qMetaTypeId<Line>()));
    engine->setDefaultPrototype(qMetaTypeId<Line>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<Line*>(), proto);
    qScriptRegisterMetaType(engine, ToScriptValue_Line, FromScriptValue_Line, proto);

    QScriptValue ctor = engine->newFunction(Line_ctor, proto, 2);
    ctor.setProperty("AreCollinear", engine->newFunction(Line_AreCollinear_float3_float3_float3_float, 4), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    engine->globalObject().setProperty("Line", ctor, QScriptValue::Undeletable | QScriptValue::ReadOnly);

    return ctor;
}

