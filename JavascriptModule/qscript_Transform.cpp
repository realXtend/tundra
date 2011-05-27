#include "QtScriptBindingsHelpers.h"

static QScriptValue Transform_Transform(QScriptContext *context, QScriptEngine *engine)
{
    Transform ret;
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Transform_Transform_Vector3df_Vector3df_Vector3df(QScriptContext *context, QScriptEngine *engine)
{
    Vector3df pos_ = TypeFromQScriptValue<Vector3df>(context->argument(0));
    Vector3df rot_ = TypeFromQScriptValue<Vector3df>(context->argument(1));
    Vector3df scale = TypeFromQScriptValue<Vector3df>(context->argument(2));
    Transform ret(pos_, rot_, scale);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Transform_Transform_float3x3(QScriptContext *context, QScriptEngine *engine)
{
    float3x3 m = TypeFromQScriptValue<float3x3>(context->argument(0));
    Transform ret(m);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Transform_Transform_float3x4(QScriptContext *context, QScriptEngine *engine)
{
    float3x4 m = TypeFromQScriptValue<float3x4>(context->argument(0));
    Transform ret(m);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Transform_Transform_float4x4(QScriptContext *context, QScriptEngine *engine)
{
    float4x4 m = TypeFromQScriptValue<float4x4>(context->argument(0));
    Transform ret(m);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Transform_SetPos_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    Transform *This = TypeFromQScriptValue<Transform*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float x = TypeFromQScriptValue<float>(context->argument(0));
    float y = TypeFromQScriptValue<float>(context->argument(1));
    float z = TypeFromQScriptValue<float>(context->argument(2));
    This->SetPos(x, y, z);
    return QScriptValue();
}

static QScriptValue Transform_SetRot_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    Transform *This = TypeFromQScriptValue<Transform*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float x = TypeFromQScriptValue<float>(context->argument(0));
    float y = TypeFromQScriptValue<float>(context->argument(1));
    float z = TypeFromQScriptValue<float>(context->argument(2));
    This->SetRot(x, y, z);
    return QScriptValue();
}

static QScriptValue Transform_SetScale_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    Transform *This = TypeFromQScriptValue<Transform*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float x = TypeFromQScriptValue<float>(context->argument(0));
    float y = TypeFromQScriptValue<float>(context->argument(1));
    float z = TypeFromQScriptValue<float>(context->argument(2));
    This->SetScale(x, y, z);
    return QScriptValue();
}

static QScriptValue Transform_ToFloat3x4(QScriptContext *context, QScriptEngine *engine)
{
    Transform *This = TypeFromQScriptValue<Transform*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3x4 ret = This->ToFloat3x4();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Transform_ToFloat4x4(QScriptContext *context, QScriptEngine *engine)
{
    Transform *This = TypeFromQScriptValue<Transform*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4x4 ret = This->ToFloat4x4();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Transform_FromFloat3x4_float3x4(QScriptContext *context, QScriptEngine *engine)
{
    Transform *This = TypeFromQScriptValue<Transform*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3x4 m = TypeFromQScriptValue<float3x4>(context->argument(0));
    This->FromFloat3x4(m);
    return QScriptValue();
}

static QScriptValue Transform_FromFloat4x4_float4x4(QScriptContext *context, QScriptEngine *engine)
{
    Transform *This = TypeFromQScriptValue<Transform*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4x4 m = TypeFromQScriptValue<float4x4>(context->argument(0));
    This->FromFloat4x4(m);
    return QScriptValue();
}

static QScriptValue Transform_SetRotation_float3x3(QScriptContext *context, QScriptEngine *engine)
{
    Transform *This = TypeFromQScriptValue<Transform*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3x3 mat = TypeFromQScriptValue<float3x3>(context->argument(0));
    This->SetRotation(mat);
    return QScriptValue();
}

static QScriptValue Transform_SetRotation_Quat(QScriptContext *context, QScriptEngine *engine)
{
    Transform *This = TypeFromQScriptValue<Transform*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    Quat q = TypeFromQScriptValue<Quat>(context->argument(0));
    This->SetRotation(q);
    return QScriptValue();
}

static QScriptValue Transform_Rotation3x3(QScriptContext *context, QScriptEngine *engine)
{
    Transform *This = TypeFromQScriptValue<Transform*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3x3 ret = This->Rotation3x3();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Transform_RotationQuat(QScriptContext *context, QScriptEngine *engine)
{
    Transform *This = TypeFromQScriptValue<Transform*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    Quat ret = This->RotationQuat();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Transform_Mul_Transform(QScriptContext *context, QScriptEngine *engine)
{
    Transform *This = TypeFromQScriptValue<Transform*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    Transform rhs = TypeFromQScriptValue<Transform>(context->argument(0));
    Transform ret = This->Mul(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Transform_toString(QScriptContext *context, QScriptEngine *engine)
{
    Transform *This = TypeFromQScriptValue<Transform*>(context->thisObject());
    if (!This && context->argumentCount() > 0) This = TypeFromQScriptValue<Transform*>(context->argument(0)); // Qt oddity (bug?): Sometimes the built-in toString() function doesn't give us this from thisObject, but as the first argument.
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    QString ret = This->toString();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Transform_ctor(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return Transform_Transform(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<Vector3df>(context->argument(0)) && QSVIsOfType<Vector3df>(context->argument(1)) && QSVIsOfType<Vector3df>(context->argument(2)))
        return Transform_Transform_Vector3df_Vector3df_Vector3df(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3x3>(context->argument(0)))
        return Transform_Transform_float3x3(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3x4>(context->argument(0)))
        return Transform_Transform_float3x4(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float4x4>(context->argument(0)))
        return Transform_Transform_float4x4(context, engine);
    printf("Transform_ctor failed to choose the right function to call! Did you use 'var x = Transform();' instead of 'var x = new Transform();'?\n"); return QScriptValue();
}

static QScriptValue Transform_SetRotation_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3x3>(context->argument(0)))
        return Transform_SetRotation_float3x3(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Quat>(context->argument(0)))
        return Transform_SetRotation_Quat(context, engine);
    printf("Transform_SetRotation_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

class Transform_scriptclass : public QScriptClass
{
public:
    QScriptValue objectPrototype;
    Transform_scriptclass(QScriptEngine *engine):QScriptClass(engine){}
    QScriptValue property(const QScriptValue &object, const QScriptString &name, uint id)
    {
        Transform *This = TypeFromQScriptValue<Transform*>(object);
        if ((QString)name == (QString)"pos") return TypeToQScriptValue(engine(), This->pos);
        if ((QString)name == (QString)"rot") return TypeToQScriptValue(engine(), This->rot);
        if ((QString)name == (QString)"scale") return TypeToQScriptValue(engine(), This->scale);
        return QScriptValue();
    }
    void setProperty(QScriptValue &object, const QScriptString &name, uint id, const QScriptValue &value)
    {
        Transform *This = TypeFromQScriptValue<Transform*>(object);
        if ((QString)name == (QString)"pos") This->pos = TypeFromQScriptValue<float3>(value);
        if ((QString)name == (QString)"rot") This->rot = TypeFromQScriptValue<float3>(value);
        if ((QString)name == (QString)"scale") This->scale = TypeFromQScriptValue<float3>(value);
    }
    QueryFlags queryProperty(const QScriptValue &object, const QScriptString &name, QueryFlags flags, uint *id)
    {
        if ((QString)name == (QString)"pos") return flags;
        if ((QString)name == (QString)"rot") return flags;
        if ((QString)name == (QString)"scale") return flags;
        return 0;
    }
    QScriptValue prototype() const { return objectPrototype; }
};
QScriptValue register_Transform_prototype(QScriptEngine *engine)
{
    engine->setDefaultPrototype(qMetaTypeId<Transform*>(), QScriptValue());
    QScriptValue proto = engine->newVariant(qVariantFromValue((Transform*)0));
    proto.setProperty("SetPos", engine->newFunction(Transform_SetPos_float_float_float, 3));
    proto.setProperty("SetRot", engine->newFunction(Transform_SetRot_float_float_float, 3));
    proto.setProperty("SetScale", engine->newFunction(Transform_SetScale_float_float_float, 3));
    proto.setProperty("ToFloat3x4", engine->newFunction(Transform_ToFloat3x4, 0));
    proto.setProperty("ToFloat4x4", engine->newFunction(Transform_ToFloat4x4, 0));
    proto.setProperty("FromFloat3x4", engine->newFunction(Transform_FromFloat3x4_float3x4, 1));
    proto.setProperty("FromFloat4x4", engine->newFunction(Transform_FromFloat4x4_float4x4, 1));
    proto.setProperty("SetRotation", engine->newFunction(Transform_SetRotation_selector, 1));
    proto.setProperty("Rotation3x3", engine->newFunction(Transform_Rotation3x3, 0));
    proto.setProperty("RotationQuat", engine->newFunction(Transform_RotationQuat, 0));
    proto.setProperty("Mul", engine->newFunction(Transform_Mul_Transform, 1));
    proto.setProperty("toString", engine->newFunction(Transform_toString, 0));
    Transform_scriptclass *sc = new Transform_scriptclass(engine);
    engine->setProperty("Transform_scriptclass", QVariant::fromValue<QScriptClass*>(sc));
    proto.setScriptClass(sc);
    sc->objectPrototype = proto;
    engine->setDefaultPrototype(qMetaTypeId<Transform>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<Transform*>(), proto);
    QScriptValue ctor = engine->newFunction(Transform_ctor, proto, 3);
    engine->globalObject().setProperty("Transform", ctor);
    return ctor;
}

