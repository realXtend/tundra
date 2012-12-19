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

static QScriptValue float3_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_float3_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float scalar = qscriptvalue_cast<float>(context->argument(0));
    float3 ret(scalar);
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

static QScriptValue float3_At_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_At_int in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    int index = qscriptvalue_cast<int>(context->argument(0));
    float & ret = This.At(index);
    ToExistingScriptValue_float3(engine, This, context->thisObject());
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_At_int_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_At_int_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    int index = qscriptvalue_cast<int>(context->argument(0));
    CONST_WIN32 float ret = This.At(index);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Add_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Add_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 v = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.Add(v);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Add_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Add_float_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float s = qscriptvalue_cast<float>(context->argument(0));
    float3 ret = This.Add(s);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Sub_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Sub_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 v = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.Sub(v);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Sub_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Sub_float_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float s = qscriptvalue_cast<float>(context->argument(0));
    float3 ret = This.Sub(s);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_SubLeft_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_SubLeft_float_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float s = qscriptvalue_cast<float>(context->argument(0));
    float3 ret = This.SubLeft(s);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Mul_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Mul_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 v = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.Mul(v);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Mul_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Mul_float_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float s = qscriptvalue_cast<float>(context->argument(0));
    float3 ret = This.Mul(s);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Div_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Div_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 v = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.Div(v);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Div_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Div_float_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float s = qscriptvalue_cast<float>(context->argument(0));
    float3 ret = This.Div(s);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_DivLeft_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_DivLeft_float_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float s = qscriptvalue_cast<float>(context->argument(0));
    float3 ret = This.DivLeft(s);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_xx_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_xx_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float2 ret = This.xx();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_xy_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_xy_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float2 ret = This.xy();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_xz_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_xz_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float2 ret = This.xz();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_yx_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_yx_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float2 ret = This.yx();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_yy_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_yy_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float2 ret = This.yy();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_yz_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_yz_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float2 ret = This.yz();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_zx_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_zx_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float2 ret = This.zx();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_zy_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_zy_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float2 ret = This.zy();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_zz_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_zz_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float2 ret = This.zz();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_xxx_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_xxx_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 ret = This.xxx();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_xxy_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_xxy_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 ret = This.xxy();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_xxz_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_xxz_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 ret = This.xxz();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_xyx_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_xyx_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 ret = This.xyx();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_xyy_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_xyy_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 ret = This.xyy();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_xyz_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_xyz_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 ret = This.xyz();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_xzx_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_xzx_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 ret = This.xzx();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_xzy_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_xzy_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 ret = This.xzy();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_xzz_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_xzz_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 ret = This.xzz();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_yxx_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_yxx_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 ret = This.yxx();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_yxy_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_yxy_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 ret = This.yxy();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_yxz_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_yxz_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 ret = This.yxz();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_yyx_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_yyx_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 ret = This.yyx();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_yyy_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_yyy_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 ret = This.yyy();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_yyz_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_yyz_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 ret = This.yyz();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_yzx_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_yzx_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 ret = This.yzx();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_yzy_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_yzy_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 ret = This.yzy();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_yzz_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_yzz_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 ret = This.yzz();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_zxx_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_zxx_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 ret = This.zxx();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_zxy_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_zxy_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 ret = This.zxy();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_zxz_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_zxz_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 ret = This.zxz();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_zyx_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_zyx_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 ret = This.zyx();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_zyy_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_zyy_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 ret = This.zyy();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_zyz_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_zyz_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 ret = This.zyz();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_zzx_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_zzx_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 ret = This.zzx();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_zzy_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_zzy_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 ret = This.zzy();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_zzz_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_zzz_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 ret = This.zzz();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Swizzled_int_int_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3_Swizzled_int_int_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    int i = qscriptvalue_cast<int>(context->argument(0));
    int j = qscriptvalue_cast<int>(context->argument(1));
    float2 ret = This.Swizzled(i, j);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Swizzled_int_int_int_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3_Swizzled_int_int_int_const in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    int i = qscriptvalue_cast<int>(context->argument(0));
    int j = qscriptvalue_cast<int>(context->argument(1));
    int k = qscriptvalue_cast<int>(context->argument(2));
    float3 ret = This.Swizzled(i, j, k);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Swizzled_int_int_int_int_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function float3_Swizzled_int_int_int_int_const in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    int i = qscriptvalue_cast<int>(context->argument(0));
    int j = qscriptvalue_cast<int>(context->argument(1));
    int k = qscriptvalue_cast<int>(context->argument(2));
    int l = qscriptvalue_cast<int>(context->argument(3));
    float4 ret = This.Swizzled(i, j, k, l);
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

