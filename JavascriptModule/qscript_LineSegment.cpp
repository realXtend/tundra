#include "QtScriptBindingsHelpers.h"

static QScriptValue LineSegment_LineSegment(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function LineSegment_LineSegment in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    LineSegment ret;
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue LineSegment_LineSegment_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function LineSegment_LineSegment_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 a = TypeFromQScriptValue<float3>(context->argument(0));
    float3 b = TypeFromQScriptValue<float3>(context->argument(1));
    LineSegment ret(a, b);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue LineSegment_LineSegment_Ray_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function LineSegment_LineSegment_Ray_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Ray ray = TypeFromQScriptValue<Ray>(context->argument(0));
    float d = TypeFromQScriptValue<float>(context->argument(1));
    LineSegment ret(ray, d);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue LineSegment_LineSegment_Line_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function LineSegment_LineSegment_Line_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Line line = TypeFromQScriptValue<Line>(context->argument(0));
    float d = TypeFromQScriptValue<float>(context->argument(1));
    LineSegment ret(line, d);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue LineSegment_GetPoint_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function LineSegment_GetPoint_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    LineSegment *This = TypeFromQScriptValue<LineSegment*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function LineSegment_GetPoint_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float d = TypeFromQScriptValue<float>(context->argument(0));
    float3 ret = This->GetPoint(d);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue LineSegment_Reverse(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function LineSegment_Reverse in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    LineSegment *This = TypeFromQScriptValue<LineSegment*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function LineSegment_Reverse in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    This->Reverse();
    return QScriptValue();
}

static QScriptValue LineSegment_Dir(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function LineSegment_Dir in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    LineSegment *This = TypeFromQScriptValue<LineSegment*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function LineSegment_Dir in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->Dir();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue LineSegment_Transform_float3x3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function LineSegment_Transform_float3x3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    LineSegment *This = TypeFromQScriptValue<LineSegment*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function LineSegment_Transform_float3x3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3x3 transform = TypeFromQScriptValue<float3x3>(context->argument(0));
    This->Transform(transform);
    return QScriptValue();
}

static QScriptValue LineSegment_Transform_float3x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function LineSegment_Transform_float3x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    LineSegment *This = TypeFromQScriptValue<LineSegment*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function LineSegment_Transform_float3x4 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3x4 transform = TypeFromQScriptValue<float3x4>(context->argument(0));
    This->Transform(transform);
    return QScriptValue();
}

static QScriptValue LineSegment_Transform_float4x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function LineSegment_Transform_float4x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    LineSegment *This = TypeFromQScriptValue<LineSegment*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function LineSegment_Transform_float4x4 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4x4 transform = TypeFromQScriptValue<float4x4>(context->argument(0));
    This->Transform(transform);
    return QScriptValue();
}

static QScriptValue LineSegment_Transform_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function LineSegment_Transform_Quat in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    LineSegment *This = TypeFromQScriptValue<LineSegment*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function LineSegment_Transform_Quat in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    Quat transform = TypeFromQScriptValue<Quat>(context->argument(0));
    This->Transform(transform);
    return QScriptValue();
}

