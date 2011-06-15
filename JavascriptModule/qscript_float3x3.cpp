#include "QtScriptBindingsHelpers.h"

void ToExistingScriptValue_float3x3(QScriptEngine *engine, const float3x3 &value, QScriptValue obj)
{
}

static QScriptValue float3x3_float3x3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_float3x3 in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 ret;
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_float3x3_float_float_float_float_float_float_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 9) { printf("Error! Invalid number of arguments passed to function float3x3_float3x3_float_float_float_float_float_float_float_float_float in file %s, line %d!\nExpected 9, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float _00 = qscriptvalue_cast<float>(context->argument(0));
    float _01 = qscriptvalue_cast<float>(context->argument(1));
    float _02 = qscriptvalue_cast<float>(context->argument(2));
    float _10 = qscriptvalue_cast<float>(context->argument(3));
    float _11 = qscriptvalue_cast<float>(context->argument(4));
    float _12 = qscriptvalue_cast<float>(context->argument(5));
    float _20 = qscriptvalue_cast<float>(context->argument(6));
    float _21 = qscriptvalue_cast<float>(context->argument(7));
    float _22 = qscriptvalue_cast<float>(context->argument(8));
    float3x3 ret(_00, _01, _02, _10, _11, _12, _20, _21, _22);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_float3x3_float3_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x3_float3x3_float3_float3_float3 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 col0 = qscriptvalue_cast<float3>(context->argument(0));
    float3 col1 = qscriptvalue_cast<float3>(context->argument(1));
    float3 col2 = qscriptvalue_cast<float3>(context->argument(2));
    float3x3 ret(col0, col1, col2);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_float3x3_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_float3x3_Quat in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat orientation = qscriptvalue_cast<Quat>(context->argument(0));
    float3x3 ret(orientation);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_ToQuat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_ToQuat in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    Quat ret = This.ToQuat();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_GetScale(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_GetScale in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float3 ret = This.GetScale();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_At_int_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_At_int_int in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    int row = qscriptvalue_cast<int>(context->argument(0));
    int col = qscriptvalue_cast<int>(context->argument(1));
    const float ret = This.At(row, col);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_Row_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_Row_int in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    int row = qscriptvalue_cast<int>(context->argument(0));
    const float3 & ret = This.Row(row);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_Col_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_Col_int in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    int col = qscriptvalue_cast<int>(context->argument(0));
    const float3 ret = This.Col(col);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_Diagonal(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_Diagonal in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    const float3 ret = This.Diagonal();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_ScaleRow_int_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_ScaleRow_int_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    int row = qscriptvalue_cast<int>(context->argument(0));
    float scalar = qscriptvalue_cast<float>(context->argument(1));
    This.ScaleRow(row, scalar);
    ToExistingScriptValue_float3x3(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x3_ScaleCol_int_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_ScaleCol_int_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    int col = qscriptvalue_cast<int>(context->argument(0));
    float scalar = qscriptvalue_cast<float>(context->argument(1));
    This.ScaleCol(col, scalar);
    ToExistingScriptValue_float3x3(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x3_WorldX(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_WorldX in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float3 ret = This.WorldX();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_WorldY(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_WorldY in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float3 ret = This.WorldY();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_WorldZ(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_WorldZ in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float3 ret = This.WorldZ();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_SetRow_int_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function float3x3_SetRow_int_float_float_float in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    int row = qscriptvalue_cast<int>(context->argument(0));
    float x = qscriptvalue_cast<float>(context->argument(1));
    float y = qscriptvalue_cast<float>(context->argument(2));
    float z = qscriptvalue_cast<float>(context->argument(3));
    This.SetRow(row, x, y, z);
    ToExistingScriptValue_float3x3(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x3_SetRow_int_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_SetRow_int_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    int row = qscriptvalue_cast<int>(context->argument(0));
    float3 rowVector = qscriptvalue_cast<float3>(context->argument(1));
    This.SetRow(row, rowVector);
    ToExistingScriptValue_float3x3(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x3_SetCol_int_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function float3x3_SetCol_int_float_float_float in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    int column = qscriptvalue_cast<int>(context->argument(0));
    float x = qscriptvalue_cast<float>(context->argument(1));
    float y = qscriptvalue_cast<float>(context->argument(2));
    float z = qscriptvalue_cast<float>(context->argument(3));
    This.SetCol(column, x, y, z);
    ToExistingScriptValue_float3x3(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x3_SetCol_int_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_SetCol_int_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    int column = qscriptvalue_cast<int>(context->argument(0));
    float3 columnVector = qscriptvalue_cast<float3>(context->argument(1));
    This.SetCol(column, columnVector);
    ToExistingScriptValue_float3x3(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x3_Set_float_float_float_float_float_float_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 9) { printf("Error! Invalid number of arguments passed to function float3x3_Set_float_float_float_float_float_float_float_float_float in file %s, line %d!\nExpected 9, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float _00 = qscriptvalue_cast<float>(context->argument(0));
    float _01 = qscriptvalue_cast<float>(context->argument(1));
    float _02 = qscriptvalue_cast<float>(context->argument(2));
    float _10 = qscriptvalue_cast<float>(context->argument(3));
    float _11 = qscriptvalue_cast<float>(context->argument(4));
    float _12 = qscriptvalue_cast<float>(context->argument(5));
    float _20 = qscriptvalue_cast<float>(context->argument(6));
    float _21 = qscriptvalue_cast<float>(context->argument(7));
    float _22 = qscriptvalue_cast<float>(context->argument(8));
    This.Set(_00, _01, _02, _10, _11, _12, _20, _21, _22);
    ToExistingScriptValue_float3x3(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x3_Set_int_int_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x3_Set_int_int_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    int row = qscriptvalue_cast<int>(context->argument(0));
    int col = qscriptvalue_cast<int>(context->argument(1));
    float value = qscriptvalue_cast<float>(context->argument(2));
    This.Set(row, col, value);
    ToExistingScriptValue_float3x3(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x3_SetIdentity(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_SetIdentity in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    This.SetIdentity();
    ToExistingScriptValue_float3x3(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x3_SwapColumns_int_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_SwapColumns_int_int in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    int col1 = qscriptvalue_cast<int>(context->argument(0));
    int col2 = qscriptvalue_cast<int>(context->argument(1));
    This.SwapColumns(col1, col2);
    ToExistingScriptValue_float3x3(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x3_SwapRows_int_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_SwapRows_int_int in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    int row1 = qscriptvalue_cast<int>(context->argument(0));
    int row2 = qscriptvalue_cast<int>(context->argument(1));
    This.SwapRows(row1, row2);
    ToExistingScriptValue_float3x3(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x3_SetRotatePartX_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_SetRotatePartX_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float angleRadians = qscriptvalue_cast<float>(context->argument(0));
    This.SetRotatePartX(angleRadians);
    ToExistingScriptValue_float3x3(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x3_SetRotatePartY_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_SetRotatePartY_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float angleRadians = qscriptvalue_cast<float>(context->argument(0));
    This.SetRotatePartY(angleRadians);
    ToExistingScriptValue_float3x3(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x3_SetRotatePartZ_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_SetRotatePartZ_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float angleRadians = qscriptvalue_cast<float>(context->argument(0));
    This.SetRotatePartZ(angleRadians);
    ToExistingScriptValue_float3x3(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x3_SetRotatePart_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_SetRotatePart_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float3 axisDirection = qscriptvalue_cast<float3>(context->argument(0));
    float angleRadians = qscriptvalue_cast<float>(context->argument(1));
    This.SetRotatePart(axisDirection, angleRadians);
    ToExistingScriptValue_float3x3(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x3_SetRotatePart_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_SetRotatePart_Quat in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    Quat orientation = qscriptvalue_cast<Quat>(context->argument(0));
    This.SetRotatePart(orientation);
    ToExistingScriptValue_float3x3(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x3_Determinant(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_Determinant in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float ret = This.Determinant();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_Inverse(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_Inverse in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    bool ret = This.Inverse();
    ToExistingScriptValue_float3x3(engine, This, context->thisObject());
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_Inverted(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_Inverted in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float3x3 ret = This.Inverted();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_InverseOrthogonal(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_InverseOrthogonal in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    bool ret = This.InverseOrthogonal();
    ToExistingScriptValue_float3x3(engine, This, context->thisObject());
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_InverseOrthogonalUniformScale(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_InverseOrthogonalUniformScale in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    bool ret = This.InverseOrthogonalUniformScale();
    ToExistingScriptValue_float3x3(engine, This, context->thisObject());
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_InverseOrthonormal(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_InverseOrthonormal in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    This.InverseOrthonormal();
    ToExistingScriptValue_float3x3(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x3_Transpose(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_Transpose in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    This.Transpose();
    ToExistingScriptValue_float3x3(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x3_Transposed(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_Transposed in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float3x3 ret = This.Transposed();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_InverseTranspose(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_InverseTranspose in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    bool ret = This.InverseTranspose();
    ToExistingScriptValue_float3x3(engine, This, context->thisObject());
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_InverseTransposed(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_InverseTransposed in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float3x3 ret = This.InverseTransposed();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_Trace(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_Trace in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float ret = This.Trace();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_Orthonormalize_int_int_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x3_Orthonormalize_int_int_int in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    int firstColumn = qscriptvalue_cast<int>(context->argument(0));
    int secondColumn = qscriptvalue_cast<int>(context->argument(1));
    int thirdColumn = qscriptvalue_cast<int>(context->argument(2));
    This.Orthonormalize(firstColumn, secondColumn, thirdColumn);
    ToExistingScriptValue_float3x3(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x3_RemoveScale(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_RemoveScale in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    This.RemoveScale();
    ToExistingScriptValue_float3x3(engine, This, context->thisObject());
    return QScriptValue();
}

static QScriptValue float3x3_Transform_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_Transform_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float3 vector = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.Transform(vector);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_Transform_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x3_Transform_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float x = qscriptvalue_cast<float>(context->argument(0));
    float y = qscriptvalue_cast<float>(context->argument(1));
    float z = qscriptvalue_cast<float>(context->argument(2));
    float3 ret = This.Transform(x, y, z);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_TransformLeft_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_TransformLeft_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float3 lhs = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.TransformLeft(lhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_Transform_float4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_Transform_float4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float4 vector = qscriptvalue_cast<float4>(context->argument(0));
    float4 ret = This.Transform(vector);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_BatchTransform_float4_ptr_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_BatchTransform_float4_ptr_int in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float4 * vectorArray = qscriptvalue_cast<float4 *>(context->argument(0));
    int numVectors = qscriptvalue_cast<int>(context->argument(1));
    This.BatchTransform(vectorArray, numVectors);
    return QScriptValue();
}

static QScriptValue float3x3_BatchTransform_float4_ptr_int_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x3_BatchTransform_float4_ptr_int_int in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float4 * vectorArray = qscriptvalue_cast<float4 *>(context->argument(0));
    int numVectors = qscriptvalue_cast<int>(context->argument(1));
    int stride = qscriptvalue_cast<int>(context->argument(2));
    This.BatchTransform(vectorArray, numVectors, stride);
    return QScriptValue();
}

static QScriptValue float3x3_IsFinite(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_IsFinite in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    bool ret = This.IsFinite();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_IsIdentity_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_IsIdentity_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float epsilon = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.IsIdentity(epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_IsLowerTriangular_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_IsLowerTriangular_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float epsilon = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.IsLowerTriangular(epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_IsUpperTriangular_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_IsUpperTriangular_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float epsilon = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.IsUpperTriangular(epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_IsInvertible_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_IsInvertible_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float epsilon = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.IsInvertible(epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_IsSymmetric_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_IsSymmetric_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float epsilon = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.IsSymmetric(epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_IsSkewSymmetric_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_IsSkewSymmetric_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float epsilon = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.IsSkewSymmetric(epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_HasUnitaryScale_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_HasUnitaryScale_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float epsilonSq = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.HasUnitaryScale(epsilonSq);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_HasNegativeScale(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_HasNegativeScale in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    bool ret = This.HasNegativeScale();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_HasUniformScale_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_HasUniformScale_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float epsilonSq = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.HasUniformScale(epsilonSq);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_IsOrthogonal_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_IsOrthogonal_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float epsilon = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.IsOrthogonal(epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_IsOrthonormal_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_IsOrthonormal_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float epsilon = qscriptvalue_cast<float>(context->argument(0));
    bool ret = This.IsOrthonormal(epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_Equals_float3x3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_Equals_float3x3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float3x3 other = qscriptvalue_cast<float3x3>(context->argument(0));
    float epsilon = qscriptvalue_cast<float>(context->argument(1));
    bool ret = This.Equals(other, epsilon);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_ToEulerXYX(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_ToEulerXYX in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float3 ret = This.ToEulerXYX();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_ToEulerXZX(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_ToEulerXZX in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float3 ret = This.ToEulerXZX();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_ToEulerYXY(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_ToEulerYXY in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float3 ret = This.ToEulerYXY();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_ToEulerYZY(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_ToEulerYZY in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float3 ret = This.ToEulerYZY();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_ToEulerZXZ(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_ToEulerZXZ in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float3 ret = This.ToEulerZXZ();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_ToEulerZYZ(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_ToEulerZYZ in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float3 ret = This.ToEulerZYZ();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_ToEulerXYZ(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_ToEulerXYZ in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float3 ret = This.ToEulerXYZ();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_ToEulerXZY(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_ToEulerXZY in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float3 ret = This.ToEulerXZY();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_ToEulerYXZ(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_ToEulerYXZ in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float3 ret = This.ToEulerYXZ();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_ToEulerYZX(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_ToEulerYZX in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float3 ret = This.ToEulerYZX();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_ToEulerZXY(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_ToEulerZXY in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float3 ret = This.ToEulerZXY();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_ToEulerZYX(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_ToEulerZYX in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float3 ret = This.ToEulerZYX();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_ExtractScale(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_ExtractScale in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float3 ret = This.ExtractScale();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_Decompose_Quat_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_Decompose_Quat_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    Quat rotate = qscriptvalue_cast<Quat>(context->argument(0));
    float3 scale = qscriptvalue_cast<float3>(context->argument(1));
    This.Decompose(rotate, scale);
    return QScriptValue();
}

static QScriptValue float3x3_Decompose_float3x3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_Decompose_float3x3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float3x3 rotate = qscriptvalue_cast<float3x3>(context->argument(0));
    float3 scale = qscriptvalue_cast<float3>(context->argument(1));
    This.Decompose(rotate, scale);
    return QScriptValue();
}

static QScriptValue float3x3_Mul_float3x3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_Mul_float3x3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float3x3 rhs = qscriptvalue_cast<float3x3>(context->argument(0));
    float3x3 ret = This.Mul(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_Mul_float3x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_Mul_float3x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float3x4 rhs = qscriptvalue_cast<float3x4>(context->argument(0));
    float3x4 ret = This.Mul(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_Mul_float4x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_Mul_float4x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float4x4 rhs = qscriptvalue_cast<float4x4>(context->argument(0));
    float4x4 ret = This.Mul(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_Mul_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_Mul_Quat in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    Quat rhs = qscriptvalue_cast<Quat>(context->argument(0));
    float3x3 ret = This.Mul(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_Mul_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_Mul_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float3 rhs = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.Mul(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_MulPos_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_MulPos_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float3 rhs = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.MulPos(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_MulDir_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_MulDir_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This = qscriptvalue_cast<float3x3>(context->thisObject());
    float3 rhs = qscriptvalue_cast<float3>(context->argument(0));
    float3 ret = This.MulDir(rhs);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_toString(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_toString in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 This;
    if (context->argumentCount() > 0) This = qscriptvalue_cast<float3x3>(context->argument(0)); // Qt oddity (bug?): Sometimes the built-in toString() function doesn't give us this from thisObject, but as the first argument.
    else This = qscriptvalue_cast<float3x3>(context->thisObject());
    QString ret = This.toString();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_RotateX_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_RotateX_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float angleRadians = qscriptvalue_cast<float>(context->argument(0));
    float3x3 ret = float3x3::RotateX(angleRadians);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_RotateY_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_RotateY_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float angleRadians = qscriptvalue_cast<float>(context->argument(0));
    float3x3 ret = float3x3::RotateY(angleRadians);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_RotateZ_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_RotateZ_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float angleRadians = qscriptvalue_cast<float>(context->argument(0));
    float3x3 ret = float3x3::RotateZ(angleRadians);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_RotateAxisAngle_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_RotateAxisAngle_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 axisDirection = qscriptvalue_cast<float3>(context->argument(0));
    float angleRadians = qscriptvalue_cast<float>(context->argument(1));
    float3x3 ret = float3x3::RotateAxisAngle(axisDirection, angleRadians);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_RotateFromTo_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_RotateFromTo_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 sourceDirection = qscriptvalue_cast<float3>(context->argument(0));
    float3 targetDirection = qscriptvalue_cast<float3>(context->argument(1));
    float3x3 ret = float3x3::RotateFromTo(sourceDirection, targetDirection);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_FromQuat_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_FromQuat_Quat in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat orientation = qscriptvalue_cast<Quat>(context->argument(0));
    float3x3 ret = float3x3::FromQuat(orientation);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_FromRS_Quat_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_FromRS_Quat_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Quat rotate = qscriptvalue_cast<Quat>(context->argument(0));
    float3 scale = qscriptvalue_cast<float3>(context->argument(1));
    float3x3 ret = float3x3::FromRS(rotate, scale);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_FromRS_float3x3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_FromRS_float3x3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 rotate = qscriptvalue_cast<float3x3>(context->argument(0));
    float3 scale = qscriptvalue_cast<float3>(context->argument(1));
    float3x3 ret = float3x3::FromRS(rotate, scale);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_FromEulerXYX_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x3_FromEulerXYX_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float ex = qscriptvalue_cast<float>(context->argument(0));
    float ey = qscriptvalue_cast<float>(context->argument(1));
    float ex2 = qscriptvalue_cast<float>(context->argument(2));
    float3x3 ret = float3x3::FromEulerXYX(ex, ey, ex2);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_FromEulerXZX_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x3_FromEulerXZX_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float ex = qscriptvalue_cast<float>(context->argument(0));
    float ez = qscriptvalue_cast<float>(context->argument(1));
    float ex2 = qscriptvalue_cast<float>(context->argument(2));
    float3x3 ret = float3x3::FromEulerXZX(ex, ez, ex2);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_FromEulerYXY_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x3_FromEulerYXY_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float ey = qscriptvalue_cast<float>(context->argument(0));
    float ex = qscriptvalue_cast<float>(context->argument(1));
    float ey2 = qscriptvalue_cast<float>(context->argument(2));
    float3x3 ret = float3x3::FromEulerYXY(ey, ex, ey2);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_FromEulerYZY_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x3_FromEulerYZY_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float ey = qscriptvalue_cast<float>(context->argument(0));
    float ez = qscriptvalue_cast<float>(context->argument(1));
    float ey2 = qscriptvalue_cast<float>(context->argument(2));
    float3x3 ret = float3x3::FromEulerYZY(ey, ez, ey2);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_FromEulerZXZ_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x3_FromEulerZXZ_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float ez = qscriptvalue_cast<float>(context->argument(0));
    float ex = qscriptvalue_cast<float>(context->argument(1));
    float ez2 = qscriptvalue_cast<float>(context->argument(2));
    float3x3 ret = float3x3::FromEulerZXZ(ez, ex, ez2);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_FromEulerZYZ_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x3_FromEulerZYZ_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float ez = qscriptvalue_cast<float>(context->argument(0));
    float ey = qscriptvalue_cast<float>(context->argument(1));
    float ez2 = qscriptvalue_cast<float>(context->argument(2));
    float3x3 ret = float3x3::FromEulerZYZ(ez, ey, ez2);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_FromEulerXYZ_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x3_FromEulerXYZ_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float ex = qscriptvalue_cast<float>(context->argument(0));
    float ey = qscriptvalue_cast<float>(context->argument(1));
    float ez = qscriptvalue_cast<float>(context->argument(2));
    float3x3 ret = float3x3::FromEulerXYZ(ex, ey, ez);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_FromEulerXZY_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x3_FromEulerXZY_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float ex = qscriptvalue_cast<float>(context->argument(0));
    float ez = qscriptvalue_cast<float>(context->argument(1));
    float ey = qscriptvalue_cast<float>(context->argument(2));
    float3x3 ret = float3x3::FromEulerXZY(ex, ez, ey);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_FromEulerYXZ_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x3_FromEulerYXZ_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float ey = qscriptvalue_cast<float>(context->argument(0));
    float ex = qscriptvalue_cast<float>(context->argument(1));
    float ez = qscriptvalue_cast<float>(context->argument(2));
    float3x3 ret = float3x3::FromEulerYXZ(ey, ex, ez);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_FromEulerYZX_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x3_FromEulerYZX_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float ey = qscriptvalue_cast<float>(context->argument(0));
    float ez = qscriptvalue_cast<float>(context->argument(1));
    float ex = qscriptvalue_cast<float>(context->argument(2));
    float3x3 ret = float3x3::FromEulerYZX(ey, ez, ex);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_FromEulerZXY_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x3_FromEulerZXY_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float ez = qscriptvalue_cast<float>(context->argument(0));
    float ex = qscriptvalue_cast<float>(context->argument(1));
    float ey = qscriptvalue_cast<float>(context->argument(2));
    float3x3 ret = float3x3::FromEulerZXY(ez, ex, ey);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_FromEulerZYX_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x3_FromEulerZYX_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float ez = qscriptvalue_cast<float>(context->argument(0));
    float ey = qscriptvalue_cast<float>(context->argument(1));
    float ex = qscriptvalue_cast<float>(context->argument(2));
    float3x3 ret = float3x3::FromEulerZYX(ez, ey, ex);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_Scale_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x3_Scale_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float sx = qscriptvalue_cast<float>(context->argument(0));
    float sy = qscriptvalue_cast<float>(context->argument(1));
    float sz = qscriptvalue_cast<float>(context->argument(2));
    ScaleOp ret = float3x3::Scale(sx, sy, sz);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_Scale_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_Scale_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 scale = qscriptvalue_cast<float3>(context->argument(0));
    ScaleOp ret = float3x3::Scale(scale);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_ScaleAlongAxis_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_ScaleAlongAxis_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3 axis = qscriptvalue_cast<float3>(context->argument(0));
    float scalingFactor = qscriptvalue_cast<float>(context->argument(1));
    float3x3 ret = float3x3::ScaleAlongAxis(axis, scalingFactor);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_UniformScale_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_UniformScale_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float uniformScale = qscriptvalue_cast<float>(context->argument(0));
    ScaleOp ret = float3x3::UniformScale(uniformScale);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_ShearX_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_ShearX_float_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float yFactor = qscriptvalue_cast<float>(context->argument(0));
    float zFactor = qscriptvalue_cast<float>(context->argument(1));
    float3x3 ret = float3x3::ShearX(yFactor, zFactor);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_ShearY_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_ShearY_float_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float xFactor = qscriptvalue_cast<float>(context->argument(0));
    float zFactor = qscriptvalue_cast<float>(context->argument(1));
    float3x3 ret = float3x3::ShearY(xFactor, zFactor);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_ShearZ_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_ShearZ_float_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float xFactor = qscriptvalue_cast<float>(context->argument(0));
    float yFactor = qscriptvalue_cast<float>(context->argument(1));
    float3x3 ret = float3x3::ShearZ(xFactor, yFactor);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_Reflect_Plane(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_Reflect_Plane in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane p = qscriptvalue_cast<Plane>(context->argument(0));
    float3x3 ret = float3x3::Reflect(p);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_MakeOrthographicProjection_float_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function float3x3_MakeOrthographicProjection_float_float_float_float in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float nearPlaneDistance = qscriptvalue_cast<float>(context->argument(0));
    float farPlaneDistance = qscriptvalue_cast<float>(context->argument(1));
    float horizontalViewportSize = qscriptvalue_cast<float>(context->argument(2));
    float verticalViewportSize = qscriptvalue_cast<float>(context->argument(3));
    float3x3 ret = float3x3::MakeOrthographicProjection(nearPlaneDistance, farPlaneDistance, horizontalViewportSize, verticalViewportSize);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_MakeOrthographicProjection_Plane(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_MakeOrthographicProjection_Plane in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Plane target = qscriptvalue_cast<Plane>(context->argument(0));
    float3x3 ret = float3x3::MakeOrthographicProjection(target);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_MakeOrthographicProjectionYZ(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_MakeOrthographicProjectionYZ in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 ret = float3x3::MakeOrthographicProjectionYZ();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_MakeOrthographicProjectionXZ(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_MakeOrthographicProjectionXZ in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 ret = float3x3::MakeOrthographicProjectionXZ();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_MakeOrthographicProjectionXY(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_MakeOrthographicProjectionXY in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float3x3 ret = float3x3::MakeOrthographicProjectionXY();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue float3x3_ctor(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return float3x3_float3x3(context, engine);
    if (context->argumentCount() == 9 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)) && QSVIsOfType<float>(context->argument(3)) && QSVIsOfType<float>(context->argument(4)) && QSVIsOfType<float>(context->argument(5)) && QSVIsOfType<float>(context->argument(6)) && QSVIsOfType<float>(context->argument(7)) && QSVIsOfType<float>(context->argument(8)))
        return float3x3_float3x3_float_float_float_float_float_float_float_float_float(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)))
        return float3x3_float3x3_float3_float3_float3(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Quat>(context->argument(0)))
        return float3x3_float3x3_Quat(context, engine);
    printf("float3x3_ctor failed to choose the right function to call! Did you use 'var x = float3x3();' instead of 'var x = new float3x3();'?\n"); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3x3_SetRow_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 4 && QSVIsOfType<int>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)) && QSVIsOfType<float>(context->argument(3)))
        return float3x3_SetRow_int_float_float_float(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<int>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return float3x3_SetRow_int_float3(context, engine);
    printf("float3x3_SetRow_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3x3_SetCol_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 4 && QSVIsOfType<int>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)) && QSVIsOfType<float>(context->argument(3)))
        return float3x3_SetCol_int_float_float_float(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<int>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return float3x3_SetCol_int_float3(context, engine);
    printf("float3x3_SetCol_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3x3_Set_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 9 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)) && QSVIsOfType<float>(context->argument(3)) && QSVIsOfType<float>(context->argument(4)) && QSVIsOfType<float>(context->argument(5)) && QSVIsOfType<float>(context->argument(6)) && QSVIsOfType<float>(context->argument(7)) && QSVIsOfType<float>(context->argument(8)))
        return float3x3_Set_float_float_float_float_float_float_float_float_float(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<int>(context->argument(0)) && QSVIsOfType<int>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return float3x3_Set_int_int_float(context, engine);
    printf("float3x3_Set_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3x3_SetRotatePart_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return float3x3_SetRotatePart_float3_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Quat>(context->argument(0)))
        return float3x3_SetRotatePart_Quat(context, engine);
    printf("float3x3_SetRotatePart_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3x3_Transform_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return float3x3_Transform_float3(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return float3x3_Transform_float_float_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float4>(context->argument(0)))
        return float3x3_Transform_float4(context, engine);
    printf("float3x3_Transform_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3x3_BatchTransform_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float4 *>(context->argument(0)) && QSVIsOfType<int>(context->argument(1)))
        return float3x3_BatchTransform_float4_ptr_int(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float4 *>(context->argument(0)) && QSVIsOfType<int>(context->argument(1)) && QSVIsOfType<int>(context->argument(2)))
        return float3x3_BatchTransform_float4_ptr_int_int(context, engine);
    printf("float3x3_BatchTransform_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3x3_Decompose_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<Quat>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return float3x3_Decompose_Quat_float3(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float3x3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return float3x3_Decompose_float3x3_float3(context, engine);
    printf("float3x3_Decompose_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3x3_Mul_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3x3>(context->argument(0)))
        return float3x3_Mul_float3x3(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3x4>(context->argument(0)))
        return float3x3_Mul_float3x4(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float4x4>(context->argument(0)))
        return float3x3_Mul_float4x4(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Quat>(context->argument(0)))
        return float3x3_Mul_Quat(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return float3x3_Mul_float3(context, engine);
    printf("float3x3_Mul_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3x3_FromRS_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<Quat>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return float3x3_FromRS_Quat_float3(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float3x3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return float3x3_FromRS_float3x3_float3(context, engine);
    printf("float3x3_FromRS_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3x3_Scale_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 3 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return float3x3_Scale_float_float_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return float3x3_Scale_float3(context, engine);
    printf("float3x3_Scale_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

static QScriptValue float3x3_MakeOrthographicProjection_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 4 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)) && QSVIsOfType<float>(context->argument(3)))
        return float3x3_MakeOrthographicProjection_float_float_float_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Plane>(context->argument(0)))
        return float3x3_MakeOrthographicProjection_Plane(context, engine);
    printf("float3x3_MakeOrthographicProjection_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); PrintCallStack(context->backtrace()); return QScriptValue();
}

void FromScriptValue_float3x3(const QScriptValue &obj, float3x3 &value)
{
}

QScriptValue ToScriptValue_float3x3(QScriptEngine *engine, const float3x3 &value)
{
    QScriptValue obj = engine->newObject();
    ToExistingScriptValue_float3x3(engine, value, obj);
    return obj;
}

QScriptValue ToScriptValue_const_float3x3(QScriptEngine *engine, const float3x3 &value)
{
    QScriptValue obj = engine->newObject();
    obj.setPrototype(engine->defaultPrototype(qMetaTypeId<float3x3>()));
    return obj;
}

QScriptValue register_float3x3_prototype(QScriptEngine *engine)
{
    QScriptValue proto = engine->newObject();
    proto.setProperty("ToQuat", engine->newFunction(float3x3_ToQuat, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("GetScale", engine->newFunction(float3x3_GetScale, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("At", engine->newFunction(float3x3_At_int_int, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Row", engine->newFunction(float3x3_Row_int, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Col", engine->newFunction(float3x3_Col_int, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Diagonal", engine->newFunction(float3x3_Diagonal, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ScaleRow", engine->newFunction(float3x3_ScaleRow_int_float, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ScaleCol", engine->newFunction(float3x3_ScaleCol_int_float, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("WorldX", engine->newFunction(float3x3_WorldX, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("WorldY", engine->newFunction(float3x3_WorldY, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("WorldZ", engine->newFunction(float3x3_WorldZ, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetRow", engine->newFunction(float3x3_SetRow_selector, 4), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetRow", engine->newFunction(float3x3_SetRow_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetCol", engine->newFunction(float3x3_SetCol_selector, 4), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetCol", engine->newFunction(float3x3_SetCol_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Set", engine->newFunction(float3x3_Set_selector, 9), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Set", engine->newFunction(float3x3_Set_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetIdentity", engine->newFunction(float3x3_SetIdentity, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SwapColumns", engine->newFunction(float3x3_SwapColumns_int_int, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SwapRows", engine->newFunction(float3x3_SwapRows_int_int, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetRotatePartX", engine->newFunction(float3x3_SetRotatePartX_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetRotatePartY", engine->newFunction(float3x3_SetRotatePartY_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetRotatePartZ", engine->newFunction(float3x3_SetRotatePartZ_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetRotatePart", engine->newFunction(float3x3_SetRotatePart_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SetRotatePart", engine->newFunction(float3x3_SetRotatePart_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Determinant", engine->newFunction(float3x3_Determinant, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Inverse", engine->newFunction(float3x3_Inverse, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Inverted", engine->newFunction(float3x3_Inverted, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("InverseOrthogonal", engine->newFunction(float3x3_InverseOrthogonal, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("InverseOrthogonalUniformScale", engine->newFunction(float3x3_InverseOrthogonalUniformScale, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("InverseOrthonormal", engine->newFunction(float3x3_InverseOrthonormal, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Transpose", engine->newFunction(float3x3_Transpose, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Transposed", engine->newFunction(float3x3_Transposed, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("InverseTranspose", engine->newFunction(float3x3_InverseTranspose, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("InverseTransposed", engine->newFunction(float3x3_InverseTransposed, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Trace", engine->newFunction(float3x3_Trace, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Orthonormalize", engine->newFunction(float3x3_Orthonormalize_int_int_int, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("RemoveScale", engine->newFunction(float3x3_RemoveScale, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Transform", engine->newFunction(float3x3_Transform_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Transform", engine->newFunction(float3x3_Transform_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("TransformLeft", engine->newFunction(float3x3_TransformLeft_float3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("BatchTransform", engine->newFunction(float3x3_BatchTransform_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("BatchTransform", engine->newFunction(float3x3_BatchTransform_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsFinite", engine->newFunction(float3x3_IsFinite, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsIdentity", engine->newFunction(float3x3_IsIdentity_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsLowerTriangular", engine->newFunction(float3x3_IsLowerTriangular_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsUpperTriangular", engine->newFunction(float3x3_IsUpperTriangular_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsInvertible", engine->newFunction(float3x3_IsInvertible_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsSymmetric", engine->newFunction(float3x3_IsSymmetric_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsSkewSymmetric", engine->newFunction(float3x3_IsSkewSymmetric_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("HasUnitaryScale", engine->newFunction(float3x3_HasUnitaryScale_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("HasNegativeScale", engine->newFunction(float3x3_HasNegativeScale, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("HasUniformScale", engine->newFunction(float3x3_HasUniformScale_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsOrthogonal", engine->newFunction(float3x3_IsOrthogonal_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("IsOrthonormal", engine->newFunction(float3x3_IsOrthonormal_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Equals", engine->newFunction(float3x3_Equals_float3x3_float, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToEulerXYX", engine->newFunction(float3x3_ToEulerXYX, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToEulerXZX", engine->newFunction(float3x3_ToEulerXZX, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToEulerYXY", engine->newFunction(float3x3_ToEulerYXY, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToEulerYZY", engine->newFunction(float3x3_ToEulerYZY, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToEulerZXZ", engine->newFunction(float3x3_ToEulerZXZ, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToEulerZYZ", engine->newFunction(float3x3_ToEulerZYZ, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToEulerXYZ", engine->newFunction(float3x3_ToEulerXYZ, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToEulerXZY", engine->newFunction(float3x3_ToEulerXZY, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToEulerYXZ", engine->newFunction(float3x3_ToEulerYXZ, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToEulerYZX", engine->newFunction(float3x3_ToEulerYZX, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToEulerZXY", engine->newFunction(float3x3_ToEulerZXY, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToEulerZYX", engine->newFunction(float3x3_ToEulerZYX, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ExtractScale", engine->newFunction(float3x3_ExtractScale, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Decompose", engine->newFunction(float3x3_Decompose_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("Mul", engine->newFunction(float3x3_Mul_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MulPos", engine->newFunction(float3x3_MulPos_float3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("MulDir", engine->newFunction(float3x3_MulDir_float3, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("toString", engine->newFunction(float3x3_toString, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("metaTypeId", engine->toScriptValue<qint32>((qint32)qMetaTypeId<float3x3>()));
    engine->setDefaultPrototype(qMetaTypeId<float3x3>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<float3x3*>(), proto);
    qScriptRegisterMetaType(engine, ToScriptValue_float3x3, FromScriptValue_float3x3, proto);

    QScriptValue ctor = engine->newFunction(float3x3_ctor, proto, 9);
    ctor.setProperty("RotateX", engine->newFunction(float3x3_RotateX_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("RotateY", engine->newFunction(float3x3_RotateY_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("RotateZ", engine->newFunction(float3x3_RotateZ_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("RotateAxisAngle", engine->newFunction(float3x3_RotateAxisAngle_float3_float, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("RotateFromTo", engine->newFunction(float3x3_RotateFromTo_float3_float3, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromQuat", engine->newFunction(float3x3_FromQuat_Quat, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromRS", engine->newFunction(float3x3_FromRS_selector, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromEulerXYX", engine->newFunction(float3x3_FromEulerXYX_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromEulerXZX", engine->newFunction(float3x3_FromEulerXZX_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromEulerYXY", engine->newFunction(float3x3_FromEulerYXY_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromEulerYZY", engine->newFunction(float3x3_FromEulerYZY_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromEulerZXZ", engine->newFunction(float3x3_FromEulerZXZ_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromEulerZYZ", engine->newFunction(float3x3_FromEulerZYZ_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromEulerXYZ", engine->newFunction(float3x3_FromEulerXYZ_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromEulerXZY", engine->newFunction(float3x3_FromEulerXZY_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromEulerYXZ", engine->newFunction(float3x3_FromEulerYXZ_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromEulerYZX", engine->newFunction(float3x3_FromEulerYZX_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromEulerZXY", engine->newFunction(float3x3_FromEulerZXY_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("FromEulerZYX", engine->newFunction(float3x3_FromEulerZYX_float_float_float, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("Scale", engine->newFunction(float3x3_Scale_selector, 3), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("Scale", engine->newFunction(float3x3_Scale_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("ScaleAlongAxis", engine->newFunction(float3x3_ScaleAlongAxis_float3_float, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("UniformScale", engine->newFunction(float3x3_UniformScale_float, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("ShearX", engine->newFunction(float3x3_ShearX_float_float, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("ShearY", engine->newFunction(float3x3_ShearY_float_float, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("ShearZ", engine->newFunction(float3x3_ShearZ_float_float, 2), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("Reflect", engine->newFunction(float3x3_Reflect_Plane, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("MakeOrthographicProjection", engine->newFunction(float3x3_MakeOrthographicProjection_selector, 4), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("MakeOrthographicProjection", engine->newFunction(float3x3_MakeOrthographicProjection_selector, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("MakeOrthographicProjectionYZ", engine->newFunction(float3x3_MakeOrthographicProjectionYZ, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("MakeOrthographicProjectionXZ", engine->newFunction(float3x3_MakeOrthographicProjectionXZ, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("MakeOrthographicProjectionXY", engine->newFunction(float3x3_MakeOrthographicProjectionXY, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("zero", qScriptValueFromValue(engine, float3x3::zero), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("identity", qScriptValueFromValue(engine, float3x3::identity), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("nan", qScriptValueFromValue(engine, float3x3::nan), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    engine->globalObject().setProperty("float3x3", ctor, QScriptValue::Undeletable | QScriptValue::ReadOnly);

    return ctor;
}

