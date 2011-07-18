#include "QtScriptBindingsHelpers.h"

void ToExistingScriptValue_float3(QScriptEngine *engine, const float3 &value, QScriptValue obj)
{
    obj.setProperty("x", qScriptValueFromValue(engine, value.x), QScriptValue::Undeletable);
    obj.setProperty("y", qScriptValueFromValue(engine, value.y), QScriptValue::Undeletable);
    obj.setProperty("z", qScriptValueFromValue(engine, value.z), QScriptValue::Undeletable);
}

static QScriptValue float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_float3 in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 ret;
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_float3_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 rhs = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_float3_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3_float3_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float x = qscriptvalue_cast<float>(context->argument(0));
    float y = qscriptvalue_cast<float>(context->argument(1));
    float z = qscriptvalue_cast<float>(context->argument(2));
    float3 ret(x, y, z);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_float3_float2_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3_float3_float2_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 xy = qscriptvalue_cast<float2>(context->argument(0));
    float z = qscriptvalue_cast<float>(context->argument(1));
    float3 ret(xy, z);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_xy(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_xy in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float2 ret = This.xy();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_LengthSq(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_LengthSq in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float ret = This.LengthSq();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Length(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_Length in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float ret = This.Length();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Normalize(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_Normalize in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float ret = This.Normalize();
    ToExistingScriptValue_float3(engine, This, context->thisObject());
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Normalized(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_Normalized in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 ret = This.Normalized();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_ScaleToLength_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_ScaleToLength_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float newLength = qscriptvalue_cast<float>(context->argument(0));
    float ret = This.ScaleToLength(newLength);
    ToExistingScriptValue_float3(engine, This, context->thisObject());
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_ScaledToLength_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_ScaledToLength_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float newLength = qscriptvalue_cast<float>(context->argument(0));
    float3 ret = This.ScaledToLength(newLength);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_IsNormalized_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_IsNormalized_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float epsilonSq = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.IsNormalized(epsilonSq);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_IsZero_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_IsZero_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float epsilonSq = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.IsZero(epsilonSq);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_IsFinite(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_IsFinite in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    bool ret = This.IsFinite();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_IsPerpendicular_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3_IsPerpendicular_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 other = qscriptvalue_cast<float3>(context->argument(0));
    float epsilon = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.IsPerpendicular(other, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Equals_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3_Equals_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 rhs = qscriptvalue_cast<float3>(context->argument(0));
    float epsilon = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.Equals(rhs, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Equals_float_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function float3_Equals_float_float_float_float in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float x = qscriptvalue_cast<float>(context->argument(0));
    float y = qscriptvalue_cast<float>(context->argument(1));
    float z = qscriptvalue_cast<float>(context->argument(2));
    float epsilon = qscriptvalue_cast<float>(context->argument(3));
    bool ret = This.Equals(x, y, z, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_SumOfElements(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_SumOfElements in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float ret = This.SumOfElements();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_ProductOfElements(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_ProductOfElements in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float ret = This.ProductOfElements();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_AverageOfElements(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_AverageOfElements in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float ret = This.AverageOfElements();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_MinElement(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_MinElement in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float ret = This.MinElement();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_MinElementIndex(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_MinElementIndex in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    int ret = This.MinElementIndex();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_MaxElement(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_MaxElement in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float ret = This.MaxElement();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_MaxElementIndex(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_MaxElementIndex in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    int ret = This.MaxElementIndex();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Abs(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_Abs in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 ret = This.Abs();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Min_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Min_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float ceil = qscriptvalue_cast<float>(context->argument(0));
    float3 ret = This.Min(ceil);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Min_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Min_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 ceil = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.Min(ceil);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Max_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Max_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float floor = qscriptvalue_cast<float>(context->argument(0));
    float3 ret = This.Max(floor);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Max_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Max_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 floor = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.Max(floor);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Clamp_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3_Clamp_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 floor = qscriptvalue_cast<float3>(context->argument(0));
    float3 ceil = qscriptvalue_cast<float3>(context->argument(1));
    float3 ret = This.Clamp(floor, ceil);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Clamp01(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_Clamp01 in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 ret = This.Clamp01();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Clamp_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3_Clamp_float_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float floor = qscriptvalue_cast<float>(context->argument(0));
    float ceil = qscriptvalue_cast<float>(context->argument(1));
    float3 ret = This.Clamp(floor, ceil);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Distance_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Distance_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 rhs = qscriptvalue_cast<float3>(context->argument(0));
    float ret = This.Distance(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_DistanceSq_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_DistanceSq_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 rhs = qscriptvalue_cast<float3>(context->argument(0));
    float ret = This.DistanceSq(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Dot_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Dot_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 rhs = qscriptvalue_cast<float3>(context->argument(0));
    float ret = This.Dot(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Cross_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Cross_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 rhs = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.Cross(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_OuterProduct_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_OuterProduct_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 rhs = qscriptvalue_cast<float3>(context->argument(0));
    float3x3 ret = This.OuterProduct(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Perpendicular_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3_Perpendicular_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 hint = qscriptvalue_cast<float3>(context->argument(0));
    float3 hint2 = qscriptvalue_cast<float3>(context->argument(1));
    float3 ret = This.Perpendicular(hint, hint2);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_AnotherPerpendicular_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3_AnotherPerpendicular_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 hint = qscriptvalue_cast<float3>(context->argument(0));
    float3 hint2 = qscriptvalue_cast<float3>(context->argument(1));
    float3 ret = This.AnotherPerpendicular(hint, hint2);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Reflect_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Reflect_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 normal = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.Reflect(normal);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Refract_float3_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3_Refract_float3_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 normal = qscriptvalue_cast<float3>(context->argument(0));
    float negativeSideRefractionIndex = qscriptvalue_cast<float>(context->argument(1));
    float positiveSideRefractionIndex = qscriptvalue_cast<float>(context->argument(2));
    float3 ret = This.Refract(normal, negativeSideRefractionIndex, positiveSideRefractionIndex);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_ProjectTo_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_ProjectTo_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 direction = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.ProjectTo(direction);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_ProjectToNorm_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_ProjectToNorm_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 direction = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.ProjectToNorm(direction);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_AngleBetween_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_AngleBetween_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 other = qscriptvalue_cast<float3>(context->argument(0));
    float ret = This.AngleBetween(other);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_AngleBetweenNorm_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_AngleBetweenNorm_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 normalizedVector = qscriptvalue_cast<float3>(context->argument(0));
    float ret = This.AngleBetweenNorm(normalizedVector);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Decompose_float3_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3_Decompose_float3_float3_float3 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 direction = qscriptvalue_cast<float3>(context->argument(0));
    float3 outParallel = qscriptvalue_cast<float3>(context->argument(1));
    float3 outPerpendicular = qscriptvalue_cast<float3>(context->argument(2));
    This.Decompose(direction, outParallel, outPerpendicular);
    return QScriptValue();
}

static QScriptValue float3_Lerp_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3_Lerp_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 b = qscriptvalue_cast<float3>(context->argument(0));
    float t = qscriptvalue_cast<float>(context->argument(1));
    float3 ret = This.Lerp(b, t);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_SetFromScalar_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_SetFromScalar_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float scalar = qscriptvalue_cast<float>(context->argument(0));
    This.SetFromScalar(scalar);
    ToExistingScriptValue_float3(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3_Set_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3_Set_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float x = qscriptvalue_cast<float>(context->argument(0));
    float y = qscriptvalue_cast<float>(context->argument(1));
    float z = qscriptvalue_cast<float>(context->argument(2));
    This.Set(x, y, z);
    ToExistingScriptValue_float3(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3_ToPos4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_ToPos4 in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float4 ret = This.ToPos4();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_ToDir4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_ToDir4 in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float4 ret = This.ToDir4();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Add_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Add_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 rhs = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.Add(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Sub_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Sub_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 rhs = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.Sub(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Mul_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Mul_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float rhs = qscriptvalue_cast<float>(context->argument(0));
    float3 ret = This.Mul(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Div_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Div_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float rhs = qscriptvalue_cast<float>(context->argument(0));
    float3 ret = This.Div(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Neg(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_Neg in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 ret = This.Neg();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Mul_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Mul_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 rhs = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.Mul(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_float3_QVector3D(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_float3_QVector3D in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    QVector3D other = qscriptvalue_cast<QVector3D>(context->argument(0));
    float3 ret(other);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_toString(QScriptContext *context, QScriptEngine *engine)
{
    float3 This;
    if (context->argumentCount() > 0) This = qscriptvalue_cast<float3>(context->argument(0)); // Qt oddity (bug?): Sometimes the built-in toString() function doesn't give us this from thisObject, but as the first argument.
    else This = qscriptvalue_cast<float3>(context->thisObject());
    QString ret = This.toString();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_ToQVector3D(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_ToQVector3D in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    QVector3D ret = This.ToQVector3D();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_ScalarTripleProduct_float3_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3_ScalarTripleProduct_float3_float3_float3 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 u = qscriptvalue_cast<float3>(context->argument(0));
    float3 v = qscriptvalue_cast<float3>(context->argument(1));
    float3 w = qscriptvalue_cast<float3>(context->argument(2));
    float ret = float3::ScalarTripleProduct(u, v, w);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Lerp_float3_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3_Lerp_float3_float3_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 a = qscriptvalue_cast<float3>(context->argument(0));
    float3 b = qscriptvalue_cast<float3>(context->argument(1));
    float t = qscriptvalue_cast<float>(context->argument(2));
    float3 ret = float3::Lerp(a, b, t);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Orthogonalize_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3_Orthogonalize_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 a = qscriptvalue_cast<float3>(context->argument(0));
    float3 b = qscriptvalue_cast<float3>(context->argument(1));
    float3::Orthogonalize(a, b);
    return QScriptValue();
}

static QScriptValue float3_Orthogonalize_float3_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3_Orthogonalize_float3_float3_float3 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 a = qscriptvalue_cast<float3>(context->argument(0));
    float3 b = qscriptvalue_cast<float3>(context->argument(1));
    float3 c = qscriptvalue_cast<float3>(context->argument(2));
    float3::Orthogonalize(a, b, c);
    return QScriptValue();
}

static QScriptValue float3_AreOrthogonal_float3_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3_AreOrthogonal_float3_float3_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 a = qscriptvalue_cast<float3>(context->argument(0));
    float3 b = qscriptvalue_cast<float3>(context->argument(1));
    float epsilon = qscriptvalue_cast<float>(context->argument(2));
    bool ret = float3::AreOrthogonal(a, b, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_AreOrthogonal_float3_float3_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function float3_AreOrthogonal_float3_float3_float3_float in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 a = qscriptvalue_cast<float3>(context->argument(0));
    float3 b = qscriptvalue_cast<float3>(context->argument(1));
    float3 c = qscriptvalue_cast<float3>(context->argument(2));
    float epsilon = qscriptvalue_cast<float>(context->argument(3));
    bool ret = float3::AreOrthogonal(a, b, c, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Orthonormalize_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3_Orthonormalize_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 a = qscriptvalue_cast<float3>(context->argument(0));
    float3 b = qscriptvalue_cast<float3>(context->argument(1));
    float3::Orthonormalize(a, b);
    return QScriptValue();
}

static QScriptValue float3_Orthonormalize_float3_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3_Orthonormalize_float3_float3_float3 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 a = qscriptvalue_cast<float3>(context->argument(0));
    float3 b = qscriptvalue_cast<float3>(context->argument(1));
    float3 c = qscriptvalue_cast<float3>(context->argument(2));
    float3::Orthonormalize(a, b, c);
    return QScriptValue();
}

static QScriptValue float3_AreOrthonormal_float3_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3_AreOrthonormal_float3_float3_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 a = qscriptvalue_cast<float3>(context->argument(0));
    float3 b = qscriptvalue_cast<float3>(context->argument(1));
    float epsilon = qscriptvalue_cast<float>(context->argument(2));
    bool ret = float3::AreOrthonormal(a, b, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_AreOrthonormal_float3_float3_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function float3_AreOrthonormal_float3_float3_float3_float in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 a = qscriptvalue_cast<float3>(context->argument(0));
    float3 b = qscriptvalue_cast<float3>(context->argument(1));
    float3 c = qscriptvalue_cast<float3>(context->argument(2));
    float epsilon = qscriptvalue_cast<float>(context->argument(3));
    bool ret = float3::AreOrthonormal(a, b, c, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_FromScalar_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_FromScalar_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float scalar = qscriptvalue_cast<float>(context->argument(0));
    float3 ret = float3::FromScalar(scalar);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_FromQVector3D_QVector3D(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_FromQVector3D_QVector3D in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    QVector3D v = qscriptvalue_cast<QVector3D>(context->argument(0));
    float3 ret = float3::FromQVector3D(v);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_FromString_QString(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_FromString_QString in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    QString str = qscriptvalue_cast<QString>(context->argument(0));
    float3 ret = float3::FromString(str);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_ctor(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return float3_float3(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return float3_float3_float3(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return float3_float3_float_float_float(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float2>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return float3_float3_float2_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<QVector3D>(context->argument(0)))
        return float3_float3_QVector3D(context, engine);
    printf("float3_ctor failed to choose the right function to call! Did you use 'var x = float3();' instead of 'var x = new float3();'?\n"); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3_Equals_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return float3_Equals_float3_float(context, engine);
    if (context->argumentCount() == 4 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)) && QSVIsOfType<float>(context->argument(3)))
        return float3_Equals_float_float_float_float(context, engine);
    printf("float3_Equals_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3_Min_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return float3_Min_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return float3_Min_float3(context, engine);
    printf("float3_Min_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3_Max_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return float3_Max_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return float3_Max_float3(context, engine);
    printf("float3_Max_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3_Clamp_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return float3_Clamp_float3_float3(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return float3_Clamp_float_float(context, engine);
    printf("float3_Clamp_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3_Lerp_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return float3_Lerp_float3_float(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return float3_Lerp_float3_float3_float(context, engine);
    printf("float3_Lerp_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3_Mul_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return float3_Mul_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return float3_Mul_float3(context, engine);
    printf("float3_Mul_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3_Orthogonalize_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return float3_Orthogonalize_float3_float3(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)))
        return float3_Orthogonalize_float3_float3_float3(context, engine);
    printf("float3_Orthogonalize_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3_AreOrthogonal_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 3 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return float3_AreOrthogonal_float3_float3_float(context, engine);
    if (context->argumentCount() == 4 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)) && QSVIsOfType<float>(context->argument(3)))
        return float3_AreOrthogonal_float3_float3_float3_float(context, engine);
    printf("float3_AreOrthogonal_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3_Orthonormalize_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return float3_Orthonormalize_float3_float3(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)))
        return float3_Orthonormalize_float3_float3_float3(context, engine);
    printf("float3_Orthonormalize_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3_AreOrthonormal_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 3 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return float3_AreOrthonormal_float3_float3_float(context, engine);
    if (context->argumentCount() == 4 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)) && QSVIsOfType<float>(context->argument(3)))
        return float3_AreOrthonormal_float3_float3_float3_float(context, engine);
    printf("float3_AreOrthonormal_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

void FromScriptValue_float3(const QScriptValue &obj, float3 &value)
{
    value.x = qScriptValueToValue<float>(obj.property("x"));
    value.y = qScriptValueToValue<float>(obj.property("y"));
    value.z = qScriptValueToValue<float>(obj.property("z"));
}

QScriptValue ToScriptValue_float3(QScriptEngine *engine, const float3 &value)
{
    QScriptValue obj = engine->newVariant(QVariant::fromValue(value)); // The contents of this variant are NOT used. The real data lies in the data() pointer of this QScriptValue. This only exists to enable overload resolution to work for QObject slots.
    ToExistingScriptValue_float3(engine, value, obj);
    return obj;
}

QScriptValue ToScriptValue_const_float3(QScriptEngine *engine, const float3 &value)
{
    QScriptValue obj = engine->newVariant(QVariant::fromValue(value)); // The contents of this variant are NOT used. The real data lies in the data() pointer of this QScriptValue. This only exists to enable overload resolution to work for QObject slots.
    obj.setPrototype(engine->defaultPrototype(qMetaTypeId<float3>()));
    obj.setProperty("x", qScriptValueFromValue(engine, value.x), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("y", qScriptValueFromValue(engine, value.y), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("z", qScriptValueFromValue(engine, value.z), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    return obj;
}

QScriptValue register_float3_prototype(QScriptEngine *engine)
{
    QScriptValue proto = engine->newObject();
    proto.setProperty("xy", engine->newFunction(float3_xy, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("LengthSq", engine->newFunction(float3_LengthSq, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Length", engine->newFunction(float3_Length, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Normalize", engine->newFunction(float3_Normalize, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Normalized", engine->newFunction(float3_Normalized, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ScaleToLength", engine->newFunction(float3_ScaleToLength_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ScaledToLength", engine->newFunction(float3_ScaledToLength_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsNormalized", engine->newFunction(float3_IsNormalized_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsZero", engine->newFunction(float3_IsZero_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsFinite", engine->newFunction(float3_IsFinite, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsPerpendicular", engine->newFunction(float3_IsPerpendicular_float3_float, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Equals", engine->newFunction(float3_Equals_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Equals", engine->newFunction(float3_Equals_selector, 4), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SumOfElements", engine->newFunction(float3_SumOfElements, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ProductOfElements", engine->newFunction(float3_ProductOfElements, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("AverageOfElements", engine->newFunction(float3_AverageOfElements, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MinElement", engine->newFunction(float3_MinElement, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MinElementIndex", engine->newFunction(float3_MinElementIndex, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MaxElement", engine->newFunction(float3_MaxElement, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MaxElementIndex", engine->newFunction(float3_MaxElementIndex, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Abs", engine->newFunction(float3_Abs, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Min", engine->newFunction(float3_Min_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Max", engine->newFunction(float3_Max_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Clamp", engine->newFunction(float3_Clamp_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Clamp01", engine->newFunction(float3_Clamp01, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Distance", engine->newFunction(float3_Distance_float3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("DistanceSq", engine->newFunction(float3_DistanceSq_float3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Dot", engine->newFunction(float3_Dot_float3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Cross", engine->newFunction(float3_Cross_float3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("OuterProduct", engine->newFunction(float3_OuterProduct_float3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Perpendicular", engine->newFunction(float3_Perpendicular_float3_float3, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("AnotherPerpendicular", engine->newFunction(float3_AnotherPerpendicular_float3_float3, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Reflect", engine->newFunction(float3_Reflect_float3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Refract", engine->newFunction(float3_Refract_float3_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ProjectTo", engine->newFunction(float3_ProjectTo_float3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ProjectToNorm", engine->newFunction(float3_ProjectToNorm_float3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("AngleBetween", engine->newFunction(float3_AngleBetween_float3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("AngleBetweenNorm", engine->newFunction(float3_AngleBetweenNorm_float3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Decompose", engine->newFunction(float3_Decompose_float3_float3_float3, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Lerp", engine->newFunction(float3_Lerp_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetFromScalar", engine->newFunction(float3_SetFromScalar_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Set", engine->newFunction(float3_Set_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToPos4", engine->newFunction(float3_ToPos4, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToDir4", engine->newFunction(float3_ToDir4, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Add", engine->newFunction(float3_Add_float3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Sub", engine->newFunction(float3_Sub_float3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Mul", engine->newFunction(float3_Mul_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Div", engine->newFunction(float3_Div_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Neg", engine->newFunction(float3_Neg, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("toString", engine->newFunction(float3_toString, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToQVector3D", engine->newFunction(float3_ToQVector3D, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("metaTypeId", engine->toScriptValue<qint32>((qint32)qMetaTypeId<float3>()));
    engine->setDefaultPrototype(qMetaTypeId<float3>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<float3*>(), proto);
    qScriptRegisterMetaType(engine, ToScriptValue_float3, FromScriptValue_float3, proto);

    QScriptValue ctor = engine->newFunction(float3_ctor, proto, 3);
    ctor.setProperty("ScalarTripleProduct", engine->newFunction(float3_ScalarTripleProduct_float3_float3_float3, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("Lerp", engine->newFunction(float3_Lerp_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("Orthogonalize", engine->newFunction(float3_Orthogonalize_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("Orthogonalize", engine->newFunction(float3_Orthogonalize_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("AreOrthogonal", engine->newFunction(float3_AreOrthogonal_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("AreOrthogonal", engine->newFunction(float3_AreOrthogonal_selector, 4), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("Orthonormalize", engine->newFunction(float3_Orthonormalize_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("Orthonormalize", engine->newFunction(float3_Orthonormalize_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("AreOrthonormal", engine->newFunction(float3_AreOrthonormal_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("AreOrthonormal", engine->newFunction(float3_AreOrthonormal_selector, 4), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromScalar", engine->newFunction(float3_FromScalar_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromQVector3D", engine->newFunction(float3_FromQVector3D_QVector3D, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromString", engine->newFunction(float3_FromString_QString, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("zero", qScriptValueFromValue(engine, float3::zero), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("one", qScriptValueFromValue(engine, float3::one), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("unitX", qScriptValueFromValue(engine, float3::unitX), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("unitY", qScriptValueFromValue(engine, float3::unitY), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("unitZ", qScriptValueFromValue(engine, float3::unitZ), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("nan", qScriptValueFromValue(engine, float3::nan), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("inf", qScriptValueFromValue(engine, float3::inf), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    engine->globalObject().setProperty("float3", ctor, QScriptValue::Undeletable | QScriptValue::ReadOnly);

    return ctor;
}

