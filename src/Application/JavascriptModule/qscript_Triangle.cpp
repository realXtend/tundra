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

static QScriptValue Triangle_Barycentric_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_Barycentric_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.Barycentric(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_Point_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Triangle_Point_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    float u = qscriptvalue_cast<float>(context->argument(0));
    float v = qscriptvalue_cast<float>(context->argument(1));
    float w = qscriptvalue_cast<float>(context->argument(2));
    float3 ret = This.Point(u, v, w);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_Point_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Triangle_Point_float_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    float u = qscriptvalue_cast<float>(context->argument(0));
    float v = qscriptvalue_cast<float>(context->argument(1));
    float3 ret = This.Point(u, v);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_Point_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_Point_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    float3 barycentric = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.Point(barycentric);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_Area(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Triangle_Area in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    float ret = This.Area();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_GetPlane(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Triangle_GetPlane in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    Plane ret = This.GetPlane();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_Normal(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Triangle_Normal in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    float3 ret = This.NormalCCW();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_UnnormalizedNormal(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Triangle_UnnormalizedNormal in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    float3 ret = This.UnnormalizedNormalCCW();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_IsDegenerate_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_IsDegenerate_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    float epsilon = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.IsDegenerate(epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_Contains_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Triangle_Contains_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float triangleThickness = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.Contains(point, triangleThickness);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_Distance_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_Distance_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    bool ret = This.Distance(point);
    ToExistingScriptValue_Triangle(engine, This, context->thisObject());
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_Intersects_Plane(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_Intersects_Plane in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    Plane plane = qscriptvalue_cast<Plane>(context->argument(0));
    bool ret = This.Intersects(plane);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_Intersects_Triangle_LineSegment_ptr(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Triangle_Intersects_Triangle_LineSegment_ptr in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    Triangle t2 = qscriptvalue_cast<Triangle>(context->argument(0));
    LineSegment * outLine = qscriptvalue_cast<LineSegment *>(context->argument(1));
    bool ret = This.Intersects(t2, outLine);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_Intersects_AABB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_Intersects_AABB in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    AABB aabb = qscriptvalue_cast<AABB>(context->argument(0));
    bool ret = This.Intersects(aabb);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_Intersects_OBB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_Intersects_OBB in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    OBB obb = qscriptvalue_cast<OBB>(context->argument(0));
    bool ret = This.Intersects(obb);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_ProjectToAxis_float3_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Triangle_ProjectToAxis_float3_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    float3 axis = qscriptvalue_cast<float3>(context->argument(0));
    float dMin = qscriptvalue_cast<float>(context->argument(1));
    float dMax = qscriptvalue_cast<float>(context->argument(2));
    This.ProjectToAxis(axis, dMin, dMax);
    return QScriptValue();
}

static QScriptValue Triangle_ClosestPoint_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_ClosestPoint_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Triangle This = qscriptvalue_cast<Triangle>(context->thisObject());
    float3 targetPoint = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.ClosestPoint(targetPoint);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Triangle_BarycentricInsideTriangle_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_BarycentricInsideTriangle_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 barycentric = qscriptvalue_cast<float3>(context->argument(0));
    bool ret = Triangle::BarycentricInsideTriangle(barycentric);
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
        return Triangle_Point_float_float_float(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return Triangle_Point_float_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return Triangle_Point_float3(context, engine);
    printf("Triangle_Point_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Triangle_IsDegenerate_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return Triangle_IsDegenerate_float(context, engine);
    if (context->argumentCount() == 4 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)) && QSVIsOfType<float>(context->argument(3)))
        return Triangle_IsDegenerate_float3_float3_float3_float(context, engine);
    printf("Triangle_IsDegenerate_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Triangle_Intersects_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<Plane>(context->argument(0)))
        return Triangle_Intersects_Plane(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<Triangle>(context->argument(0)) && QSVIsOfType<LineSegment *>(context->argument(1)))
        return Triangle_Intersects_Triangle_LineSegment_ptr(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<AABB>(context->argument(0)))
        return Triangle_Intersects_AABB(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<OBB>(context->argument(0)))
        return Triangle_Intersects_OBB(context, engine);
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
    proto.setProperty("Barycentric", engine->newFunction(Triangle_Barycentric_float3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Point", engine->newFunction(Triangle_Point_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Point", engine->newFunction(Triangle_Point_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Point", engine->newFunction(Triangle_Point_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Area", engine->newFunction(Triangle_Area, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("GetPlane", engine->newFunction(Triangle_GetPlane, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Normal", engine->newFunction(Triangle_Normal, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("UnnormalizedNormal", engine->newFunction(Triangle_UnnormalizedNormal, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsDegenerate", engine->newFunction(Triangle_IsDegenerate_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Contains", engine->newFunction(Triangle_Contains_float3_float, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Distance", engine->newFunction(Triangle_Distance_float3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Intersects", engine->newFunction(Triangle_Intersects_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Intersects", engine->newFunction(Triangle_Intersects_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ProjectToAxis", engine->newFunction(Triangle_ProjectToAxis_float3_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ClosestPoint", engine->newFunction(Triangle_ClosestPoint_float3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("metaTypeId", engine->toScriptValue<qint32>((qint32)qMetaTypeId<Triangle>()));
    engine->setDefaultPrototype(qMetaTypeId<Triangle>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<Triangle*>(), proto);
    qScriptRegisterMetaType(engine, ToScriptValue_Triangle, FromScriptValue_Triangle, proto);

    QScriptValue ctor = engine->newFunction(Triangle_ctor, proto, 3);
    ctor.setProperty("BarycentricInsideTriangle", engine->newFunction(Triangle_BarycentricInsideTriangle_float3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("Area2D", engine->newFunction(Triangle_Area2D_float2_float2_float2, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("SignedArea", engine->newFunction(Triangle_SignedArea_float3_float3_float3_float3, 4), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("IsDegenerate", engine->newFunction(Triangle_IsDegenerate_selector, 4), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    engine->globalObject().setProperty("Triangle", ctor, QScriptValue::Undeletable | QScriptValue::ReadOnly);

    return ctor;
}

