#include "QtScriptBindingsHelpers.h"

static QScriptValue LCG_LCG(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function LCG_LCG in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    LCG ret;
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue LCG_LCG_u32_u32_u32_u32(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function LCG_LCG_u32_u32_u32_u32 in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    u32 seed = TypeFromQScriptValue<u32>(context->argument(0));
    u32 multiplier = TypeFromQScriptValue<u32>(context->argument(1));
    u32 increment = TypeFromQScriptValue<u32>(context->argument(2));
    u32 modulus = TypeFromQScriptValue<u32>(context->argument(3));
    LCG ret(seed, multiplier, increment, modulus);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue LCG_Seed_u32_u32_u32_u32(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function LCG_Seed_u32_u32_u32_u32 in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    LCG *This = TypeFromQScriptValue<LCG*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function LCG_Seed_u32_u32_u32_u32 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    u32 seed = TypeFromQScriptValue<u32>(context->argument(0));
    u32 mul = TypeFromQScriptValue<u32>(context->argument(1));
    u32 inc = TypeFromQScriptValue<u32>(context->argument(2));
    u32 mod = TypeFromQScriptValue<u32>(context->argument(3));
    This->Seed(seed, mul, inc, mod);
    return QScriptValue();
}

static QScriptValue LCG_Int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function LCG_Int in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    LCG *This = TypeFromQScriptValue<LCG*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function LCG_Int in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    u32 ret = This->Int();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue LCG_MaxInt(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function LCG_MaxInt in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    LCG *This = TypeFromQScriptValue<LCG*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function LCG_MaxInt in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    u32 ret = This->MaxInt();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue LCG_IntFast(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function LCG_IntFast in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    LCG *This = TypeFromQScriptValue<LCG*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function LCG_IntFast in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    u32 ret = This->IntFast();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue LCG_Int_int_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function LCG_Int_int_int in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    LCG *This = TypeFromQScriptValue<LCG*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function LCG_Int_int_int in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int a = TypeFromQScriptValue<int>(context->argument(0));
    int b = TypeFromQScriptValue<int>(context->argument(1));
    int ret = This->Int(a, b);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue LCG_Float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function LCG_Float in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    LCG *This = TypeFromQScriptValue<LCG*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function LCG_Float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->Float();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue LCG_Float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function LCG_Float_float_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    LCG *This = TypeFromQScriptValue<LCG*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function LCG_Float_float_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float a = TypeFromQScriptValue<float>(context->argument(0));
    float b = TypeFromQScriptValue<float>(context->argument(1));
    float ret = This->Float(a, b);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue LCG_ctor(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return LCG_LCG(context, engine);
    if (context->argumentCount() == 4 && QSVIsOfType<u32>(context->argument(0)) && QSVIsOfType<u32>(context->argument(1)) && QSVIsOfType<u32>(context->argument(2)) && QSVIsOfType<u32>(context->argument(3)))
        return LCG_LCG_u32_u32_u32_u32(context, engine);
    printf("LCG_ctor failed to choose the right function to call! Did you use 'var x = LCG();' instead of 'var x = new LCG();'?\n"); return QScriptValue();
}

static QScriptValue LCG_Int_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return LCG_Int(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<int>(context->argument(0)) && QSVIsOfType<int>(context->argument(1)))
        return LCG_Int_int_int(context, engine);
    printf("LCG_Int_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue LCG_Float_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return LCG_Float(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return LCG_Float_float_float(context, engine);
    printf("LCG_Float_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

class LCG_scriptclass : public QScriptClass
{
public:
    QScriptValue objectPrototype;
    LCG_scriptclass(QScriptEngine *engine):QScriptClass(engine){}
    QScriptValue property(const QScriptValue &object, const QScriptString &name, uint id)
    {
        LCG *This = TypeFromQScriptValue<LCG*>(object);
        if (!This) { printf("Error! Cannot convert QScriptValue to type LCG in file %s, line %d!\nTry using LCG.get%s() and LCG.set%s() to query the member variable '%s'!\n", __FILE__, __LINE__, Capitalize((QString)name).c_str(), Capitalize((QString)name).c_str(), ((QString)name).toStdString().c_str()); return QScriptValue(); }
        return QScriptValue();
    }
    void setProperty(QScriptValue &object, const QScriptString &name, uint id, const QScriptValue &value)
    {
        LCG *This = TypeFromQScriptValue<LCG*>(object);
        if (!This) { printf("Error! Cannot convert QScriptValue to type LCG in file %s, line %d!\nTry using LCG.get%s() and LCG.set%s() to query the member variable '%s'!\n", __FILE__, __LINE__, Capitalize((QString)name).c_str(), Capitalize((QString)name).c_str(), ((QString)name).toStdString().c_str()); return; }
    }
    QueryFlags queryProperty(const QScriptValue &object, const QScriptString &name, QueryFlags flags, uint *id)
    {
        return 0;
    }
    QScriptValue prototype() const { return objectPrototype; }
};
QScriptValue register_LCG_prototype(QScriptEngine *engine)
{
    engine->setDefaultPrototype(qMetaTypeId<LCG*>(), QScriptValue());
    QScriptValue proto = engine->newVariant(qVariantFromValue((LCG*)0));
    proto.setProperty("Seed", engine->newFunction(LCG_Seed_u32_u32_u32_u32, 4));
    proto.setProperty("Int", engine->newFunction(LCG_Int_selector, 0));
    proto.setProperty("MaxInt", engine->newFunction(LCG_MaxInt, 0));
    proto.setProperty("IntFast", engine->newFunction(LCG_IntFast, 0));
    proto.setProperty("Int", engine->newFunction(LCG_Int_selector, 2));
    proto.setProperty("Float", engine->newFunction(LCG_Float_selector, 0));
    proto.setProperty("Float", engine->newFunction(LCG_Float_selector, 2));
    LCG_scriptclass *sc = new LCG_scriptclass(engine);
    engine->setProperty("LCG_scriptclass", QVariant::fromValue<QScriptClass*>(sc));
    proto.setScriptClass(sc);
    sc->objectPrototype = proto;
    engine->setDefaultPrototype(qMetaTypeId<LCG>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<LCG*>(), proto);
    QScriptValue ctor = engine->newFunction(LCG_ctor, proto, 4);
    engine->globalObject().setProperty("LCG", ctor);
    return ctor;
}

