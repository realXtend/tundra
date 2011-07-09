#include "QtScriptBindingsHelpers.h"

void ToExistingScriptValue_float2(QScriptEngine *engine, const float2 &value, QScriptValue obj)
{
    obj.setProperty("x", qScriptValueFromValue(engine, value.x), QScriptValue::Undeletable);
    obj.setProperty("y", qScriptValueFromValue(engine, value.y), QScriptValue::Undeletable);
}

static QScriptValue float2_float2(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_float2 in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 ret;
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_float2_float2(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_float2_float2 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 rhs = qscriptvalue_cast<float2>(context->argument(0));
    float2 ret(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_float2_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float2_float2_float_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float x = qscriptvalue_cast<float>(context->argument(0));
    float y = qscriptvalue_cast<float>(context->argument(1));
    float2 ret(x, y);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_LengthSq(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_LengthSq in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float ret = This.LengthSq();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Length(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_Length in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float ret = This.Length();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Normalize(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_Normalize in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float ret = This.Normalize();
    ToExistingScriptValue_float2(engine, This, context->thisObject());
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Normalized(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_Normalized in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 ret = This.Normalized();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_ScaleToLength_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_ScaleToLength_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float newLength = qscriptvalue_cast<float>(context->argument(0));
    float ret = This.ScaleToLength(newLength);
    ToExistingScriptValue_float2(engine, This, context->thisObject());
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_ScaledToLength_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_ScaledToLength_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float newLength = qscriptvalue_cast<float>(context->argument(0));
    float2 ret = This.ScaledToLength(newLength);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_IsNormalized_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_IsNormalized_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float epsilonSq = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.IsNormalized(epsilonSq);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_IsZero_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_IsZero_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float epsilonSq = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.IsZero(epsilonSq);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_IsFinite(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_IsFinite in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    bool ret = This.IsFinite();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_IsPerpendicular_float2_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float2_IsPerpendicular_float2_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 other = qscriptvalue_cast<float2>(context->argument(0));
    float epsilon = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.IsPerpendicular(other, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Equals_float2_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float2_Equals_float2_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 rhs = qscriptvalue_cast<float2>(context->argument(0));
    float epsilon = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.Equals(rhs, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Equals_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float2_Equals_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float x = qscriptvalue_cast<float>(context->argument(0));
    float y = qscriptvalue_cast<float>(context->argument(1));
    float epsilon = qscriptvalue_cast<float>(context->argument(2));
    bool ret = This.Equals(x, y, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_SumOfElements(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_SumOfElements in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float ret = This.SumOfElements();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_ProductOfElements(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_ProductOfElements in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float ret = This.ProductOfElements();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_AverageOfElements(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_AverageOfElements in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float ret = This.AverageOfElements();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_MinElement(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_MinElement in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float ret = This.MinElement();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_MinElementIndex(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_MinElementIndex in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    int ret = This.MinElementIndex();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_MaxElement(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_MaxElement in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float ret = This.MaxElement();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_MaxElementIndex(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_MaxElementIndex in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    int ret = This.MaxElementIndex();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Abs(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_Abs in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 ret = This.Abs();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Min_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_Min_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float ceil = qscriptvalue_cast<float>(context->argument(0));
    float2 ret = This.Min(ceil);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Min_float2(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_Min_float2 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 ceil = qscriptvalue_cast<float2>(context->argument(0));
    float2 ret = This.Min(ceil);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Max_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_Max_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float floor = qscriptvalue_cast<float>(context->argument(0));
    float2 ret = This.Max(floor);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Max_float2(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_Max_float2 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 floor = qscriptvalue_cast<float2>(context->argument(0));
    float2 ret = This.Max(floor);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Clamp_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float2_Clamp_float_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float floor = qscriptvalue_cast<float>(context->argument(0));
    float ceil = qscriptvalue_cast<float>(context->argument(1));
    float2 ret = This.Clamp(floor, ceil);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Clamp_float2_float2(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float2_Clamp_float2_float2 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 floor = qscriptvalue_cast<float2>(context->argument(0));
    float2 ceil = qscriptvalue_cast<float2>(context->argument(1));
    float2 ret = This.Clamp(floor, ceil);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Clamp01(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_Clamp01 in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 ret = This.Clamp01();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Distance_float2(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_Distance_float2 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 rhs = qscriptvalue_cast<float2>(context->argument(0));
    float ret = This.Distance(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_DistanceSq_float2(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_DistanceSq_float2 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 rhs = qscriptvalue_cast<float2>(context->argument(0));
    float ret = This.DistanceSq(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Dot_float2(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_Dot_float2 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 rhs = qscriptvalue_cast<float2>(context->argument(0));
    float ret = This.Dot(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_PerpDot_float2(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_PerpDot_float2 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 rhs = qscriptvalue_cast<float2>(context->argument(0));
    float ret = This.PerpDot(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Reflect_float2(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_Reflect_float2 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 normal = qscriptvalue_cast<float2>(context->argument(0));
    float2 ret = This.Reflect(normal);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Refract_float2_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float2_Refract_float2_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 normal = qscriptvalue_cast<float2>(context->argument(0));
    float negativeSideRefractionIndex = qscriptvalue_cast<float>(context->argument(1));
    float positiveSideRefractionIndex = qscriptvalue_cast<float>(context->argument(2));
    float2 ret = This.Refract(normal, negativeSideRefractionIndex, positiveSideRefractionIndex);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_ProjectTo_float2(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_ProjectTo_float2 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 direction = qscriptvalue_cast<float2>(context->argument(0));
    float2 ret = This.ProjectTo(direction);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_ProjectToNorm_float2(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_ProjectToNorm_float2 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 direction = qscriptvalue_cast<float2>(context->argument(0));
    float2 ret = This.ProjectToNorm(direction);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_AngleBetween_float2(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_AngleBetween_float2 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 other = qscriptvalue_cast<float2>(context->argument(0));
    float ret = This.AngleBetween(other);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_AngleBetweenNorm_float2(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_AngleBetweenNorm_float2 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 normalizedVector = qscriptvalue_cast<float2>(context->argument(0));
    float ret = This.AngleBetweenNorm(normalizedVector);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Decompose_float2_float2_float2(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float2_Decompose_float2_float2_float2 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 direction = qscriptvalue_cast<float2>(context->argument(0));
    float2 outParallel = qscriptvalue_cast<float2>(context->argument(1));
    float2 outPerpendicular = qscriptvalue_cast<float2>(context->argument(2));
    This.Decompose(direction, outParallel, outPerpendicular);
    return QScriptValue();
}

static QScriptValue float2_Lerp_float2_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float2_Lerp_float2_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 b = qscriptvalue_cast<float2>(context->argument(0));
    float t = qscriptvalue_cast<float>(context->argument(1));
    float2 ret = This.Lerp(b, t);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_SetFromScalar_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_SetFromScalar_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float scalar = qscriptvalue_cast<float>(context->argument(0));
    This.SetFromScalar(scalar);
    ToExistingScriptValue_float2(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float2_Set_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float2_Set_float_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float x = qscriptvalue_cast<float>(context->argument(0));
    float y = qscriptvalue_cast<float>(context->argument(1));
    This.Set(x, y);
    ToExistingScriptValue_float2(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float2_Rotate90CW(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_Rotate90CW in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    This.Rotate90CW();
    ToExistingScriptValue_float2(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float2_Rotated90CW(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_Rotated90CW in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 ret = This.Rotated90CW();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Rotate90CCW(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_Rotate90CCW in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    This.Rotate90CCW();
    ToExistingScriptValue_float2(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float2_Rotated90CCW(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_Rotated90CCW in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 ret = This.Rotated90CCW();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Add_float2(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_Add_float2 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 rhs = qscriptvalue_cast<float2>(context->argument(0));
    float2 ret = This.Add(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Sub_float2(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_Sub_float2 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 rhs = qscriptvalue_cast<float2>(context->argument(0));
    float2 ret = This.Sub(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Mul_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_Mul_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float rhs = qscriptvalue_cast<float>(context->argument(0));
    float2 ret = This.Mul(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Div_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_Div_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float rhs = qscriptvalue_cast<float>(context->argument(0));
    float2 ret = This.Div(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Neg(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_Neg in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 ret = This.Neg();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Mul_float2(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_Mul_float2 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 rhs = qscriptvalue_cast<float2>(context->argument(0));
    float2 ret = This.Mul(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_float2_QVector2D(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_float2_QVector2D in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    QVector2D other = qscriptvalue_cast<QVector2D>(context->argument(0));
    float2 ret(other);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_toString(QScriptContext *context, QScriptEngine *engine)
{
    float2 This;
    if (context->argumentCount() > 0) This = qscriptvalue_cast<float2>(context->argument(0)); // Qt oddity (bug?): Sometimes the built-in toString() function doesn't give us this from thisObject, but as the first argument.
    else This = qscriptvalue_cast<float2>(context->thisObject());
    QString ret = This.toString();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_ToQVector2D(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_ToQVector2D in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    QVector2D ret = This.ToQVector2D();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Lerp_float2_float2_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float2_Lerp_float2_float2_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 a = qscriptvalue_cast<float2>(context->argument(0));
    float2 b = qscriptvalue_cast<float2>(context->argument(1));
    float t = qscriptvalue_cast<float>(context->argument(2));
    float2 ret = float2::Lerp(a, b, t);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Orthogonalize_float2_float2(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float2_Orthogonalize_float2_float2 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 a = qscriptvalue_cast<float2>(context->argument(0));
    float2 b = qscriptvalue_cast<float2>(context->argument(1));
    float2::Orthogonalize(a, b);
    return QScriptValue();
}

static QScriptValue float2_Orthonormalize_float2_float2(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float2_Orthonormalize_float2_float2 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 a = qscriptvalue_cast<float2>(context->argument(0));
    float2 b = qscriptvalue_cast<float2>(context->argument(1));
    float2::Orthonormalize(a, b);
    return QScriptValue();
}

static QScriptValue float2_FromScalar_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_FromScalar_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float scalar = qscriptvalue_cast<float>(context->argument(0));
    float2 ret = float2::FromScalar(scalar);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_OrientedCCW_float2_float2_float2(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float2_OrientedCCW_float2_float2_float2 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 a = qscriptvalue_cast<float2>(context->argument(0));
    float2 b = qscriptvalue_cast<float2>(context->argument(1));
    float2 c = qscriptvalue_cast<float2>(context->argument(2));
    bool ret = float2::OrientedCCW(a, b, c);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_FromQVector2D_QVector2D(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_FromQVector2D_QVector2D in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    QVector2D v = qscriptvalue_cast<QVector2D>(context->argument(0));
    float2 ret = float2::FromQVector2D(v);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_FromString_QString(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_FromString_QString in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    QString str = qscriptvalue_cast<QString>(context->argument(0));
    float2 ret = float2::FromString(str);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_ctor(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return float2_float2(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float2>(context->argument(0)))
        return float2_float2_float2(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return float2_float2_float_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<QVector2D>(context->argument(0)))
        return float2_float2_QVector2D(context, engine);
    printf("float2_ctor failed to choose the right function to call! Did you use 'var x = float2();' instead of 'var x = new float2();'?\n"); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float2_Equals_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float2>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return float2_Equals_float2_float(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return float2_Equals_float_float_float(context, engine);
    printf("float2_Equals_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float2_Min_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return float2_Min_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float2>(context->argument(0)))
        return float2_Min_float2(context, engine);
    printf("float2_Min_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float2_Max_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return float2_Max_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float2>(context->argument(0)))
        return float2_Max_float2(context, engine);
    printf("float2_Max_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float2_Clamp_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return float2_Clamp_float_float(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float2>(context->argument(0)) && QSVIsOfType<float2>(context->argument(1)))
        return float2_Clamp_float2_float2(context, engine);
    printf("float2_Clamp_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float2_Lerp_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float2>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return float2_Lerp_float2_float(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float2>(context->argument(0)) && QSVIsOfType<float2>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return float2_Lerp_float2_float2_float(context, engine);
    printf("float2_Lerp_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float2_Mul_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return float2_Mul_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float2>(context->argument(0)))
        return float2_Mul_float2(context, engine);
    printf("float2_Mul_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

void FromScriptValue_float2(const QScriptValue &obj, float2 &value)
{
    value.x = qScriptValueToValue<float>(obj.property("x"));
    value.y = qScriptValueToValue<float>(obj.property("y"));
}

QScriptValue ToScriptValue_float2(QScriptEngine *engine, const float2 &value)
{
    QScriptValue obj = engine->newVariant(QVariant::fromValue(value)); // The contents of this variant are NOT used. The real data lies in the data() pointer of this QScriptValue. This only exists to enable overload resolution to work for QObject slots.
    ToExistingScriptValue_float2(engine, value, obj);
    return obj;
}

QScriptValue ToScriptValue_const_float2(QScriptEngine *engine, const float2 &value)
{
    QScriptValue obj = engine->newVariant(QVariant::fromValue(value)); // The contents of this variant are NOT used. The real data lies in the data() pointer of this QScriptValue. This only exists to enable overload resolution to work for QObject slots.
    obj.setPrototype(engine->defaultPrototype(qMetaTypeId<float2>()));
    obj.setProperty("x", qScriptValueFromValue(engine, value.x), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("y", qScriptValueFromValue(engine, value.y), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    return obj;
}

QScriptValue register_float2_prototype(QScriptEngine *engine)
{
    QScriptValue proto = engine->newObject();
    proto.setProperty("LengthSq", engine->newFunction(float2_LengthSq, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Length", engine->newFunction(float2_Length, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Normalize", engine->newFunction(float2_Normalize, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Normalized", engine->newFunction(float2_Normalized, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ScaleToLength", engine->newFunction(float2_ScaleToLength_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ScaledToLength", engine->newFunction(float2_ScaledToLength_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsNormalized", engine->newFunction(float2_IsNormalized_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsZero", engine->newFunction(float2_IsZero_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsFinite", engine->newFunction(float2_IsFinite, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsPerpendicular", engine->newFunction(float2_IsPerpendicular_float2_float, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Equals", engine->newFunction(float2_Equals_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Equals", engine->newFunction(float2_Equals_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SumOfElements", engine->newFunction(float2_SumOfElements, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ProductOfElements", engine->newFunction(float2_ProductOfElements, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("AverageOfElements", engine->newFunction(float2_AverageOfElements, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MinElement", engine->newFunction(float2_MinElement, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MinElementIndex", engine->newFunction(float2_MinElementIndex, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MaxElement", engine->newFunction(float2_MaxElement, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MaxElementIndex", engine->newFunction(float2_MaxElementIndex, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Abs", engine->newFunction(float2_Abs, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Min", engine->newFunction(float2_Min_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Max", engine->newFunction(float2_Max_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Clamp", engine->newFunction(float2_Clamp_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Clamp01", engine->newFunction(float2_Clamp01, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Distance", engine->newFunction(float2_Distance_float2, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("DistanceSq", engine->newFunction(float2_DistanceSq_float2, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Dot", engine->newFunction(float2_Dot_float2, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("PerpDot", engine->newFunction(float2_PerpDot_float2, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Reflect", engine->newFunction(float2_Reflect_float2, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Refract", engine->newFunction(float2_Refract_float2_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ProjectTo", engine->newFunction(float2_ProjectTo_float2, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ProjectToNorm", engine->newFunction(float2_ProjectToNorm_float2, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("AngleBetween", engine->newFunction(float2_AngleBetween_float2, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("AngleBetweenNorm", engine->newFunction(float2_AngleBetweenNorm_float2, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Decompose", engine->newFunction(float2_Decompose_float2_float2_float2, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Lerp", engine->newFunction(float2_Lerp_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetFromScalar", engine->newFunction(float2_SetFromScalar_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Set", engine->newFunction(float2_Set_float_float, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Rotate90CW", engine->newFunction(float2_Rotate90CW, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Rotated90CW", engine->newFunction(float2_Rotated90CW, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Rotate90CCW", engine->newFunction(float2_Rotate90CCW, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Rotated90CCW", engine->newFunction(float2_Rotated90CCW, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Add", engine->newFunction(float2_Add_float2, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Sub", engine->newFunction(float2_Sub_float2, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Mul", engine->newFunction(float2_Mul_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Div", engine->newFunction(float2_Div_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Neg", engine->newFunction(float2_Neg, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("toString", engine->newFunction(float2_toString, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToQVector2D", engine->newFunction(float2_ToQVector2D, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("metaTypeId", engine->toScriptValue<qint32>((qint32)qMetaTypeId<float2>()));
    engine->setDefaultPrototype(qMetaTypeId<float2>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<float2*>(), proto);
    qScriptRegisterMetaType(engine, ToScriptValue_float2, FromScriptValue_float2, proto);

    QScriptValue ctor = engine->newFunction(float2_ctor, proto, 2);
    ctor.setProperty("Lerp", engine->newFunction(float2_Lerp_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("Orthogonalize", engine->newFunction(float2_Orthogonalize_float2_float2, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("Orthonormalize", engine->newFunction(float2_Orthonormalize_float2_float2, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromScalar", engine->newFunction(float2_FromScalar_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("OrientedCCW", engine->newFunction(float2_OrientedCCW_float2_float2_float2, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromQVector2D", engine->newFunction(float2_FromQVector2D_QVector2D, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromString", engine->newFunction(float2_FromString_QString, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("zero", qScriptValueFromValue(engine, float2::zero), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("one", qScriptValueFromValue(engine, float2::one), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("unitX", qScriptValueFromValue(engine, float2::unitX), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("unitY", qScriptValueFromValue(engine, float2::unitY), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("nan", qScriptValueFromValue(engine, float2::nan), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("inf", qScriptValueFromValue(engine, float2::inf), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    engine->globalObject().setProperty("float2", ctor, QScriptValue::Undeletable | QScriptValue::ReadOnly);

    return ctor;
}

