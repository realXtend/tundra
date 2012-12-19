#include "QtScriptBindingsHelpers.h"

void ToExistingScriptValue_LCG(QScriptEngine * /*engine*/, const LCG & /*value*/, QScriptValue obj)
{
}

static QScriptValue LCG_LCG(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function LCG_LCG in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LCG ret;
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue LCG_LCG_u32_u32_u32_u32(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function LCG_LCG_u32_u32_u32_u32 in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    u32 seed = qscriptvalue_cast<u32>(context->argument(0));
    u32 multiplier = qscriptvalue_cast<u32>(context->argument(1));
    u32 increment = qscriptvalue_cast<u32>(context->argument(2));
    u32 modulus = qscriptvalue_cast<u32>(context->argument(3));
    LCG ret(seed, multiplier, increment, modulus);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue LCG_Seed_u32_u32_u32_u32(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function LCG_Seed_u32_u32_u32_u32 in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LCG This = qscriptvalue_cast<LCG>(context->thisObject());
    u32 seed = qscriptvalue_cast<u32>(context->argument(0));
    u32 multiplier = qscriptvalue_cast<u32>(context->argument(1));
    u32 increment = qscriptvalue_cast<u32>(context->argument(2));
    u32 modulus = qscriptvalue_cast<u32>(context->argument(3));
    This.Seed(seed, multiplier, increment, modulus);
    ToExistingScriptValue_LCG(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue LCG_Int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function LCG_Int in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LCG This = qscriptvalue_cast<LCG>(context->thisObject());
    u32 ret = This.Int();
    ToExistingScriptValue_LCG(engine, This, context->thisObject());
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue LCG_MaxInt_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function LCG_MaxInt_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LCG This = qscriptvalue_cast<LCG>(context->thisObject());
    u32 ret = This.MaxInt();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue LCG_IntFast(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function LCG_IntFast in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LCG This = qscriptvalue_cast<LCG>(context->thisObject());
    u32 ret = This.IntFast();
    ToExistingScriptValue_LCG(engine, This, context->thisObject());
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue LCG_Int_int_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function LCG_Int_int_int in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LCG This = qscriptvalue_cast<LCG>(context->thisObject());
    int a = qscriptvalue_cast<int>(context->argument(0));
    int b = qscriptvalue_cast<int>(context->argument(1));
    int ret = This.Int(a, b);
    ToExistingScriptValue_LCG(engine, This, context->thisObject());
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue LCG_Float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function LCG_Float in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LCG This = qscriptvalue_cast<LCG>(context->thisObject());
    float ret = This.Float();
    ToExistingScriptValue_LCG(engine, This, context->thisObject());
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue LCG_Float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function LCG_Float_float_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LCG This = qscriptvalue_cast<LCG>(context->thisObject());
    float a = qscriptvalue_cast<float>(context->argument(0));
    float b = qscriptvalue_cast<float>(context->argument(1));
    float ret = This.Float(a, b);
    ToExistingScriptValue_LCG(engine, This, context->thisObject());
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue LCG_ctor(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return LCG_LCG(context, engine);
    if (context->argumentCount() == 4 && QSVIsOfType<u32>(context->argument(0)) && QSVIsOfType<u32>(context->argument(1)) && QSVIsOfType<u32>(context->argument(2)) && QSVIsOfType<u32>(context->argument(3)))
        return LCG_LCG_u32_u32_u32_u32(context, engine);
    printf("LCG_ctor failed to choose the right function to call! Did you use 'var x = LCG();' instead of 'var x = new LCG();'?\n"); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue LCG_Int_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return LCG_Int(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<int>(context->argument(0)) && QSVIsOfType<int>(context->argument(1)))
        return LCG_Int_int_int(context, engine);
    printf("LCG_Int_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue LCG_Float_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return LCG_Float(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return LCG_Float_float_float(context, engine);
    printf("LCG_Float_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

void FromScriptValue_LCG(const QScriptValue & /*obj*/, LCG & /*value*/)
{
}

QScriptValue ToScriptValue_LCG(QScriptEngine *engine, const LCG &value)
{
    QScriptValue obj = engine->newVariant(QVariant::fromValue(value)); // The contents of this variant are NOT used. The real data lies in the data() pointer of this QScriptValue. This only exists to enable overload resolution to work for QObject slots.
    ToExistingScriptValue_LCG(engine, value, obj);
    return obj;
}

QScriptValue ToScriptValue_const_LCG(QScriptEngine *engine, const LCG &value)
{
    QScriptValue obj = engine->newVariant(QVariant::fromValue(value)); // The contents of this variant are NOT used. The real data lies in the data() pointer of this QScriptValue. This only exists to enable overload resolution to work for QObject slots.
    obj.setPrototype(engine->defaultPrototype(qMetaTypeId<LCG>()));
    return obj;
}

QScriptValue register_LCG_prototype(QScriptEngine *engine)
{
    QScriptValue proto = engine->newObject();
    proto.setProperty("Seed", engine->newFunction(LCG_Seed_u32_u32_u32_u32, 4), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Int", engine->newFunction(LCG_Int_selector, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MaxInt", engine->newFunction(LCG_MaxInt_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IntFast", engine->newFunction(LCG_IntFast, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Int", engine->newFunction(LCG_Int_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Float", engine->newFunction(LCG_Float_selector, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Float", engine->newFunction(LCG_Float_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("metaTypeId", engine->toScriptValue<qint32>((qint32)qMetaTypeId<LCG>()));
    engine->setDefaultPrototype(qMetaTypeId<LCG>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<LCG*>(), proto);
    qScriptRegisterMetaType(engine, ToScriptValue_LCG, FromScriptValue_LCG, proto);

    QScriptValue ctor = engine->newFunction(LCG_ctor, proto, 4);
    engine->globalObject().setProperty("LCG", ctor, QScriptValue::Undeletable | QScriptValue::ReadOnly);

    return ctor;
}

