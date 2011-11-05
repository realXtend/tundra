#include "QtScriptBindingsHelpers.h"

void ToExistingScriptValue_TranslateOp(QScriptEngine *engine, const TranslateOp &value, QScriptValue obj)
{
    obj.setProperty("x", qScriptValueFromValue(engine, value.x), QScriptValue::Undeletable);
    obj.setProperty("y", qScriptValueFromValue(engine, value.y), QScriptValue::Undeletable);
    obj.setProperty("z", qScriptValueFromValue(engine, value.z), QScriptValue::Undeletable);
}

static QScriptValue TranslateOp_TranslateOp(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function TranslateOp_TranslateOp in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    TranslateOp ret;
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue TranslateOp_TranslateOp_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function TranslateOp_TranslateOp_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 offset = qscriptvalue_cast<float3>(context->argument(0));
    TranslateOp ret(offset);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue TranslateOp_TranslateOp_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function TranslateOp_TranslateOp_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float x = qscriptvalue_cast<float>(context->argument(0));
    float y = qscriptvalue_cast<float>(context->argument(1));
    float z = qscriptvalue_cast<float>(context->argument(2));
    TranslateOp ret(x, y, z);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue TranslateOp_Offset_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function TranslateOp_Offset_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    TranslateOp This = qscriptvalue_cast<TranslateOp>(context->thisObject());
    float3 ret = This.Offset();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue TranslateOp_ToFloat3x4_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function TranslateOp_ToFloat3x4_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    TranslateOp This = qscriptvalue_cast<TranslateOp>(context->thisObject());
    float3x4 ret = This.ToFloat3x4();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue TranslateOp_ToFloat4x4_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function TranslateOp_ToFloat4x4_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    TranslateOp This = qscriptvalue_cast<TranslateOp>(context->thisObject());
    float4x4 ret = This.ToFloat4x4();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue TranslateOp_ctor(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return TranslateOp_TranslateOp(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return TranslateOp_TranslateOp_float3(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return TranslateOp_TranslateOp_float_float_float(context, engine);
    printf("TranslateOp_ctor failed to choose the right function to call! Did you use 'var x = TranslateOp();' instead of 'var x = new TranslateOp();'?\n"); PrintCallStack(context->backtrace()); return QScriptValue();
}

void FromScriptValue_TranslateOp(const QScriptValue &obj, TranslateOp &value)
{
    value.x = qScriptValueToValue<float>(obj.property("x"));
    value.y = qScriptValueToValue<float>(obj.property("y"));
    value.z = qScriptValueToValue<float>(obj.property("z"));
}

QScriptValue ToScriptValue_TranslateOp(QScriptEngine *engine, const TranslateOp &value)
{
    QScriptValue obj = engine->newVariant(QVariant::fromValue(value)); // The contents of this variant are NOT used. The real data lies in the data() pointer of this QScriptValue. This only exists to enable overload resolution to work for QObject slots.
    ToExistingScriptValue_TranslateOp(engine, value, obj);
    return obj;
}

QScriptValue ToScriptValue_const_TranslateOp(QScriptEngine *engine, const TranslateOp &value)
{
    QScriptValue obj = engine->newVariant(QVariant::fromValue(value)); // The contents of this variant are NOT used. The real data lies in the data() pointer of this QScriptValue. This only exists to enable overload resolution to work for QObject slots.
    obj.setPrototype(engine->defaultPrototype(qMetaTypeId<TranslateOp>()));
    obj.setProperty("x", qScriptValueFromValue(engine, value.x), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("y", qScriptValueFromValue(engine, value.y), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("z", qScriptValueFromValue(engine, value.z), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    return obj;
}

QScriptValue register_TranslateOp_prototype(QScriptEngine *engine)
{
    QScriptValue proto = engine->newObject();
    proto.setProperty("Offset", engine->newFunction(TranslateOp_Offset_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToFloat3x4", engine->newFunction(TranslateOp_ToFloat3x4_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToFloat4x4", engine->newFunction(TranslateOp_ToFloat4x4_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("metaTypeId", engine->toScriptValue<qint32>((qint32)qMetaTypeId<TranslateOp>()));
    engine->setDefaultPrototype(qMetaTypeId<TranslateOp>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<TranslateOp*>(), proto);
    qScriptRegisterMetaType(engine, ToScriptValue_TranslateOp, FromScriptValue_TranslateOp, proto);

    QScriptValue ctor = engine->newFunction(TranslateOp_ctor, proto, 3);
    engine->globalObject().setProperty("TranslateOp", ctor, QScriptValue::Undeletable | QScriptValue::ReadOnly);

    return ctor;
}

