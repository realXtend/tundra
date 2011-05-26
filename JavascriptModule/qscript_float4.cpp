#include "QtScriptBindingsHelpers.h"

static QScriptValue float4_float4(QScriptContext *context, QScriptEngine *engine)
{
    float4 ret;
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_float4_float_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    float x = TypeFromQScriptValue<float>(context->argument(0));
    float y = TypeFromQScriptValue<float>(context->argument(1));
    float z = TypeFromQScriptValue<float>(context->argument(2));
    float w = TypeFromQScriptValue<float>(context->argument(3));
    float4 ret(x, y, z, w);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_float4_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    float3 xyz = TypeFromQScriptValue<float3>(context->argument(0));
    float w = TypeFromQScriptValue<float>(context->argument(1));
    float4 ret(xyz, w);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_xyz(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->xyz();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_LengthSq3(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->LengthSq3();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_Length3(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->Length3();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_LengthSq4(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->LengthSq4();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_Length4(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->Length4();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_Normalize3(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->Normalize3();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_Normalize4(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->Normalize4();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_Normalized3(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4 ret = This->Normalized3();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_Normalized4(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4 ret = This->Normalized4();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_NormalizeW(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    bool ret = This->NormalizeW();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_IsWZeroOrOne_float(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float epsilon = TypeFromQScriptValue<float>(context->argument(0));
    bool ret = This->IsWZeroOrOne(epsilon);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_IsZero3_float(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float epsilonSq = TypeFromQScriptValue<float>(context->argument(0));
    bool ret = This->IsZero3(epsilonSq);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_IsZero4_float(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float epsilonSq = TypeFromQScriptValue<float>(context->argument(0));
    bool ret = This->IsZero4(epsilonSq);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_IsNormalized3_float(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float epsilonSq = TypeFromQScriptValue<float>(context->argument(0));
    bool ret = This->IsNormalized3(epsilonSq);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_IsNormalized4_float(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float epsilonSq = TypeFromQScriptValue<float>(context->argument(0));
    bool ret = This->IsNormalized4(epsilonSq);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_Scale3_float(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float scalar = TypeFromQScriptValue<float>(context->argument(0));
    This->Scale3(scalar);
    return QScriptValue();
}

static QScriptValue float4_ScaleToLength3_float(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float newLength = TypeFromQScriptValue<float>(context->argument(0));
    float ret = This->ScaleToLength3(newLength);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_IsFinite(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    bool ret = This->IsFinite();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_IsPerpendicular3_float4_float(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4 other = TypeFromQScriptValue<float4>(context->argument(0));
    float epsilon = TypeFromQScriptValue<float>(context->argument(1));
    bool ret = This->IsPerpendicular3(other, epsilon);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_SumOfElements(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->SumOfElements();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_ProductOfElements(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->ProductOfElements();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_AverageOfElements(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->AverageOfElements();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_MinElement(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->MinElement();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_MinElementIndex(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int ret = This->MinElementIndex();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_MaxElement(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->MaxElement();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_MaxElementIndex(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int ret = This->MaxElementIndex();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_Abs(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4 ret = This->Abs();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_Min_float(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ceil = TypeFromQScriptValue<float>(context->argument(0));
    float4 ret = This->Min(ceil);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_Min_float4(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4 ceil = TypeFromQScriptValue<float4>(context->argument(0));
    float4 ret = This->Min(ceil);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_Max_float(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float floor = TypeFromQScriptValue<float>(context->argument(0));
    float4 ret = This->Max(floor);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_Max_float4(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4 floor = TypeFromQScriptValue<float4>(context->argument(0));
    float4 ret = This->Max(floor);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_Clamp_float4_float4(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4 floor = TypeFromQScriptValue<float4>(context->argument(0));
    float4 ceil = TypeFromQScriptValue<float4>(context->argument(1));
    float4 ret = This->Clamp(floor, ceil);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_Clamp01(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4 ret = This->Clamp01();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_Clamp_float_float(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float floor = TypeFromQScriptValue<float>(context->argument(0));
    float ceil = TypeFromQScriptValue<float>(context->argument(1));
    float4 ret = This->Clamp(floor, ceil);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_Lerp_float4_float(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4 b = TypeFromQScriptValue<float4>(context->argument(0));
    float t = TypeFromQScriptValue<float>(context->argument(1));
    float4 ret = This->Lerp(b, t);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_Distance3Sq_float4(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4 rhs = TypeFromQScriptValue<float4>(context->argument(0));
    float ret = This->Distance3Sq(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_Distance3_float4(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4 rhs = TypeFromQScriptValue<float4>(context->argument(0));
    float ret = This->Distance3(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_Dot3_float3(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 rhs = TypeFromQScriptValue<float3>(context->argument(0));
    float ret = This->Dot3(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_Dot3_float4(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4 rhs = TypeFromQScriptValue<float4>(context->argument(0));
    float ret = This->Dot3(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_Dot4_float4(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4 rhs = TypeFromQScriptValue<float4>(context->argument(0));
    float ret = This->Dot4(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_Cross3_float3(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 rhs = TypeFromQScriptValue<float3>(context->argument(0));
    float4 ret = This->Cross3(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_Cross3_float4(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4 rhs = TypeFromQScriptValue<float4>(context->argument(0));
    float4 ret = This->Cross3(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_OuterProduct_float4(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4 rhs = TypeFromQScriptValue<float4>(context->argument(0));
    float4x4 ret = This->OuterProduct(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_Perpendicular3_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 hint = TypeFromQScriptValue<float3>(context->argument(0));
    float3 hint2 = TypeFromQScriptValue<float3>(context->argument(1));
    float4 ret = This->Perpendicular3(hint, hint2);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_AnotherPerpendicular3_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 hint = TypeFromQScriptValue<float3>(context->argument(0));
    float3 hint2 = TypeFromQScriptValue<float3>(context->argument(1));
    float4 ret = This->AnotherPerpendicular3(hint, hint2);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_Reflect3_float3(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 normal = TypeFromQScriptValue<float3>(context->argument(0));
    float4 ret = This->Reflect3(normal);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_AngleBetween3_float4(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4 other = TypeFromQScriptValue<float4>(context->argument(0));
    float ret = This->AngleBetween3(other);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_AngleBetweenNorm3_float4(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4 normalizedVector = TypeFromQScriptValue<float4>(context->argument(0));
    float ret = This->AngleBetweenNorm3(normalizedVector);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_AngleBetween4_float4(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4 other = TypeFromQScriptValue<float4>(context->argument(0));
    float ret = This->AngleBetween4(other);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_AngleBetweenNorm4_float4(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4 normalizedVector = TypeFromQScriptValue<float4>(context->argument(0));
    float ret = This->AngleBetweenNorm4(normalizedVector);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_ProjectTo3_float3(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 target = TypeFromQScriptValue<float3>(context->argument(0));
    float4 ret = This->ProjectTo3(target);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_ProjectToNorm3_float3(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 target = TypeFromQScriptValue<float3>(context->argument(0));
    float4 ret = This->ProjectToNorm3(target);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_SetFromScalar_float(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float scalar = TypeFromQScriptValue<float>(context->argument(0));
    This->SetFromScalar(scalar);
    return QScriptValue();
}

static QScriptValue float4_SetFromScalar_float_float(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float scalar = TypeFromQScriptValue<float>(context->argument(0));
    float w = TypeFromQScriptValue<float>(context->argument(1));
    This->SetFromScalar(scalar, w);
    return QScriptValue();
}

static QScriptValue float4_Set_float_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float x = TypeFromQScriptValue<float>(context->argument(0));
    float y = TypeFromQScriptValue<float>(context->argument(1));
    float z = TypeFromQScriptValue<float>(context->argument(2));
    float w = TypeFromQScriptValue<float>(context->argument(3));
    This->Set(x, y, z, w);
    return QScriptValue();
}

static QScriptValue float4_Equals_float4_float(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4 other = TypeFromQScriptValue<float4>(context->argument(0));
    float epsilon = TypeFromQScriptValue<float>(context->argument(1));
    bool ret = This->Equals(other, epsilon);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_Equals_float_float_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float x = TypeFromQScriptValue<float>(context->argument(0));
    float y = TypeFromQScriptValue<float>(context->argument(1));
    float z = TypeFromQScriptValue<float>(context->argument(2));
    float w = TypeFromQScriptValue<float>(context->argument(3));
    float epsilon = TypeFromQScriptValue<float>(context->argument(4));
    bool ret = This->Equals(x, y, z, w, epsilon);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_Add_float4(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4 rhs = TypeFromQScriptValue<float4>(context->argument(0));
    float4 ret = This->Add(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_Sub_float4(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4 rhs = TypeFromQScriptValue<float4>(context->argument(0));
    float4 ret = This->Sub(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_Mul_float(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float rhs = TypeFromQScriptValue<float>(context->argument(0));
    float4 ret = This->Mul(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_Div_float(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float rhs = TypeFromQScriptValue<float>(context->argument(0));
    float4 ret = This->Div(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_Neg(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4 ret = This->Neg();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_float4_QVector4D(QScriptContext *context, QScriptEngine *engine)
{
    QVector4D other = TypeFromQScriptValue<QVector4D>(context->argument(0));
    float4 ret(other);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_toString(QScriptContext *context, QScriptEngine *engine)
{
    float4 *This = TypeFromQScriptValue<float4*>(context->thisObject());
    if (!This && context->argumentCount() > 0) This = TypeFromQScriptValue<float4*>(context->argument(0)); // Qt oddity (bug?): Sometimes the built-in toString() function doesn't give us this from thisObject, but as the first argument.
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    QString ret = This->toString();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_FromScalar_float(QScriptContext *context, QScriptEngine *engine)
{
    float scalar = TypeFromQScriptValue<float>(context->argument(0));
    float4 ret = float4::FromScalar(scalar);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_FromScalar_float_float(QScriptContext *context, QScriptEngine *engine)
{
    float scalar = TypeFromQScriptValue<float>(context->argument(0));
    float w = TypeFromQScriptValue<float>(context->argument(1));
    float4 ret = float4::FromScalar(scalar, w);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4_ctor(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return float4_float4(context, engine);
    if (context->argumentCount() == 4 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)) && QSVIsOfType<float>(context->argument(3)))
        return float4_float4_float_float_float_float(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return float4_float4_float3_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<QVector4D>(context->argument(0)))
        return float4_float4_QVector4D(context, engine);
    printf("float4_ctor failed to choose the right function to call! Did you use 'var x = float4();' instead of 'var x = new float4();'?\n"); return QScriptValue();
}

static QScriptValue float4_Min_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return float4_Min_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float4>(context->argument(0)))
        return float4_Min_float4(context, engine);
    printf("float4_Min_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float4_Max_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return float4_Max_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float4>(context->argument(0)))
        return float4_Max_float4(context, engine);
    printf("float4_Max_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float4_Clamp_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float4>(context->argument(0)) && QSVIsOfType<float4>(context->argument(1)))
        return float4_Clamp_float4_float4(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return float4_Clamp_float_float(context, engine);
    printf("float4_Clamp_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float4_Dot3_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return float4_Dot3_float3(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float4>(context->argument(0)))
        return float4_Dot3_float4(context, engine);
    printf("float4_Dot3_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float4_Cross3_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return float4_Cross3_float3(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float4>(context->argument(0)))
        return float4_Cross3_float4(context, engine);
    printf("float4_Cross3_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float4_SetFromScalar_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return float4_SetFromScalar_float(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return float4_SetFromScalar_float_float(context, engine);
    printf("float4_SetFromScalar_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float4_Equals_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float4>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return float4_Equals_float4_float(context, engine);
    if (context->argumentCount() == 5 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)) && QSVIsOfType<float>(context->argument(3)) && QSVIsOfType<float>(context->argument(4)))
        return float4_Equals_float_float_float_float_float(context, engine);
    printf("float4_Equals_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float4_FromScalar_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return float4_FromScalar_float(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return float4_FromScalar_float_float(context, engine);
    printf("float4_FromScalar_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

class float4_scriptclass : public QScriptClass
{
public:
    QScriptValue objectPrototype;
    float4_scriptclass(QScriptEngine *engine):QScriptClass(engine){}
    QScriptValue property(const QScriptValue &object, const QScriptString &name, uint id)
    {
        float4 *This = TypeFromQScriptValue<float4*>(object);
        if ((QString)name == (QString)"x") return TypeToQScriptValue(engine(), This->x);
        if ((QString)name == (QString)"y") return TypeToQScriptValue(engine(), This->y);
        if ((QString)name == (QString)"z") return TypeToQScriptValue(engine(), This->z);
        if ((QString)name == (QString)"w") return TypeToQScriptValue(engine(), This->w);
        if ((QString)name == (QString)"zero") return TypeToQScriptValue(engine(), This->zero);
        if ((QString)name == (QString)"one") return TypeToQScriptValue(engine(), This->one);
        if ((QString)name == (QString)"unitX") return TypeToQScriptValue(engine(), This->unitX);
        if ((QString)name == (QString)"unitY") return TypeToQScriptValue(engine(), This->unitY);
        if ((QString)name == (QString)"unitZ") return TypeToQScriptValue(engine(), This->unitZ);
        if ((QString)name == (QString)"unitW") return TypeToQScriptValue(engine(), This->unitW);
        return QScriptValue();
    }
    void setProperty(QScriptValue &object, const QScriptString &name, uint id, const QScriptValue &value)
    {
        float4 *This = TypeFromQScriptValue<float4*>(object);
        if ((QString)name == (QString)"x") This->x = TypeFromQScriptValue<float>(value);
        if ((QString)name == (QString)"y") This->y = TypeFromQScriptValue<float>(value);
        if ((QString)name == (QString)"z") This->z = TypeFromQScriptValue<float>(value);
        if ((QString)name == (QString)"w") This->w = TypeFromQScriptValue<float>(value);
    }
    QueryFlags queryProperty(const QScriptValue &object, const QScriptString &name, QueryFlags flags, uint *id)
    {
        if ((QString)name == (QString)"x") return flags;
        if ((QString)name == (QString)"y") return flags;
        if ((QString)name == (QString)"z") return flags;
        if ((QString)name == (QString)"w") return flags;
        if ((QString)name == (QString)"zero") return flags;
        if ((QString)name == (QString)"one") return flags;
        if ((QString)name == (QString)"unitX") return flags;
        if ((QString)name == (QString)"unitY") return flags;
        if ((QString)name == (QString)"unitZ") return flags;
        if ((QString)name == (QString)"unitW") return flags;
        return 0;
    }
    QScriptValue prototype() const { return objectPrototype; }
};
QScriptValue register_float4_prototype(QScriptEngine *engine)
{
    engine->setDefaultPrototype(qMetaTypeId<float4*>(), QScriptValue());
    QScriptValue proto = engine->newVariant(qVariantFromValue((float4*)0));
    proto.setProperty("xyz", engine->newFunction(float4_xyz, 0));
    proto.setProperty("LengthSq3", engine->newFunction(float4_LengthSq3, 0));
    proto.setProperty("Length3", engine->newFunction(float4_Length3, 0));
    proto.setProperty("LengthSq4", engine->newFunction(float4_LengthSq4, 0));
    proto.setProperty("Length4", engine->newFunction(float4_Length4, 0));
    proto.setProperty("Normalize3", engine->newFunction(float4_Normalize3, 0));
    proto.setProperty("Normalize4", engine->newFunction(float4_Normalize4, 0));
    proto.setProperty("Normalized3", engine->newFunction(float4_Normalized3, 0));
    proto.setProperty("Normalized4", engine->newFunction(float4_Normalized4, 0));
    proto.setProperty("NormalizeW", engine->newFunction(float4_NormalizeW, 0));
    proto.setProperty("IsWZeroOrOne", engine->newFunction(float4_IsWZeroOrOne_float, 1));
    proto.setProperty("IsZero3", engine->newFunction(float4_IsZero3_float, 1));
    proto.setProperty("IsZero4", engine->newFunction(float4_IsZero4_float, 1));
    proto.setProperty("IsNormalized3", engine->newFunction(float4_IsNormalized3_float, 1));
    proto.setProperty("IsNormalized4", engine->newFunction(float4_IsNormalized4_float, 1));
    proto.setProperty("Scale3", engine->newFunction(float4_Scale3_float, 1));
    proto.setProperty("ScaleToLength3", engine->newFunction(float4_ScaleToLength3_float, 1));
    proto.setProperty("IsFinite", engine->newFunction(float4_IsFinite, 0));
    proto.setProperty("IsPerpendicular3", engine->newFunction(float4_IsPerpendicular3_float4_float, 2));
    proto.setProperty("SumOfElements", engine->newFunction(float4_SumOfElements, 0));
    proto.setProperty("ProductOfElements", engine->newFunction(float4_ProductOfElements, 0));
    proto.setProperty("AverageOfElements", engine->newFunction(float4_AverageOfElements, 0));
    proto.setProperty("MinElement", engine->newFunction(float4_MinElement, 0));
    proto.setProperty("MinElementIndex", engine->newFunction(float4_MinElementIndex, 0));
    proto.setProperty("MaxElement", engine->newFunction(float4_MaxElement, 0));
    proto.setProperty("MaxElementIndex", engine->newFunction(float4_MaxElementIndex, 0));
    proto.setProperty("Abs", engine->newFunction(float4_Abs, 0));
    proto.setProperty("Min", engine->newFunction(float4_Min_selector, 1));
    proto.setProperty("Max", engine->newFunction(float4_Max_selector, 1));
    proto.setProperty("Clamp", engine->newFunction(float4_Clamp_selector, 2));
    proto.setProperty("Clamp01", engine->newFunction(float4_Clamp01, 0));
    proto.setProperty("Lerp", engine->newFunction(float4_Lerp_float4_float, 2));
    proto.setProperty("Distance3Sq", engine->newFunction(float4_Distance3Sq_float4, 1));
    proto.setProperty("Distance3", engine->newFunction(float4_Distance3_float4, 1));
    proto.setProperty("Dot3", engine->newFunction(float4_Dot3_selector, 1));
    proto.setProperty("Dot4", engine->newFunction(float4_Dot4_float4, 1));
    proto.setProperty("Cross3", engine->newFunction(float4_Cross3_selector, 1));
    proto.setProperty("OuterProduct", engine->newFunction(float4_OuterProduct_float4, 1));
    proto.setProperty("Perpendicular3", engine->newFunction(float4_Perpendicular3_float3_float3, 2));
    proto.setProperty("AnotherPerpendicular3", engine->newFunction(float4_AnotherPerpendicular3_float3_float3, 2));
    proto.setProperty("Reflect3", engine->newFunction(float4_Reflect3_float3, 1));
    proto.setProperty("AngleBetween3", engine->newFunction(float4_AngleBetween3_float4, 1));
    proto.setProperty("AngleBetweenNorm3", engine->newFunction(float4_AngleBetweenNorm3_float4, 1));
    proto.setProperty("AngleBetween4", engine->newFunction(float4_AngleBetween4_float4, 1));
    proto.setProperty("AngleBetweenNorm4", engine->newFunction(float4_AngleBetweenNorm4_float4, 1));
    proto.setProperty("ProjectTo3", engine->newFunction(float4_ProjectTo3_float3, 1));
    proto.setProperty("ProjectToNorm3", engine->newFunction(float4_ProjectToNorm3_float3, 1));
    proto.setProperty("SetFromScalar", engine->newFunction(float4_SetFromScalar_selector, 1));
    proto.setProperty("SetFromScalar", engine->newFunction(float4_SetFromScalar_selector, 2));
    proto.setProperty("Set", engine->newFunction(float4_Set_float_float_float_float, 4));
    proto.setProperty("Equals", engine->newFunction(float4_Equals_selector, 2));
    proto.setProperty("Equals", engine->newFunction(float4_Equals_selector, 5));
    proto.setProperty("Add", engine->newFunction(float4_Add_float4, 1));
    proto.setProperty("Sub", engine->newFunction(float4_Sub_float4, 1));
    proto.setProperty("Mul", engine->newFunction(float4_Mul_float, 1));
    proto.setProperty("Div", engine->newFunction(float4_Div_float, 1));
    proto.setProperty("Neg", engine->newFunction(float4_Neg, 0));
    proto.setProperty("toString", engine->newFunction(float4_toString, 0));
    float4_scriptclass *sc = new float4_scriptclass(engine);
    engine->setProperty("float4_scriptclass", QVariant::fromValue<QScriptClass*>(sc));
    proto.setScriptClass(sc);
    sc->objectPrototype = proto;
    engine->setDefaultPrototype(qMetaTypeId<float4>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<float4*>(), proto);
    QScriptValue ctor = engine->newFunction(float4_ctor, proto, 4);
    ctor.setProperty("FromScalar", engine->newFunction(float4_FromScalar_selector, 1));
    ctor.setProperty("FromScalar", engine->newFunction(float4_FromScalar_selector, 2));
    ctor.setProperty("zero", TypeToQScriptValue(engine, float4::zero));
    ctor.setProperty("one", TypeToQScriptValue(engine, float4::one));
    ctor.setProperty("unitX", TypeToQScriptValue(engine, float4::unitX));
    ctor.setProperty("unitY", TypeToQScriptValue(engine, float4::unitY));
    ctor.setProperty("unitZ", TypeToQScriptValue(engine, float4::unitZ));
    ctor.setProperty("unitW", TypeToQScriptValue(engine, float4::unitW));
    engine->globalObject().setProperty("float4", ctor);
    return ctor;
}

