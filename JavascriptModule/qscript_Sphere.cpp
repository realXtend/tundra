#include "QtScriptBindingsHelpers.h"

void ToExistingScriptValue_Sphere(QScriptEngine *engine, const Sphere &value, QScriptValue obj)
{
    obj.setProperty("pos", qScriptValueFromValue(engine, value.pos), QScriptValue::Undeletable);
    obj.setProperty("r", qScriptValueFromValue(engine, value.r), QScriptValue::Undeletable);
}

static QScriptValue Sphere_Sphere(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Sphere_Sphere in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere ret;
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Sphere_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Sphere_Sphere_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 center = qscriptvalue_cast<float3>(context->argument(0));
    float radius = qscriptvalue_cast<float>(context->argument(1));
    Sphere ret(center, radius);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Sphere_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Sphere_Sphere_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 pointA = qscriptvalue_cast<float3>(context->argument(0));
    float3 pointB = qscriptvalue_cast<float3>(context->argument(1));
    Sphere ret(pointA, pointB);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Sphere_float3_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Sphere_Sphere_float3_float3_float3 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 pointA = qscriptvalue_cast<float3>(context->argument(0));
    float3 pointB = qscriptvalue_cast<float3>(context->argument(1));
    float3 pointC = qscriptvalue_cast<float3>(context->argument(2));
    Sphere ret(pointA, pointB, pointC);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Sphere_float3_float3_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function Sphere_Sphere_float3_float3_float3_float3 in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 pointA = qscriptvalue_cast<float3>(context->argument(0));
    float3 pointB = qscriptvalue_cast<float3>(context->argument(1));
    float3 pointC = qscriptvalue_cast<float3>(context->argument(2));
    float3 pointD = qscriptvalue_cast<float3>(context->argument(3));
    Sphere ret(pointA, pointB, pointC, pointD);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_MinimalEnclosingAABB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Sphere_MinimalEnclosingAABB in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    AABB ret = This.MinimalEnclosingAABB();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_MaximalContainedAABB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Sphere_MaximalContainedAABB in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    AABB ret = This.MaximalContainedAABB();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_SetNegativeInfinity(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Sphere_SetNegativeInfinity in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    This.SetNegativeInfinity();
    ToExistingScriptValue_Sphere(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Sphere_Volume(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Sphere_Volume in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    float ret = This.Volume();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_SurfaceArea(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Sphere_SurfaceArea in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    float ret = This.SurfaceArea();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_IsFinite(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Sphere_IsFinite in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    bool ret = This.IsFinite();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_IsDegenerate(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Sphere_IsDegenerate in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    bool ret = This.IsDegenerate();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Contains_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Contains_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    bool ret = This.Contains(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Distance_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Distance_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float ret = This.Distance(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_ClosestPoint_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_ClosestPoint_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.ClosestPoint(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Intersects_Plane(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Intersects_Plane in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    Plane plane = qscriptvalue_cast<Plane>(context->argument(0));
    bool ret = This.Intersects(plane);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Intersects_Sphere(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Intersects_Sphere in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    Sphere sphere = qscriptvalue_cast<Sphere>(context->argument(0));
    bool ret = This.Intersects(sphere);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Sphere_Enclose_AABB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Enclose_AABB in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    AABB aabb = qscriptvalue_cast<AABB>(context->argument(0));
    This.Enclose(aabb);
    ToExistingScriptValue_Sphere(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Sphere_Enclose_OBB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Enclose_OBB in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    OBB obb = qscriptvalue_cast<OBB>(context->argument(0));
    This.Enclose(obb);
    ToExistingScriptValue_Sphere(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Sphere_Enclose_Sphere(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Enclose_Sphere in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    Sphere sphere = qscriptvalue_cast<Sphere>(context->argument(0));
    This.Enclose(sphere);
    ToExistingScriptValue_Sphere(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Sphere_Enclose_LineSegment(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Enclose_LineSegment in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    LineSegment lineSegment = qscriptvalue_cast<LineSegment>(context->argument(0));
    This.Enclose(lineSegment);
    ToExistingScriptValue_Sphere(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Sphere_Enclose_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Enclose_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Sphere This = qscriptvalue_cast<Sphere>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    This.Enclose(point);
    ToExistingScriptValue_Sphere(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Sphere_ctor(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return Sphere_Sphere(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return Sphere_Sphere_float3_float(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return Sphere_Sphere_float3_float3(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)))
        return Sphere_Sphere_float3_float3_float3(context, engine);
    if (context->argumentCount() == 4 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)) && QSVIsOfType<float3>(context->argument(3)))
        return Sphere_Sphere_float3_float3_float3_float3(context, engine);
    printf("Sphere_ctor failed to choose the right function to call! Did you use 'var x = Sphere();' instead of 'var x = new Sphere();'?\n"); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Sphere_Intersects_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<Plane>(context->argument(0)))
        return Sphere_Intersects_Plane(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Sphere>(context->argument(0)))
        return Sphere_Intersects_Sphere(context, engine);
    printf("Sphere_Intersects_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Sphere_Enclose_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<AABB>(context->argument(0)))
        return Sphere_Enclose_AABB(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<OBB>(context->argument(0)))
        return Sphere_Enclose_OBB(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Sphere>(context->argument(0)))
        return Sphere_Enclose_Sphere(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<LineSegment>(context->argument(0)))
        return Sphere_Enclose_LineSegment(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return Sphere_Enclose_float3(context, engine);
    printf("Sphere_Enclose_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

void FromScriptValue_Sphere(const QScriptValue &obj, Sphere &value)
{
    value.pos = qScriptValueToValue<float3>(obj.property("pos"));
    value.r = qScriptValueToValue<float>(obj.property("r"));
}

QScriptValue ToScriptValue_Sphere(QScriptEngine *engine, const Sphere &value)
{
    QScriptValue obj = engine->newVariant(QVariant::fromValue(value)); // The contents of this variant are NOT used. The real data lies in the data() pointer of this QScriptValue. This only exists to enable overload resolution to work for QObject slots.
    ToExistingScriptValue_Sphere(engine, value, obj);
    return obj;
}

QScriptValue ToScriptValue_const_Sphere(QScriptEngine *engine, const Sphere &value)
{
    QScriptValue obj = engine->newVariant(QVariant::fromValue(value)); // The contents of this variant are NOT used. The real data lies in the data() pointer of this QScriptValue. This only exists to enable overload resolution to work for QObject slots.
    obj.setPrototype(engine->defaultPrototype(qMetaTypeId<Sphere>()));
    obj.setProperty("pos", ToScriptValue_const_float3(engine, value.pos), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("r", qScriptValueFromValue(engine, value.r), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    return obj;
}

QScriptValue register_Sphere_prototype(QScriptEngine *engine)
{
    QScriptValue proto = engine->newObject();
    proto.setProperty("MinimalEnclosingAABB", engine->newFunction(Sphere_MinimalEnclosingAABB, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MaximalContainedAABB", engine->newFunction(Sphere_MaximalContainedAABB, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetNegativeInfinity", engine->newFunction(Sphere_SetNegativeInfinity, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Volume", engine->newFunction(Sphere_Volume, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SurfaceArea", engine->newFunction(Sphere_SurfaceArea, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsFinite", engine->newFunction(Sphere_IsFinite, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsDegenerate", engine->newFunction(Sphere_IsDegenerate, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Contains", engine->newFunction(Sphere_Contains_float3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Distance", engine->newFunction(Sphere_Distance_float3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ClosestPoint", engine->newFunction(Sphere_ClosestPoint_float3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Intersects", engine->newFunction(Sphere_Intersects_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Enclose", engine->newFunction(Sphere_Enclose_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("metaTypeId", engine->toScriptValue<qint32>((qint32)qMetaTypeId<Sphere>()));
    engine->setDefaultPrototype(qMetaTypeId<Sphere>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<Sphere*>(), proto);
    qScriptRegisterMetaType(engine, ToScriptValue_Sphere, FromScriptValue_Sphere, proto);

    QScriptValue ctor = engine->newFunction(Sphere_ctor, proto, 4);
    engine->globalObject().setProperty("Sphere", ctor, QScriptValue::Undeletable | QScriptValue::ReadOnly);

    return ctor;
}

