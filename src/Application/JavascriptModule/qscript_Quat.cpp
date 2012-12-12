#include "QtScriptBindingsHelpers.h"

void ToExistingScriptValue_Quat(QScriptEngine *engine, const Quat &value, QScriptValue obj)
{
    obj.setProperty("x", qScriptValueFromValue(engine, value.x), QScriptValue::Undeletable);
    obj.setProperty("y", qScriptValueFromValue(engine, value.y), QScriptValue::Undeletable);
    obj.setProperty("z", qScriptValueFromValue(engine, value.z), QScriptValue::Undeletable);
    obj.setProperty("w", qScriptValueFromValue(engine, value.w), QScriptValue::Undeletable);
}

static QScriptValue Quat_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_Quat in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat ret;
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_Quat_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_Quat_Quat in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat rhs = qscriptvalue_cast<Quat>(context->argument(0));
    Quat ret(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_Quat_float3x3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_Quat_float3x3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 rotationMatrix = qscriptvalue_cast<float3x3>(context->argument(0));
    Quat ret(rotationMatrix);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_Quat_float3x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_Quat_float3x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 rotationMatrix = qscriptvalue_cast<float3x4>(context->argument(0));
    Quat ret(rotationMatrix);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_Quat_float4x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_Quat_float4x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float4x4 rotationMatrix = qscriptvalue_cast<float4x4>(context->argument(0));
    Quat ret(rotationMatrix);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_Quat_float_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function Quat_Quat_float_float_float_float in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float x = qscriptvalue_cast<float>(context->argument(0));
    float y = qscriptvalue_cast<float>(context->argument(1));
    float z = qscriptvalue_cast<float>(context->argument(2));
    float w = qscriptvalue_cast<float>(context->argument(3));
    Quat ret(x, y, z, w);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_Quat_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Quat_Quat_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 rotationAxis = qscriptvalue_cast<float3>(context->argument(0));
    float rotationAngleRadians = qscriptvalue_cast<float>(context->argument(1));
    Quat ret(rotationAxis, rotationAngleRadians);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_WorldX_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_WorldX_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float3 ret = This.WorldX();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_WorldY_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_WorldY_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float3 ret = This.WorldY();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_WorldZ_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_WorldZ_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float3 ret = This.WorldZ();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_Axis_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_Axis_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float3 ret = This.Axis();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_Angle_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_Angle_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float ret = This.Angle();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_Dot_Quat_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_Dot_Quat_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    Quat rhs = qscriptvalue_cast<Quat>(context->argument(0));
    float ret = This.Dot(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_LengthSq_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_LengthSq_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float ret = This.LengthSq();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_Length_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_Length_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float ret = This.Length();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_Normalize(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_Normalize in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float ret = This.Normalize();
    ToExistingScriptValue_Quat(engine, This, context->thisObject());
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_Normalized_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_Normalized_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    Quat ret = This.Normalized();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_IsNormalized_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_IsNormalized_float_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float epsilon = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.IsNormalized(epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_IsInvertible_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_IsInvertible_float_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float epsilon = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.IsInvertible(epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_IsFinite_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_IsFinite_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    bool ret = This.IsFinite();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_Equals_Quat_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Quat_Equals_Quat_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    Quat rhs = qscriptvalue_cast<Quat>(context->argument(0));
    float epsilon = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.Equals(rhs, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_Inverse(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_Inverse in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    This.Inverse();
    ToExistingScriptValue_Quat(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Quat_Inverted_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_Inverted_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    Quat ret = This.Inverted();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_InverseAndNormalize(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_InverseAndNormalize in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float ret = This.InverseAndNormalize();
    ToExistingScriptValue_Quat(engine, This, context->thisObject());
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_Conjugate(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_Conjugate in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    This.Conjugate();
    ToExistingScriptValue_Quat(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Quat_Conjugated_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_Conjugated_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    Quat ret = This.Conjugated();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_Transform_float_float_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Quat_Transform_float_float_float_const in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float x = qscriptvalue_cast<float>(context->argument(0));
    float y = qscriptvalue_cast<float>(context->argument(1));
    float z = qscriptvalue_cast<float>(context->argument(2));
    float3 ret = This.Transform(x, y, z);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_Transform_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_Transform_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float3 vec = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.Transform(vec);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_Transform_float4_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_Transform_float4_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float4 vec = qscriptvalue_cast<float4>(context->argument(0));
    float4 ret = This.Transform(vec);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_Lerp_Quat_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Quat_Lerp_Quat_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    Quat target = qscriptvalue_cast<Quat>(context->argument(0));
    float t = qscriptvalue_cast<float>(context->argument(1));
    Quat ret = This.Lerp(target, t);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_Slerp_Quat_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Quat_Slerp_Quat_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    Quat target = qscriptvalue_cast<Quat>(context->argument(0));
    float t = qscriptvalue_cast<float>(context->argument(1));
    Quat ret = This.Slerp(target, t);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_AngleBetween_Quat_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_AngleBetween_Quat_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    Quat target = qscriptvalue_cast<Quat>(context->argument(0));
    float ret = This.AngleBetween(target);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_AxisFromTo_Quat_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_AxisFromTo_Quat_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    Quat target = qscriptvalue_cast<Quat>(context->argument(0));
    float3 ret = This.AxisFromTo(target);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_ToAxisAngle_float3_float_const(QScriptContext *context, QScriptEngine * /*engine*/)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Quat_ToAxisAngle_float3_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float3 rotationAxis = qscriptvalue_cast<float3>(context->argument(0));
    float rotationAngleRadians = qscriptvalue_cast<float>(context->argument(1));
    This.ToAxisAngle(rotationAxis, rotationAngleRadians);
    return QScriptValue();
}

static QScriptValue Quat_SetFromAxisAngle_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Quat_SetFromAxisAngle_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float3 rotationAxis = qscriptvalue_cast<float3>(context->argument(0));
    float rotationAngleRadians = qscriptvalue_cast<float>(context->argument(1));
    This.SetFromAxisAngle(rotationAxis, rotationAngleRadians);
    ToExistingScriptValue_Quat(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Quat_Set_float3x3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_Set_float3x3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float3x3 matrix = qscriptvalue_cast<float3x3>(context->argument(0));
    This.Set(matrix);
    ToExistingScriptValue_Quat(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Quat_Set_float3x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_Set_float3x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float3x4 matrix = qscriptvalue_cast<float3x4>(context->argument(0));
    This.Set(matrix);
    ToExistingScriptValue_Quat(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Quat_Set_float4x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_Set_float4x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float4x4 matrix = qscriptvalue_cast<float4x4>(context->argument(0));
    This.Set(matrix);
    ToExistingScriptValue_Quat(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Quat_Set_float_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function Quat_Set_float_float_float_float in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float x = qscriptvalue_cast<float>(context->argument(0));
    float y = qscriptvalue_cast<float>(context->argument(1));
    float z = qscriptvalue_cast<float>(context->argument(2));
    float w = qscriptvalue_cast<float>(context->argument(3));
    This.Set(x, y, z, w);
    ToExistingScriptValue_Quat(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue Quat_ToEulerXYX_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_ToEulerXYX_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float3 ret = This.ToEulerXYX();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_ToEulerXZX_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_ToEulerXZX_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float3 ret = This.ToEulerXZX();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_ToEulerYXY_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_ToEulerYXY_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float3 ret = This.ToEulerYXY();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_ToEulerYZY_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_ToEulerYZY_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float3 ret = This.ToEulerYZY();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_ToEulerZXZ_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_ToEulerZXZ_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float3 ret = This.ToEulerZXZ();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_ToEulerZYZ_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_ToEulerZYZ_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float3 ret = This.ToEulerZYZ();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_ToEulerXYZ_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_ToEulerXYZ_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float3 ret = This.ToEulerXYZ();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_ToEulerXZY_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_ToEulerXZY_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float3 ret = This.ToEulerXZY();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_ToEulerYXZ_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_ToEulerYXZ_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float3 ret = This.ToEulerYXZ();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_ToEulerYZX_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_ToEulerYZX_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float3 ret = This.ToEulerYZX();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_ToEulerZXY_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_ToEulerZXY_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float3 ret = This.ToEulerZXY();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_ToEulerZYX_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_ToEulerZYX_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float3 ret = This.ToEulerZYX();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_ToFloat3x3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_ToFloat3x3_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float3x3 ret = This.ToFloat3x3();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_ToFloat3x4_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_ToFloat3x4_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float3x4 ret = This.ToFloat3x4();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_ToFloat4x4_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_ToFloat4x4_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float4x4 ret = This.ToFloat4x4();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_Quat_QQuaternion(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_Quat_QQuaternion in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    QQuaternion other = qscriptvalue_cast<QQuaternion>(context->argument(0));
    Quat ret(other);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_toString_const(QScriptContext *context, QScriptEngine *engine)
{
    Quat This;
    if (context->argumentCount() > 0) This = qscriptvalue_cast<Quat>(context->argument(0)); // Qt oddity (bug?): Sometimes the built-in toString() function doesn't give us this from thisObject, but as the first argument.
    else This = qscriptvalue_cast<Quat>(context->thisObject());
    QString ret = This.toString();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_ToQQuaternion_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_ToQQuaternion_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    QQuaternion ret = This.ToQQuaternion();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_Mul_Quat_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_Mul_Quat_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    Quat rhs = qscriptvalue_cast<Quat>(context->argument(0));
    Quat ret = This.Mul(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_Mul_float3x3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_Mul_float3x3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float3x3 rhs = qscriptvalue_cast<float3x3>(context->argument(0));
    Quat ret = This.Mul(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_Mul_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_Mul_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float3 vector = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.Mul(vector);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_Mul_float4_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_Mul_float4_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat This = qscriptvalue_cast<Quat>(context->thisObject());
    float4 vector = qscriptvalue_cast<float4>(context->argument(0));
    float4 ret = This.Mul(vector);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_Lerp_Quat_Quat_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Quat_Lerp_Quat_Quat_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat source = qscriptvalue_cast<Quat>(context->argument(0));
    Quat target = qscriptvalue_cast<Quat>(context->argument(1));
    float t = qscriptvalue_cast<float>(context->argument(2));
    Quat ret = Quat::Lerp(source, target, t);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_Slerp_Quat_Quat_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Quat_Slerp_Quat_Quat_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat source = qscriptvalue_cast<Quat>(context->argument(0));
    Quat target = qscriptvalue_cast<Quat>(context->argument(1));
    float t = qscriptvalue_cast<float>(context->argument(2));
    Quat ret = Quat::Slerp(source, target, t);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_LookAt_float3_float3_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function Quat_LookAt_float3_float3_float3_float3 in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 localForward = qscriptvalue_cast<float3>(context->argument(0));
    float3 targetDirection = qscriptvalue_cast<float3>(context->argument(1));
    float3 localUp = qscriptvalue_cast<float3>(context->argument(2));
    float3 worldUp = qscriptvalue_cast<float3>(context->argument(3));
    Quat ret = Quat::LookAt(localForward, targetDirection, localUp, worldUp);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_RotateX_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_RotateX_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float angleRadians = qscriptvalue_cast<float>(context->argument(0));
    Quat ret = Quat::RotateX(angleRadians);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_RotateY_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_RotateY_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float angleRadians = qscriptvalue_cast<float>(context->argument(0));
    Quat ret = Quat::RotateY(angleRadians);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_RotateZ_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_RotateZ_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float angleRadians = qscriptvalue_cast<float>(context->argument(0));
    Quat ret = Quat::RotateZ(angleRadians);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_RotateAxisAngle_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Quat_RotateAxisAngle_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 axisDirection = qscriptvalue_cast<float3>(context->argument(0));
    float angleRadians = qscriptvalue_cast<float>(context->argument(1));
    Quat ret = Quat::RotateAxisAngle(axisDirection, angleRadians);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_RotateFromTo_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Quat_RotateFromTo_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 sourceDirection = qscriptvalue_cast<float3>(context->argument(0));
    float3 targetDirection = qscriptvalue_cast<float3>(context->argument(1));
    Quat ret = Quat::RotateFromTo(sourceDirection, targetDirection);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_RotateFromTo_float3_float3_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function Quat_RotateFromTo_float3_float3_float3_float3 in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 sourceDirection = qscriptvalue_cast<float3>(context->argument(0));
    float3 targetDirection = qscriptvalue_cast<float3>(context->argument(1));
    float3 sourceDirection2 = qscriptvalue_cast<float3>(context->argument(2));
    float3 targetDirection2 = qscriptvalue_cast<float3>(context->argument(3));
    Quat ret = Quat::RotateFromTo(sourceDirection, targetDirection, sourceDirection2, targetDirection2);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_FromEulerXYX_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Quat_FromEulerXYX_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float x2 = qscriptvalue_cast<float>(context->argument(0));
    float y = qscriptvalue_cast<float>(context->argument(1));
    float x = qscriptvalue_cast<float>(context->argument(2));
    Quat ret = Quat::FromEulerXYX(x2, y, x);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_FromEulerXZX_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Quat_FromEulerXZX_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float x2 = qscriptvalue_cast<float>(context->argument(0));
    float z = qscriptvalue_cast<float>(context->argument(1));
    float x = qscriptvalue_cast<float>(context->argument(2));
    Quat ret = Quat::FromEulerXZX(x2, z, x);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_FromEulerYXY_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Quat_FromEulerYXY_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float y2 = qscriptvalue_cast<float>(context->argument(0));
    float x = qscriptvalue_cast<float>(context->argument(1));
    float y = qscriptvalue_cast<float>(context->argument(2));
    Quat ret = Quat::FromEulerYXY(y2, x, y);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_FromEulerYZY_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Quat_FromEulerYZY_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float y2 = qscriptvalue_cast<float>(context->argument(0));
    float z = qscriptvalue_cast<float>(context->argument(1));
    float y = qscriptvalue_cast<float>(context->argument(2));
    Quat ret = Quat::FromEulerYZY(y2, z, y);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_FromEulerZXZ_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Quat_FromEulerZXZ_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float z2 = qscriptvalue_cast<float>(context->argument(0));
    float x = qscriptvalue_cast<float>(context->argument(1));
    float z = qscriptvalue_cast<float>(context->argument(2));
    Quat ret = Quat::FromEulerZXZ(z2, x, z);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_FromEulerZYZ_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Quat_FromEulerZYZ_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float z2 = qscriptvalue_cast<float>(context->argument(0));
    float y = qscriptvalue_cast<float>(context->argument(1));
    float z = qscriptvalue_cast<float>(context->argument(2));
    Quat ret = Quat::FromEulerZYZ(z2, y, z);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_FromEulerXYZ_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Quat_FromEulerXYZ_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float x = qscriptvalue_cast<float>(context->argument(0));
    float y = qscriptvalue_cast<float>(context->argument(1));
    float z = qscriptvalue_cast<float>(context->argument(2));
    Quat ret = Quat::FromEulerXYZ(x, y, z);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_FromEulerXZY_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Quat_FromEulerXZY_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float x = qscriptvalue_cast<float>(context->argument(0));
    float z = qscriptvalue_cast<float>(context->argument(1));
    float y = qscriptvalue_cast<float>(context->argument(2));
    Quat ret = Quat::FromEulerXZY(x, z, y);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_FromEulerYXZ_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Quat_FromEulerYXZ_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float y = qscriptvalue_cast<float>(context->argument(0));
    float x = qscriptvalue_cast<float>(context->argument(1));
    float z = qscriptvalue_cast<float>(context->argument(2));
    Quat ret = Quat::FromEulerYXZ(y, x, z);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_FromEulerYZX_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Quat_FromEulerYZX_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float y = qscriptvalue_cast<float>(context->argument(0));
    float z = qscriptvalue_cast<float>(context->argument(1));
    float x = qscriptvalue_cast<float>(context->argument(2));
    Quat ret = Quat::FromEulerYZX(y, z, x);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_FromEulerZXY_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Quat_FromEulerZXY_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float z = qscriptvalue_cast<float>(context->argument(0));
    float x = qscriptvalue_cast<float>(context->argument(1));
    float y = qscriptvalue_cast<float>(context->argument(2));
    Quat ret = Quat::FromEulerZXY(z, x, y);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_FromEulerZYX_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Quat_FromEulerZYX_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float z = qscriptvalue_cast<float>(context->argument(0));
    float y = qscriptvalue_cast<float>(context->argument(1));
    float x = qscriptvalue_cast<float>(context->argument(2));
    Quat ret = Quat::FromEulerZYX(z, y, x);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_RandomRotation_LCG(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_RandomRotation_LCG in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LCG lcg = qscriptvalue_cast<LCG>(context->argument(0));
    Quat ret = Quat::RandomRotation(lcg);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_FromQQuaternion_QQuaternion(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_FromQQuaternion_QQuaternion in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    QQuaternion q = qscriptvalue_cast<QQuaternion>(context->argument(0));
    Quat ret = Quat::FromQQuaternion(q);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_FromString_QString(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_FromString_QString in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    QString str = qscriptvalue_cast<QString>(context->argument(0));
    Quat ret = Quat::FromString(str);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Quat_ctor(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return Quat_Quat(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Quat>(context->argument(0)))
        return Quat_Quat_Quat(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3x3>(context->argument(0)))
        return Quat_Quat_float3x3(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3x4>(context->argument(0)))
        return Quat_Quat_float3x4(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float4x4>(context->argument(0)))
        return Quat_Quat_float4x4(context, engine);
    if (context->argumentCount() == 4 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)) && QSVIsOfType<float>(context->argument(3)))
        return Quat_Quat_float_float_float_float(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return Quat_Quat_float3_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<QQuaternion>(context->argument(0)))
        return Quat_Quat_QQuaternion(context, engine);
    printf("Quat_ctor failed to choose the right function to call! Did you use 'var x = Quat();' instead of 'var x = new Quat();'?\n"); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Quat_Transform_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 3 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return Quat_Transform_float_float_float_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return Quat_Transform_float3_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float4>(context->argument(0)))
        return Quat_Transform_float4_const(context, engine);
    printf("Quat_Transform_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Quat_Lerp_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<Quat>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return Quat_Lerp_Quat_float_const(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<Quat>(context->argument(0)) && QSVIsOfType<Quat>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return Quat_Lerp_Quat_Quat_float(context, engine);
    printf("Quat_Lerp_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Quat_Slerp_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<Quat>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return Quat_Slerp_Quat_float_const(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<Quat>(context->argument(0)) && QSVIsOfType<Quat>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return Quat_Slerp_Quat_Quat_float(context, engine);
    printf("Quat_Slerp_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Quat_Set_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3x3>(context->argument(0)))
        return Quat_Set_float3x3(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3x4>(context->argument(0)))
        return Quat_Set_float3x4(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float4x4>(context->argument(0)))
        return Quat_Set_float4x4(context, engine);
    if (context->argumentCount() == 4 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)) && QSVIsOfType<float>(context->argument(3)))
        return Quat_Set_float_float_float_float(context, engine);
    printf("Quat_Set_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Quat_Mul_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<Quat>(context->argument(0)))
        return Quat_Mul_Quat_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3x3>(context->argument(0)))
        return Quat_Mul_float3x3_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return Quat_Mul_float3_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float4>(context->argument(0)))
        return Quat_Mul_float4_const(context, engine);
    printf("Quat_Mul_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue Quat_RotateFromTo_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return Quat_RotateFromTo_float3_float3(context, engine);
    if (context->argumentCount() == 4 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)) && QSVIsOfType<float3>(context->argument(3)))
        return Quat_RotateFromTo_float3_float3_float3_float3(context, engine);
    printf("Quat_RotateFromTo_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

void FromScriptValue_Quat(const QScriptValue &obj, Quat &value)
{
    value.x = qScriptValueToValue<float>(obj.property("x"));
    value.y = qScriptValueToValue<float>(obj.property("y"));
    value.z = qScriptValueToValue<float>(obj.property("z"));
    value.w = qScriptValueToValue<float>(obj.property("w"));
}

QScriptValue ToScriptValue_Quat(QScriptEngine *engine, const Quat &value)
{
    QScriptValue obj = engine->newVariant(QVariant::fromValue(value)); // The contents of this variant are NOT used. The real data lies in the data() pointer of this QScriptValue. This only exists to enable overload resolution to work for QObject slots.
    ToExistingScriptValue_Quat(engine, value, obj);
    return obj;
}

QScriptValue ToScriptValue_const_Quat(QScriptEngine *engine, const Quat &value)
{
    QScriptValue obj = engine->newVariant(QVariant::fromValue(value)); // The contents of this variant are NOT used. The real data lies in the data() pointer of this QScriptValue. This only exists to enable overload resolution to work for QObject slots.
    obj.setPrototype(engine->defaultPrototype(qMetaTypeId<Quat>()));
    obj.setProperty("x", qScriptValueFromValue(engine, value.x), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("y", qScriptValueFromValue(engine, value.y), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("z", qScriptValueFromValue(engine, value.z), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("w", qScriptValueFromValue(engine, value.w), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    return obj;
}

QScriptValue register_Quat_prototype(QScriptEngine *engine)
{
    QScriptValue proto = engine->newObject();
    proto.setProperty("WorldX", engine->newFunction(Quat_WorldX_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("WorldY", engine->newFunction(Quat_WorldY_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("WorldZ", engine->newFunction(Quat_WorldZ_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Axis", engine->newFunction(Quat_Axis_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Angle", engine->newFunction(Quat_Angle_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Dot", engine->newFunction(Quat_Dot_Quat_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("LengthSq", engine->newFunction(Quat_LengthSq_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Length", engine->newFunction(Quat_Length_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Normalize", engine->newFunction(Quat_Normalize, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Normalized", engine->newFunction(Quat_Normalized_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsNormalized", engine->newFunction(Quat_IsNormalized_float_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsInvertible", engine->newFunction(Quat_IsInvertible_float_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsFinite", engine->newFunction(Quat_IsFinite_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Equals", engine->newFunction(Quat_Equals_Quat_float_const, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Inverse", engine->newFunction(Quat_Inverse, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Inverted", engine->newFunction(Quat_Inverted_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("InverseAndNormalize", engine->newFunction(Quat_InverseAndNormalize, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Conjugate", engine->newFunction(Quat_Conjugate, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Conjugated", engine->newFunction(Quat_Conjugated_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Transform", engine->newFunction(Quat_Transform_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Transform", engine->newFunction(Quat_Transform_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Lerp", engine->newFunction(Quat_Lerp_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Slerp", engine->newFunction(Quat_Slerp_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("AngleBetween", engine->newFunction(Quat_AngleBetween_Quat_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("AxisFromTo", engine->newFunction(Quat_AxisFromTo_Quat_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToAxisAngle", engine->newFunction(Quat_ToAxisAngle_float3_float_const, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetFromAxisAngle", engine->newFunction(Quat_SetFromAxisAngle_float3_float, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Set", engine->newFunction(Quat_Set_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Set", engine->newFunction(Quat_Set_selector, 4), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToEulerXYX", engine->newFunction(Quat_ToEulerXYX_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToEulerXZX", engine->newFunction(Quat_ToEulerXZX_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToEulerYXY", engine->newFunction(Quat_ToEulerYXY_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToEulerYZY", engine->newFunction(Quat_ToEulerYZY_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToEulerZXZ", engine->newFunction(Quat_ToEulerZXZ_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToEulerZYZ", engine->newFunction(Quat_ToEulerZYZ_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToEulerXYZ", engine->newFunction(Quat_ToEulerXYZ_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToEulerXZY", engine->newFunction(Quat_ToEulerXZY_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToEulerYXZ", engine->newFunction(Quat_ToEulerYXZ_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToEulerYZX", engine->newFunction(Quat_ToEulerYZX_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToEulerZXY", engine->newFunction(Quat_ToEulerZXY_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToEulerZYX", engine->newFunction(Quat_ToEulerZYX_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToFloat3x3", engine->newFunction(Quat_ToFloat3x3_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToFloat3x4", engine->newFunction(Quat_ToFloat3x4_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToFloat4x4", engine->newFunction(Quat_ToFloat4x4_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("toString", engine->newFunction(Quat_toString_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToQQuaternion", engine->newFunction(Quat_ToQQuaternion_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Mul", engine->newFunction(Quat_Mul_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("metaTypeId", engine->toScriptValue<qint32>((qint32)qMetaTypeId<Quat>()));
    engine->setDefaultPrototype(qMetaTypeId<Quat>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<Quat*>(), proto);
    qScriptRegisterMetaType(engine, ToScriptValue_Quat, FromScriptValue_Quat, proto);

    QScriptValue ctor = engine->newFunction(Quat_ctor, proto, 4);
    ctor.setProperty("Lerp", engine->newFunction(Quat_Lerp_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("Slerp", engine->newFunction(Quat_Slerp_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("LookAt", engine->newFunction(Quat_LookAt_float3_float3_float3_float3, 4), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("RotateX", engine->newFunction(Quat_RotateX_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("RotateY", engine->newFunction(Quat_RotateY_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("RotateZ", engine->newFunction(Quat_RotateZ_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("RotateAxisAngle", engine->newFunction(Quat_RotateAxisAngle_float3_float, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("RotateFromTo", engine->newFunction(Quat_RotateFromTo_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("RotateFromTo", engine->newFunction(Quat_RotateFromTo_selector, 4), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromEulerXYX", engine->newFunction(Quat_FromEulerXYX_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromEulerXZX", engine->newFunction(Quat_FromEulerXZX_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromEulerYXY", engine->newFunction(Quat_FromEulerYXY_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromEulerYZY", engine->newFunction(Quat_FromEulerYZY_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromEulerZXZ", engine->newFunction(Quat_FromEulerZXZ_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromEulerZYZ", engine->newFunction(Quat_FromEulerZYZ_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromEulerXYZ", engine->newFunction(Quat_FromEulerXYZ_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromEulerXZY", engine->newFunction(Quat_FromEulerXZY_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromEulerYXZ", engine->newFunction(Quat_FromEulerYXZ_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromEulerYZX", engine->newFunction(Quat_FromEulerYZX_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromEulerZXY", engine->newFunction(Quat_FromEulerZXY_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromEulerZYX", engine->newFunction(Quat_FromEulerZYX_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("RandomRotation", engine->newFunction(Quat_RandomRotation_LCG, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromQQuaternion", engine->newFunction(Quat_FromQQuaternion_QQuaternion, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromString", engine->newFunction(Quat_FromString_QString, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("identity", qScriptValueFromValue(engine, Quat::identity), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("nan", qScriptValueFromValue(engine, Quat::nan), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    engine->globalObject().setProperty("Quat", ctor, QScriptValue::Undeletable | QScriptValue::ReadOnly);

    return ctor;
}

