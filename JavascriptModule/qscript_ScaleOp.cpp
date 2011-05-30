#include "QtScriptBindingsHelpers.h"

static QScriptValue ScaleOp_ScaleOp(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function ScaleOp_ScaleOp in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    ScaleOp ret;
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue ScaleOp_ScaleOp_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function ScaleOp_ScaleOp_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 scale = TypeFromQScriptValue<float3>(context->argument(0));
    ScaleOp ret(scale);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue ScaleOp_ScaleOp_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function ScaleOp_ScaleOp_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float sx = TypeFromQScriptValue<float>(context->argument(0));
    float sy = TypeFromQScriptValue<float>(context->argument(1));
    float sz = TypeFromQScriptValue<float>(context->argument(2));
    ScaleOp ret(sx, sy, sz);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue ScaleOp_Offset(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function ScaleOp_Offset in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    ScaleOp *This = TypeFromQScriptValue<ScaleOp*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function ScaleOp_Offset in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->Offset();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue ScaleOp_x_get(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function ScaleOp_x_get in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    ScaleOp *This = TypeFromQScriptValue<ScaleOp*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    return qScriptValueFromValue(context->engine(), This->x);
}

static QScriptValue ScaleOp_x_set(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function ScaleOp_x_set in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    ScaleOp *This = TypeFromQScriptValue<ScaleOp*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float x = qscriptvalue_cast<float>(context->argument(0));
    This->x = x;
    return QScriptValue();
}

static QScriptValue ScaleOp_y_get(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function ScaleOp_y_get in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    ScaleOp *This = TypeFromQScriptValue<ScaleOp*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    return qScriptValueFromValue(context->engine(), This->y);
}

static QScriptValue ScaleOp_y_set(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function ScaleOp_y_set in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    ScaleOp *This = TypeFromQScriptValue<ScaleOp*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float y = qscriptvalue_cast<float>(context->argument(0));
    This->y = y;
    return QScriptValue();
}

static QScriptValue ScaleOp_z_get(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function ScaleOp_z_get in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    ScaleOp *This = TypeFromQScriptValue<ScaleOp*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    return qScriptValueFromValue(context->engine(), This->z);
}

static QScriptValue ScaleOp_z_set(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function ScaleOp_z_set in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    ScaleOp *This = TypeFromQScriptValue<ScaleOp*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float z = qscriptvalue_cast<float>(context->argument(0));
    This->z = z;
    return QScriptValue();
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

class ScaleOp_scriptclass : public QScriptClass
{
public:
    QScriptValue objectPrototype;
    ScaleOp_scriptclass(QScriptEngine *engine):QScriptClass(engine){}
    QScriptValue property(const QScriptValue &object, const QScriptString &name, uint id)
    {
        ScaleOp *This = TypeFromQScriptValue<ScaleOp*>(object);
        if (!This) { printf("Error! Cannot convert QScriptValue to type ScaleOp in file %s, line %d!\nTry using ScaleOp.get%s() and ScaleOp.set%s() to query the member variable '%s'!\n", __FILE__, __LINE__, Capitalize((QString)name).c_str(), Capitalize((QString)name).c_str(), ((QString)name).toStdString().c_str()); return QScriptValue(); }
        QString name_ = (QString)name;
        if (name_ == "x_") return TypeToQScriptValue(engine(), This->x);
        if (name_ == "y_") return TypeToQScriptValue(engine(), This->y);
        if (name_ == "z_") return TypeToQScriptValue(engine(), This->z);
        return QScriptValue();
    }
    void setProperty(QScriptValue &object, const QScriptString &name, uint id, const QScriptValue &value)
    {
        ScaleOp *This = TypeFromQScriptValue<ScaleOp*>(object);
        if (!This) { printf("Error! Cannot convert QScriptValue to type ScaleOp in file %s, line %d!\nTry using ScaleOp.get%s() and ScaleOp.set%s() to query the member variable '%s'!\n", __FILE__, __LINE__, Capitalize((QString)name).c_str(), Capitalize((QString)name).c_str(), ((QString)name).toStdString().c_str()); return; }
        QString name_ = (QString)name;
        if (name_ == "x_") This->x = TypeFromQScriptValue<float>(value);
        if (name_ == "y_") This->y = TypeFromQScriptValue<float>(value);
        if (name_ == "z_") This->z = TypeFromQScriptValue<float>(value);
    }
    QueryFlags queryProperty(const QScriptValue &object, const QScriptString &name, QueryFlags flags, uint *id)
    {
        QString name_ = (QString)name;
        if (name_ == "x_") return flags;
        if (name_ == "y_") return flags;
        if (name_ == "z_") return flags;
        return 0;
    }
    QScriptValue prototype() const { return objectPrototype; }
};
QScriptValue register_ScaleOp_prototype(QScriptEngine *engine)
{
    engine->setDefaultPrototype(qMetaTypeId<ScaleOp*>(), QScriptValue());
    QScriptValue proto = engine->newVariant(qVariantFromValue((ScaleOp*)0));
    proto.setProperty("Offset", engine->newFunction(ScaleOp_Offset, 0));
    proto.setProperty("x", engine->newFunction(ScaleOp_x_get, 1));
    proto.setProperty("setX", engine->newFunction(ScaleOp_x_set, 1));
    proto.setProperty("y", engine->newFunction(ScaleOp_y_get, 1));
    proto.setProperty("setY", engine->newFunction(ScaleOp_y_set, 1));
    proto.setProperty("z", engine->newFunction(ScaleOp_z_get, 1));
    proto.setProperty("setZ", engine->newFunction(ScaleOp_z_set, 1));
    ScaleOp_scriptclass *sc = new ScaleOp_scriptclass(engine);
    engine->setProperty("ScaleOp_scriptclass", QVariant::fromValue<QScriptClass*>(sc));
    proto.setScriptClass(sc);
    sc->objectPrototype = proto;
    engine->setDefaultPrototype(qMetaTypeId<ScaleOp>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<ScaleOp*>(), proto);
    QScriptValue ctor = engine->newFunction(ScaleOp_ctor, proto, 3);
    engine->globalObject().setProperty("ScaleOp", ctor);
    return ctor;
}