static QScriptValue float3_ToPos4_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_ToPos4_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float4 ret = This.ToPos4();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_ToDir4_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_ToDir4_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float4 ret = This.ToDir4();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Length_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_Length_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float ret = This.Length();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_LengthSq_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_LengthSq_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float ret = This.LengthSq();
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

static QScriptValue float3_Normalized_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_Normalized_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
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

static QScriptValue float3_ScaledToLength_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_ScaledToLength_float_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float newLength = qscriptvalue_cast<float>(context->argument(0));
    float3 ret = This.ScaledToLength(newLength);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_IsNormalized_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_IsNormalized_float_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float epsilonSq = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.IsNormalized(epsilonSq);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_IsZero_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_IsZero_float_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float epsilonSq = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.IsZero(epsilonSq);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_IsFinite_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_IsFinite_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    bool ret = This.IsFinite();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_IsPerpendicular_float3_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3_IsPerpendicular_float3_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 other = qscriptvalue_cast<float3>(context->argument(0));
    float epsilon = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.IsPerpendicular(other, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Equals_float3_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3_Equals_float3_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 other = qscriptvalue_cast<float3>(context->argument(0));
    float epsilon = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.Equals(other, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Equals_float_float_float_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function float3_Equals_float_float_float_float_const in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float x = qscriptvalue_cast<float>(context->argument(0));
    float y = qscriptvalue_cast<float>(context->argument(1));
    float z = qscriptvalue_cast<float>(context->argument(2));
    float epsilon = qscriptvalue_cast<float>(context->argument(3));
    bool ret = This.Equals(x, y, z, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_SumOfElements_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_SumOfElements_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float ret = This.SumOfElements();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_ProductOfElements_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_ProductOfElements_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float ret = This.ProductOfElements();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_AverageOfElements_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_AverageOfElements_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float ret = This.AverageOfElements();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_MinElement_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_MinElement_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float ret = This.MinElement();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_MinElementIndex_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_MinElementIndex_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    int ret = This.MinElementIndex();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_MaxElement_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_MaxElement_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float ret = This.MaxElement();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_MaxElementIndex_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_MaxElementIndex_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    int ret = This.MaxElementIndex();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Abs_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_Abs_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 ret = This.Abs();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Neg_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_Neg_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 ret = This.Neg();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Recip_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_Recip_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 ret = This.Recip();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Min_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Min_float_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float ceil = qscriptvalue_cast<float>(context->argument(0));
    float3 ret = This.Min(ceil);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Min_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Min_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 ceil = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.Min(ceil);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Max_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Max_float_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float floor = qscriptvalue_cast<float>(context->argument(0));
    float3 ret = This.Max(floor);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Max_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Max_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 floor = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.Max(floor);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Clamp_float_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3_Clamp_float_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float floor = qscriptvalue_cast<float>(context->argument(0));
    float ceil = qscriptvalue_cast<float>(context->argument(1));
    float3 ret = This.Clamp(floor, ceil);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Clamp_float3_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3_Clamp_float3_float3_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 floor = qscriptvalue_cast<float3>(context->argument(0));
    float3 ceil = qscriptvalue_cast<float3>(context->argument(1));
    float3 ret = This.Clamp(floor, ceil);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Clamp01_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_Clamp01_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 ret = This.Clamp01();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Distance_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Distance_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float ret = This.Distance(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Distance_Line_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Distance_Line_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    Line line = qscriptvalue_cast<Line>(context->argument(0));
    float ret = This.Distance(line);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Distance_Ray_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Distance_Ray_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    Ray ray = qscriptvalue_cast<Ray>(context->argument(0));
    float ret = This.Distance(ray);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Distance_LineSegment_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Distance_LineSegment_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    LineSegment lineSegment = qscriptvalue_cast<LineSegment>(context->argument(0));
    float ret = This.Distance(lineSegment);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Distance_Plane_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Distance_Plane_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    Plane plane = qscriptvalue_cast<Plane>(context->argument(0));
    float ret = This.Distance(plane);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Distance_Triangle_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Distance_Triangle_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    Triangle triangle = qscriptvalue_cast<Triangle>(context->argument(0));
    float ret = This.Distance(triangle);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Distance_AABB_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Distance_AABB_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    AABB aabb = qscriptvalue_cast<AABB>(context->argument(0));
    float ret = This.Distance(aabb);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Distance_OBB_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Distance_OBB_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    OBB obb = qscriptvalue_cast<OBB>(context->argument(0));
    float ret = This.Distance(obb);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Distance_Sphere_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Distance_Sphere_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    Sphere sphere = qscriptvalue_cast<Sphere>(context->argument(0));
    float ret = This.Distance(sphere);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Distance_Capsule_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Distance_Capsule_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    Capsule capsule = qscriptvalue_cast<Capsule>(context->argument(0));
    float ret = This.Distance(capsule);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_DistanceSq_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_DistanceSq_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 point = qscriptvalue_cast<float3>(context->argument(0));
    float ret = This.DistanceSq(point);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Dot_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Dot_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 v = qscriptvalue_cast<float3>(context->argument(0));
    float ret = This.Dot(v);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Cross_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Cross_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 v = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.Cross(v);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_OuterProduct_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_OuterProduct_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 rhs = qscriptvalue_cast<float3>(context->argument(0));
    float3x3 ret = This.OuterProduct(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Perpendicular_float3_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3_Perpendicular_float3_float3_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 hint = qscriptvalue_cast<float3>(context->argument(0));
    float3 hint2 = qscriptvalue_cast<float3>(context->argument(1));
    float3 ret = This.Perpendicular(hint, hint2);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_AnotherPerpendicular_float3_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3_AnotherPerpendicular_float3_float3_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 hint = qscriptvalue_cast<float3>(context->argument(0));
    float3 hint2 = qscriptvalue_cast<float3>(context->argument(1));
    float3 ret = This.AnotherPerpendicular(hint, hint2);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Reflect_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Reflect_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 normal = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.Reflect(normal);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Refract_float3_float_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3_Refract_float3_float_float_const in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 normal = qscriptvalue_cast<float3>(context->argument(0));
    float negativeSideRefractionIndex = qscriptvalue_cast<float>(context->argument(1));
    float positiveSideRefractionIndex = qscriptvalue_cast<float>(context->argument(2));
    float3 ret = This.Refract(normal, negativeSideRefractionIndex, positiveSideRefractionIndex);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_ProjectTo_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_ProjectTo_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 direction = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.ProjectTo(direction);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_ProjectToNorm_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_ProjectToNorm_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 direction = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.ProjectToNorm(direction);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_AngleBetween_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_AngleBetween_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 other = qscriptvalue_cast<float3>(context->argument(0));
    float ret = This.AngleBetween(other);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_AngleBetweenNorm_float3_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_AngleBetweenNorm_float3_const in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 normalizedVector = qscriptvalue_cast<float3>(context->argument(0));
    float ret = This.AngleBetweenNorm(normalizedVector);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_Decompose_float3_float3_float3_const(QScriptContext *context, QScriptEngine * /*engine*/)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3_Decompose_float3_float3_float3_const in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 direction = qscriptvalue_cast<float3>(context->argument(0));
    float3 outParallel = qscriptvalue_cast<float3>(context->argument(1));
    float3 outPerpendicular = qscriptvalue_cast<float3>(context->argument(2));
    This.Decompose(direction, outParallel, outPerpendicular);
    return QScriptValue();
}

static QScriptValue float3_Lerp_float3_float_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3_Lerp_float3_float_const in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    float3 b = qscriptvalue_cast<float3>(context->argument(0));
    float t = qscriptvalue_cast<float>(context->argument(1));
    float3 ret = This.Lerp(b, t);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_float3_QVector3D(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_float3_QVector3D in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    QVector3D other = qscriptvalue_cast<QVector3D>(context->argument(0));
    float3 ret(other);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_toString_const(QScriptContext *context, QScriptEngine *engine)
{
    float3 This;
    if (context->argumentCount() > 0) This = qscriptvalue_cast<float3>(context->argument(0)); // Qt oddity (bug?): Sometimes the built-in toString() function doesn't give us this from thisObject, but as the first argument.
    else This = qscriptvalue_cast<float3>(context->thisObject());
    QString ret = This.toString();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_ToQVector3D_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_ToQVector3D_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 This = qscriptvalue_cast<float3>(context->thisObject());
    QVector3D ret = This.ToQVector3D();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_FromScalar_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_FromScalar_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float scalar = qscriptvalue_cast<float>(context->argument(0));
    float3 ret = float3::FromScalar(scalar);
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

static QScriptValue float3_Orthogonalize_float3_float3(QScriptContext *context, QScriptEngine * /*engine*/)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3_Orthogonalize_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 a = qscriptvalue_cast<float3>(context->argument(0));
    float3 b = qscriptvalue_cast<float3>(context->argument(1));
    float3::Orthogonalize(a, b);
    return QScriptValue();
}

static QScriptValue float3_Orthogonalize_float3_float3_float3(QScriptContext *context, QScriptEngine * /*engine*/)
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

static QScriptValue float3_Orthonormalize_float3_float3(QScriptContext *context, QScriptEngine * /*engine*/)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3_Orthonormalize_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 a = qscriptvalue_cast<float3>(context->argument(0));
    float3 b = qscriptvalue_cast<float3>(context->argument(1));
    float3::Orthonormalize(a, b);
    return QScriptValue();
}

static QScriptValue float3_Orthonormalize_float3_float3_float3(QScriptContext *context, QScriptEngine * /*engine*/)
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

static QScriptValue float3_RandomDir_LCG_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3_RandomDir_LCG_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LCG lcg = qscriptvalue_cast<LCG>(context->argument(0));
    float length = qscriptvalue_cast<float>(context->argument(1));
    float3 ret = float3::RandomDir(lcg, length);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_RandomSphere_LCG_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3_RandomSphere_LCG_float3_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LCG lcg = qscriptvalue_cast<LCG>(context->argument(0));
    float3 center = qscriptvalue_cast<float3>(context->argument(1));
    float radius = qscriptvalue_cast<float>(context->argument(2));
    float3 ret = float3::RandomSphere(lcg, center, radius);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_RandomBox_LCG_float_float_float_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 7) { printf("Error! Invalid number of arguments passed to function float3_RandomBox_LCG_float_float_float_float_float_float in file %s, line %d!\nExpected 7, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LCG lcg = qscriptvalue_cast<LCG>(context->argument(0));
    float xmin = qscriptvalue_cast<float>(context->argument(1));
    float xmax = qscriptvalue_cast<float>(context->argument(2));
    float ymin = qscriptvalue_cast<float>(context->argument(3));
    float ymax = qscriptvalue_cast<float>(context->argument(4));
    float zmin = qscriptvalue_cast<float>(context->argument(5));
    float zmax = qscriptvalue_cast<float>(context->argument(6));
    float3 ret = float3::RandomBox(lcg, xmin, xmax, ymin, ymax, zmin, zmax);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3_RandomBox_LCG_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3_RandomBox_LCG_float3_float3 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    LCG lcg = qscriptvalue_cast<LCG>(context->argument(0));
    float3 minValues = qscriptvalue_cast<float3>(context->argument(1));
    float3 maxValues = qscriptvalue_cast<float3>(context->argument(2));
    float3 ret = float3::RandomBox(lcg, minValues, maxValues);
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
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return float3_float3_float(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float2>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return float3_float3_float2_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<QVector3D>(context->argument(0)))
        return float3_float3_QVector3D(context, engine);
    printf("float3_ctor failed to choose the right function to call! Did you use 'var x = float3();' instead of 'var x = new float3();'?\n"); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3_At_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<int>(context->argument(0)))
        return float3_At_int(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<int>(context->argument(0)))
        return float3_At_int_const(context, engine);
    printf("float3_At_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3_Add_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return float3_Add_float3_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return float3_Add_float_const(context, engine);
    printf("float3_Add_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3_Sub_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return float3_Sub_float3_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return float3_Sub_float_const(context, engine);
    printf("float3_Sub_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3_Mul_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return float3_Mul_float3_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return float3_Mul_float_const(context, engine);
    printf("float3_Mul_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3_Div_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return float3_Div_float3_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return float3_Div_float_const(context, engine);
    printf("float3_Div_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3_Swizzled_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<int>(context->argument(0)) && QSVIsOfType<int>(context->argument(1)))
        return float3_Swizzled_int_int_const(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<int>(context->argument(0)) && QSVIsOfType<int>(context->argument(1)) && QSVIsOfType<int>(context->argument(2)))
        return float3_Swizzled_int_int_int_const(context, engine);
    if (context->argumentCount() == 4 && QSVIsOfType<int>(context->argument(0)) && QSVIsOfType<int>(context->argument(1)) && QSVIsOfType<int>(context->argument(2)) && QSVIsOfType<int>(context->argument(3)))
        return float3_Swizzled_int_int_int_int_const(context, engine);
    printf("float3_Swizzled_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3_Equals_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return float3_Equals_float3_float_const(context, engine);
    if (context->argumentCount() == 4 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)) && QSVIsOfType<float>(context->argument(3)))
        return float3_Equals_float_float_float_float_const(context, engine);
    printf("float3_Equals_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3_Min_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return float3_Min_float_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return float3_Min_float3_const(context, engine);
    printf("float3_Min_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3_Max_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return float3_Max_float_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return float3_Max_float3_const(context, engine);
    printf("float3_Max_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3_Clamp_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return float3_Clamp_float_float_const(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return float3_Clamp_float3_float3_const(context, engine);
    printf("float3_Clamp_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3_Distance_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return float3_Distance_float3_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Line>(context->argument(0)))
        return float3_Distance_Line_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Ray>(context->argument(0)))
        return float3_Distance_Ray_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<LineSegment>(context->argument(0)))
        return float3_Distance_LineSegment_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Plane>(context->argument(0)))
        return float3_Distance_Plane_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Triangle>(context->argument(0)))
        return float3_Distance_Triangle_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<AABB>(context->argument(0)))
        return float3_Distance_AABB_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<OBB>(context->argument(0)))
        return float3_Distance_OBB_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Sphere>(context->argument(0)))
        return float3_Distance_Sphere_const(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Capsule>(context->argument(0)))
        return float3_Distance_Capsule_const(context, engine);
    printf("float3_Distance_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3_Lerp_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return float3_Lerp_float3_float_const(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return float3_Lerp_float3_float3_float(context, engine);
    printf("float3_Lerp_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
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

static QScriptValue float3_RandomBox_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 7 && QSVIsOfType<LCG>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)) && QSVIsOfType<float>(context->argument(3)) && QSVIsOfType<float>(context->argument(4)) && QSVIsOfType<float>(context->argument(5)) && QSVIsOfType<float>(context->argument(6)))
        return float3_RandomBox_LCG_float_float_float_float_float_float(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<LCG>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)))
        return float3_RandomBox_LCG_float3_float3(context, engine);
    printf("float3_RandomBox_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
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
    proto.setProperty("At", engine->newFunction(float3_At_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Add", engine->newFunction(float3_Add_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Sub", engine->newFunction(float3_Sub_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SubLeft", engine->newFunction(float3_SubLeft_float_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Mul", engine->newFunction(float3_Mul_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Div", engine->newFunction(float3_Div_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("DivLeft", engine->newFunction(float3_DivLeft_float_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("xx", engine->newFunction(float3_xx_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("xy", engine->newFunction(float3_xy_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("xz", engine->newFunction(float3_xz_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("yx", engine->newFunction(float3_yx_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("yy", engine->newFunction(float3_yy_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("yz", engine->newFunction(float3_yz_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("zx", engine->newFunction(float3_zx_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("zy", engine->newFunction(float3_zy_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("zz", engine->newFunction(float3_zz_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("xxx", engine->newFunction(float3_xxx_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("xxy", engine->newFunction(float3_xxy_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("xxz", engine->newFunction(float3_xxz_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("xyx", engine->newFunction(float3_xyx_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("xyy", engine->newFunction(float3_xyy_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("xyz", engine->newFunction(float3_xyz_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("xzx", engine->newFunction(float3_xzx_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("xzy", engine->newFunction(float3_xzy_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("xzz", engine->newFunction(float3_xzz_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("yxx", engine->newFunction(float3_yxx_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("yxy", engine->newFunction(float3_yxy_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("yxz", engine->newFunction(float3_yxz_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("yyx", engine->newFunction(float3_yyx_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("yyy", engine->newFunction(float3_yyy_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("yyz", engine->newFunction(float3_yyz_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("yzx", engine->newFunction(float3_yzx_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("yzy", engine->newFunction(float3_yzy_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("yzz", engine->newFunction(float3_yzz_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("zxx", engine->newFunction(float3_zxx_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("zxy", engine->newFunction(float3_zxy_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("zxz", engine->newFunction(float3_zxz_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("zyx", engine->newFunction(float3_zyx_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("zyy", engine->newFunction(float3_zyy_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("zyz", engine->newFunction(float3_zyz_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("zzx", engine->newFunction(float3_zzx_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("zzy", engine->newFunction(float3_zzy_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("zzz", engine->newFunction(float3_zzz_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Swizzled", engine->newFunction(float3_Swizzled_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Swizzled", engine->newFunction(float3_Swizzled_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Swizzled", engine->newFunction(float3_Swizzled_selector, 4), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetFromScalar", engine->newFunction(float3_SetFromScalar_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Set", engine->newFunction(float3_Set_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToPos4", engine->newFunction(float3_ToPos4_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToDir4", engine->newFunction(float3_ToDir4_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Length", engine->newFunction(float3_Length_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("LengthSq", engine->newFunction(float3_LengthSq_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Normalize", engine->newFunction(float3_Normalize, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Normalized", engine->newFunction(float3_Normalized_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ScaleToLength", engine->newFunction(float3_ScaleToLength_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ScaledToLength", engine->newFunction(float3_ScaledToLength_float_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsNormalized", engine->newFunction(float3_IsNormalized_float_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsZero", engine->newFunction(float3_IsZero_float_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsFinite", engine->newFunction(float3_IsFinite_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsPerpendicular", engine->newFunction(float3_IsPerpendicular_float3_float_const, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Equals", engine->newFunction(float3_Equals_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Equals", engine->newFunction(float3_Equals_selector, 4), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SumOfElements", engine->newFunction(float3_SumOfElements_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ProductOfElements", engine->newFunction(float3_ProductOfElements_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("AverageOfElements", engine->newFunction(float3_AverageOfElements_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MinElement", engine->newFunction(float3_MinElement_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MinElementIndex", engine->newFunction(float3_MinElementIndex_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MaxElement", engine->newFunction(float3_MaxElement_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MaxElementIndex", engine->newFunction(float3_MaxElementIndex_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Abs", engine->newFunction(float3_Abs_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Neg", engine->newFunction(float3_Neg_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Recip", engine->newFunction(float3_Recip_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Min", engine->newFunction(float3_Min_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Max", engine->newFunction(float3_Max_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Clamp", engine->newFunction(float3_Clamp_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Clamp01", engine->newFunction(float3_Clamp01_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Distance", engine->newFunction(float3_Distance_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("DistanceSq", engine->newFunction(float3_DistanceSq_float3_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Dot", engine->newFunction(float3_Dot_float3_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Cross", engine->newFunction(float3_Cross_float3_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("OuterProduct", engine->newFunction(float3_OuterProduct_float3_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Perpendicular", engine->newFunction(float3_Perpendicular_float3_float3_const, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("AnotherPerpendicular", engine->newFunction(float3_AnotherPerpendicular_float3_float3_const, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Reflect", engine->newFunction(float3_Reflect_float3_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Refract", engine->newFunction(float3_Refract_float3_float_float_const, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ProjectTo", engine->newFunction(float3_ProjectTo_float3_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ProjectToNorm", engine->newFunction(float3_ProjectToNorm_float3_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("AngleBetween", engine->newFunction(float3_AngleBetween_float3_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("AngleBetweenNorm", engine->newFunction(float3_AngleBetweenNorm_float3_const, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Decompose", engine->newFunction(float3_Decompose_float3_float3_float3_const, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Lerp", engine->newFunction(float3_Lerp_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("toString", engine->newFunction(float3_toString_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToQVector3D", engine->newFunction(float3_ToQVector3D_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("metaTypeId", engine->toScriptValue<qint32>((qint32)qMetaTypeId<float3>()));
    engine->setDefaultPrototype(qMetaTypeId<float3>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<float3*>(), proto);
    qScriptRegisterMetaType(engine, ToScriptValue_float3, FromScriptValue_float3, proto);

    QScriptValue ctor = engine->newFunction(float3_ctor, proto, 3);
    ctor.setProperty("FromScalar", engine->newFunction(float3_FromScalar_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
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
    ctor.setProperty("RandomDir", engine->newFunction(float3_RandomDir_LCG_float, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("RandomSphere", engine->newFunction(float3_RandomSphere_LCG_float3_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("RandomBox", engine->newFunction(float3_RandomBox_selector, 7), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("RandomBox", engine->newFunction(float3_RandomBox_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
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

