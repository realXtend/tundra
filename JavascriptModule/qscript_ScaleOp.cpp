#include "QtScriptBindingsHelpers.h"

void ToExistingScriptValue_ScaleOp(QScriptEngine *engine, const ScaleOp &value, QScriptValue obj)
{
    obj.setProperty("x", qScriptValueFromValue(engine, value.x), QScriptValue::Undeletable);
    obj.setProperty("y", qScriptValueFromValue(engine, value.y), QScriptValue::Undeletable);
    obj.setProperty("z", qScriptValueFromValue(engine, value.z), QScriptValue::Undeletable);
}

static QScriptValue ScaleOp_ScaleOp(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function ScaleOp_ScaleOp in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    ScaleOp ret;
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue ScaleOp_ScaleOp_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function ScaleOp_ScaleOp_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 scale = qscriptvalue_cast<float3>(context->argument(0));
    ScaleOp ret(scale);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue ScaleOp_ScaleOp_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function ScaleOp_ScaleOp_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float sx = qscriptvalue_cast<float>(context->argument(0));
    float sy = qscriptvalue_cast<float>(context->argument(1));
    float sz = qscriptvalue_cast<float>(context->argument(2));
    ScaleOp ret(sx, sy, sz);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue ScaleOp_Offset(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function ScaleOp_Offset in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    ScaleOp This = qscriptvalue_cast<ScaleOp>(context->thisObject());
    float3 ret = This.Offset();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue ScaleOp_ToFloat3x3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function ScaleOp_ToFloat3x3 in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    ScaleOp This = qscriptvalue_cast<ScaleOp>(context->thisObject());
    float3x3 ret = This.ToFloat3x3();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue ScaleOp_ToFloat3x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function ScaleOp_ToFloat3x4 in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    ScaleOp This = qscriptvalue_cast<ScaleOp>(context->thisObject());
    float3x4 ret = This.ToFloat3x4();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue ScaleOp_ToFloat4x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function ScaleOp_ToFloat4x4 in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    ScaleOp This = qscriptvalue_cast<ScaleOp>(context->thisObject());
    float4x4 ret = This.ToFloat4x4();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue ScaleOp_ctor(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return ScaleOp_ScaleOp(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return ScaleOp_ScaleOp_float3(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return ScaleOp_ScaleOp_float_float_float(context, engine);
    printf("ScaleOp_ctor failed to choose the right function to call! Did you use 'var x = ScaleOp();' instead of 'var x = new ScaleOp();'?\n"); return QScriptValue();
}

void FromScriptValue_ScaleOp(const QScriptValue &obj, ScaleOp &value)
{
    value.x = qScriptValueToValue<float>(obj.property("x"));
    value.y = qScriptValueToValue<float>(obj.property("y"));
    value.z = qScriptValueToValue<float>(obj.property("z"));
}

QScriptValue ToScriptValue_ScaleOp(QScriptEngine *engine, const ScaleOp &value)
{
    QScriptValue obj = engine->newObject();
    ToExistingScriptValue_ScaleOp(engine, value, obj);
    return obj;
}

QScriptValue ToScriptValue_const_ScaleOp(QScriptEngine *engine, const ScaleOp &value)
{
    QScriptValue obj = engine->newObject();
    obj.setPrototype(engine->defaultPrototype(qMetaTypeId<ScaleOp>()));
    obj.setProperty("x", qScriptValueFromValue(engine, value.x), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("y", qScriptValueFromValue(engine, value.y), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("z", qScriptValueFromValue(engine, value.z), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    return obj;
}

QScriptValue register_ScaleOp_prototype(QScriptEngine *engine)
{
    QScriptValue proto = engine->newObject();
    proto.setProperty("Offset", engine->newFunction(ScaleOp_Offset, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToFloat3x3", engine->newFunction(ScaleOp_ToFloat3x3, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToFloat3x4", engine->newFunction(ScaleOp_ToFloat3x4, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToFloat4x4", engine->newFunction(ScaleOp_ToFloat4x4, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("metaTypeId", engine->toScriptValue<qint32>((qint32)qMetaTypeId<ScaleOp>()));
    engine->setDefaultPrototype(qMetaTypeId<ScaleOp>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<ScaleOp*>(), proto);
    qScriptRegisterMetaType(engine, ToScriptValue_ScaleOp, FromScriptValue_ScaleOp, proto);

    QScriptValue ctor = engine->newFunction(ScaleOp_ctor, proto, 3);
    engine->globalObject().setProperty("ScaleOp", ctor, QScriptValue::Undeletable | QScriptValue::ReadOnly);

    return ctor;
}

