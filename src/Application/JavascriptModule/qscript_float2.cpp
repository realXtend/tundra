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

static QScriptValue float2_float2_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_float2_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float scalar = qscriptvalue_cast<float>(context->argument(0));
    float2 ret(scalar);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_At_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_At_int in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    int index = qscriptvalue_cast<int>(context->argument(0));
    float & ret = This.At(index);
    ToExistingScriptValue_float2(engine, This, context->thisObject());
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_At_int_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_At_int_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    int index = qscriptvalue_cast<int>(context->argument(0));
    CONST_WIN32 float ret = This.At(index);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Add_float2_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_Add_float2_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 v = qscriptvalue_cast<float2>(context->argument(0));
    float2 ret = This.Add(v);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Add_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_Add_float_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float s = qscriptvalue_cast<float>(context->argument(0));
    float2 ret = This.Add(s);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Sub_float2_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_Sub_float2_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 v = qscriptvalue_cast<float2>(context->argument(0));
    float2 ret = This.Sub(v);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Sub_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_Sub_float_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float s = qscriptvalue_cast<float>(context->argument(0));
    float2 ret = This.Sub(s);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_SubLeft_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_SubLeft_float_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float s = qscriptvalue_cast<float>(context->argument(0));
    float2 ret = This.SubLeft(s);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Mul_float2_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_Mul_float2_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 v = qscriptvalue_cast<float2>(context->argument(0));
    float2 ret = This.Mul(v);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Mul_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_Mul_float_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float s = qscriptvalue_cast<float>(context->argument(0));
    float2 ret = This.Mul(s);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Div_float2_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_Div_float2_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 v = qscriptvalue_cast<float2>(context->argument(0));
    float2 ret = This.Div(v);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Div_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_Div_float_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float s = qscriptvalue_cast<float>(context->argument(0));
    float2 ret = This.Div(s);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_DivLeft_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_DivLeft_float_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float s = qscriptvalue_cast<float>(context->argument(0));
    float2 ret = This.DivLeft(s);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_xx_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_xx_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 ret = This.xx();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_xy_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_xy_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 ret = This.xy();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_yx_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_yx_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 ret = This.yx();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_yy_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_yy_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 ret = This.yy();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Swizzled_int_int_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float2_Swizzled_int_int_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    int i = qscriptvalue_cast<int>(context->argument(0));
    int j = qscriptvalue_cast<int>(context->argument(1));
    float2 ret = This.Swizzled(i, j);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Swizzled_int_int_int_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float2_Swizzled_int_int_int_const in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    int i = qscriptvalue_cast<int>(context->argument(0));
    int j = qscriptvalue_cast<int>(context->argument(1));
    int k = qscriptvalue_cast<int>(context->argument(2));
    float3 ret = This.Swizzled(i, j, k);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Swizzled_int_int_int_int_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function float2_Swizzled_int_int_int_int_const in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    int i = qscriptvalue_cast<int>(context->argument(0));
    int j = qscriptvalue_cast<int>(context->argument(1));
    int k = qscriptvalue_cast<int>(context->argument(2));
    int l = qscriptvalue_cast<int>(context->argument(3));
    float4 ret = This.Swizzled(i, j, k, l);
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

static QScriptValue float2_Length_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_Length_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float ret = This.Length();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_LengthSq_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_LengthSq_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float ret = This.LengthSq();
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

static QScriptValue float2_Normalized_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_Normalized_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
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