static QScriptValue LineSegment_Length(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function LineSegment_Length in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    LineSegment *This = TypeFromQScriptValue<LineSegment*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function LineSegment_Length in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->Length();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue LineSegment_LengthSq(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function LineSegment_LengthSq in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    LineSegment *This = TypeFromQScriptValue<LineSegment*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function LineSegment_LengthSq in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->LengthSq();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue LineSegment_Intersects_Plane(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function LineSegment_Intersects_Plane in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    LineSegment *This = TypeFromQScriptValue<LineSegment*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function LineSegment_Intersects_Plane in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    Plane plane = TypeFromQScriptValue<Plane>(context->argument(0));
    bool ret = This->Intersects(plane);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue LineSegment_ToRay(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function LineSegment_ToRay in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    LineSegment *This = TypeFromQScriptValue<LineSegment*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function LineSegment_ToRay in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    Ray ret = This->ToRay();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue LineSegment_ToLine(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function LineSegment_ToLine in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    LineSegment *This = TypeFromQScriptValue<LineSegment*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function LineSegment_ToLine in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    Line ret = This->ToLine();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue LineSegment_a_get(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function LineSegment_a_get in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    LineSegment *This = TypeFromQScriptValue<LineSegment*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    return qScriptValueFromValue(context->engine(), This->a);
}

static QScriptValue LineSegment_a_set(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function LineSegment_a_set in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    LineSegment *This = TypeFromQScriptValue<LineSegment*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 a = qscriptvalue_cast<float3>(context->argument(0));
    This->a = a;
    return QScriptValue();
}

static QScriptValue LineSegment_b_get(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function LineSegment_b_get in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    LineSegment *This = TypeFromQScriptValue<LineSegment*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    return qScriptValueFromValue(context->engine(), This->b);
}

static QScriptValue LineSegment_b_set(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function LineSegment_b_set in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    LineSegment *This = TypeFromQScriptValue<LineSegment*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 b = qscriptvalue_cast<float3>(context->argument(0));
    This->b = b;
    return QScriptValue();
}

static QScriptValue LineSegment_ctor(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return LineSegment_LineSegment(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return LineSegment_LineSegment_float3_float3(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<Ray>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return LineSegment_LineSegment_Ray_float(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<Line>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return LineSegment_LineSegment_Line_float(context, engine);
    printf("LineSegment_ctor failed to choose the right function to call! Did you use 'var x = LineSegment();' instead of 'var x = new LineSegment();'?\n"); return QScriptValue();
}

static QScriptValue LineSegment_Transform_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3x3>(context->argument(0)))
        return LineSegment_Transform_float3x3(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3x4>(context->argument(0)))
        return LineSegment_Transform_float3x4(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float4x4>(context->argument(0)))
        return LineSegment_Transform_float4x4(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Quat>(context->argument(0)))
        return LineSegment_Transform_Quat(context, engine);
    printf("LineSegment_Transform_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

class LineSegment_scriptclass : public QScriptClass
{
public:
    QScriptValue objectPrototype;
    LineSegment_scriptclass(QScriptEngine *engine):QScriptClass(engine){}
    QScriptValue property(const QScriptValue &object, const QScriptString &name, uint id)
    {
        LineSegment *This = TypeFromQScriptValue<LineSegment*>(object);
        if (!This) { printf("Error! Cannot convert QScriptValue to type LineSegment in file %s, line %d!\nTry using LineSegment.get%s() and LineSegment.set%s() to query the member variable '%s'!\n", __FILE__, __LINE__, Capitalize((QString)name).c_str(), Capitalize((QString)name).c_str(), ((QString)name).toStdString().c_str()); return QScriptValue(); }
        QString name_ = (QString)name;
        if (name_ == "a_") return TypeToQScriptValue(engine(), This->a);
        if (name_ == "a_ptr") return TypeToQScriptValue(engine(), &This->a);
        if (name_ == "b_") return TypeToQScriptValue(engine(), This->b);
        if (name_ == "b_ptr") return TypeToQScriptValue(engine(), &This->b);
        return QScriptValue();
    }
    void setProperty(QScriptValue &object, const QScriptString &name, uint id, const QScriptValue &value)
    {
        LineSegment *This = TypeFromQScriptValue<LineSegment*>(object);
        if (!This) { printf("Error! Cannot convert QScriptValue to type LineSegment in file %s, line %d!\nTry using LineSegment.get%s() and LineSegment.set%s() to query the member variable '%s'!\n", __FILE__, __LINE__, Capitalize((QString)name).c_str(), Capitalize((QString)name).c_str(), ((QString)name).toStdString().c_str()); return; }
        QString name_ = (QString)name;
        if (name_ == "a_") This->a = TypeFromQScriptValue<float3>(value);
        if (name_ == "a_ptr") This->a = *TypeFromQScriptValue<float3*>(value);
        if (name_ == "b_") This->b = TypeFromQScriptValue<float3>(value);
        if (name_ == "b_ptr") This->b = *TypeFromQScriptValue<float3*>(value);
    }
    QueryFlags queryProperty(const QScriptValue &object, const QScriptString &name, QueryFlags flags, uint *id)
    {
        QString name_ = (QString)name;
        if (name_ == "a_" || name_ == "a_ptr") return flags;
        if (name_ == "b_" || name_ == "b_ptr") return flags;
        return 0;
    }
    QScriptValue prototype() const { return objectPrototype; }
};
QScriptValue register_LineSegment_prototype(QScriptEngine *engine)
{
    engine->setDefaultPrototype(qMetaTypeId<LineSegment*>(), QScriptValue());
    QScriptValue proto = engine->newVariant(qVariantFromValue((LineSegment*)0));
    proto.setProperty("GetPoint", engine->newFunction(LineSegment_GetPoint_float, 1));
    proto.setProperty("Reverse", engine->newFunction(LineSegment_Reverse, 0));
    proto.setProperty("Dir", engine->newFunction(LineSegment_Dir, 0));
    proto.setProperty("Transform", engine->newFunction(LineSegment_Transform_selector, 1));
    proto.setProperty("Length", engine->newFunction(LineSegment_Length, 0));
    proto.setProperty("LengthSq", engine->newFunction(LineSegment_LengthSq, 0));
    proto.setProperty("Intersects", engine->newFunction(LineSegment_Intersects_Plane, 1));
    proto.setProperty("ToRay", engine->newFunction(LineSegment_ToRay, 0));
    proto.setProperty("ToLine", engine->newFunction(LineSegment_ToLine, 0));
    proto.setProperty("a", engine->newFunction(LineSegment_a_get, 1));
    proto.setProperty("setA", engine->newFunction(LineSegment_a_set, 1));
    proto.setProperty("b", engine->newFunction(LineSegment_b_get, 1));
    proto.setProperty("setB", engine->newFunction(LineSegment_b_set, 1));
    LineSegment_scriptclass *sc = new LineSegment_scriptclass(engine);
    engine->setProperty("LineSegment_scriptclass", QVariant::fromValue<QScriptClass*>(sc));
    proto.setScriptClass(sc);
    sc->objectPrototype = proto;
    engine->setDefaultPrototype(qMetaTypeId<LineSegment>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<LineSegment*>(), proto);
    QScriptValue ctor = engine->newFunction(LineSegment_ctor, proto, 2);
    engine->globalObject().setProperty("LineSegment", ctor);
    return ctor;
}

