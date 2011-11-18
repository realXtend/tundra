#include "QtScriptBindingsHelpers.h"

void ToExistingScriptValue_Circle(QScriptEngine *engine, const Circle &value, QScriptValue obj)
{
    obj.setProperty("pos", qScriptValueFromValue(engine, value.pos), QScriptValue::Undeletable);
    obj.setProperty("normal", qScriptValueFromValue(engine, value.normal), QScriptValue::Undeletable);
    obj.setProperty("r", qScriptValueFromValue(engine, value.r), QScriptValue::Undeletable);
}

static QScriptValue Circle_Circle(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Circle_Circle in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Circle ret;
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Circle_Circle_float3_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Circle_Circle_float3_float3_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 center = qscriptvalue_cast<float3>(context->argument(0));
    float3 normal = qscriptvalue_cast<float3>(context->argument(1));
    float radius = qscriptvalue_cast<float>(context->argument(2));
    Circle ret(center, normal, radius);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Circle_BasisU_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Circle_BasisU_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Circle This = qscriptvalue_cast<Circle>(context->thisObject());
    float3 ret = This.BasisU();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Circle_BasisV_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Circle_BasisV_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Circle This = qscriptvalue_cast<Circle>(context->thisObject());
    float3 ret = This.BasisV();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Circle_GetPoint_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Circle_GetPoint_float_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Circle This = qscriptvalue_cast<Circle>(context->thisObject());
    float angleRadians = qscriptvalue_cast<float>(context->argument(0));
    float3 ret = This.GetPoint(angleRadians);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Circle_GetPoint_float_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Circle_GetPoint_float_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Circle This = qscriptvalue_cast<Circle>(context->thisObject());
    float angleRadians = qscriptvalue_cast<float>(context->argument(0));
    float d = qscriptvalue_cast<float>(context->argument(1));
    float3 ret = This.GetPoint(angleRadians, d);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Circle_CenterPoint_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Circle_CenterPoint_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Circle This = qscriptvalue_cast<Circle>(context->thisObject());
    float3 ret = This.CenterPoint();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Circle_Centroid_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Circle_Centroid_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Circle This = qscriptvalue_cast<Circle>(context->thisObject());
    float3 ret = This.Centroid();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Circle_ExtremePoint_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Circle_ExtremePoint_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Circle This = qscriptvalue_cast<Circle>(context->thisObject());
    float3 direction = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.ExtremePoint(direction);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Circle_ContainingPlane_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Circle_ContainingPlane_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Circle This = qscriptvalue_cast<Circle>(context->thisObject());
    Plane ret = This.ContainingPlane();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Circle_EdgeContains_float3_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Circle_EdgeContains_float3_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Circle This = qscriptvalue_cast<Circle>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float maxDistance = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.EdgeContains(point, maxDistance);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Circle_DistanceToEdge_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Circle_DistanceToEdge_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Circle This = qscriptvalue_cast<Circle>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float ret = This.DistanceToEdge(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Circle_DistanceToDisc_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Circle_DistanceToDisc_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Circle This = qscriptvalue_cast<Circle>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float ret = This.DistanceToDisc(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Circle_ClosestPointToEdge_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Circle_ClosestPointToEdge_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Circle This = qscriptvalue_cast<Circle>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.ClosestPointToEdge(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Circle_ClosestPointToDisc_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Circle_ClosestPointToDisc_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Circle This = qscriptvalue_cast<Circle>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.ClosestPointToDisc(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Circle_Intersects_Plane_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Circle_Intersects_Plane_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Circle This = qscriptvalue_cast<Circle>(context->thisObject());
    Plane plane = qscriptvalue_cast<Plane>(context->argument(0));
    int ret = This.Intersects(plane);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Circle_IntersectsDisc_Line_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Circle_IntersectsDisc_Line_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Circle This = qscriptvalue_cast<Circle>(context->thisObject());
    Line line = qscriptvalue_cast<Line>(context->argument(0));
    bool ret = This.IntersectsDisc(line);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Circle_IntersectsDisc_LineSegment_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Circle_IntersectsDisc_LineSegment_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Circle This = qscriptvalue_cast<Circle>(context->thisObject());
    LineSegment lineSegment = qscriptvalue_cast<LineSegment>(context->argument(0));
    bool ret = This.IntersectsDisc(lineSegment);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Circle_IntersectsDisc_Ray_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Circle_IntersectsDisc_Ray_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Circle This = qscriptvalue_cast<Circle>(context->thisObject());
    Ray ray = qscriptvalue_cast<Ray>(context->argument(0));
    bool ret = This.IntersectsDisc(ray);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Circle_toString_const(QScriptContext *context, QScriptEngine *engine)
{
    Circle This;
    if (context->argumentCount() > 0) This = qscriptvalue_cast<Circle>(context->argument(0)); // Qt oddity (bug?): Sometimes the built-in toString() function doesn't give us this from thisObject, but as the first argument.
    else This = qscriptvalue_cast<Circle>(context->thisObject());
    QString ret = This.toString();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Circle_IntersectsFaces_manual(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Circle_IntersectsDisc_Ray_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Circle This = qscriptvalue_cast<Circle>(context->thisObject());
    OBB obb = qscriptvalue_cast<OBB>(context->argument(0));
    std::vector<float3> ret = This.IntersectsFaces(obb);
    QScriptValue retObj = engine->newArray(ret.size());
    for(size_t i = 0; i < ret.size(); ++i)
        retObj.setProperty(i, qScriptValueFromValue(engine, ret[i]));
    return retObj;
}

static QScriptValue Circle_ctor(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return Circle_Circle(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return Circle_Circle_float3_float3_float(context, engine);
    printf("Circle_ctor failed to choose the right function to call! Did you use 'var x = Circle();' instead of 'var x = new Circle();'?\n"); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Circle_GetPoint_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return Circle_GetPoint_float_const(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return Circle_GetPoint_float_float_const(context, engine);
    printf("Circle_GetPoint_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Circle_IntersectsDisc_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<Line>(context->argument(0)))
        return Circle_IntersectsDisc_Line_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<LineSegment>(context->argument(0)))
        return Circle_IntersectsDisc_LineSegment_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Ray>(context->argument(0)))
        return Circle_IntersectsDisc_Ray_const(context, engine);
    printf("Circle_IntersectsDisc_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

void FromScriptValue_Circle(const QScriptValue &obj, Circle &value)
{
    value.pos = qScriptValueToValue<float3>(obj.property("pos"));
    value.normal = qScriptValueToValue<float3>(obj.property("normal"));
    value.r = qScriptValueToValue<float>(obj.property("r"));
}

QScriptValue ToScriptValue_Circle(QScriptEngine *engine, const Circle &value)
{
    QScriptValue obj = engine->newVariant(QVariant::fromValue(value)); // The contents of this variant are NOT used. The real data lies in the data() pointer of this QScriptValue. This only exists to enable overload resolution to work for QObject slots.
    ToExistingScriptValue_Circle(engine, value, obj);
    return obj;
}

QScriptValue ToScriptValue_const_Circle(QScriptEngine *engine, const Circle &value)
{
    QScriptValue obj = engine->newVariant(QVariant::fromValue(value)); // The contents of this variant are NOT used. The real data lies in the data() pointer of this QScriptValue. This only exists to enable overload resolution to work for QObject slots.
    obj.setPrototype(engine->defaultPrototype(qMetaTypeId<Circle>()));
    obj.setProperty("pos", ToScriptValue_const_float3(engine, value.pos), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("normal", ToScriptValue_const_float3(engine, value.normal), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("r", qScriptValueFromValue(engine, value.r), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    return obj;
}

QScriptValue register_Circle_prototype(QScriptEngine *engine)
{
    QScriptValue proto = engine->newObject();
    proto.setProperty("BasisU", engine->newFunction(Circle_BasisU_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("BasisV", engine->newFunction(Circle_BasisV_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("GetPoint", engine->newFunction(Circle_GetPoint_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("GetPoint", engine->newFunction(Circle_GetPoint_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("CenterPoint", engine->newFunction(Circle_CenterPoint_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Centroid", engine->newFunction(Circle_Centroid_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ExtremePoint", engine->newFunction(Circle_ExtremePoint_float3_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ContainingPlane", engine->newFunction(Circle_ContainingPlane_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("EdgeContains", engine->newFunction(Circle_EdgeContains_float3_float_const, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("DistanceToEdge", engine->newFunction(Circle_DistanceToEdge_float3_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("DistanceToDisc", engine->newFunction(Circle_DistanceToDisc_float3_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ClosestPointToEdge", engine->newFunction(Circle_ClosestPointToEdge_float3_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ClosestPointToDisc", engine->newFunction(Circle_ClosestPointToDisc_float3_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Intersects", engine->newFunction(Circle_Intersects_Plane_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IntersectsDisc", engine->newFunction(Circle_IntersectsDisc_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("toString", engine->newFunction(Circle_toString_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IntersectsFaces", engine->newFunction(Circle_IntersectsFaces_manual, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("metaTypeId", engine->toScriptValue<qint32>((qint32)qMetaTypeId<Circle>()));
    engine->setDefaultPrototype(qMetaTypeId<Circle>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<Circle*>(), proto);
    qScriptRegisterMetaType(engine, ToScriptValue_Circle, FromScriptValue_Circle, proto);

    QScriptValue ctor = engine->newFunction(Circle_ctor, proto, 3);
    engine->globalObject().setProperty("Circle", ctor, QScriptValue::Undeletable | QScriptValue::ReadOnly);

    return ctor;
}

