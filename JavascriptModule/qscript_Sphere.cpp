#include "QtScriptBindingsHelpers.h"

static QScriptValue Sphere_Sphere(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Sphere_Sphere in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Sphere ret;
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Sphere_Sphere_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Sphere_Sphere_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 center = TypeFromQScriptValue<float3>(context->argument(0));
    float radius = TypeFromQScriptValue<float>(context->argument(1));
    Sphere ret(center, radius);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Sphere_Sphere_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Sphere_Sphere_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 pointA = TypeFromQScriptValue<float3>(context->argument(0));
    float3 pointB = TypeFromQScriptValue<float3>(context->argument(1));
    Sphere ret(pointA, pointB);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Sphere_MinimalEnclosingAABB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Sphere_MinimalEnclosingAABB in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Sphere *This = TypeFromQScriptValue<Sphere*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Sphere_MinimalEnclosingAABB in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    AABB ret = This->MinimalEnclosingAABB();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Sphere_MaximalContainedAABB(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Sphere_MaximalContainedAABB in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Sphere *This = TypeFromQScriptValue<Sphere*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Sphere_MaximalContainedAABB in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    AABB ret = This->MaximalContainedAABB();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Sphere_Volume(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Sphere_Volume in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Sphere *This = TypeFromQScriptValue<Sphere*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Sphere_Volume in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->Volume();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Sphere_SurfaceArea(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Sphere_SurfaceArea in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Sphere *This = TypeFromQScriptValue<Sphere*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Sphere_SurfaceArea in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->SurfaceArea();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Sphere_IsFinite(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Sphere_IsFinite in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Sphere *This = TypeFromQScriptValue<Sphere*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Sphere_IsFinite in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    bool ret = This->IsFinite();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Sphere_IsDegenerate(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Sphere_IsDegenerate in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Sphere *This = TypeFromQScriptValue<Sphere*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Sphere_IsDegenerate in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    bool ret = This->IsDegenerate();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Sphere_Contains_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Contains_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Sphere *This = TypeFromQScriptValue<Sphere*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Sphere_Contains_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 point = TypeFromQScriptValue<float3>(context->argument(0));
    bool ret = This->Contains(point);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Sphere_Distance_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_Distance_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Sphere *This = TypeFromQScriptValue<Sphere*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Sphere_Distance_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 point = TypeFromQScriptValue<float3>(context->argument(0));
    float ret = This->Distance(point);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Sphere_pos_get(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Sphere_pos_get in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Sphere *This = TypeFromQScriptValue<Sphere*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    return qScriptValueFromValue(context->engine(), This->pos);
}

static QScriptValue Sphere_pos_set(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_pos_set in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Sphere *This = TypeFromQScriptValue<Sphere*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 pos = qscriptvalue_cast<float3>(context->argument(0));
    This->pos = pos;
    return QScriptValue();
}

static QScriptValue Sphere_r_get(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Sphere_r_get in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Sphere *This = TypeFromQScriptValue<Sphere*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    return qScriptValueFromValue(context->engine(), This->r);
}

static QScriptValue Sphere_r_set(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Sphere_r_set in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Sphere *This = TypeFromQScriptValue<Sphere*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float r = qscriptvalue_cast<float>(context->argument(0));
    This->r = r;
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
    printf("Sphere_ctor failed to choose the right function to call! Did you use 'var x = Sphere();' instead of 'var x = new Sphere();'?\n"); return QScriptValue();
}

class Sphere_scriptclass : public QScriptClass
{
public:
    QScriptValue objectPrototype;
    Sphere_scriptclass(QScriptEngine *engine):QScriptClass(engine){}
    QScriptValue property(const QScriptValue &object, const QScriptString &name, uint id)
    {
        Sphere *This = TypeFromQScriptValue<Sphere*>(object);
        if (!This) { printf("Error! Cannot convert QScriptValue to type Sphere in file %s, line %d!\nTry using Sphere.get%s() and Sphere.set%s() to query the member variable '%s'!\n", __FILE__, __LINE__, Capitalize((QString)name).c_str(), Capitalize((QString)name).c_str(), ((QString)name).toStdString().c_str()); return QScriptValue(); }
        if ((QString)name == (QString)"pos") return TypeToQScriptValue(engine(), This->pos);
        if ((QString)name == (QString)"r") return TypeToQScriptValue(engine(), This->r);
        return QScriptValue();
    }
    void setProperty(QScriptValue &object, const QScriptString &name, uint id, const QScriptValue &value)
    {
        Sphere *This = TypeFromQScriptValue<Sphere*>(object);
        if (!This) { printf("Error! Cannot convert QScriptValue to type Sphere in file %s, line %d!\nTry using Sphere.get%s() and Sphere.set%s() to query the member variable '%s'!\n", __FILE__, __LINE__, Capitalize((QString)name).c_str(), Capitalize((QString)name).c_str(), ((QString)name).toStdString().c_str()); return; }
        if ((QString)name == (QString)"pos") This->pos = TypeFromQScriptValue<float3>(value);
        if ((QString)name == (QString)"r") This->r = TypeFromQScriptValue<float>(value);
    }
    QueryFlags queryProperty(const QScriptValue &object, const QScriptString &name, QueryFlags flags, uint *id)
    {
        if ((QString)name == (QString)"pos") return flags;
        if ((QString)name == (QString)"r") return flags;
        return 0;
    }
    QScriptValue prototype() const { return objectPrototype; }
};
QScriptValue register_Sphere_prototype(QScriptEngine *engine)
{
    engine->setDefaultPrototype(qMetaTypeId<Sphere*>(), QScriptValue());
    QScriptValue proto = engine->newVariant(qVariantFromValue((Sphere*)0));
    proto.setProperty("MinimalEnclosingAABB", engine->newFunction(Sphere_MinimalEnclosingAABB, 0));
    proto.setProperty("MaximalContainedAABB", engine->newFunction(Sphere_MaximalContainedAABB, 0));
    proto.setProperty("Volume", engine->newFunction(Sphere_Volume, 0));
    proto.setProperty("SurfaceArea", engine->newFunction(Sphere_SurfaceArea, 0));
    proto.setProperty("IsFinite", engine->newFunction(Sphere_IsFinite, 0));
    proto.setProperty("IsDegenerate", engine->newFunction(Sphere_IsDegenerate, 0));
    proto.setProperty("Contains", engine->newFunction(Sphere_Contains_float3, 1));
    proto.setProperty("Distance", engine->newFunction(Sphere_Distance_float3, 1));
    proto.setProperty("getPos", engine->newFunction(Sphere_pos_get, 1));
    proto.setProperty("setPos", engine->newFunction(Sphere_pos_set, 1));
    proto.setProperty("getR", engine->newFunction(Sphere_r_get, 1));
    proto.setProperty("setR", engine->newFunction(Sphere_r_set, 1));
    Sphere_scriptclass *sc = new Sphere_scriptclass(engine);
    engine->setProperty("Sphere_scriptclass", QVariant::fromValue<QScriptClass*>(sc));
    proto.setScriptClass(sc);
    sc->objectPrototype = proto;
    engine->setDefaultPrototype(qMetaTypeId<Sphere>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<Sphere*>(), proto);
    QScriptValue ctor = engine->newFunction(Sphere_ctor, proto, 2);
    engine->globalObject().setProperty("Sphere", ctor);
    return ctor;
}

