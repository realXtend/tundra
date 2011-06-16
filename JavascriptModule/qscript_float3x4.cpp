#include "QtScriptBindingsHelpers.h"

void ToExistingScriptValue_float3x4(QScriptEngine *engine, const float3x4 &value, QScriptValue obj)
{
    obj.setData(engine->newVariant(QVariant::fromValue(value)));
}

static QScriptValue float3x4_float3x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_float3x4 in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 ret;
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_float3x4_float3x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_float3x4_float3x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 rhs = qscriptvalue_cast<float3x4>(context->argument(0));
    float3x4 ret(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_float3x4_float_float_float_float_float_float_float_float_float_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 12) { printf("Error! Invalid number of arguments passed to function float3x4_float3x4_float_float_float_float_float_float_float_float_float_float_float_float in file %s, line %d!\nExpected 12, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float _00 = qscriptvalue_cast<float>(context->argument(0));
    float _01 = qscriptvalue_cast<float>(context->argument(1));
    float _02 = qscriptvalue_cast<float>(context->argument(2));
    float _03 = qscriptvalue_cast<float>(context->argument(3));
    float _10 = qscriptvalue_cast<float>(context->argument(4));
    float _11 = qscriptvalue_cast<float>(context->argument(5));
    float _12 = qscriptvalue_cast<float>(context->argument(6));
    float _13 = qscriptvalue_cast<float>(context->argument(7));
    float _20 = qscriptvalue_cast<float>(context->argument(8));
    float _21 = qscriptvalue_cast<float>(context->argument(9));
    float _22 = qscriptvalue_cast<float>(context->argument(10));
    float _23 = qscriptvalue_cast<float>(context->argument(11));
    float3x4 ret(_00, _01, _02, _03, _10, _11, _12, _13, _20, _21, _22, _23);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_float3x4_float3x3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_float3x4_float3x3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 other = qscriptvalue_cast<float3x3>(context->argument(0));
    float3x4 ret(other);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_float3x4_float3x3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x4_float3x4_float3x3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 other = qscriptvalue_cast<float3x3>(context->argument(0));
    float3 translate = qscriptvalue_cast<float3>(context->argument(1));
    float3x4 ret(other, translate);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_float3x4_float3_float3_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function float3x4_float3x4_float3_float3_float3_float3 in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 col0 = qscriptvalue_cast<float3>(context->argument(0));
    float3 col1 = qscriptvalue_cast<float3>(context->argument(1));
    float3 col2 = qscriptvalue_cast<float3>(context->argument(2));
    float3 col3 = qscriptvalue_cast<float3>(context->argument(3));
    float3x4 ret(col0, col1, col2, col3);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_float3x4_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_float3x4_Quat in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat orientation = qscriptvalue_cast<Quat>(context->argument(0));
    float3x4 ret(orientation);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_float3x4_Quat_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x4_float3x4_Quat_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat orientation = qscriptvalue_cast<Quat>(context->argument(0));
    float3 translation = qscriptvalue_cast<float3>(context->argument(1));
    float3x4 ret(orientation, translation);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_GetScale(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_GetScale in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3 ret = This.GetScale();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_At_int_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x4_At_int_int in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    int row = qscriptvalue_cast<int>(context->argument(0));
    int col = qscriptvalue_cast<int>(context->argument(1));
    const float ret = This.At(row, col);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_Row_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_Row_int in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    int row = qscriptvalue_cast<int>(context->argument(0));
    const float4 & ret = This.Row(row);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_Row3_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_Row3_int in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    int row = qscriptvalue_cast<int>(context->argument(0));
    const float3 & ret = This.Row3(row);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_Col_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_Col_int in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    int col = qscriptvalue_cast<int>(context->argument(0));
    const float3 ret = This.Col(col);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_Diagonal(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_Diagonal in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    const float3 ret = This.Diagonal();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_ScaleRow3_int_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x4_ScaleRow3_int_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    int row = qscriptvalue_cast<int>(context->argument(0));
    float scalar = qscriptvalue_cast<float>(context->argument(1));
    This.ScaleRow3(row, scalar);
    ToExistingScriptValue_float3x4(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x4_ScaleRow_int_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x4_ScaleRow_int_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    int row = qscriptvalue_cast<int>(context->argument(0));
    float scalar = qscriptvalue_cast<float>(context->argument(1));
    This.ScaleRow(row, scalar);
    ToExistingScriptValue_float3x4(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x4_ScaleCol_int_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x4_ScaleCol_int_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    int col = qscriptvalue_cast<int>(context->argument(0));
    float scalar = qscriptvalue_cast<float>(context->argument(1));
    This.ScaleCol(col, scalar);
    ToExistingScriptValue_float3x4(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x4_Float3x3Part(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_Float3x3Part in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3x3 ret = This.Float3x3Part();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_TranslatePart(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_TranslatePart in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3 ret = This.TranslatePart();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_RotatePart(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_RotatePart in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3x3 ret = This.RotatePart();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_WorldX(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_WorldX in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3 ret = This.WorldX();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_WorldY(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_WorldY in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3 ret = This.WorldY();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_WorldZ(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_WorldZ in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3 ret = This.WorldZ();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_SetRow_int_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x4_SetRow_int_float3_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    int row = qscriptvalue_cast<int>(context->argument(0));
    float3 rowVector = qscriptvalue_cast<float3>(context->argument(1));
    float m_r3 = qscriptvalue_cast<float>(context->argument(2));
    This.SetRow(row, rowVector, m_r3);
    ToExistingScriptValue_float3x4(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x4_SetRow_int_float4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x4_SetRow_int_float4 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    int row = qscriptvalue_cast<int>(context->argument(0));
    float4 rowVector = qscriptvalue_cast<float4>(context->argument(1));
    This.SetRow(row, rowVector);
    ToExistingScriptValue_float3x4(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x4_SetRow_int_float_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 5) { printf("Error! Invalid number of arguments passed to function float3x4_SetRow_int_float_float_float_float in file %s, line %d!\nExpected 5, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    int row = qscriptvalue_cast<int>(context->argument(0));
    float m_r0 = qscriptvalue_cast<float>(context->argument(1));
    float m_r1 = qscriptvalue_cast<float>(context->argument(2));
    float m_r2 = qscriptvalue_cast<float>(context->argument(3));
    float m_r3 = qscriptvalue_cast<float>(context->argument(4));
    This.SetRow(row, m_r0, m_r1, m_r2, m_r3);
    ToExistingScriptValue_float3x4(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x4_SetCol_int_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x4_SetCol_int_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    int column = qscriptvalue_cast<int>(context->argument(0));
    float3 columnVector = qscriptvalue_cast<float3>(context->argument(1));
    This.SetCol(column, columnVector);
    ToExistingScriptValue_float3x4(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x4_SetCol_int_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function float3x4_SetCol_int_float_float_float in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    int column = qscriptvalue_cast<int>(context->argument(0));
    float m_0c = qscriptvalue_cast<float>(context->argument(1));
    float m_1c = qscriptvalue_cast<float>(context->argument(2));
    float m_2c = qscriptvalue_cast<float>(context->argument(3));
    This.SetCol(column, m_0c, m_1c, m_2c);
    ToExistingScriptValue_float3x4(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x4_Set_float_float_float_float_float_float_float_float_float_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 12) { printf("Error! Invalid number of arguments passed to function float3x4_Set_float_float_float_float_float_float_float_float_float_float_float_float in file %s, line %d!\nExpected 12, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float _00 = qscriptvalue_cast<float>(context->argument(0));
    float _01 = qscriptvalue_cast<float>(context->argument(1));
    float _02 = qscriptvalue_cast<float>(context->argument(2));
    float _03 = qscriptvalue_cast<float>(context->argument(3));
    float _10 = qscriptvalue_cast<float>(context->argument(4));
    float _11 = qscriptvalue_cast<float>(context->argument(5));
    float _12 = qscriptvalue_cast<float>(context->argument(6));
    float _13 = qscriptvalue_cast<float>(context->argument(7));
    float _20 = qscriptvalue_cast<float>(context->argument(8));
    float _21 = qscriptvalue_cast<float>(context->argument(9));
    float _22 = qscriptvalue_cast<float>(context->argument(10));
    float _23 = qscriptvalue_cast<float>(context->argument(11));
    This.Set(_00, _01, _02, _03, _10, _11, _12, _13, _20, _21, _22, _23);
    ToExistingScriptValue_float3x4(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x4_Set_int_int_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x4_Set_int_int_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    int row = qscriptvalue_cast<int>(context->argument(0));
    int col = qscriptvalue_cast<int>(context->argument(1));
    float value = qscriptvalue_cast<float>(context->argument(2));
    This.Set(row, col, value);
    ToExistingScriptValue_float3x4(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x4_Set3x3Part_float3x3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_Set3x3Part_float3x3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3x3 rotation = qscriptvalue_cast<float3x3>(context->argument(0));
    This.Set3x3Part(rotation);
    ToExistingScriptValue_float3x4(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x4_SetIdentity(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_SetIdentity in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    This.SetIdentity();
    ToExistingScriptValue_float3x4(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x4_SwapColumns_int_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x4_SwapColumns_int_int in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    int col1 = qscriptvalue_cast<int>(context->argument(0));
    int col2 = qscriptvalue_cast<int>(context->argument(1));
    This.SwapColumns(col1, col2);
    ToExistingScriptValue_float3x4(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x4_SwapRows_int_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x4_SwapRows_int_int in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    int row1 = qscriptvalue_cast<int>(context->argument(0));
    int row2 = qscriptvalue_cast<int>(context->argument(1));
    This.SwapRows(row1, row2);
    ToExistingScriptValue_float3x4(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x4_SetTranslatePart_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x4_SetTranslatePart_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float tx = qscriptvalue_cast<float>(context->argument(0));
    float ty = qscriptvalue_cast<float>(context->argument(1));
    float tz = qscriptvalue_cast<float>(context->argument(2));
    This.SetTranslatePart(tx, ty, tz);
    ToExistingScriptValue_float3x4(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x4_SetTranslatePart_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_SetTranslatePart_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3 offset = qscriptvalue_cast<float3>(context->argument(0));
    This.SetTranslatePart(offset);
    ToExistingScriptValue_float3x4(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x4_SetRotatePartX_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_SetRotatePartX_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float angleRadians = qscriptvalue_cast<float>(context->argument(0));
    This.SetRotatePartX(angleRadians);
    ToExistingScriptValue_float3x4(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x4_SetRotatePartY_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_SetRotatePartY_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float angleRadians = qscriptvalue_cast<float>(context->argument(0));
    This.SetRotatePartY(angleRadians);
    ToExistingScriptValue_float3x4(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x4_SetRotatePartZ_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_SetRotatePartZ_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float angleRadians = qscriptvalue_cast<float>(context->argument(0));
    This.SetRotatePartZ(angleRadians);
    ToExistingScriptValue_float3x4(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x4_SetRotatePart_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x4_SetRotatePart_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3 axisDirection = qscriptvalue_cast<float3>(context->argument(0));
    float angleRadians = qscriptvalue_cast<float>(context->argument(1));
    This.SetRotatePart(axisDirection, angleRadians);
    ToExistingScriptValue_float3x4(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x4_SetRotatePart_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_SetRotatePart_Quat in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    Quat orientation = qscriptvalue_cast<Quat>(context->argument(0));
    This.SetRotatePart(orientation);
    ToExistingScriptValue_float3x4(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x4_SetRotatePart_float3x3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_SetRotatePart_float3x3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3x3 rotation = qscriptvalue_cast<float3x3>(context->argument(0));
    This.SetRotatePart(rotation);
    ToExistingScriptValue_float3x4(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x4_LookAt_float3_float3_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function float3x4_LookAt_float3_float3_float3_float3 in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3 localForward = qscriptvalue_cast<float3>(context->argument(0));
    float3 targetPosition = qscriptvalue_cast<float3>(context->argument(1));
    float3 localUp = qscriptvalue_cast<float3>(context->argument(2));
    float3 worldUp = qscriptvalue_cast<float3>(context->argument(3));
    This.LookAt(localForward, targetPosition, localUp, worldUp);
    ToExistingScriptValue_float3x4(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x4_Determinant(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_Determinant in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float ret = This.Determinant();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_Inverse(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_Inverse in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    bool ret = This.Inverse();
    ToExistingScriptValue_float3x4(engine, This, context->thisObject());
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_Inverted(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_Inverted in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3x4 ret = This.Inverted();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_InverseOrthogonal(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_InverseOrthogonal in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    bool ret = This.InverseOrthogonal();
    ToExistingScriptValue_float3x4(engine, This, context->thisObject());
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_InverseOrthogonalUniformScale(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_InverseOrthogonalUniformScale in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    bool ret = This.InverseOrthogonalUniformScale();
    ToExistingScriptValue_float3x4(engine, This, context->thisObject());
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_InverseOrthonormal(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_InverseOrthonormal in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    This.InverseOrthonormal();
    ToExistingScriptValue_float3x4(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x4_Transpose3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_Transpose3 in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    This.Transpose3();
    ToExistingScriptValue_float3x4(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x4_Transposed3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_Transposed3 in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3x4 ret = This.Transposed3();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_InverseTranspose(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_InverseTranspose in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    bool ret = This.InverseTranspose();
    ToExistingScriptValue_float3x4(engine, This, context->thisObject());
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_InverseTransposed(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_InverseTransposed in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3x4 ret = This.InverseTransposed();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_Trace(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_Trace in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float ret = This.Trace();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_Orthonormalize_int_int_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x4_Orthonormalize_int_int_int in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    int firstColumn = qscriptvalue_cast<int>(context->argument(0));
    int secondColumn = qscriptvalue_cast<int>(context->argument(1));
    int thirdColumn = qscriptvalue_cast<int>(context->argument(2));
    This.Orthonormalize(firstColumn, secondColumn, thirdColumn);
    ToExistingScriptValue_float3x4(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x4_RemoveScale(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_RemoveScale in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    This.RemoveScale();
    ToExistingScriptValue_float3x4(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x4_TransformPos_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_TransformPos_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3 pointVector = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.TransformPos(pointVector);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_TransformPos_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x4_TransformPos_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float x = qscriptvalue_cast<float>(context->argument(0));
    float y = qscriptvalue_cast<float>(context->argument(1));
    float z = qscriptvalue_cast<float>(context->argument(2));
    float3 ret = This.TransformPos(x, y, z);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_TransformDir_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_TransformDir_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3 directionVector = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.TransformDir(directionVector);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_TransformDir_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x4_TransformDir_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float x = qscriptvalue_cast<float>(context->argument(0));
    float y = qscriptvalue_cast<float>(context->argument(1));
    float z = qscriptvalue_cast<float>(context->argument(2));
    float3 ret = This.TransformDir(x, y, z);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_Transform_float4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_Transform_float4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float4 vector = qscriptvalue_cast<float4>(context->argument(0));
    float4 ret = This.Transform(vector);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_BatchTransform_float4_ptr_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x4_BatchTransform_float4_ptr_int in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float4 * vectorArray = qscriptvalue_cast<float4 *>(context->argument(0));
    int numVectors = qscriptvalue_cast<int>(context->argument(1));
    This.BatchTransform(vectorArray, numVectors);
    return QScriptValue();
}

static QScriptValue float3x4_BatchTransform_float4_ptr_int_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x4_BatchTransform_float4_ptr_int_int in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float4 * vectorArray = qscriptvalue_cast<float4 *>(context->argument(0));
    int numVectors = qscriptvalue_cast<int>(context->argument(1));
    int stride = qscriptvalue_cast<int>(context->argument(2));
    This.BatchTransform(vectorArray, numVectors, stride);
    return QScriptValue();
}

static QScriptValue float3x4_IsFinite(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_IsFinite in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    bool ret = This.IsFinite();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_IsIdentity_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_IsIdentity_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float epsilon = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.IsIdentity(epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_IsLowerTriangular_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_IsLowerTriangular_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float epsilon = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.IsLowerTriangular(epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_IsUpperTriangular_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_IsUpperTriangular_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float epsilon = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.IsUpperTriangular(epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_IsInvertible_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_IsInvertible_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float epsilon = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.IsInvertible(epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_IsSymmetric_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_IsSymmetric_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float epsilon = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.IsSymmetric(epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_IsSkewSymmetric_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_IsSkewSymmetric_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float epsilon = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.IsSkewSymmetric(epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_HasUnitaryScale_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_HasUnitaryScale_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float epsilonSq = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.HasUnitaryScale(epsilonSq);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_HasNegativeScale(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_HasNegativeScale in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    bool ret = This.HasNegativeScale();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_HasUniformScale_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_HasUniformScale_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float epsilonSq = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.HasUniformScale(epsilonSq);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_IsOrthogonal_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_IsOrthogonal_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float epsilon = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.IsOrthogonal(epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_IsOrthonormal_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_IsOrthonormal_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float epsilon = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.IsOrthonormal(epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_Equals_float3x4_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x4_Equals_float3x4_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3x4 other = qscriptvalue_cast<float3x4>(context->argument(0));
    float epsilon = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.Equals(other, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_ToEulerXYX(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_ToEulerXYX in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3 ret = This.ToEulerXYX();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_ToEulerXZX(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_ToEulerXZX in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3 ret = This.ToEulerXZX();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_ToEulerYXY(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_ToEulerYXY in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3 ret = This.ToEulerYXY();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_ToEulerYZY(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_ToEulerYZY in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3 ret = This.ToEulerYZY();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_ToEulerZXZ(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_ToEulerZXZ in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3 ret = This.ToEulerZXZ();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_ToEulerZYZ(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_ToEulerZYZ in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3 ret = This.ToEulerZYZ();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_ToEulerXYZ(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_ToEulerXYZ in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3 ret = This.ToEulerXYZ();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_ToEulerXZY(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_ToEulerXZY in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3 ret = This.ToEulerXZY();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_ToEulerYXZ(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_ToEulerYXZ in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3 ret = This.ToEulerYXZ();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_ToEulerYZX(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_ToEulerYZX in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3 ret = This.ToEulerYZX();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_ToEulerZXY(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_ToEulerZXY in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3 ret = This.ToEulerZXY();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_ToEulerZYX(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_ToEulerZYX in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3 ret = This.ToEulerZYX();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_ExtractScale(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_ExtractScale in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3 ret = This.ExtractScale();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_Decompose_float3_Quat_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x4_Decompose_float3_Quat_float3 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3 translate = qscriptvalue_cast<float3>(context->argument(0));
    Quat rotate = qscriptvalue_cast<Quat>(context->argument(1));
    float3 scale = qscriptvalue_cast<float3>(context->argument(2));
    This.Decompose(translate, rotate, scale);
    return QScriptValue();
}

static QScriptValue float3x4_Decompose_float3_float3x3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x4_Decompose_float3_float3x3_float3 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3 translate = qscriptvalue_cast<float3>(context->argument(0));
    float3x3 rotate = qscriptvalue_cast<float3x3>(context->argument(1));
    float3 scale = qscriptvalue_cast<float3>(context->argument(2));
    This.Decompose(translate, rotate, scale);
    return QScriptValue();
}

static QScriptValue float3x4_Decompose_float3_float3x4_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x4_Decompose_float3_float3x4_float3 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3 translate = qscriptvalue_cast<float3>(context->argument(0));
    float3x4 rotate = qscriptvalue_cast<float3x4>(context->argument(1));
    float3 scale = qscriptvalue_cast<float3>(context->argument(2));
    This.Decompose(translate, rotate, scale);
    return QScriptValue();
}

static QScriptValue float3x4_Mul_float3x3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_Mul_float3x3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3x3 rhs = qscriptvalue_cast<float3x3>(context->argument(0));
    float3x4 ret = This.Mul(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_Mul_float3x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_Mul_float3x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3x4 rhs = qscriptvalue_cast<float3x4>(context->argument(0));
    float3x4 ret = This.Mul(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_Mul_float4x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_Mul_float4x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float4x4 rhs = qscriptvalue_cast<float4x4>(context->argument(0));
    float4x4 ret = This.Mul(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_Mul_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_Mul_Quat in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    Quat rhs = qscriptvalue_cast<Quat>(context->argument(0));
    float3x4 ret = This.Mul(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_MulPos_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_MulPos_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3 pointVector = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.MulPos(pointVector);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_MulDir_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_MulDir_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float3 directionVector = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.MulDir(directionVector);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_Mul_float4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_Mul_float4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 This = qscriptvalue_cast<float3x4>(context->thisObject());
    float4 vector = qscriptvalue_cast<float4>(context->argument(0));
    float4 ret = This.Mul(vector);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_toString(QScriptContext *context, QScriptEngine *engine)
{
    float3x4 This;
    if (context->argumentCount() > 0) This = qscriptvalue_cast<float3x4>(context->argument(0)); // Qt oddity (bug?): Sometimes the built-in toString() function doesn't give us this from thisObject, but as the first argument.
    else This = qscriptvalue_cast<float3x4>(context->thisObject());
    QString ret = This.toString();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_Translate_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x4_Translate_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float tx = qscriptvalue_cast<float>(context->argument(0));
    float ty = qscriptvalue_cast<float>(context->argument(1));
    float tz = qscriptvalue_cast<float>(context->argument(2));
    TranslateOp ret = float3x4::Translate(tx, ty, tz);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_Translate_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_Translate_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 offset = qscriptvalue_cast<float3>(context->argument(0));
    TranslateOp ret = float3x4::Translate(offset);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_RotateX_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_RotateX_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float angleRadians = qscriptvalue_cast<float>(context->argument(0));
    float3x4 ret = float3x4::RotateX(angleRadians);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_RotateX_float_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x4_RotateX_float_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float angleRadians = qscriptvalue_cast<float>(context->argument(0));
    float3 pointOnAxis = qscriptvalue_cast<float3>(context->argument(1));
    float3x4 ret = float3x4::RotateX(angleRadians, pointOnAxis);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_RotateY_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_RotateY_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float angleRadians = qscriptvalue_cast<float>(context->argument(0));
    float3x4 ret = float3x4::RotateY(angleRadians);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_RotateY_float_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x4_RotateY_float_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float angleRadians = qscriptvalue_cast<float>(context->argument(0));
    float3 pointOnAxis = qscriptvalue_cast<float3>(context->argument(1));
    float3x4 ret = float3x4::RotateY(angleRadians, pointOnAxis);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_RotateZ_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_RotateZ_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float angleRadians = qscriptvalue_cast<float>(context->argument(0));
    float3x4 ret = float3x4::RotateZ(angleRadians);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_RotateZ_float_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x4_RotateZ_float_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float angleRadians = qscriptvalue_cast<float>(context->argument(0));
    float3 pointOnAxis = qscriptvalue_cast<float3>(context->argument(1));
    float3x4 ret = float3x4::RotateZ(angleRadians, pointOnAxis);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_RotateAxisAngle_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x4_RotateAxisAngle_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 axisDirection = qscriptvalue_cast<float3>(context->argument(0));
    float angleRadians = qscriptvalue_cast<float>(context->argument(1));
    float3x4 ret = float3x4::RotateAxisAngle(axisDirection, angleRadians);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_RotateAxisAngle_float3_float_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x4_RotateAxisAngle_float3_float_float3 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 axisDirection = qscriptvalue_cast<float3>(context->argument(0));
    float angleRadians = qscriptvalue_cast<float>(context->argument(1));
    float3 pointOnAxis = qscriptvalue_cast<float3>(context->argument(2));
    float3x4 ret = float3x4::RotateAxisAngle(axisDirection, angleRadians, pointOnAxis);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_RotateFromTo_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x4_RotateFromTo_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 sourceDirection = qscriptvalue_cast<float3>(context->argument(0));
    float3 targetDirection = qscriptvalue_cast<float3>(context->argument(1));
    float3x4 ret = float3x4::RotateFromTo(sourceDirection, targetDirection);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_RotateFromTo_float3_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x4_RotateFromTo_float3_float3_float3 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 sourceDirection = qscriptvalue_cast<float3>(context->argument(0));
    float3 targetDirection = qscriptvalue_cast<float3>(context->argument(1));
    float3 centerPoint = qscriptvalue_cast<float3>(context->argument(2));
    float3x4 ret = float3x4::RotateFromTo(sourceDirection, targetDirection, centerPoint);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_FromQuat_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_FromQuat_Quat in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat orientation = qscriptvalue_cast<Quat>(context->argument(0));
    float3x4 ret = float3x4::FromQuat(orientation);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_FromQuat_Quat_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x4_FromQuat_Quat_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat orientation = qscriptvalue_cast<Quat>(context->argument(0));
    float3 pointOnAxis = qscriptvalue_cast<float3>(context->argument(1));
    float3x4 ret = float3x4::FromQuat(orientation, pointOnAxis);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_FromTRS_float3_Quat_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x4_FromTRS_float3_Quat_float3 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 translate = qscriptvalue_cast<float3>(context->argument(0));
    Quat rotate = qscriptvalue_cast<Quat>(context->argument(1));
    float3 scale = qscriptvalue_cast<float3>(context->argument(2));
    float3x4 ret = float3x4::FromTRS(translate, rotate, scale);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_FromTRS_float3_float3x3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x4_FromTRS_float3_float3x3_float3 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 translate = qscriptvalue_cast<float3>(context->argument(0));
    float3x3 rotate = qscriptvalue_cast<float3x3>(context->argument(1));
    float3 scale = qscriptvalue_cast<float3>(context->argument(2));
    float3x4 ret = float3x4::FromTRS(translate, rotate, scale);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_FromTRS_float3_float3x4_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x4_FromTRS_float3_float3x4_float3 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 translate = qscriptvalue_cast<float3>(context->argument(0));
    float3x4 rotate = qscriptvalue_cast<float3x4>(context->argument(1));
    float3 scale = qscriptvalue_cast<float3>(context->argument(2));
    float3x4 ret = float3x4::FromTRS(translate, rotate, scale);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_FromEulerXYX_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x4_FromEulerXYX_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float ex = qscriptvalue_cast<float>(context->argument(0));
    float ey = qscriptvalue_cast<float>(context->argument(1));
    float ex2 = qscriptvalue_cast<float>(context->argument(2));
    float3x4 ret = float3x4::FromEulerXYX(ex, ey, ex2);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_FromEulerXZX_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x4_FromEulerXZX_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float ex = qscriptvalue_cast<float>(context->argument(0));
    float ez = qscriptvalue_cast<float>(context->argument(1));
    float ex2 = qscriptvalue_cast<float>(context->argument(2));
    float3x4 ret = float3x4::FromEulerXZX(ex, ez, ex2);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_FromEulerYXY_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x4_FromEulerYXY_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float ey = qscriptvalue_cast<float>(context->argument(0));
    float ex = qscriptvalue_cast<float>(context->argument(1));
    float ey2 = qscriptvalue_cast<float>(context->argument(2));
    float3x4 ret = float3x4::FromEulerYXY(ey, ex, ey2);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_FromEulerYZY_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x4_FromEulerYZY_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float ey = qscriptvalue_cast<float>(context->argument(0));
    float ez = qscriptvalue_cast<float>(context->argument(1));
    float ey2 = qscriptvalue_cast<float>(context->argument(2));
    float3x4 ret = float3x4::FromEulerYZY(ey, ez, ey2);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_FromEulerZXZ_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x4_FromEulerZXZ_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float ez = qscriptvalue_cast<float>(context->argument(0));
    float ex = qscriptvalue_cast<float>(context->argument(1));
    float ez2 = qscriptvalue_cast<float>(context->argument(2));
    float3x4 ret = float3x4::FromEulerZXZ(ez, ex, ez2);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_FromEulerZYZ_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x4_FromEulerZYZ_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float ez = qscriptvalue_cast<float>(context->argument(0));
    float ey = qscriptvalue_cast<float>(context->argument(1));
    float ez2 = qscriptvalue_cast<float>(context->argument(2));
    float3x4 ret = float3x4::FromEulerZYZ(ez, ey, ez2);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_FromEulerXYZ_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x4_FromEulerXYZ_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float ex = qscriptvalue_cast<float>(context->argument(0));
    float ey = qscriptvalue_cast<float>(context->argument(1));
    float ez = qscriptvalue_cast<float>(context->argument(2));
    float3x4 ret = float3x4::FromEulerXYZ(ex, ey, ez);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_FromEulerXZY_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x4_FromEulerXZY_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float ex = qscriptvalue_cast<float>(context->argument(0));
    float ez = qscriptvalue_cast<float>(context->argument(1));
    float ey = qscriptvalue_cast<float>(context->argument(2));
    float3x4 ret = float3x4::FromEulerXZY(ex, ez, ey);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_FromEulerYXZ_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x4_FromEulerYXZ_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float ey = qscriptvalue_cast<float>(context->argument(0));
    float ex = qscriptvalue_cast<float>(context->argument(1));
    float ez = qscriptvalue_cast<float>(context->argument(2));
    float3x4 ret = float3x4::FromEulerYXZ(ey, ex, ez);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_FromEulerYZX_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x4_FromEulerYZX_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float ey = qscriptvalue_cast<float>(context->argument(0));
    float ez = qscriptvalue_cast<float>(context->argument(1));
    float ex = qscriptvalue_cast<float>(context->argument(2));
    float3x4 ret = float3x4::FromEulerYZX(ey, ez, ex);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_FromEulerZXY_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x4_FromEulerZXY_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float ez = qscriptvalue_cast<float>(context->argument(0));
    float ex = qscriptvalue_cast<float>(context->argument(1));
    float ey = qscriptvalue_cast<float>(context->argument(2));
    float3x4 ret = float3x4::FromEulerZXY(ez, ex, ey);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_FromEulerZYX_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x4_FromEulerZYX_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float ez = qscriptvalue_cast<float>(context->argument(0));
    float ey = qscriptvalue_cast<float>(context->argument(1));
    float ex = qscriptvalue_cast<float>(context->argument(2));
    float3x4 ret = float3x4::FromEulerZYX(ez, ey, ex);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_Scale_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x4_Scale_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float sx = qscriptvalue_cast<float>(context->argument(0));
    float sy = qscriptvalue_cast<float>(context->argument(1));
    float sz = qscriptvalue_cast<float>(context->argument(2));
    ScaleOp ret = float3x4::Scale(sx, sy, sz);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_Scale_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_Scale_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 scale = qscriptvalue_cast<float3>(context->argument(0));
    ScaleOp ret = float3x4::Scale(scale);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_Scale_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x4_Scale_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 scale = qscriptvalue_cast<float3>(context->argument(0));
    float3 scaleCenter = qscriptvalue_cast<float3>(context->argument(1));
    float3x4 ret = float3x4::Scale(scale, scaleCenter);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_ScaleAlongAxis_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x4_ScaleAlongAxis_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 axis = qscriptvalue_cast<float3>(context->argument(0));
    float scalingFactor = qscriptvalue_cast<float>(context->argument(1));
    float3x4 ret = float3x4::ScaleAlongAxis(axis, scalingFactor);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_ScaleAlongAxis_float3_float_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x4_ScaleAlongAxis_float3_float_float3 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 axis = qscriptvalue_cast<float3>(context->argument(0));
    float scalingFactor = qscriptvalue_cast<float>(context->argument(1));
    float3 scaleCenter = qscriptvalue_cast<float3>(context->argument(2));
    float3x4 ret = float3x4::ScaleAlongAxis(axis, scalingFactor, scaleCenter);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_UniformScale_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_UniformScale_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float uniformScale = qscriptvalue_cast<float>(context->argument(0));
    ScaleOp ret = float3x4::UniformScale(uniformScale);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_ShearX_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x4_ShearX_float_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float yFactor = qscriptvalue_cast<float>(context->argument(0));
    float zFactor = qscriptvalue_cast<float>(context->argument(1));
    float3x4 ret = float3x4::ShearX(yFactor, zFactor);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_ShearY_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x4_ShearY_float_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float xFactor = qscriptvalue_cast<float>(context->argument(0));
    float zFactor = qscriptvalue_cast<float>(context->argument(1));
    float3x4 ret = float3x4::ShearY(xFactor, zFactor);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_ShearZ_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x4_ShearZ_float_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float xFactor = qscriptvalue_cast<float>(context->argument(0));
    float yFactor = qscriptvalue_cast<float>(context->argument(1));
    float3x4 ret = float3x4::ShearZ(xFactor, yFactor);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_Reflect_Plane(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_Reflect_Plane in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane p = qscriptvalue_cast<Plane>(context->argument(0));
    float3x4 ret = float3x4::Reflect(p);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_MakeOrthographicProjection_float_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function float3x4_MakeOrthographicProjection_float_float_float_float in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float nearPlaneDistance = qscriptvalue_cast<float>(context->argument(0));
    float farPlaneDistance = qscriptvalue_cast<float>(context->argument(1));
    float horizontalViewportSize = qscriptvalue_cast<float>(context->argument(2));
    float verticalViewportSize = qscriptvalue_cast<float>(context->argument(3));
    float3x4 ret = float3x4::MakeOrthographicProjection(nearPlaneDistance, farPlaneDistance, horizontalViewportSize, verticalViewportSize);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_MakeOrthographicProjection_Plane(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x4_MakeOrthographicProjection_Plane in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane target = qscriptvalue_cast<Plane>(context->argument(0));
    float3x4 ret = float3x4::MakeOrthographicProjection(target);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_MakeOrthographicProjectionYZ(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_MakeOrthographicProjectionYZ in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 ret = float3x4::MakeOrthographicProjectionYZ();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_MakeOrthographicProjectionXZ(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_MakeOrthographicProjectionXZ in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 ret = float3x4::MakeOrthographicProjectionXZ();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_MakeOrthographicProjectionXY(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x4_MakeOrthographicProjectionXY in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x4 ret = float3x4::MakeOrthographicProjectionXY();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x4_ctor(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return float3x4_float3x4(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3x4>(context->argument(0)))
        return float3x4_float3x4_float3x4(context, engine);
    if (context->argumentCount() == 12 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)) && QSVIsOfType<float>(context->argument(3)) && QSVIsOfType<float>(context->argument(4)) && QSVIsOfType<float>(context->argument(5)) && QSVIsOfType<float>(context->argument(6)) && QSVIsOfType<float>(context->argument(7)) && QSVIsOfType<float>(context->argument(8)) && QSVIsOfType<float>(context->argument(9)) && QSVIsOfType<float>(context->argument(10)) && QSVIsOfType<float>(context->argument(11)))
        return float3x4_float3x4_float_float_float_float_float_float_float_float_float_float_float_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3x3>(context->argument(0)))
        return float3x4_float3x4_float3x3(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float3x3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return float3x4_float3x4_float3x3_float3(context, engine);
    if (context->argumentCount() == 4 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)) && QSVIsOfType<float3>(context->argument(3)))
        return float3x4_float3x4_float3_float3_float3_float3(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Quat>(context->argument(0)))
        return float3x4_float3x4_Quat(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<Quat>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return float3x4_float3x4_Quat_float3(context, engine);
    printf("float3x4_ctor failed to choose the right function to call! Did you use 'var x = float3x4();' instead of 'var x = new float3x4();'?\n"); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3x4_SetRow_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 3 && QSVIsOfType<int>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return float3x4_SetRow_int_float3_float(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<int>(context->argument(0)) && QSVIsOfType<float4>(context->argument(1)))
        return float3x4_SetRow_int_float4(context, engine);
    if (context->argumentCount() == 5 && QSVIsOfType<int>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)) && QSVIsOfType<float>(context->argument(3)) && QSVIsOfType<float>(context->argument(4)))
        return float3x4_SetRow_int_float_float_float_float(context, engine);
    printf("float3x4_SetRow_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3x4_SetCol_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<int>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return float3x4_SetCol_int_float3(context, engine);
    if (context->argumentCount() == 4 && QSVIsOfType<int>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)) && QSVIsOfType<float>(context->argument(3)))
        return float3x4_SetCol_int_float_float_float(context, engine);
    printf("float3x4_SetCol_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3x4_Set_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 12 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)) && QSVIsOfType<float>(context->argument(3)) && QSVIsOfType<float>(context->argument(4)) && QSVIsOfType<float>(context->argument(5)) && QSVIsOfType<float>(context->argument(6)) && QSVIsOfType<float>(context->argument(7)) && QSVIsOfType<float>(context->argument(8)) && QSVIsOfType<float>(context->argument(9)) && QSVIsOfType<float>(context->argument(10)) && QSVIsOfType<float>(context->argument(11)))
        return float3x4_Set_float_float_float_float_float_float_float_float_float_float_float_float(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<int>(context->argument(0)) && QSVIsOfType<int>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return float3x4_Set_int_int_float(context, engine);
    printf("float3x4_Set_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3x4_SetTranslatePart_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 3 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return float3x4_SetTranslatePart_float_float_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return float3x4_SetTranslatePart_float3(context, engine);
    printf("float3x4_SetTranslatePart_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3x4_SetRotatePart_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return float3x4_SetRotatePart_float3_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Quat>(context->argument(0)))
        return float3x4_SetRotatePart_Quat(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3x3>(context->argument(0)))
        return float3x4_SetRotatePart_float3x3(context, engine);
    printf("float3x4_SetRotatePart_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3x4_TransformPos_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return float3x4_TransformPos_float3(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return float3x4_TransformPos_float_float_float(context, engine);
    printf("float3x4_TransformPos_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3x4_TransformDir_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return float3x4_TransformDir_float3(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return float3x4_TransformDir_float_float_float(context, engine);
    printf("float3x4_TransformDir_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3x4_BatchTransform_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float4 *>(context->argument(0)) && QSVIsOfType<int>(context->argument(1)))
        return float3x4_BatchTransform_float4_ptr_int(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float4 *>(context->argument(0)) && QSVIsOfType<int>(context->argument(1)) && QSVIsOfType<int>(context->argument(2)))
        return float3x4_BatchTransform_float4_ptr_int_int(context, engine);
    printf("float3x4_BatchTransform_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3x4_Decompose_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 3 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<Quat>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)))
        return float3x4_Decompose_float3_Quat_float3(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3x3>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)))
        return float3x4_Decompose_float3_float3x3_float3(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3x4>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)))
        return float3x4_Decompose_float3_float3x4_float3(context, engine);
    printf("float3x4_Decompose_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3x4_Mul_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3x3>(context->argument(0)))
        return float3x4_Mul_float3x3(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3x4>(context->argument(0)))
        return float3x4_Mul_float3x4(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float4x4>(context->argument(0)))
        return float3x4_Mul_float4x4(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Quat>(context->argument(0)))
        return float3x4_Mul_Quat(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float4>(context->argument(0)))
        return float3x4_Mul_float4(context, engine);
    printf("float3x4_Mul_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3x4_Translate_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 3 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return float3x4_Translate_float_float_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return float3x4_Translate_float3(context, engine);
    printf("float3x4_Translate_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3x4_RotateX_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return float3x4_RotateX_float(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return float3x4_RotateX_float_float3(context, engine);
    printf("float3x4_RotateX_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3x4_RotateY_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return float3x4_RotateY_float(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return float3x4_RotateY_float_float3(context, engine);
    printf("float3x4_RotateY_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3x4_RotateZ_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return float3x4_RotateZ_float(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return float3x4_RotateZ_float_float3(context, engine);
    printf("float3x4_RotateZ_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3x4_RotateAxisAngle_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return float3x4_RotateAxisAngle_float3_float(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)))
        return float3x4_RotateAxisAngle_float3_float_float3(context, engine);
    printf("float3x4_RotateAxisAngle_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3x4_RotateFromTo_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return float3x4_RotateFromTo_float3_float3(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)))
        return float3x4_RotateFromTo_float3_float3_float3(context, engine);
    printf("float3x4_RotateFromTo_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3x4_FromQuat_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<Quat>(context->argument(0)))
        return float3x4_FromQuat_Quat(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<Quat>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return float3x4_FromQuat_Quat_float3(context, engine);
    printf("float3x4_FromQuat_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3x4_FromTRS_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 3 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<Quat>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)))
        return float3x4_FromTRS_float3_Quat_float3(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3x3>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)))
        return float3x4_FromTRS_float3_float3x3_float3(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3x4>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)))
        return float3x4_FromTRS_float3_float3x4_float3(context, engine);
    printf("float3x4_FromTRS_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3x4_Scale_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 3 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return float3x4_Scale_float_float_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return float3x4_Scale_float3(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return float3x4_Scale_float3_float3(context, engine);
    printf("float3x4_Scale_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3x4_ScaleAlongAxis_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return float3x4_ScaleAlongAxis_float3_float(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)))
        return float3x4_ScaleAlongAxis_float3_float_float3(context, engine);
    printf("float3x4_ScaleAlongAxis_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3x4_MakeOrthographicProjection_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 4 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)) && QSVIsOfType<float>(context->argument(3)))
        return float3x4_MakeOrthographicProjection_float_float_float_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Plane>(context->argument(0)))
        return float3x4_MakeOrthographicProjection_Plane(context, engine);
    printf("float3x4_MakeOrthographicProjection_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

void FromScriptValue_float3x4(const QScriptValue &obj, float3x4 &value)
{
    value = obj.data().toVariant().value<float3x4>();
}

QScriptValue ToScriptValue_float3x4(QScriptEngine *engine, const float3x4 &value)
{
    QScriptValue obj = engine->newObject();
    ToExistingScriptValue_float3x4(engine, value, obj);
    return obj;
}

QScriptValue ToScriptValue_const_float3x4(QScriptEngine *engine, const float3x4 &value)
{
    QScriptValue obj = engine->newObject();
    obj.setPrototype(engine->defaultPrototype(qMetaTypeId<float3x4>()));
    obj.setData(engine->newVariant(QVariant::fromValue(value)));
    return obj;
}

QScriptValue register_float3x4_prototype(QScriptEngine *engine)
{
    QScriptValue proto = engine->newObject();
    proto.setProperty("GetScale", engine->newFunction(float3x4_GetScale, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("At", engine->newFunction(float3x4_At_int_int, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Row", engine->newFunction(float3x4_Row_int, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Row3", engine->newFunction(float3x4_Row3_int, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Col", engine->newFunction(float3x4_Col_int, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Diagonal", engine->newFunction(float3x4_Diagonal, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ScaleRow3", engine->newFunction(float3x4_ScaleRow3_int_float, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ScaleRow", engine->newFunction(float3x4_ScaleRow_int_float, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ScaleCol", engine->newFunction(float3x4_ScaleCol_int_float, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Float3x3Part", engine->newFunction(float3x4_Float3x3Part, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("TranslatePart", engine->newFunction(float3x4_TranslatePart, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("RotatePart", engine->newFunction(float3x4_RotatePart, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("WorldX", engine->newFunction(float3x4_WorldX, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("WorldY", engine->newFunction(float3x4_WorldY, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("WorldZ", engine->newFunction(float3x4_WorldZ, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetRow", engine->newFunction(float3x4_SetRow_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetRow", engine->newFunction(float3x4_SetRow_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetRow", engine->newFunction(float3x4_SetRow_selector, 5), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetCol", engine->newFunction(float3x4_SetCol_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetCol", engine->newFunction(float3x4_SetCol_selector, 4), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Set", engine->newFunction(float3x4_Set_selector, 12), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Set", engine->newFunction(float3x4_Set_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Set3x3Part", engine->newFunction(float3x4_Set3x3Part_float3x3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetIdentity", engine->newFunction(float3x4_SetIdentity, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SwapColumns", engine->newFunction(float3x4_SwapColumns_int_int, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SwapRows", engine->newFunction(float3x4_SwapRows_int_int, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetTranslatePart", engine->newFunction(float3x4_SetTranslatePart_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetTranslatePart", engine->newFunction(float3x4_SetTranslatePart_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetRotatePartX", engine->newFunction(float3x4_SetRotatePartX_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetRotatePartY", engine->newFunction(float3x4_SetRotatePartY_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetRotatePartZ", engine->newFunction(float3x4_SetRotatePartZ_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetRotatePart", engine->newFunction(float3x4_SetRotatePart_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetRotatePart", engine->newFunction(float3x4_SetRotatePart_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("LookAt", engine->newFunction(float3x4_LookAt_float3_float3_float3_float3, 4), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Determinant", engine->newFunction(float3x4_Determinant, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Inverse", engine->newFunction(float3x4_Inverse, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Inverted", engine->newFunction(float3x4_Inverted, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("InverseOrthogonal", engine->newFunction(float3x4_InverseOrthogonal, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("InverseOrthogonalUniformScale", engine->newFunction(float3x4_InverseOrthogonalUniformScale, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("InverseOrthonormal", engine->newFunction(float3x4_InverseOrthonormal, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Transpose3", engine->newFunction(float3x4_Transpose3, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Transposed3", engine->newFunction(float3x4_Transposed3, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("InverseTranspose", engine->newFunction(float3x4_InverseTranspose, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("InverseTransposed", engine->newFunction(float3x4_InverseTransposed, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Trace", engine->newFunction(float3x4_Trace, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Orthonormalize", engine->newFunction(float3x4_Orthonormalize_int_int_int, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("RemoveScale", engine->newFunction(float3x4_RemoveScale, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("TransformPos", engine->newFunction(float3x4_TransformPos_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("TransformPos", engine->newFunction(float3x4_TransformPos_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("TransformDir", engine->newFunction(float3x4_TransformDir_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("TransformDir", engine->newFunction(float3x4_TransformDir_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Transform", engine->newFunction(float3x4_Transform_float4, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("BatchTransform", engine->newFunction(float3x4_BatchTransform_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("BatchTransform", engine->newFunction(float3x4_BatchTransform_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsFinite", engine->newFunction(float3x4_IsFinite, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsIdentity", engine->newFunction(float3x4_IsIdentity_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsLowerTriangular", engine->newFunction(float3x4_IsLowerTriangular_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsUpperTriangular", engine->newFunction(float3x4_IsUpperTriangular_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsInvertible", engine->newFunction(float3x4_IsInvertible_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsSymmetric", engine->newFunction(float3x4_IsSymmetric_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsSkewSymmetric", engine->newFunction(float3x4_IsSkewSymmetric_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("HasUnitaryScale", engine->newFunction(float3x4_HasUnitaryScale_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("HasNegativeScale", engine->newFunction(float3x4_HasNegativeScale, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("HasUniformScale", engine->newFunction(float3x4_HasUniformScale_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsOrthogonal", engine->newFunction(float3x4_IsOrthogonal_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsOrthonormal", engine->newFunction(float3x4_IsOrthonormal_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Equals", engine->newFunction(float3x4_Equals_float3x4_float, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToEulerXYX", engine->newFunction(float3x4_ToEulerXYX, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToEulerXZX", engine->newFunction(float3x4_ToEulerXZX, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToEulerYXY", engine->newFunction(float3x4_ToEulerYXY, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToEulerYZY", engine->newFunction(float3x4_ToEulerYZY, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToEulerZXZ", engine->newFunction(float3x4_ToEulerZXZ, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToEulerZYZ", engine->newFunction(float3x4_ToEulerZYZ, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToEulerXYZ", engine->newFunction(float3x4_ToEulerXYZ, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToEulerXZY", engine->newFunction(float3x4_ToEulerXZY, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToEulerYXZ", engine->newFunction(float3x4_ToEulerYXZ, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToEulerYZX", engine->newFunction(float3x4_ToEulerYZX, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToEulerZXY", engine->newFunction(float3x4_ToEulerZXY, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToEulerZYX", engine->newFunction(float3x4_ToEulerZYX, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ExtractScale", engine->newFunction(float3x4_ExtractScale, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Decompose", engine->newFunction(float3x4_Decompose_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Mul", engine->newFunction(float3x4_Mul_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MulPos", engine->newFunction(float3x4_MulPos_float3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MulDir", engine->newFunction(float3x4_MulDir_float3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("toString", engine->newFunction(float3x4_toString, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("metaTypeId", engine->toScriptValue<qint32>((qint32)qMetaTypeId<float3x4>()));
    engine->setDefaultPrototype(qMetaTypeId<float3x4>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<float3x4*>(), proto);
    qScriptRegisterMetaType(engine, ToScriptValue_float3x4, FromScriptValue_float3x4, proto);

    QScriptValue ctor = engine->newFunction(float3x4_ctor, proto, 12);
    ctor.setProperty("Translate", engine->newFunction(float3x4_Translate_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("Translate", engine->newFunction(float3x4_Translate_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("RotateX", engine->newFunction(float3x4_RotateX_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("RotateX", engine->newFunction(float3x4_RotateX_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("RotateY", engine->newFunction(float3x4_RotateY_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("RotateY", engine->newFunction(float3x4_RotateY_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("RotateZ", engine->newFunction(float3x4_RotateZ_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("RotateZ", engine->newFunction(float3x4_RotateZ_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("RotateAxisAngle", engine->newFunction(float3x4_RotateAxisAngle_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("RotateAxisAngle", engine->newFunction(float3x4_RotateAxisAngle_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("RotateFromTo", engine->newFunction(float3x4_RotateFromTo_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("RotateFromTo", engine->newFunction(float3x4_RotateFromTo_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromQuat", engine->newFunction(float3x4_FromQuat_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromQuat", engine->newFunction(float3x4_FromQuat_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromTRS", engine->newFunction(float3x4_FromTRS_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromEulerXYX", engine->newFunction(float3x4_FromEulerXYX_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromEulerXZX", engine->newFunction(float3x4_FromEulerXZX_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromEulerYXY", engine->newFunction(float3x4_FromEulerYXY_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromEulerYZY", engine->newFunction(float3x4_FromEulerYZY_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromEulerZXZ", engine->newFunction(float3x4_FromEulerZXZ_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromEulerZYZ", engine->newFunction(float3x4_FromEulerZYZ_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromEulerXYZ", engine->newFunction(float3x4_FromEulerXYZ_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromEulerXZY", engine->newFunction(float3x4_FromEulerXZY_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromEulerYXZ", engine->newFunction(float3x4_FromEulerYXZ_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromEulerYZX", engine->newFunction(float3x4_FromEulerYZX_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromEulerZXY", engine->newFunction(float3x4_FromEulerZXY_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromEulerZYX", engine->newFunction(float3x4_FromEulerZYX_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("Scale", engine->newFunction(float3x4_Scale_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("Scale", engine->newFunction(float3x4_Scale_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("Scale", engine->newFunction(float3x4_Scale_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("ScaleAlongAxis", engine->newFunction(float3x4_ScaleAlongAxis_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("ScaleAlongAxis", engine->newFunction(float3x4_ScaleAlongAxis_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("UniformScale", engine->newFunction(float3x4_UniformScale_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("ShearX", engine->newFunction(float3x4_ShearX_float_float, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("ShearY", engine->newFunction(float3x4_ShearY_float_float, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("ShearZ", engine->newFunction(float3x4_ShearZ_float_float, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("Reflect", engine->newFunction(float3x4_Reflect_Plane, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("MakeOrthographicProjection", engine->newFunction(float3x4_MakeOrthographicProjection_selector, 4), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("MakeOrthographicProjection", engine->newFunction(float3x4_MakeOrthographicProjection_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("MakeOrthographicProjectionYZ", engine->newFunction(float3x4_MakeOrthographicProjectionYZ, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("MakeOrthographicProjectionXZ", engine->newFunction(float3x4_MakeOrthographicProjectionXZ, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("MakeOrthographicProjectionXY", engine->newFunction(float3x4_MakeOrthographicProjectionXY, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("zero", qScriptValueFromValue(engine, float3x4::zero), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("identity", qScriptValueFromValue(engine, float3x4::identity), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("nan", qScriptValueFromValue(engine, float3x4::nan), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    engine->globalObject().setProperty("float3x4", ctor, QScriptValue::Undeletable | QScriptValue::ReadOnly);

    return ctor;
}

