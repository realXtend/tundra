#include "QtScriptBindingsHelpers.h"

static QScriptValue float2_float2(QScriptContext *context, QScriptEngine *engine)
{
    float2 ret;
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_float2_float_float(QScriptContext *context, QScriptEngine *engine)
{
    float x = TypeFromQScriptValue<float>(context->argument(0));
    float y = TypeFromQScriptValue<float>(context->argument(1));
    float2 ret(x, y);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_LengthSq(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->LengthSq();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_Length(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->Length();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_Normalize(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->Normalize();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_Normalized(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float2 ret = This->Normalized();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_ScaleToLength_float(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float newLength = TypeFromQScriptValue<float>(context->argument(0));
    float ret = This->ScaleToLength(newLength);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_IsNormalized_float(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float epsilonSq = TypeFromQScriptValue<float>(context->argument(0));
    bool ret = This->IsNormalized(epsilonSq);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_IsZero_float(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float epsilonSq = TypeFromQScriptValue<float>(context->argument(0));
    bool ret = This->IsZero(epsilonSq);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_IsFinite(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    bool ret = This->IsFinite();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_IsPerpendicular_float2_float(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float2 other = TypeFromQScriptValue<float2>(context->argument(0));
    float epsilon = TypeFromQScriptValue<float>(context->argument(1));
    bool ret = This->IsPerpendicular(other, epsilon);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_Equals_float2_float(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float2 rhs = TypeFromQScriptValue<float2>(context->argument(0));
    float epsilon = TypeFromQScriptValue<float>(context->argument(1));
    bool ret = This->Equals(rhs, epsilon);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_Equals_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float x = TypeFromQScriptValue<float>(context->argument(0));
    float y = TypeFromQScriptValue<float>(context->argument(1));
    float epsilon = TypeFromQScriptValue<float>(context->argument(2));
    bool ret = This->Equals(x, y, epsilon);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_SumOfElements(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->SumOfElements();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_ProductOfElements(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->ProductOfElements();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_AverageOfElements(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->AverageOfElements();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_MinElement(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->MinElement();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_MinElementIndex(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int ret = This->MinElementIndex();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_MaxElement(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->MaxElement();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_MaxElementIndex(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int ret = This->MaxElementIndex();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_Abs(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float2 ret = This->Abs();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_Min_float(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ceil = TypeFromQScriptValue<float>(context->argument(0));
    float2 ret = This->Min(ceil);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_Min_float2(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float2 ceil = TypeFromQScriptValue<float2>(context->argument(0));
    float2 ret = This->Min(ceil);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_Max_float(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float floor = TypeFromQScriptValue<float>(context->argument(0));
    float2 ret = This->Max(floor);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_Max_float2(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float2 floor = TypeFromQScriptValue<float2>(context->argument(0));
    float2 ret = This->Max(floor);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_Clamp_float_float(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float floor = TypeFromQScriptValue<float>(context->argument(0));
    float ceil = TypeFromQScriptValue<float>(context->argument(1));
    float2 ret = This->Clamp(floor, ceil);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_Clamp_float2_float2(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float2 floor = TypeFromQScriptValue<float2>(context->argument(0));
    float2 ceil = TypeFromQScriptValue<float2>(context->argument(1));
    float2 ret = This->Clamp(floor, ceil);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_Clamp01(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float2 ret = This->Clamp01();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_Distance_float2(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float2 rhs = TypeFromQScriptValue<float2>(context->argument(0));
    float ret = This->Distance(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_DistanceSq_float2(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float2 rhs = TypeFromQScriptValue<float2>(context->argument(0));
    float ret = This->DistanceSq(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_Dot_float2(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float2 rhs = TypeFromQScriptValue<float2>(context->argument(0));
    float ret = This->Dot(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_PerpDot_float2(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float2 rhs = TypeFromQScriptValue<float2>(context->argument(0));
    float ret = This->PerpDot(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_Reflect_float2(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float2 normal = TypeFromQScriptValue<float2>(context->argument(0));
    float2 ret = This->Reflect(normal);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_Refract_float2_float_float(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float2 normal = TypeFromQScriptValue<float2>(context->argument(0));
    float negativeSideRefractionIndex = TypeFromQScriptValue<float>(context->argument(1));
    float positiveSideRefractionIndex = TypeFromQScriptValue<float>(context->argument(2));
    float2 ret = This->Refract(normal, negativeSideRefractionIndex, positiveSideRefractionIndex);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_ProjectTo_float2(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float2 direction = TypeFromQScriptValue<float2>(context->argument(0));
    float2 ret = This->ProjectTo(direction);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_ProjectToNorm_float2(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float2 direction = TypeFromQScriptValue<float2>(context->argument(0));
    float2 ret = This->ProjectToNorm(direction);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_AngleBetween_float2(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float2 other = TypeFromQScriptValue<float2>(context->argument(0));
    float ret = This->AngleBetween(other);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_AngleBetweenNorm_float2(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float2 normalizedVector = TypeFromQScriptValue<float2>(context->argument(0));
    float ret = This->AngleBetweenNorm(normalizedVector);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_Decompose_float2_float2_float2(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float2 direction = TypeFromQScriptValue<float2>(context->argument(0));
    float2 outParallel = TypeFromQScriptValue<float2>(context->argument(1));
    float2 outPerpendicular = TypeFromQScriptValue<float2>(context->argument(2));
    This->Decompose(direction, outParallel, outPerpendicular);
    return QScriptValue();
}

static QScriptValue float2_Lerp_float2_float(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float2 b = TypeFromQScriptValue<float2>(context->argument(0));
    float t = TypeFromQScriptValue<float>(context->argument(1));
    float2 ret = This->Lerp(b, t);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_SetFromScalar_float(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float scalar = TypeFromQScriptValue<float>(context->argument(0));
    This->SetFromScalar(scalar);
    return QScriptValue();
}

static QScriptValue float2_Set_float_float(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float x = TypeFromQScriptValue<float>(context->argument(0));
    float y = TypeFromQScriptValue<float>(context->argument(1));
    This->Set(x, y);
    return QScriptValue();
}

static QScriptValue float2_Rotate90CW(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    This->Rotate90CW();
    return QScriptValue();
}

static QScriptValue float2_Rotate90CCW(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    This->Rotate90CCW();
    return QScriptValue();
}

static QScriptValue float2_Add_float2(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float2 rhs = TypeFromQScriptValue<float2>(context->argument(0));
    float2 ret = This->Add(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_Sub_float2(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float2 rhs = TypeFromQScriptValue<float2>(context->argument(0));
    float2 ret = This->Sub(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_Mul_float(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float rhs = TypeFromQScriptValue<float>(context->argument(0));
    float2 ret = This->Mul(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_Div_float(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float rhs = TypeFromQScriptValue<float>(context->argument(0));
    float2 ret = This->Div(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_Neg(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float2 ret = This->Neg();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_float2_QVector2D(QScriptContext *context, QScriptEngine *engine)
{
    QVector2D other = TypeFromQScriptValue<QVector2D>(context->argument(0));
    float2 ret(other);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_toString(QScriptContext *context, QScriptEngine *engine)
{
    float2 *This = TypeFromQScriptValue<float2*>(context->thisObject());
    if (!This && context->argumentCount() > 0) This = TypeFromQScriptValue<float2*>(context->argument(0)); // Qt oddity (bug?): Sometimes the built-in toString() function doesn't give us this from thisObject, but as the first argument.
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    QString ret = This->toString();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_Orthogonalize_float2_float2(QScriptContext *context, QScriptEngine *engine)
{
    float2 a = TypeFromQScriptValue<float2>(context->argument(0));
    float2 b = TypeFromQScriptValue<float2>(context->argument(1));
    float2::Orthogonalize(a, b);
    return QScriptValue();
}

static QScriptValue float2_Orthonormalize_float2_float2(QScriptContext *context, QScriptEngine *engine)
{
    float2 a = TypeFromQScriptValue<float2>(context->argument(0));
    float2 b = TypeFromQScriptValue<float2>(context->argument(1));
    float2::Orthonormalize(a, b);
    return QScriptValue();
}

static QScriptValue float2_FromScalar_float(QScriptContext *context, QScriptEngine *engine)
{
    float scalar = TypeFromQScriptValue<float>(context->argument(0));
    float2 ret = float2::FromScalar(scalar);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_OrientedCCW_float2_float2_float2(QScriptContext *context, QScriptEngine *engine)
{
    float2 a = TypeFromQScriptValue<float2>(context->argument(0));
    float2 b = TypeFromQScriptValue<float2>(context->argument(1));
    float2 c = TypeFromQScriptValue<float2>(context->argument(2));
    bool ret = float2::OrientedCCW(a, b, c);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float2_ctor(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return float2_float2(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return float2_float2_float_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<QVector2D>(context->argument(0)))
        return float2_float2_QVector2D(context, engine);
    printf("float2_ctor failed to choose the right function to call! Did you use 'var x = float2();' instead of 'var x = new float2();'?\n"); return QScriptValue();
}

static QScriptValue float2_Equals_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float2>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return float2_Equals_float2_float(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return float2_Equals_float_float_float(context, engine);
    printf("float2_Equals_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float2_Min_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return float2_Min_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float2>(context->argument(0)))
        return float2_Min_float2(context, engine);
    printf("float2_Min_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float2_Max_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return float2_Max_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float2>(context->argument(0)))
        return float2_Max_float2(context, engine);
    printf("float2_Max_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float2_Clamp_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return float2_Clamp_float_float(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float2>(context->argument(0)) && QSVIsOfType<float2>(context->argument(1)))
        return float2_Clamp_float2_float2(context, engine);
    printf("float2_Clamp_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

class float2_scriptclass : public QScriptClass
{
public:
    QScriptValue objectPrototype;
    float2_scriptclass(QScriptEngine *engine):QScriptClass(engine){}
    QScriptValue property(const QScriptValue &object, const QScriptString &name, uint id)
    {
        float2 *This = TypeFromQScriptValue<float2*>(object);
        if ((QString)name == (QString)"x") return TypeToQScriptValue(engine(), This->x);
        if ((QString)name == (QString)"y") return TypeToQScriptValue(engine(), This->y);
        if ((QString)name == (QString)"zero") return TypeToQScriptValue(engine(), This->zero);
        if ((QString)name == (QString)"one") return TypeToQScriptValue(engine(), This->one);
        if ((QString)name == (QString)"unitX") return TypeToQScriptValue(engine(), This->unitX);
        if ((QString)name == (QString)"unitY") return TypeToQScriptValue(engine(), This->unitY);
        return QScriptValue();
    }
    void setProperty(QScriptValue &object, const QScriptString &name, uint id, const QScriptValue &value)
    {
        float2 *This = TypeFromQScriptValue<float2*>(object);
        if ((QString)name == (QString)"x") This->x = TypeFromQScriptValue<float>(value);
        if ((QString)name == (QString)"y") This->y = TypeFromQScriptValue<float>(value);
    }
    QueryFlags queryProperty(const QScriptValue &object, const QScriptString &name, QueryFlags flags, uint *id)
    {
        if ((QString)name == (QString)"x") return flags;
        if ((QString)name == (QString)"y") return flags;
        if ((QString)name == (QString)"zero") return flags;
        if ((QString)name == (QString)"one") return flags;
        if ((QString)name == (QString)"unitX") return flags;
        if ((QString)name == (QString)"unitY") return flags;
        return 0;
    }
    QScriptValue prototype() const { return objectPrototype; }
};
QScriptValue register_float2_prototype(QScriptEngine *engine)
{
    engine->setDefaultPrototype(qMetaTypeId<float2*>(), QScriptValue());
    QScriptValue proto = engine->newVariant(qVariantFromValue((float2*)0));
    proto.setProperty("LengthSq", engine->newFunction(float2_LengthSq, 0));
    proto.setProperty("Length", engine->newFunction(float2_Length, 0));
    proto.setProperty("Normalize", engine->newFunction(float2_Normalize, 0));
    proto.setProperty("Normalized", engine->newFunction(float2_Normalized, 0));
    proto.setProperty("ScaleToLength", engine->newFunction(float2_ScaleToLength_float, 1));
    proto.setProperty("IsNormalized", engine->newFunction(float2_IsNormalized_float, 1));
    proto.setProperty("IsZero", engine->newFunction(float2_IsZero_float, 1));
    proto.setProperty("IsFinite", engine->newFunction(float2_IsFinite, 0));
    proto.setProperty("IsPerpendicular", engine->newFunction(float2_IsPerpendicular_float2_float, 2));
    proto.setProperty("Equals", engine->newFunction(float2_Equals_selector, 2));
    proto.setProperty("Equals", engine->newFunction(float2_Equals_selector, 3));
    proto.setProperty("SumOfElements", engine->newFunction(float2_SumOfElements, 0));
    proto.setProperty("ProductOfElements", engine->newFunction(float2_ProductOfElements, 0));
    proto.setProperty("AverageOfElements", engine->newFunction(float2_AverageOfElements, 0));
    proto.setProperty("MinElement", engine->newFunction(float2_MinElement, 0));
    proto.setProperty("MinElementIndex", engine->newFunction(float2_MinElementIndex, 0));
    proto.setProperty("MaxElement", engine->newFunction(float2_MaxElement, 0));
    proto.setProperty("MaxElementIndex", engine->newFunction(float2_MaxElementIndex, 0));
    proto.setProperty("Abs", engine->newFunction(float2_Abs, 0));
    proto.setProperty("Min", engine->newFunction(float2_Min_selector, 1));
    proto.setProperty("Max", engine->newFunction(float2_Max_selector, 1));
    proto.setProperty("Clamp", engine->newFunction(float2_Clamp_selector, 2));
    proto.setProperty("Clamp01", engine->newFunction(float2_Clamp01, 0));
    proto.setProperty("Distance", engine->newFunction(float2_Distance_float2, 1));
    proto.setProperty("DistanceSq", engine->newFunction(float2_DistanceSq_float2, 1));
    proto.setProperty("Dot", engine->newFunction(float2_Dot_float2, 1));
    proto.setProperty("PerpDot", engine->newFunction(float2_PerpDot_float2, 1));
    proto.setProperty("Reflect", engine->newFunction(float2_Reflect_float2, 1));
    proto.setProperty("Refract", engine->newFunction(float2_Refract_float2_float_float, 3));
    proto.setProperty("ProjectTo", engine->newFunction(float2_ProjectTo_float2, 1));
    proto.setProperty("ProjectToNorm", engine->newFunction(float2_ProjectToNorm_float2, 1));
    proto.setProperty("AngleBetween", engine->newFunction(float2_AngleBetween_float2, 1));
    proto.setProperty("AngleBetweenNorm", engine->newFunction(float2_AngleBetweenNorm_float2, 1));
    proto.setProperty("Decompose", engine->newFunction(float2_Decompose_float2_float2_float2, 3));
    proto.setProperty("Lerp", engine->newFunction(float2_Lerp_float2_float, 2));
    proto.setProperty("SetFromScalar", engine->newFunction(float2_SetFromScalar_float, 1));
    proto.setProperty("Set", engine->newFunction(float2_Set_float_float, 2));
    proto.setProperty("Rotate90CW", engine->newFunction(float2_Rotate90CW, 0));
    proto.setProperty("Rotate90CCW", engine->newFunction(float2_Rotate90CCW, 0));
    proto.setProperty("Add", engine->newFunction(float2_Add_float2, 1));
    proto.setProperty("Sub", engine->newFunction(float2_Sub_float2, 1));
    proto.setProperty("Mul", engine->newFunction(float2_Mul_float, 1));
    proto.setProperty("Div", engine->newFunction(float2_Div_float, 1));
    proto.setProperty("Neg", engine->newFunction(float2_Neg, 0));
    proto.setProperty("toString", engine->newFunction(float2_toString, 0));
    float2_scriptclass *sc = new float2_scriptclass(engine);
    engine->setProperty("float2_scriptclass", QVariant::fromValue<QScriptClass*>(sc));
    proto.setScriptClass(sc);
    sc->objectPrototype = proto;
    engine->setDefaultPrototype(qMetaTypeId<float2>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<float2*>(), proto);
    QScriptValue ctor = engine->newFunction(float2_ctor, proto, 2);
    ctor.setProperty("Orthogonalize", engine->newFunction(float2_Orthogonalize_float2_float2, 2));
    ctor.setProperty("Orthonormalize", engine->newFunction(float2_Orthonormalize_float2_float2, 2));
    ctor.setProperty("FromScalar", engine->newFunction(float2_FromScalar_float, 1));
    ctor.setProperty("OrientedCCW", engine->newFunction(float2_OrientedCCW_float2_float2_float2, 3));
    ctor.setProperty("zero", TypeToQScriptValue(engine, float2::zero));
    ctor.setProperty("one", TypeToQScriptValue(engine, float2::one));
    ctor.setProperty("unitX", TypeToQScriptValue(engine, float2::unitX));
    ctor.setProperty("unitY", TypeToQScriptValue(engine, float2::unitY));
    engine->globalObject().setProperty("float2", ctor);
    return ctor;
}

