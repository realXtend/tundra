#include "QtScriptBindingsHelpers.h"

static QScriptValue Line_Line(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Line_Line in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Line ret;
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Line_Line_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Line_Line_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 pos = TypeFromQScriptValue<float3>(context->argument(0));
    float3 dir = TypeFromQScriptValue<float3>(context->argument(1));
    Line ret(pos, dir);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Line_Line_Ray(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Line_Line_Ray in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Ray ray = TypeFromQScriptValue<Ray>(context->argument(0));
    Line ret(ray);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Line_Line_LineSegment(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Line_Line_LineSegment in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    LineSegment lineSegment = TypeFromQScriptValue<LineSegment>(context->argument(0));
    Line ret(lineSegment);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Line_GetPoint_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Line_GetPoint_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Line *This = TypeFromQScriptValue<Line*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Line_GetPoint_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float distance = TypeFromQScriptValue<float>(context->argument(0));
    float3 ret = This->GetPoint(distance);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Line_Transform_float3x3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Line_Transform_float3x3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Line *This = TypeFromQScriptValue<Line*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Line_Transform_float3x3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3x3 transform = TypeFromQScriptValue<float3x3>(context->argument(0));
    This->Transform(transform);
    return QScriptValue();
}

static QScriptValue Line_Transform_float3x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Line_Transform_float3x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Line *This = TypeFromQScriptValue<Line*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Line_Transform_float3x4 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3x4 transform = TypeFromQScriptValue<float3x4>(context->argument(0));
    This->Transform(transform);
    return QScriptValue();
}

static QScriptValue Line_Transform_float4x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Line_Transform_float4x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Line *This = TypeFromQScriptValue<Line*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Line_Transform_float4x4 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4x4 transform = TypeFromQScriptValue<float4x4>(context->argument(0));
    This->Transform(transform);
    return QScriptValue();
}

static QScriptValue Line_Transform_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Line_Transform_Quat in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Line *This = TypeFromQScriptValue<Line*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Line_Transform_Quat in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    Quat transform = TypeFromQScriptValue<Quat>(context->argument(0));
    This->Transform(transform);
    return QScriptValue();
}

