#include "QtScriptBindingsHelpers.h"

static QScriptValue Ray_Ray(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Ray_Ray in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Ray ret;
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Ray_Ray_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Ray_Ray_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 pos = TypeFromQScriptValue<float3>(context->argument(0));
    float3 dir = TypeFromQScriptValue<float3>(context->argument(1));
    Ray ret(pos, dir);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Ray_Ray_Line(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Ray_Ray_Line in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Line line = TypeFromQScriptValue<Line>(context->argument(0));
    Ray ret(line);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Ray_Ray_LineSegment(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Ray_Ray_LineSegment in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    LineSegment lineSegment = TypeFromQScriptValue<LineSegment>(context->argument(0));
    Ray ret(lineSegment);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Ray_GetPoint_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Ray_GetPoint_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Ray *This = TypeFromQScriptValue<Ray*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Ray_GetPoint_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float distance = TypeFromQScriptValue<float>(context->argument(0));
    float3 ret = This->GetPoint(distance);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Ray_Transform_float3x3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Ray_Transform_float3x3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Ray *This = TypeFromQScriptValue<Ray*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Ray_Transform_float3x3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3x3 transform = TypeFromQScriptValue<float3x3>(context->argument(0));
    This->Transform(transform);
    return QScriptValue();
}

static QScriptValue Ray_Transform_float3x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Ray_Transform_float3x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Ray *This = TypeFromQScriptValue<Ray*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Ray_Transform_float3x4 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3x4 transform = TypeFromQScriptValue<float3x4>(context->argument(0));
    This->Transform(transform);
    return QScriptValue();
}

static QScriptValue Ray_Transform_float4x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Ray_Transform_float4x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Ray *This = TypeFromQScriptValue<Ray*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Ray_Transform_float4x4 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4x4 transform = TypeFromQScriptValue<float4x4>(context->argument(0));
    This->Transform(transform);
    return QScriptValue();
}

static QScriptValue Ray_Transform_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Ray_Transform_Quat in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Ray *This = TypeFromQScriptValue<Ray*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Ray_Transform_Quat in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    Quat transform = TypeFromQScriptValue<Quat>(context->argument(0));
    This->Transform(transform);
    return QScriptValue();
}

static QScriptValue Ray_ToLine(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Ray_ToLine in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Ray *This = TypeFromQScriptValue<Ray*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Ray_ToLine in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    Line ret = This->ToLine();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Ray_ToLineSegment_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Ray_ToLineSegment_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Ray *This = TypeFromQScriptValue<Ray*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Ray_ToLineSegment_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float d = TypeFromQScriptValue<float>(context->argument(0));
    LineSegment ret = This->ToLineSegment(d);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Ray_pos_get(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Ray_pos_get in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Ray *This = TypeFromQScriptValue<Ray*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    return qScriptValueFromValue(context->engine(), This->pos);
}

static QScriptValue Ray_pos_set(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Ray_pos_get in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Ray *This = TypeFromQScriptValue<Ray*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 pos = qscriptvalue_cast<float3>(context->argument(0));
    This->pos = pos;
    return QScriptValue();
}

static QScriptValue Ray_dir_get(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Ray_dir_get in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Ray *This = TypeFromQScriptValue<Ray*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    return qScriptValueFromValue(context->engine(), This->dir);
}

static QScriptValue Ray_dir_set(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Ray_dir_get in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Ray *This = TypeFromQScriptValue<Ray*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 dir = qscriptvalue_cast<float3>(context->argument(0));
    This->dir = dir;
    return QScriptValue();
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
    printf("Ray_ctor failed to choose the right function to call! Did you use 'var x = Ray();' instead of 'var x = new Ray();'?\n"); return QScriptValue();
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
    printf("Ray_Transform_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

class Ray_scriptclass : public QScriptClass
{
public:
    QScriptValue objectPrototype;
    Ray_scriptclass(QScriptEngine *engine):QScriptClass(engine){}
    QScriptValue property(const QScriptValue &object, const QScriptString &name, uint id)
    {
        Ray *This = TypeFromQScriptValue<Ray*>(object);
        if (!This) { printf("Error! Cannot convert QScriptValue to type Ray in file %s, line %d!\nTry using Ray.get%s() and Ray.set%s() to query the member variable '%s'!\n", __FILE__, __LINE__, Capitalize((QString)name).c_str(), Capitalize((QString)name).c_str(), ((QString)name).toStdString().c_str()); return QScriptValue(); }
        if ((QString)name == (QString)"pos") return TypeToQScriptValue(engine(), This->pos);
        if ((QString)name == (QString)"dir") return TypeToQScriptValue(engine(), This->dir);
        return QScriptValue();
    }
    void setProperty(QScriptValue &object, const QScriptString &name, uint id, const QScriptValue &value)
    {
        Ray *This = TypeFromQScriptValue<Ray*>(object);
        if (!This) { printf("Error! Cannot convert QScriptValue to type Ray in file %s, line %d!\nTry using Ray.get%s() and Ray.set%s() to query the member variable '%s'!\n", __FILE__, __LINE__, Capitalize((QString)name).c_str(), Capitalize((QString)name).c_str(), ((QString)name).toStdString().c_str()); return; }
        if ((QString)name == (QString)"pos") This->pos = TypeFromQScriptValue<float3>(value);
        if ((QString)name == (QString)"dir") This->dir = TypeFromQScriptValue<float3>(value);
    }
    QueryFlags queryProperty(const QScriptValue &object, const QScriptString &name, QueryFlags flags, uint *id)
    {
        if ((QString)name == (QString)"pos") return flags;
        if ((QString)name == (QString)"dir") return flags;
        return 0;
    }
    QScriptValue prototype() const { return objectPrototype; }
};
QScriptValue register_Ray_prototype(QScriptEngine *engine)
{
    engine->setDefaultPrototype(qMetaTypeId<Ray*>(), QScriptValue());
    QScriptValue proto = engine->newVariant(qVariantFromValue((Ray*)0));
    proto.setProperty("GetPoint", engine->newFunction(Ray_GetPoint_float, 1));
    proto.setProperty("Transform", engine->newFunction(Ray_Transform_selector, 1));
    proto.setProperty("ToLine", engine->newFunction(Ray_ToLine, 0));
    proto.setProperty("ToLineSegment", engine->newFunction(Ray_ToLineSegment_float, 1));
    proto.setProperty("getPos", engine->newFunction(Ray_pos_get, 1));
    proto.setProperty("setPos", engine->newFunction(Ray_pos_set, 1));
    proto.setProperty("getDir", engine->newFunction(Ray_dir_get, 1));
    proto.setProperty("setDir", engine->newFunction(Ray_dir_set, 1));
    Ray_scriptclass *sc = new Ray_scriptclass(engine);
    engine->setProperty("Ray_scriptclass", QVariant::fromValue<QScriptClass*>(sc));
    proto.setScriptClass(sc);
    sc->objectPrototype = proto;
    engine->setDefaultPrototype(qMetaTypeId<Ray>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<Ray*>(), proto);
    QScriptValue ctor = engine->newFunction(Ray_ctor, proto, 2);
    engine->globalObject().setProperty("Ray", ctor);
    return ctor;
}

