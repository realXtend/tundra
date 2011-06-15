#include "QtScriptBindingsHelpers.h"

void ToExistingScriptValue_Ray(QScriptEngine *engine, const Ray &value, QScriptValue obj)
{
    obj.setProperty("pos", qScriptValueFromValue(engine, value.pos), QScriptValue::Undeletable);
    obj.setProperty("dir", qScriptValueFromValue(engine, value.dir), QScriptValue::Undeletable);
}

static QScriptValue Ray_Ray(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Ray_Ray in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Ray ret;
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Ray_Ray_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Ray_Ray_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 pos = qscriptvalue_cast<float3>(context->argument(0));
    float3 dir = qscriptvalue_cast<float3>(context->argument(1));
    Ray ret(pos, dir);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Ray_Ray_Line(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Ray_Ray_Line in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Line line = qscriptvalue_cast<Line>(context->argument(0));
    Ray ret(line);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Ray_Ray_LineSegment(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Ray_Ray_LineSegment in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LineSegment lineSegment = qscriptvalue_cast<LineSegment>(context->argument(0));
    Ray ret(lineSegment);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Ray_GetPoint_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Ray_GetPoint_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Ray This = qscriptvalue_cast<Ray>(context->thisObject());
    float distance = qscriptvalue_cast<float>(context->argument(0));
    float3 ret = This.GetPoint(distance);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Ray_Transform_float3x3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Ray_Transform_float3x3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Ray This = qscriptvalue_cast<Ray>(context->thisObject());
    float3x3 transform = qscriptvalue_cast<float3x3>(context->argument(0));
    This.Transform(transform);
    ToExistingScriptValue_Ray(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Ray_Transform_float3x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Ray_Transform_float3x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Ray This = qscriptvalue_cast<Ray>(context->thisObject());
    float3x4 transform = qscriptvalue_cast<float3x4>(context->argument(0));
    This.Transform(transform);
    ToExistingScriptValue_Ray(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Ray_Transform_float4x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Ray_Transform_float4x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Ray This = qscriptvalue_cast<Ray>(context->thisObject());
    float4x4 transform = qscriptvalue_cast<float4x4>(context->argument(0));
    This.Transform(transform);
    ToExistingScriptValue_Ray(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Ray_Transform_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Ray_Transform_Quat in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Ray This = qscriptvalue_cast<Ray>(context->thisObject());
    Quat transform = qscriptvalue_cast<Quat>(context->argument(0));
    This.Transform(transform);
    ToExistingScriptValue_Ray(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Ray_Contains_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Ray_Contains_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Ray This = qscriptvalue_cast<Ray>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float distanceThreshold = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.Contains(point, distanceThreshold);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Ray_ToLine(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Ray_ToLine in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Ray This = qscriptvalue_cast<Ray>(context->thisObject());
    Line ret = This.ToLine();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Ray_ToLineSegment_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Ray_ToLineSegment_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Ray This = qscriptvalue_cast<Ray>(context->thisObject());
    float d = qscriptvalue_cast<float>(context->argument(0));
    LineSegment ret = This.ToLineSegment(d);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Ray_ctor(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return Ray_Ray(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return Ray_Ray_float3_float3(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Line>(context->argument(0)))
        return Ray_Ray_Line(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<LineSegment>(context->argument(0)))
        return Ray_Ray_LineSegment(context, engine);
    printf("Ray_ctor failed to choose the right function to call! Did you use 'var x = Ray();' instead of 'var x = new Ray();'?\n"); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Ray_Transform_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3x3>(context->argument(0)))
        return Ray_Transform_float3x3(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3x4>(context->argument(0)))
        return Ray_Transform_float3x4(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float4x4>(context->argument(0)))
        return Ray_Transform_float4x4(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Quat>(context->argument(0)))
        return Ray_Transform_Quat(context, engine);
    printf("Ray_Transform_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

void FromScriptValue_Ray(const QScriptValue &obj, Ray &value)
{
    value.pos = qScriptValueToValue<float3>(obj.property("pos"));
    value.dir = qScriptValueToValue<float3>(obj.property("dir"));
}

QScriptValue ToScriptValue_Ray(QScriptEngine *engine, const Ray &value)
{
    QScriptValue obj = engine->newObject();
    ToExistingScriptValue_Ray(engine, value, obj);
    return obj;
}

QScriptValue ToScriptValue_const_Ray(QScriptEngine *engine, const Ray &value)
{
    QScriptValue obj = engine->newObject();
    obj.setPrototype(engine->defaultPrototype(qMetaTypeId<Ray>()));
    obj.setProperty("pos", ToScriptValue_const_float3(engine, value.pos), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("dir", ToScriptValue_const_float3(engine, value.dir), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    return obj;
}

QScriptValue register_Ray_prototype(QScriptEngine *engine)
{
    QScriptValue proto = engine->newObject();
    proto.setProperty("GetPoint", engine->newFunction(Ray_GetPoint_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Transform", engine->newFunction(Ray_Transform_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Contains", engine->newFunction(Ray_Contains_float3_float, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToLine", engine->newFunction(Ray_ToLine, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToLineSegment", engine->newFunction(Ray_ToLineSegment_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("metaTypeId", engine->toScriptValue<qint32>((qint32)qMetaTypeId<Ray>()));
    engine->setDefaultPrototype(qMetaTypeId<Ray>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<Ray*>(), proto);
    qScriptRegisterMetaType(engine, ToScriptValue_Ray, FromScriptValue_Ray, proto);

    QScriptValue ctor = engine->newFunction(Ray_ctor, proto, 2);
    engine->globalObject().setProperty("Ray", ctor, QScriptValue::Undeletable | QScriptValue::ReadOnly);

    return ctor;
}