static QScriptValue Line_ToRay(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Line_ToRay in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Line *This = TypeFromQScriptValue<Line*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Line_ToRay in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    Ray ret = This->ToRay();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Line_ToLineSegment_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Line_ToLineSegment_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Line *This = TypeFromQScriptValue<Line*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Line_ToLineSegment_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float d = TypeFromQScriptValue<float>(context->argument(0));
    LineSegment ret = This->ToLineSegment(d);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Line_AreCollinear_float3_float3_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function Line_AreCollinear_float3_float3_float3_float in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 p1 = TypeFromQScriptValue<float3>(context->argument(0));
    float3 p2 = TypeFromQScriptValue<float3>(context->argument(1));
    float3 p3 = TypeFromQScriptValue<float3>(context->argument(2));
    float epsilon = TypeFromQScriptValue<float>(context->argument(3));
    bool ret = Line::AreCollinear(p1, p2, p3, epsilon);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Line_pos_get(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Line_pos_get in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Line *This = TypeFromQScriptValue<Line*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    return qScriptValueFromValue(context->engine(), This->pos);
}

static QScriptValue Line_pos_set(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Line_pos_set in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Line *This = TypeFromQScriptValue<Line*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 pos = qscriptvalue_cast<float3>(context->argument(0));
    This->pos = pos;
    return QScriptValue();
}

static QScriptValue Line_dir_get(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Line_dir_get in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Line *This = TypeFromQScriptValue<Line*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    return qScriptValueFromValue(context->engine(), This->dir);
}

static QScriptValue Line_dir_set(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Line_dir_set in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Line *This = TypeFromQScriptValue<Line*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 dir = qscriptvalue_cast<float3>(context->argument(0));
    This->dir = dir;
    return QScriptValue();
}

static QScriptValue Line_ctor(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return Line_Line(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return Line_Line_float3_float3(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Ray>(context->argument(0)))
        return Line_Line_Ray(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<LineSegment>(context->argument(0)))
        return Line_Line_LineSegment(context, engine);
    printf("Line_ctor failed to choose the right function to call! Did you use 'var x = Line();' instead of 'var x = new Line();'?\n"); return QScriptValue();
}

static QScriptValue Line_Transform_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3x3>(context->argument(0)))
        return Line_Transform_float3x3(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3x4>(context->argument(0)))
        return Line_Transform_float3x4(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float4x4>(context->argument(0)))
        return Line_Transform_float4x4(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Quat>(context->argument(0)))
        return Line_Transform_Quat(context, engine);
    printf("Line_Transform_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

class Line_scriptclass : public QScriptClass
{
public:
    QScriptValue objectPrototype;
    Line_scriptclass(QScriptEngine *engine):QScriptClass(engine){}
    QScriptValue property(const QScriptValue &object, const QScriptString &name, uint id)
    {
        Line *This = TypeFromQScriptValue<Line*>(object);
        if (!This) { printf("Error! Cannot convert QScriptValue to type Line in file %s, line %d!\nTry using Line.get%s() and Line.set%s() to query the member variable '%s'!\n", __FILE__, __LINE__, Capitalize((QString)name).c_str(), Capitalize((QString)name).c_str(), ((QString)name).toStdString().c_str()); return QScriptValue(); }
        QString name_ = (QString)name;
        if (name_ == "pos_") return TypeToQScriptValue(engine(), This->pos);
        if (name_ == "pos_ptr") return TypeToQScriptValue(engine(), &This->pos);
        if (name_ == "dir_") return TypeToQScriptValue(engine(), This->dir);
        if (name_ == "dir_ptr") return TypeToQScriptValue(engine(), &This->dir);
        return QScriptValue();
    }
    void setProperty(QScriptValue &object, const QScriptString &name, uint id, const QScriptValue &value)
    {
        Line *This = TypeFromQScriptValue<Line*>(object);
        if (!This) { printf("Error! Cannot convert QScriptValue to type Line in file %s, line %d!\nTry using Line.get%s() and Line.set%s() to query the member variable '%s'!\n", __FILE__, __LINE__, Capitalize((QString)name).c_str(), Capitalize((QString)name).c_str(), ((QString)name).toStdString().c_str()); return; }
        QString name_ = (QString)name;
        if (name_ == "pos_") This->pos = TypeFromQScriptValue<float3>(value);
        if (name_ == "pos_ptr") This->pos = *TypeFromQScriptValue<float3*>(value);
        if (name_ == "dir_") This->dir = TypeFromQScriptValue<float3>(value);
        if (name_ == "dir_ptr") This->dir = *TypeFromQScriptValue<float3*>(value);
    }
    QueryFlags queryProperty(const QScriptValue &object, const QScriptString &name, QueryFlags flags, uint *id)
    {
        QString name_ = (QString)name;
        if (name_ == "pos_" || name_ == "pos_ptr") return flags;
        if (name_ == "dir_" || name_ == "dir_ptr") return flags;
        return 0;
    }
    QScriptValue prototype() const { return objectPrototype; }
};
QScriptValue register_Line_prototype(QScriptEngine *engine)
{
    engine->setDefaultPrototype(qMetaTypeId<Line*>(), QScriptValue());
    QScriptValue proto = engine->newVariant(qVariantFromValue((Line*)0));
    proto.setProperty("GetPoint", engine->newFunction(Line_GetPoint_float, 1));
    proto.setProperty("Transform", engine->newFunction(Line_Transform_selector, 1));
    proto.setProperty("ToRay", engine->newFunction(Line_ToRay, 0));
    proto.setProperty("ToLineSegment", engine->newFunction(Line_ToLineSegment_float, 1));
    proto.setProperty("pos", engine->newFunction(Line_pos_get, 1));
    proto.setProperty("setPos", engine->newFunction(Line_pos_set, 1));
    proto.setProperty("dir", engine->newFunction(Line_dir_get, 1));
    proto.setProperty("setDir", engine->newFunction(Line_dir_set, 1));
    Line_scriptclass *sc = new Line_scriptclass(engine);
    engine->setProperty("Line_scriptclass", QVariant::fromValue<QScriptClass*>(sc));
    proto.setScriptClass(sc);
    sc->objectPrototype = proto;
    engine->setDefaultPrototype(qMetaTypeId<Line>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<Line*>(), proto);
    QScriptValue ctor = engine->newFunction(Line_ctor, proto, 2);
    ctor.setProperty("AreCollinear", engine->newFunction(Line_AreCollinear_float3_float3_float3_float, 4));
    engine->globalObject().setProperty("Line", ctor);
    return ctor;
}

