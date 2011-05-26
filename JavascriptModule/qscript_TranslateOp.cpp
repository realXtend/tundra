#include "QtScriptBindingsHelpers.h"

static QScriptValue TranslateOp_TranslateOp(QScriptContext *context, QScriptEngine *engine)
{
    TranslateOp ret;
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue TranslateOp_TranslateOp_float3(QScriptContext *context, QScriptEngine *engine)
{
    float3 offset = TypeFromQScriptValue<float3>(context->argument(0));
    TranslateOp ret(offset);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue TranslateOp_TranslateOp_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    float x = TypeFromQScriptValue<float>(context->argument(0));
    float y = TypeFromQScriptValue<float>(context->argument(1));
    float z = TypeFromQScriptValue<float>(context->argument(2));
    TranslateOp ret(x, y, z);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue TranslateOp_ToFloat3x4(QScriptContext *context, QScriptEngine *engine)
{
    TranslateOp *This = TypeFromQScriptValue<TranslateOp*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3x4 ret = This->ToFloat3x4();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue TranslateOp_ToFloat4x4(QScriptContext *context, QScriptEngine *engine)
{
    TranslateOp *This = TypeFromQScriptValue<TranslateOp*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4x4 ret = This->ToFloat4x4();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue TranslateOp_Offset(QScriptContext *context, QScriptEngine *engine)
{
    TranslateOp *This = TypeFromQScriptValue<TranslateOp*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->Offset();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue TranslateOp_ctor(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return TranslateOp_TranslateOp(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return TranslateOp_TranslateOp_float3(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return TranslateOp_TranslateOp_float_float_float(context, engine);
    printf("TranslateOp_ctor failed to choose the right function to call! Did you use 'var x = TranslateOp();' instead of 'var x = new TranslateOp();'?\n"); return QScriptValue();
}

class TranslateOp_scriptclass : public QScriptClass
{
public:
    QScriptValue objectPrototype;
    TranslateOp_scriptclass(QScriptEngine *engine):QScriptClass(engine){}
    QScriptValue property(const QScriptValue &object, const QScriptString &name, uint id)
    {
        TranslateOp *This = TypeFromQScriptValue<TranslateOp*>(object);
        if ((QString)name == (QString)"x") return TypeToQScriptValue(engine(), This->x);
        if ((QString)name == (QString)"y") return TypeToQScriptValue(engine(), This->y);
        if ((QString)name == (QString)"z") return TypeToQScriptValue(engine(), This->z);
        return QScriptValue();
    }
    void setProperty(QScriptValue &object, const QScriptString &name, uint id, const QScriptValue &value)
    {
        TranslateOp *This = TypeFromQScriptValue<TranslateOp*>(object);
        if ((QString)name == (QString)"x") This->x = TypeFromQScriptValue<float>(value);
        if ((QString)name == (QString)"y") This->y = TypeFromQScriptValue<float>(value);
        if ((QString)name == (QString)"z") This->z = TypeFromQScriptValue<float>(value);
    }
    QueryFlags queryProperty(const QScriptValue &object, const QScriptString &name, QueryFlags flags, uint *id)
    {
        if ((QString)name == (QString)"x") return flags;
        if ((QString)name == (QString)"y") return flags;
        if ((QString)name == (QString)"z") return flags;
        return 0;
    }
    QScriptValue prototype() const { return objectPrototype; }
};
QScriptValue register_TranslateOp_prototype(QScriptEngine *engine)
{
    engine->setDefaultPrototype(qMetaTypeId<TranslateOp*>(), QScriptValue());
    QScriptValue proto = engine->newVariant(qVariantFromValue((TranslateOp*)0));
    proto.setProperty("ToFloat3x4", engine->newFunction(TranslateOp_ToFloat3x4, 0));
    proto.setProperty("ToFloat4x4", engine->newFunction(TranslateOp_ToFloat4x4, 0));
    proto.setProperty("Offset", engine->newFunction(TranslateOp_Offset, 0));
    TranslateOp_scriptclass *sc = new TranslateOp_scriptclass(engine);
    engine->setProperty("TranslateOp_scriptclass", QVariant::fromValue<QScriptClass*>(sc));
    proto.setScriptClass(sc);
    sc->objectPrototype = proto;
    engine->setDefaultPrototype(qMetaTypeId<TranslateOp>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<TranslateOp*>(), proto);
    QScriptValue ctor = engine->newFunction(TranslateOp_ctor, proto, 3);
    engine->globalObject().setProperty("TranslateOp", ctor);
    return ctor;
}

