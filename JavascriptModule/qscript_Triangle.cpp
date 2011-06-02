#include "QtScriptBindingsHelpers.h"

static QScriptValue Triangle_Triangle(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Triangle_Triangle in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Triangle ret;
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Triangle_Triangle_float3_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Triangle_Triangle_float3_float3_float3 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 a = TypeFromQScriptValue<float3>(context->argument(0));
    float3 b = TypeFromQScriptValue<float3>(context->argument(1));
    float3 c = TypeFromQScriptValue<float3>(context->argument(2));
    Triangle ret(a, b, c);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Triangle_Barycentric_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_Barycentric_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Triangle *This = TypeFromQScriptValue<Triangle*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Triangle_Barycentric_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 point = TypeFromQScriptValue<float3>(context->argument(0));
    float3 ret = This->Barycentric(point);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Triangle_Point_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Triangle_Point_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Triangle *This = TypeFromQScriptValue<Triangle*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Triangle_Point_float_float_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float u = TypeFromQScriptValue<float>(context->argument(0));
    float v = TypeFromQScriptValue<float>(context->argument(1));
    float w = TypeFromQScriptValue<float>(context->argument(2));
    float3 ret = This->Point(u, v, w);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Triangle_Point_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Triangle_Point_float_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Triangle *This = TypeFromQScriptValue<Triangle*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Triangle_Point_float_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float u = TypeFromQScriptValue<float>(context->argument(0));
    float v = TypeFromQScriptValue<float>(context->argument(1));
    float3 ret = This->Point(u, v);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Triangle_Point_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_Point_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Triangle *This = TypeFromQScriptValue<Triangle*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Triangle_Point_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 barycentric = TypeFromQScriptValue<float3>(context->argument(0));
    float3 ret = This->Point(barycentric);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Triangle_Area(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Triangle_Area in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Triangle *This = TypeFromQScriptValue<Triangle*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Triangle_Area in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->Area();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Triangle_GetPlane(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Triangle_GetPlane in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Triangle *This = TypeFromQScriptValue<Triangle*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Triangle_GetPlane in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    Plane ret = This->GetPlane();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Triangle_IsDegenerate_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_IsDegenerate_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Triangle *This = TypeFromQScriptValue<Triangle*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Triangle_IsDegenerate_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float epsilon = TypeFromQScriptValue<float>(context->argument(0));
    bool ret = This->IsDegenerate(epsilon);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Triangle_Contains_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Triangle_Contains_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Triangle *This = TypeFromQScriptValue<Triangle*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Triangle_Contains_float3_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 point = TypeFromQScriptValue<float3>(context->argument(0));
    float triangleThickness = TypeFromQScriptValue<float>(context->argument(1));
    bool ret = This->Contains(point, triangleThickness);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Triangle_ClosestPoint_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_ClosestPoint_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Triangle *This = TypeFromQScriptValue<Triangle*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Triangle_ClosestPoint_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 targetPoint = TypeFromQScriptValue<float3>(context->argument(0));
    float3 ret = This->ClosestPoint(targetPoint);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Triangle_Area2D_float2_float2_float2(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Triangle_Area2D_float2_float2_float2 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float2 p1 = TypeFromQScriptValue<float2>(context->argument(0));
    float2 p2 = TypeFromQScriptValue<float2>(context->argument(1));
    float2 p3 = TypeFromQScriptValue<float2>(context->argument(2));
    float ret = Triangle::Area2D(p1, p2, p3);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Triangle_SignedArea_float3_float3_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function Triangle_SignedArea_float3_float3_float3_float3 in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 point = TypeFromQScriptValue<float3>(context->argument(0));
    float3 a = TypeFromQScriptValue<float3>(context->argument(1));
    float3 b = TypeFromQScriptValue<float3>(context->argument(2));
    float3 c = TypeFromQScriptValue<float3>(context->argument(3));
    float ret = Triangle::SignedArea(point, a, b, c);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Triangle_IsDegenerate_float3_float3_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function Triangle_IsDegenerate_float3_float3_float3_float in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 p1 = TypeFromQScriptValue<float3>(context->argument(0));
    float3 p2 = TypeFromQScriptValue<float3>(context->argument(1));
    float3 p3 = TypeFromQScriptValue<float3>(context->argument(2));
    float epsilon = TypeFromQScriptValue<float>(context->argument(3));
    bool ret = Triangle::IsDegenerate(p1, p2, p3, epsilon);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Triangle_a_get(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Triangle_a_get in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Triangle *This = TypeFromQScriptValue<Triangle*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    return qScriptValueFromValue(context->engine(), This->a);
}

static QScriptValue Triangle_a_set(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_a_set in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Triangle *This = TypeFromQScriptValue<Triangle*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 a = qscriptvalue_cast<float3>(context->argument(0));
    This->a = a;
    return QScriptValue();
}

static QScriptValue Triangle_b_get(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Triangle_b_get in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Triangle *This = TypeFromQScriptValue<Triangle*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    return qScriptValueFromValue(context->engine(), This->b);
}

static QScriptValue Triangle_b_set(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_b_set in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Triangle *This = TypeFromQScriptValue<Triangle*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 b = qscriptvalue_cast<float3>(context->argument(0));
    This->b = b;
    return QScriptValue();
}

static QScriptValue Triangle_c_get(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Triangle_c_get in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Triangle *This = TypeFromQScriptValue<Triangle*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    return qScriptValueFromValue(context->engine(), This->c);
}

static QScriptValue Triangle_c_set(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Triangle_c_set in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Triangle *This = TypeFromQScriptValue<Triangle*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 c = qscriptvalue_cast<float3>(context->argument(0));
    This->c = c;
    return QScriptValue();
}

static QScriptValue Triangle_ctor(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return Triangle_Triangle(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)))
        return Triangle_Triangle_float3_float3_float3(context, engine);
    printf("Triangle_ctor failed to choose the right function to call! Did you use 'var x = Triangle();' instead of 'var x = new Triangle();'?\n"); return QScriptValue();
}

static QScriptValue Triangle_Point_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 3 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return Triangle_Point_float_float_float(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return Triangle_Point_float_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return Triangle_Point_float3(context, engine);
    printf("Triangle_Point_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue Triangle_IsDegenerate_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return Triangle_IsDegenerate_float(context, engine);
    if (context->argumentCount() == 4 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)) && QSVIsOfType<float>(context->argument(3)))
        return Triangle_IsDegenerate_float3_float3_float3_float(context, engine);
    printf("Triangle_IsDegenerate_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

class Triangle_scriptclass : public QScriptClass
{
public:
    QScriptValue objectPrototype;
    Triangle_scriptclass(QScriptEngine *engine):QScriptClass(engine){}
    QScriptValue property(const QScriptValue &object, const QScriptString &name, uint id)
    {
        Triangle *This = TypeFromQScriptValue<Triangle*>(object);
        if (!This) { printf("Error! Cannot convert QScriptValue to type Triangle in file %s, line %d!\nTry using Triangle.get%s() and Triangle.set%s() to query the member variable '%s'!\n", __FILE__, __LINE__, Capitalize((QString)name).c_str(), Capitalize((QString)name).c_str(), ((QString)name).toStdString().c_str()); return QScriptValue(); }
        QString name_ = (QString)name;
        if (name_ == "a_") return TypeToQScriptValue(engine(), This->a);
        if (name_ == "a_ptr") return TypeToQScriptValue(engine(), &This->a);
        if (name_ == "b_") return TypeToQScriptValue(engine(), This->b);
        if (name_ == "b_ptr") return TypeToQScriptValue(engine(), &This->b);
        if (name_ == "c_") return TypeToQScriptValue(engine(), This->c);
        if (name_ == "c_ptr") return TypeToQScriptValue(engine(), &This->c);
        return QScriptValue();
    }
    void setProperty(QScriptValue &object, const QScriptString &name, uint id, const QScriptValue &value)
    {
        Triangle *This = TypeFromQScriptValue<Triangle*>(object);
        if (!This) { printf("Error! Cannot convert QScriptValue to type Triangle in file %s, line %d!\nTry using Triangle.get%s() and Triangle.set%s() to query the member variable '%s'!\n", __FILE__, __LINE__, Capitalize((QString)name).c_str(), Capitalize((QString)name).c_str(), ((QString)name).toStdString().c_str()); return; }
        QString name_ = (QString)name;
        if (name_ == "a_") This->a = TypeFromQScriptValue<float3>(value);
        if (name_ == "a_ptr") This->a = *TypeFromQScriptValue<float3*>(value);
        if (name_ == "b_") This->b = TypeFromQScriptValue<float3>(value);
        if (name_ == "b_ptr") This->b = *TypeFromQScriptValue<float3*>(value);
        if (name_ == "c_") This->c = TypeFromQScriptValue<float3>(value);
        if (name_ == "c_ptr") This->c = *TypeFromQScriptValue<float3*>(value);
    }
    QueryFlags queryProperty(const QScriptValue &object, const QScriptString &name, QueryFlags flags, uint *id)
    {
        QString name_ = (QString)name;
        if (name_ == "a_" || name_ == "a_ptr") return flags;
        if (name_ == "b_" || name_ == "b_ptr") return flags;
        if (name_ == "c_" || name_ == "c_ptr") return flags;
        return 0;
    }
    QScriptValue prototype() const { return objectPrototype; }
};
QScriptValue register_Triangle_prototype(QScriptEngine *engine)
{
    engine->setDefaultPrototype(qMetaTypeId<Triangle*>(), QScriptValue());
    QScriptValue proto = engine->newVariant(qVariantFromValue((Triangle*)0));
    proto.setProperty("Barycentric", engine->newFunction(Triangle_Barycentric_float3, 1));
    proto.setProperty("Point", engine->newFunction(Triangle_Point_selector, 3));
    proto.setProperty("Point", engine->newFunction(Triangle_Point_selector, 2));
    proto.setProperty("Point", engine->newFunction(Triangle_Point_selector, 1));
    proto.setProperty("Area", engine->newFunction(Triangle_Area, 0));
    proto.setProperty("GetPlane", engine->newFunction(Triangle_GetPlane, 0));
    proto.setProperty("IsDegenerate", engine->newFunction(Triangle_IsDegenerate_selector, 1));
    proto.setProperty("Contains", engine->newFunction(Triangle_Contains_float3_float, 2));
    proto.setProperty("ClosestPoint", engine->newFunction(Triangle_ClosestPoint_float3, 1));
    proto.setProperty("a", engine->newFunction(Triangle_a_get, 1));
    proto.setProperty("setA", engine->newFunction(Triangle_a_set, 1));
    proto.setProperty("b", engine->newFunction(Triangle_b_get, 1));
    proto.setProperty("setB", engine->newFunction(Triangle_b_set, 1));
    proto.setProperty("c", engine->newFunction(Triangle_c_get, 1));
    proto.setProperty("setC", engine->newFunction(Triangle_c_set, 1));
    Triangle_scriptclass *sc = new Triangle_scriptclass(engine);
    engine->setProperty("Triangle_scriptclass", QVariant::fromValue<QScriptClass*>(sc));
    proto.setScriptClass(sc);
    sc->objectPrototype = proto;
    engine->setDefaultPrototype(qMetaTypeId<Triangle>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<Triangle*>(), proto);
    QScriptValue ctor = engine->newFunction(Triangle_ctor, proto, 3);
    ctor.setProperty("Area2D", engine->newFunction(Triangle_Area2D_float2_float2_float2, 3));
    ctor.setProperty("SignedArea", engine->newFunction(Triangle_SignedArea_float3_float3_float3_float3, 4));
    ctor.setProperty("IsDegenerate", engine->newFunction(Triangle_IsDegenerate_selector, 4));
    engine->globalObject().setProperty("Triangle", ctor);
    return ctor;
}