static QScriptValue float2_ScaledToLength_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_ScaledToLength_float_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float newLength = qscriptvalue_cast<float>(context->argument(0));
    float2 ret = This.ScaledToLength(newLength);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_IsNormalized_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_IsNormalized_float_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float epsilonSq = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.IsNormalized(epsilonSq);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_IsZero_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_IsZero_float_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float epsilonSq = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.IsZero(epsilonSq);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_IsFinite_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_IsFinite_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    bool ret = This.IsFinite();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_IsPerpendicular_float2_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float2_IsPerpendicular_float2_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 other = qscriptvalue_cast<float2>(context->argument(0));
    float epsilon = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.IsPerpendicular(other, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Equals_float2_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float2_Equals_float2_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 other = qscriptvalue_cast<float2>(context->argument(0));
    float epsilon = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.Equals(other, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Equals_float_float_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float2_Equals_float_float_float_const in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float x = qscriptvalue_cast<float>(context->argument(0));
    float y = qscriptvalue_cast<float>(context->argument(1));
    float epsilon = qscriptvalue_cast<float>(context->argument(2));
    bool ret = This.Equals(x, y, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_SumOfElements_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_SumOfElements_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float ret = This.SumOfElements();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_ProductOfElements_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_ProductOfElements_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float ret = This.ProductOfElements();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_AverageOfElements_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_AverageOfElements_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float ret = This.AverageOfElements();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_MinElement_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_MinElement_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float ret = This.MinElement();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_MinElementIndex_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_MinElementIndex_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    int ret = This.MinElementIndex();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_MaxElement_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_MaxElement_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float ret = This.MaxElement();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_MaxElementIndex_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_MaxElementIndex_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    int ret = This.MaxElementIndex();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Abs_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_Abs_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 ret = This.Abs();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Neg_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_Neg_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 ret = This.Neg();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Recip_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_Recip_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 ret = This.Recip();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Min_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_Min_float_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float ceil = qscriptvalue_cast<float>(context->argument(0));
    float2 ret = This.Min(ceil);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Min_float2_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_Min_float2_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 ceil = qscriptvalue_cast<float2>(context->argument(0));
    float2 ret = This.Min(ceil);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Max_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_Max_float_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float floor = qscriptvalue_cast<float>(context->argument(0));
    float2 ret = This.Max(floor);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Max_float2_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_Max_float2_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 floor = qscriptvalue_cast<float2>(context->argument(0));
    float2 ret = This.Max(floor);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Clamp_float_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float2_Clamp_float_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float floor = qscriptvalue_cast<float>(context->argument(0));
    float ceil = qscriptvalue_cast<float>(context->argument(1));
    float2 ret = This.Clamp(floor, ceil);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Clamp_float2_float2_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float2_Clamp_float2_float2_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 floor = qscriptvalue_cast<float2>(context->argument(0));
    float2 ceil = qscriptvalue_cast<float2>(context->argument(1));
    float2 ret = This.Clamp(floor, ceil);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Clamp01_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_Clamp01_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 ret = This.Clamp01();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Distance_float2_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_Distance_float2_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 point = qscriptvalue_cast<float2>(context->argument(0));
    float ret = This.Distance(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_DistanceSq_float2_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_DistanceSq_float2_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 point = qscriptvalue_cast<float2>(context->argument(0));
    float ret = This.DistanceSq(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Dot_float2_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_Dot_float2_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 v = qscriptvalue_cast<float2>(context->argument(0));
    float ret = This.Dot(v);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Perp_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_Perp_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 ret = This.Perp();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_PerpDot_float2_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_PerpDot_float2_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 rhs = qscriptvalue_cast<float2>(context->argument(0));
    float ret = This.PerpDot(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Rotate90CW(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_Rotate90CW in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    This.Rotate90CW();
    ToExistingScriptValue_float2(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float2_Rotated90CW_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_Rotated90CW_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
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

static QScriptValue float2_Rotated90CCW_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_Rotated90CCW_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 ret = This.Rotated90CCW();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Reflect_float2_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_Reflect_float2_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 normal = qscriptvalue_cast<float2>(context->argument(0));
    float2 ret = This.Reflect(normal);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Refract_float2_float_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float2_Refract_float2_float_float_const in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 normal = qscriptvalue_cast<float2>(context->argument(0));
    float negativeSideRefractionIndex = qscriptvalue_cast<float>(context->argument(1));
    float positiveSideRefractionIndex = qscriptvalue_cast<float>(context->argument(2));
    float2 ret = This.Refract(normal, negativeSideRefractionIndex, positiveSideRefractionIndex);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_ProjectTo_float2_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_ProjectTo_float2_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 direction = qscriptvalue_cast<float2>(context->argument(0));
    float2 ret = This.ProjectTo(direction);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_ProjectToNorm_float2_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_ProjectToNorm_float2_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 direction = qscriptvalue_cast<float2>(context->argument(0));
    float2 ret = This.ProjectToNorm(direction);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_AngleBetween_float2_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_AngleBetween_float2_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 other = qscriptvalue_cast<float2>(context->argument(0));
    float ret = This.AngleBetween(other);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_AngleBetweenNorm_float2_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_AngleBetweenNorm_float2_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 normalizedVector = qscriptvalue_cast<float2>(context->argument(0));
    float ret = This.AngleBetweenNorm(normalizedVector);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Decompose_float2_float2_float2_const(QScriptContext *context, QScriptEngine * /*engine*/)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float2_Decompose_float2_float2_float2_const in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 direction = qscriptvalue_cast<float2>(context->argument(0));
    float2 outParallel = qscriptvalue_cast<float2>(context->argument(1));
    float2 outPerpendicular = qscriptvalue_cast<float2>(context->argument(2));
    This.Decompose(direction, outParallel, outPerpendicular);
    return QScriptValue();
}

static QScriptValue float2_Lerp_float2_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float2_Lerp_float2_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    float2 b = qscriptvalue_cast<float2>(context->argument(0));
    float t = qscriptvalue_cast<float>(context->argument(1));
    float2 ret = This.Lerp(b, t);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_float2_QVector2D(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_float2_QVector2D in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    QVector2D other = qscriptvalue_cast<QVector2D>(context->argument(0));
    float2 ret(other);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_toString_const(QScriptContext *context, QScriptEngine *engine)
{
    float2 This;
    if (context->argumentCount() > 0) This = qscriptvalue_cast<float2>(context->argument(0)); // Qt oddity (bug?): Sometimes the built-in toString() function doesn't give us this from thisObject, but as the first argument.
    else This = qscriptvalue_cast<float2>(context->thisObject());
    QString ret = This.toString();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_ToQVector2D_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float2_ToQVector2D_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 This = qscriptvalue_cast<float2>(context->thisObject());
    QVector2D ret = This.ToQVector2D();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_FromScalar_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float2_FromScalar_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float scalar = qscriptvalue_cast<float>(context->argument(0));
    float2 ret = float2::FromScalar(scalar);
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

static QScriptValue float2_Orthogonalize_float2_float2(QScriptContext *context, QScriptEngine * /*engine*/)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float2_Orthogonalize_float2_float2 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 a = qscriptvalue_cast<float2>(context->argument(0));
    float2 b = qscriptvalue_cast<float2>(context->argument(1));
    float2::Orthogonalize(a, b);
    return QScriptValue();
}

static QScriptValue float2_AreOrthogonal_float2_float2_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float2_AreOrthogonal_float2_float2_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 a = qscriptvalue_cast<float2>(context->argument(0));
    float2 b = qscriptvalue_cast<float2>(context->argument(1));
    float epsilon = qscriptvalue_cast<float>(context->argument(2));
    bool ret = float2::AreOrthogonal(a, b, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float2_Orthonormalize_float2_float2(QScriptContext *context, QScriptEngine * /*engine*/)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float2_Orthonormalize_float2_float2 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float2 a = qscriptvalue_cast<float2>(context->argument(0));
    float2 b = qscriptvalue_cast<float2>(context->argument(1));
    float2::Orthonormalize(a, b);
    return QScriptValue();
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

static QScriptValue float2_RandomDir_LCG_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float2_RandomDir_LCG_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LCG lcg = qscriptvalue_cast<LCG>(context->argument(0));
    float length = qscriptvalue_cast<float>(context->argument(1));
    float2 ret = float2::RandomDir(lcg, length);
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
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return float2_float2_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<QVector2D>(context->argument(0)))
        return float2_float2_QVector2D(context, engine);
    printf("float2_ctor failed to choose the right function to call! Did you use 'var x = float2();' instead of 'var x = new float2();'?\n"); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float2_At_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<int>(context->argument(0)))
        return float2_At_int(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<int>(context->argument(0)))
        return float2_At_int_const(context, engine);
    printf("float2_At_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float2_Add_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float2>(context->argument(0)))
        return float2_Add_float2_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return float2_Add_float_const(context, engine);
    printf("float2_Add_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float2_Sub_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float2>(context->argument(0)))
        return float2_Sub_float2_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return float2_Sub_float_const(context, engine);
    printf("float2_Sub_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float2_Mul_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float2>(context->argument(0)))
        return float2_Mul_float2_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return float2_Mul_float_const(context, engine);
    printf("float2_Mul_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float2_Div_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float2>(context->argument(0)))
        return float2_Div_float2_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return float2_Div_float_const(context, engine);
    printf("float2_Div_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float2_Swizzled_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<int>(context->argument(0)) && QSVIsOfType<int>(context->argument(1)))
        return float2_Swizzled_int_int_const(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<int>(context->argument(0)) && QSVIsOfType<int>(context->argument(1)) && QSVIsOfType<int>(context->argument(2)))
        return float2_Swizzled_int_int_int_const(context, engine);
    if (context->argumentCount() == 4 && QSVIsOfType<int>(context->argument(0)) && QSVIsOfType<int>(context->argument(1)) && QSVIsOfType<int>(context->argument(2)) && QSVIsOfType<int>(context->argument(3)))
        return float2_Swizzled_int_int_int_int_const(context, engine);
    printf("float2_Swizzled_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float2_Equals_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float2>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return float2_Equals_float2_float_const(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return float2_Equals_float_float_float_const(context, engine);
    printf("float2_Equals_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float2_Min_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return float2_Min_float_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float2>(context->argument(0)))
        return float2_Min_float2_const(context, engine);
    printf("float2_Min_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float2_Max_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return float2_Max_float_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float2>(context->argument(0)))
        return float2_Max_float2_const(context, engine);
    printf("float2_Max_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float2_Clamp_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return float2_Clamp_float_float_const(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float2>(context->argument(0)) && QSVIsOfType<float2>(context->argument(1)))
        return float2_Clamp_float2_float2_const(context, engine);
    printf("float2_Clamp_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float2_Lerp_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float2>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return float2_Lerp_float2_float_const(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float2>(context->argument(0)) && QSVIsOfType<float2>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return float2_Lerp_float2_float2_float(context, engine);
    printf("float2_Lerp_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
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
    proto.setProperty("At", engine->newFunction(float2_At_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Add", engine->newFunction(float2_Add_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Sub", engine->newFunction(float2_Sub_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SubLeft", engine->newFunction(float2_SubLeft_float_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Mul", engine->newFunction(float2_Mul_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Div", engine->newFunction(float2_Div_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("DivLeft", engine->newFunction(float2_DivLeft_float_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("xx", engine->newFunction(float2_xx_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("xy", engine->newFunction(float2_xy_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("yx", engine->newFunction(float2_yx_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("yy", engine->newFunction(float2_yy_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Swizzled", engine->newFunction(float2_Swizzled_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Swizzled", engine->newFunction(float2_Swizzled_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Swizzled", engine->newFunction(float2_Swizzled_selector, 4), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetFromScalar", engine->newFunction(float2_SetFromScalar_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Set", engine->newFunction(float2_Set_float_float, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Length", engine->newFunction(float2_Length_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("LengthSq", engine->newFunction(float2_LengthSq_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Normalize", engine->newFunction(float2_Normalize, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Normalized", engine->newFunction(float2_Normalized_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ScaleToLength", engine->newFunction(float2_ScaleToLength_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ScaledToLength", engine->newFunction(float2_ScaledToLength_float_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsNormalized", engine->newFunction(float2_IsNormalized_float_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsZero", engine->newFunction(float2_IsZero_float_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsFinite", engine->newFunction(float2_IsFinite_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsPerpendicular", engine->newFunction(float2_IsPerpendicular_float2_float_const, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Equals", engine->newFunction(float2_Equals_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Equals", engine->newFunction(float2_Equals_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SumOfElements", engine->newFunction(float2_SumOfElements_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ProductOfElements", engine->newFunction(float2_ProductOfElements_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("AverageOfElements", engine->newFunction(float2_AverageOfElements_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MinElement", engine->newFunction(float2_MinElement_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MinElementIndex", engine->newFunction(float2_MinElementIndex_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MaxElement", engine->newFunction(float2_MaxElement_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MaxElementIndex", engine->newFunction(float2_MaxElementIndex_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Abs", engine->newFunction(float2_Abs_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Neg", engine->newFunction(float2_Neg_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Recip", engine->newFunction(float2_Recip_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Min", engine->newFunction(float2_Min_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Max", engine->newFunction(float2_Max_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Clamp", engine->newFunction(float2_Clamp_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Clamp01", engine->newFunction(float2_Clamp01_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Distance", engine->newFunction(float2_Distance_float2_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("DistanceSq", engine->newFunction(float2_DistanceSq_float2_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Dot", engine->newFunction(float2_Dot_float2_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Perp", engine->newFunction(float2_Perp_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("PerpDot", engine->newFunction(float2_PerpDot_float2_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Rotate90CW", engine->newFunction(float2_Rotate90CW, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Rotated90CW", engine->newFunction(float2_Rotated90CW_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Rotate90CCW", engine->newFunction(float2_Rotate90CCW, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Rotated90CCW", engine->newFunction(float2_Rotated90CCW_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Reflect", engine->newFunction(float2_Reflect_float2_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Refract", engine->newFunction(float2_Refract_float2_float_float_const, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ProjectTo", engine->newFunction(float2_ProjectTo_float2_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ProjectToNorm", engine->newFunction(float2_ProjectToNorm_float2_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("AngleBetween", engine->newFunction(float2_AngleBetween_float2_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("AngleBetweenNorm", engine->newFunction(float2_AngleBetweenNorm_float2_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Decompose", engine->newFunction(float2_Decompose_float2_float2_float2_const, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Lerp", engine->newFunction(float2_Lerp_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("toString", engine->newFunction(float2_toString_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToQVector2D", engine->newFunction(float2_ToQVector2D_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("metaTypeId", engine->toScriptValue<qint32>((qint32)qMetaTypeId<float2>()));
    engine->setDefaultPrototype(qMetaTypeId<float2>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<float2*>(), proto);
    qScriptRegisterMetaType(engine, ToScriptValue_float2, FromScriptValue_float2, proto);

    QScriptValue ctor = engine->newFunction(float2_ctor, proto, 2);
    ctor.setProperty("FromScalar", engine->newFunction(float2_FromScalar_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("Lerp", engine->newFunction(float2_Lerp_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("Orthogonalize", engine->newFunction(float2_Orthogonalize_float2_float2, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("AreOrthogonal", engine->newFunction(float2_AreOrthogonal_float2_float2_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("Orthonormalize", engine->newFunction(float2_Orthonormalize_float2_float2, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("OrientedCCW", engine->newFunction(float2_OrientedCCW_float2_float2_float2, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("RandomDir", engine->newFunction(float2_RandomDir_LCG_float, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
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

