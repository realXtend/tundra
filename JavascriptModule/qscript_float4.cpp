#include "QtScriptBindingsHelpers.h"

void ToExistingScriptValue_float4(QScriptEngine *engine, const float4 &value, QScriptValue obj)
{
    obj.setProperty("x", qScriptValueFromValue(engine, value.x), QScriptValue::Undeletable);
    obj.setProperty("y", qScriptValueFromValue(engine, value.y), QScriptValue::Undeletable);
    obj.setProperty("z", qScriptValueFromValue(engine, value.z), QScriptValue::Undeletable);
    obj.setProperty("w", qScriptValueFromValue(engine, value.w), QScriptValue::Undeletable);
}

static QScriptValue float4_float4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4_float4 in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 ret;
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_float4_float_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function float4_float4_float_float_float_float in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float x = qscriptvalue_cast<float>(context->argument(0));
    float y = qscriptvalue_cast<float>(context->argument(1));
    float z = qscriptvalue_cast<float>(context->argument(2));
    float w = qscriptvalue_cast<float>(context->argument(3));
    float4 ret(x, y, z, w);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_float4_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4_float4_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 xyz = qscriptvalue_cast<float3>(context->argument(0));
    float w = qscriptvalue_cast<float>(context->argument(1));
    float4 ret(xyz, w);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_xyz(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4_xyz in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float3 ret = This.xyz();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_LengthSq3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4_LengthSq3 in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float ret = This.LengthSq3();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_Length3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4_Length3 in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float ret = This.Length3();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_LengthSq4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4_LengthSq4 in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float ret = This.LengthSq4();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_Length4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4_Length4 in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float ret = This.Length4();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_Normalize3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4_Normalize3 in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float ret = This.Normalize3();
    ToExistingScriptValue_float4(engine, This, context->thisObject());
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_Normalize4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4_Normalize4 in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float ret = This.Normalize4();
    ToExistingScriptValue_float4(engine, This, context->thisObject());
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_Normalized3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4_Normalized3 in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float4 ret = This.Normalized3();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_Normalized4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4_Normalized4 in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float4 ret = This.Normalized4();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_NormalizeW(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4_NormalizeW in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    bool ret = This.NormalizeW();
    ToExistingScriptValue_float4(engine, This, context->thisObject());
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_IsWZeroOrOne_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4_IsWZeroOrOne_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float epsilon = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.IsWZeroOrOne(epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_IsZero3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4_IsZero3_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float epsilonSq = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.IsZero3(epsilonSq);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_IsZero4_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4_IsZero4_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float epsilonSq = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.IsZero4(epsilonSq);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_IsNormalized3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4_IsNormalized3_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float epsilonSq = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.IsNormalized3(epsilonSq);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_IsNormalized4_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4_IsNormalized4_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float epsilonSq = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.IsNormalized4(epsilonSq);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_Scale3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4_Scale3_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float scalar = qscriptvalue_cast<float>(context->argument(0));
    This.Scale3(scalar);
    ToExistingScriptValue_float4(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float4_ScaleToLength3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4_ScaleToLength3_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float newLength = qscriptvalue_cast<float>(context->argument(0));
    float ret = This.ScaleToLength3(newLength);
    ToExistingScriptValue_float4(engine, This, context->thisObject());
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_IsFinite(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4_IsFinite in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    bool ret = This.IsFinite();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_IsPerpendicular3_float4_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4_IsPerpendicular3_float4_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float4 other = qscriptvalue_cast<float4>(context->argument(0));
    float epsilon = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.IsPerpendicular3(other, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_SumOfElements(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4_SumOfElements in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float ret = This.SumOfElements();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_ProductOfElements(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4_ProductOfElements in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float ret = This.ProductOfElements();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_AverageOfElements(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4_AverageOfElements in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float ret = This.AverageOfElements();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_MinElement(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4_MinElement in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float ret = This.MinElement();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_MinElementIndex(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4_MinElementIndex in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    int ret = This.MinElementIndex();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_MaxElement(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4_MaxElement in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float ret = This.MaxElement();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_MaxElementIndex(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4_MaxElementIndex in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    int ret = This.MaxElementIndex();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_Abs(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4_Abs in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float4 ret = This.Abs();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_Min_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4_Min_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float ceil = qscriptvalue_cast<float>(context->argument(0));
    float4 ret = This.Min(ceil);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_Min_float4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4_Min_float4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float4 ceil = qscriptvalue_cast<float4>(context->argument(0));
    float4 ret = This.Min(ceil);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_Max_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4_Max_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float floor = qscriptvalue_cast<float>(context->argument(0));
    float4 ret = This.Max(floor);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_Max_float4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4_Max_float4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float4 floor = qscriptvalue_cast<float4>(context->argument(0));
    float4 ret = This.Max(floor);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_Clamp_float4_float4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4_Clamp_float4_float4 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float4 floor = qscriptvalue_cast<float4>(context->argument(0));
    float4 ceil = qscriptvalue_cast<float4>(context->argument(1));
    float4 ret = This.Clamp(floor, ceil);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_Clamp01(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4_Clamp01 in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float4 ret = This.Clamp01();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_Clamp_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4_Clamp_float_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float floor = qscriptvalue_cast<float>(context->argument(0));
    float ceil = qscriptvalue_cast<float>(context->argument(1));
    float4 ret = This.Clamp(floor, ceil);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_Lerp_float4_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4_Lerp_float4_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float4 b = qscriptvalue_cast<float4>(context->argument(0));
    float t = qscriptvalue_cast<float>(context->argument(1));
    float4 ret = This.Lerp(b, t);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_Distance3Sq_float4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4_Distance3Sq_float4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float4 rhs = qscriptvalue_cast<float4>(context->argument(0));
    float ret = This.Distance3Sq(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_Distance3_float4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4_Distance3_float4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float4 rhs = qscriptvalue_cast<float4>(context->argument(0));
    float ret = This.Distance3(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_Dot3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4_Dot3_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float3 rhs = qscriptvalue_cast<float3>(context->argument(0));
    float ret = This.Dot3(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_Dot3_float4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4_Dot3_float4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float4 rhs = qscriptvalue_cast<float4>(context->argument(0));
    float ret = This.Dot3(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_Dot4_float4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4_Dot4_float4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float4 rhs = qscriptvalue_cast<float4>(context->argument(0));
    float ret = This.Dot4(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_Cross3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4_Cross3_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float3 rhs = qscriptvalue_cast<float3>(context->argument(0));
    float4 ret = This.Cross3(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_Cross3_float4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4_Cross3_float4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float4 rhs = qscriptvalue_cast<float4>(context->argument(0));
    float4 ret = This.Cross3(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_OuterProduct_float4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4_OuterProduct_float4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float4 rhs = qscriptvalue_cast<float4>(context->argument(0));
    float4x4 ret = This.OuterProduct(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_Perpendicular3_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4_Perpendicular3_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float3 hint = qscriptvalue_cast<float3>(context->argument(0));
    float3 hint2 = qscriptvalue_cast<float3>(context->argument(1));
    float4 ret = This.Perpendicular3(hint, hint2);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_AnotherPerpendicular3_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4_AnotherPerpendicular3_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float3 hint = qscriptvalue_cast<float3>(context->argument(0));
    float3 hint2 = qscriptvalue_cast<float3>(context->argument(1));
    float4 ret = This.AnotherPerpendicular3(hint, hint2);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_Reflect3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4_Reflect3_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float3 normal = qscriptvalue_cast<float3>(context->argument(0));
    float4 ret = This.Reflect3(normal);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_AngleBetween3_float4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4_AngleBetween3_float4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float4 other = qscriptvalue_cast<float4>(context->argument(0));
    float ret = This.AngleBetween3(other);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_AngleBetweenNorm3_float4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4_AngleBetweenNorm3_float4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float4 normalizedVector = qscriptvalue_cast<float4>(context->argument(0));
    float ret = This.AngleBetweenNorm3(normalizedVector);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_AngleBetween4_float4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4_AngleBetween4_float4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float4 other = qscriptvalue_cast<float4>(context->argument(0));
    float ret = This.AngleBetween4(other);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_AngleBetweenNorm4_float4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4_AngleBetweenNorm4_float4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float4 normalizedVector = qscriptvalue_cast<float4>(context->argument(0));
    float ret = This.AngleBetweenNorm4(normalizedVector);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_ProjectTo3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4_ProjectTo3_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float3 target = qscriptvalue_cast<float3>(context->argument(0));
    float4 ret = This.ProjectTo3(target);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_ProjectToNorm3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4_ProjectToNorm3_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float3 target = qscriptvalue_cast<float3>(context->argument(0));
    float4 ret = This.ProjectToNorm3(target);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_SetFromScalar_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4_SetFromScalar_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float scalar = qscriptvalue_cast<float>(context->argument(0));
    This.SetFromScalar(scalar);
    ToExistingScriptValue_float4(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float4_SetFromScalar_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4_SetFromScalar_float_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float scalar = qscriptvalue_cast<float>(context->argument(0));
    float w = qscriptvalue_cast<float>(context->argument(1));
    This.SetFromScalar(scalar, w);
    ToExistingScriptValue_float4(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float4_Set_float_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function float4_Set_float_float_float_float in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float x = qscriptvalue_cast<float>(context->argument(0));
    float y = qscriptvalue_cast<float>(context->argument(1));
    float z = qscriptvalue_cast<float>(context->argument(2));
    float w = qscriptvalue_cast<float>(context->argument(3));
    This.Set(x, y, z, w);
    ToExistingScriptValue_float4(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float4_Equals_float4_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4_Equals_float4_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float4 other = qscriptvalue_cast<float4>(context->argument(0));
    float epsilon = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.Equals(other, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_Equals_float_float_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 5) { printf("Error! Invalid number of arguments passed to function float4_Equals_float_float_float_float_float in file %s, line %d!\nExpected 5, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float x = qscriptvalue_cast<float>(context->argument(0));
    float y = qscriptvalue_cast<float>(context->argument(1));
    float z = qscriptvalue_cast<float>(context->argument(2));
    float w = qscriptvalue_cast<float>(context->argument(3));
    float epsilon = qscriptvalue_cast<float>(context->argument(4));
    bool ret = This.Equals(x, y, z, w, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_Add_float4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4_Add_float4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float4 rhs = qscriptvalue_cast<float4>(context->argument(0));
    float4 ret = This.Add(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_Sub_float4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4_Sub_float4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float4 rhs = qscriptvalue_cast<float4>(context->argument(0));
    float4 ret = This.Sub(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_Mul_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4_Mul_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float rhs = qscriptvalue_cast<float>(context->argument(0));
    float4 ret = This.Mul(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_Div_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4_Div_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float rhs = qscriptvalue_cast<float>(context->argument(0));
    float4 ret = This.Div(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_Neg(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4_Neg in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    float4 ret = This.Neg();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_float4_QVector4D(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4_float4_QVector4D in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    QVector4D other = qscriptvalue_cast<QVector4D>(context->argument(0));
    float4 ret(other);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_toString(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4_toString in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This;
    if (context->argumentCount() > 0) This = qscriptvalue_cast<float4>(context->argument(0)); // Qt oddity (bug?): Sometimes the built-in toString() function doesn't give us this from thisObject, but as the first argument.
    else This = qscriptvalue_cast<float4>(context->thisObject());
    QString ret = This.toString();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_ToQVector4D(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4_ToQVector4D in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 This = qscriptvalue_cast<float4>(context->thisObject());
    QVector4D ret = This.ToQVector4D();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_FromScalar_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4_FromScalar_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float scalar = qscriptvalue_cast<float>(context->argument(0));
    float4 ret = float4::FromScalar(scalar);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_FromScalar_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4_FromScalar_float_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float scalar = qscriptvalue_cast<float>(context->argument(0));
    float w = qscriptvalue_cast<float>(context->argument(1));
    float4 ret = float4::FromScalar(scalar, w);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float4_FromQVector4D_QVector4D(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4_FromQVector4D_QVector4D in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    QVector4D v = qscriptvalue_cast<QVector4D>(context->argument(0));
    float4 ret = float4::FromQVector4D(v);
    return qScriptValueFromValue(engine, ret);
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

void FromScriptValue_float4(const QScriptValue &obj, float4 &value)
{
    value.x = qScriptValueToValue<float>(obj.property("x"));
    value.y = qScriptValueToValue<float>(obj.property("y"));
    value.z = qScriptValueToValue<float>(obj.property("z"));
    value.w = qScriptValueToValue<float>(obj.property("w"));
}

QScriptValue ToScriptValue_float4(QScriptEngine *engine, const float4 &value)
{
    QScriptValue obj = engine->newObject();
    ToExistingScriptValue_float4(engine, value, obj);
    return obj;
}

QScriptValue ToScriptValue_const_float4(QScriptEngine *engine, const float4 &value)
{
    QScriptValue obj = engine->newObject();
    obj.setPrototype(engine->defaultPrototype(qMetaTypeId<float4>()));
    obj.setProperty("x", qScriptValueFromValue(engine, value.x), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("y", qScriptValueFromValue(engine, value.y), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("z", qScriptValueFromValue(engine, value.z), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("w", qScriptValueFromValue(engine, value.w), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    return obj;
}

QScriptValue register_float4_prototype(QScriptEngine *engine)
{
    QScriptValue proto = engine->newObject();
    proto.setProperty("xyz", engine->newFunction(float4_xyz, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("LengthSq3", engine->newFunction(float4_LengthSq3, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Length3", engine->newFunction(float4_Length3, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("LengthSq4", engine->newFunction(float4_LengthSq4, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Length4", engine->newFunction(float4_Length4, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Normalize3", engine->newFunction(float4_Normalize3, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Normalize4", engine->newFunction(float4_Normalize4, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Normalized3", engine->newFunction(float4_Normalized3, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Normalized4", engine->newFunction(float4_Normalized4, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("NormalizeW", engine->newFunction(float4_NormalizeW, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsWZeroOrOne", engine->newFunction(float4_IsWZeroOrOne_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsZero3", engine->newFunction(float4_IsZero3_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsZero4", engine->newFunction(float4_IsZero4_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsNormalized3", engine->newFunction(float4_IsNormalized3_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsNormalized4", engine->newFunction(float4_IsNormalized4_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Scale3", engine->newFunction(float4_Scale3_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ScaleToLength3", engine->newFunction(float4_ScaleToLength3_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsFinite", engine->newFunction(float4_IsFinite, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsPerpendicular3", engine->newFunction(float4_IsPerpendicular3_float4_float, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SumOfElements", engine->newFunction(float4_SumOfElements, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ProductOfElements", engine->newFunction(float4_ProductOfElements, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("AverageOfElements", engine->newFunction(float4_AverageOfElements, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MinElement", engine->newFunction(float4_MinElement, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MinElementIndex", engine->newFunction(float4_MinElementIndex, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MaxElement", engine->newFunction(float4_MaxElement, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MaxElementIndex", engine->newFunction(float4_MaxElementIndex, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Abs", engine->newFunction(float4_Abs, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Min", engine->newFunction(float4_Min_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Max", engine->newFunction(float4_Max_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Clamp", engine->newFunction(float4_Clamp_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Clamp01", engine->newFunction(float4_Clamp01, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Lerp", engine->newFunction(float4_Lerp_float4_float, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Distance3Sq", engine->newFunction(float4_Distance3Sq_float4, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Distance3", engine->newFunction(float4_Distance3_float4, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Dot3", engine->newFunction(float4_Dot3_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Dot4", engine->newFunction(float4_Dot4_float4, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Cross3", engine->newFunction(float4_Cross3_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("OuterProduct", engine->newFunction(float4_OuterProduct_float4, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Perpendicular3", engine->newFunction(float4_Perpendicular3_float3_float3, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("AnotherPerpendicular3", engine->newFunction(float4_AnotherPerpendicular3_float3_float3, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Reflect3", engine->newFunction(float4_Reflect3_float3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("AngleBetween3", engine->newFunction(float4_AngleBetween3_float4, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("AngleBetweenNorm3", engine->newFunction(float4_AngleBetweenNorm3_float4, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("AngleBetween4", engine->newFunction(float4_AngleBetween4_float4, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("AngleBetweenNorm4", engine->newFunction(float4_AngleBetweenNorm4_float4, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ProjectTo3", engine->newFunction(float4_ProjectTo3_float3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ProjectToNorm3", engine->newFunction(float4_ProjectToNorm3_float3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetFromScalar", engine->newFunction(float4_SetFromScalar_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetFromScalar", engine->newFunction(float4_SetFromScalar_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Set", engine->newFunction(float4_Set_float_float_float_float, 4), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Equals", engine->newFunction(float4_Equals_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Equals", engine->newFunction(float4_Equals_selector, 5), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Add", engine->newFunction(float4_Add_float4, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Sub", engine->newFunction(float4_Sub_float4, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Mul", engine->newFunction(float4_Mul_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Div", engine->newFunction(float4_Div_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Neg", engine->newFunction(float4_Neg, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("toString", engine->newFunction(float4_toString, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToQVector4D", engine->newFunction(float4_ToQVector4D, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("metaTypeId", engine->toScriptValue<qint32>((qint32)qMetaTypeId<float4>()));
    engine->setDefaultPrototype(qMetaTypeId<float4>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<float4*>(), proto);
    qScriptRegisterMetaType(engine, ToScriptValue_float4, FromScriptValue_float4, proto);

    QScriptValue ctor = engine->newFunction(float4_ctor, proto, 4);
    ctor.setProperty("FromScalar", engine->newFunction(float4_FromScalar_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromScalar", engine->newFunction(float4_FromScalar_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromQVector4D", engine->newFunction(float4_FromQVector4D_QVector4D, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("zero", qScriptValueFromValue(engine, float4::zero), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("one", qScriptValueFromValue(engine, float4::one), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("unitX", qScriptValueFromValue(engine, float4::unitX), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("unitY", qScriptValueFromValue(engine, float4::unitY), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("unitZ", qScriptValueFromValue(engine, float4::unitZ), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("unitW", qScriptValueFromValue(engine, float4::unitW), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("nan", qScriptValueFromValue(engine, float4::nan), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("inf", qScriptValueFromValue(engine, float4::inf), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    engine->globalObject().setProperty("float4", ctor, QScriptValue::Undeletable | QScriptValue::ReadOnly);

    return ctor;
}

