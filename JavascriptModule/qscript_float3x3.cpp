#include "QtScriptBindingsHelpers.h"

static QScriptValue float3x3_float3x3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_float3x3 in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 ret;
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_float3x3_float_float_float_float_float_float_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 9) { printf("Error! Invalid number of arguments passed to function float3x3_float3x3_float_float_float_float_float_float_float_float_float in file %s, line %d!\nExpected 9, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float _00 = TypeFromQScriptValue<float>(context->argument(0));
    float _01 = TypeFromQScriptValue<float>(context->argument(1));
    float _02 = TypeFromQScriptValue<float>(context->argument(2));
    float _10 = TypeFromQScriptValue<float>(context->argument(3));
    float _11 = TypeFromQScriptValue<float>(context->argument(4));
    float _12 = TypeFromQScriptValue<float>(context->argument(5));
    float _20 = TypeFromQScriptValue<float>(context->argument(6));
    float _21 = TypeFromQScriptValue<float>(context->argument(7));
    float _22 = TypeFromQScriptValue<float>(context->argument(8));
    float3x3 ret(_00, _01, _02, _10, _11, _12, _20, _21, _22);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_float3x3_float3_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x3_float3x3_float3_float3_float3 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 col0 = TypeFromQScriptValue<float3>(context->argument(0));
    float3 col1 = TypeFromQScriptValue<float3>(context->argument(1));
    float3 col2 = TypeFromQScriptValue<float3>(context->argument(2));
    float3x3 ret(col0, col1, col2);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_float3x3_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_float3x3_Quat in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat orientation = TypeFromQScriptValue<Quat>(context->argument(0));
    float3x3 ret(orientation);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_GetScale(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_GetScale in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_GetScale in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->GetScale();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_At_int_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_At_int_int in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_At_int_int in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int row = TypeFromQScriptValue<int>(context->argument(0));
    int col = TypeFromQScriptValue<int>(context->argument(1));
    const float ret = This->At(row, col);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_Row_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_Row_int in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_Row_int in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int row = TypeFromQScriptValue<int>(context->argument(0));
    const float3 & ret = This->Row(row);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_Col_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_Col_int in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_Col_int in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int col = TypeFromQScriptValue<int>(context->argument(0));
    const float3 ret = This->Col(col);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_Diagonal(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_Diagonal in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_Diagonal in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    const float3 ret = This->Diagonal();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_ScaleRow_int_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_ScaleRow_int_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_ScaleRow_int_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int row = TypeFromQScriptValue<int>(context->argument(0));
    float scalar = TypeFromQScriptValue<float>(context->argument(1));
    This->ScaleRow(row, scalar);
    return QScriptValue();
}

static QScriptValue float3x3_ScaleCol_int_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_ScaleCol_int_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_ScaleCol_int_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int col = TypeFromQScriptValue<int>(context->argument(0));
    float scalar = TypeFromQScriptValue<float>(context->argument(1));
    This->ScaleCol(col, scalar);
    return QScriptValue();
}

static QScriptValue float3x3_WorldX(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_WorldX in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_WorldX in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->WorldX();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_WorldY(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_WorldY in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_WorldY in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->WorldY();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_WorldZ(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_WorldZ in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_WorldZ in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->WorldZ();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_SetRow_int_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function float3x3_SetRow_int_float_float_float in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_SetRow_int_float_float_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int row = TypeFromQScriptValue<int>(context->argument(0));
    float x = TypeFromQScriptValue<float>(context->argument(1));
    float y = TypeFromQScriptValue<float>(context->argument(2));
    float z = TypeFromQScriptValue<float>(context->argument(3));
    This->SetRow(row, x, y, z);
    return QScriptValue();
}

static QScriptValue float3x3_SetRow_int_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_SetRow_int_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_SetRow_int_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int row = TypeFromQScriptValue<int>(context->argument(0));
    float3 rowVector = TypeFromQScriptValue<float3>(context->argument(1));
    This->SetRow(row, rowVector);
    return QScriptValue();
}

static QScriptValue float3x3_SetCol_int_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function float3x3_SetCol_int_float_float_float in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_SetCol_int_float_float_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int column = TypeFromQScriptValue<int>(context->argument(0));
    float x = TypeFromQScriptValue<float>(context->argument(1));
    float y = TypeFromQScriptValue<float>(context->argument(2));
    float z = TypeFromQScriptValue<float>(context->argument(3));
    This->SetCol(column, x, y, z);
    return QScriptValue();
}

static QScriptValue float3x3_SetCol_int_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_SetCol_int_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_SetCol_int_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int column = TypeFromQScriptValue<int>(context->argument(0));
    float3 columnVector = TypeFromQScriptValue<float3>(context->argument(1));
    This->SetCol(column, columnVector);
    return QScriptValue();
}

static QScriptValue float3x3_Set_float_float_float_float_float_float_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 9) { printf("Error! Invalid number of arguments passed to function float3x3_Set_float_float_float_float_float_float_float_float_float in file %s, line %d!\nExpected 9, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_Set_float_float_float_float_float_float_float_float_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float _00 = TypeFromQScriptValue<float>(context->argument(0));
    float _01 = TypeFromQScriptValue<float>(context->argument(1));
    float _02 = TypeFromQScriptValue<float>(context->argument(2));
    float _10 = TypeFromQScriptValue<float>(context->argument(3));
    float _11 = TypeFromQScriptValue<float>(context->argument(4));
    float _12 = TypeFromQScriptValue<float>(context->argument(5));
    float _20 = TypeFromQScriptValue<float>(context->argument(6));
    float _21 = TypeFromQScriptValue<float>(context->argument(7));
    float _22 = TypeFromQScriptValue<float>(context->argument(8));
    This->Set(_00, _01, _02, _10, _11, _12, _20, _21, _22);
    return QScriptValue();
}

static QScriptValue float3x3_SetIdentity(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_SetIdentity in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_SetIdentity in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    This->SetIdentity();
    return QScriptValue();
}

static QScriptValue float3x3_SwapColumns_int_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_SwapColumns_int_int in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_SwapColumns_int_int in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int col1 = TypeFromQScriptValue<int>(context->argument(0));
    int col2 = TypeFromQScriptValue<int>(context->argument(1));
    This->SwapColumns(col1, col2);
    return QScriptValue();
}

static QScriptValue float3x3_SwapRows_int_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_SwapRows_int_int in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_SwapRows_int_int in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int row1 = TypeFromQScriptValue<int>(context->argument(0));
    int row2 = TypeFromQScriptValue<int>(context->argument(1));
    This->SwapRows(row1, row2);
    return QScriptValue();
}

static QScriptValue float3x3_SetRotatePartX_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_SetRotatePartX_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_SetRotatePartX_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float angle = TypeFromQScriptValue<float>(context->argument(0));
    This->SetRotatePartX(angle);
    return QScriptValue();
}

static QScriptValue float3x3_SetRotatePartY_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_SetRotatePartY_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_SetRotatePartY_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float angle = TypeFromQScriptValue<float>(context->argument(0));
    This->SetRotatePartY(angle);
    return QScriptValue();
}

static QScriptValue float3x3_SetRotatePartZ_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_SetRotatePartZ_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_SetRotatePartZ_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float angle = TypeFromQScriptValue<float>(context->argument(0));
    This->SetRotatePartZ(angle);
    return QScriptValue();
}

static QScriptValue float3x3_SetRotatePart_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_SetRotatePart_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_SetRotatePart_float3_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 axisDirection = TypeFromQScriptValue<float3>(context->argument(0));
    float angle = TypeFromQScriptValue<float>(context->argument(1));
    This->SetRotatePart(axisDirection, angle);
    return QScriptValue();
}

static QScriptValue float3x3_SetRotatePart_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_SetRotatePart_Quat in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_SetRotatePart_Quat in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    Quat orientation = TypeFromQScriptValue<Quat>(context->argument(0));
    This->SetRotatePart(orientation);
    return QScriptValue();
}

static QScriptValue float3x3_Determinant(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_Determinant in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_Determinant in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->Determinant();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_Inverse(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_Inverse in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_Inverse in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    bool ret = This->Inverse();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_Inverted(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_Inverted in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_Inverted in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3x3 ret = This->Inverted();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_InverseOrthogonal(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_InverseOrthogonal in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_InverseOrthogonal in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    bool ret = This->InverseOrthogonal();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_InverseOrthogonalUniformScale(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_InverseOrthogonalUniformScale in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_InverseOrthogonalUniformScale in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    bool ret = This->InverseOrthogonalUniformScale();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_InverseOrthonormal(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_InverseOrthonormal in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_InverseOrthonormal in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    This->InverseOrthonormal();
    return QScriptValue();
}

static QScriptValue float3x3_Transpose(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_Transpose in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_Transpose in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    This->Transpose();
    return QScriptValue();
}

static QScriptValue float3x3_Transposed(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_Transposed in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_Transposed in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3x3 ret = This->Transposed();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_InverseTranspose(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_InverseTranspose in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_InverseTranspose in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    bool ret = This->InverseTranspose();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_InverseTransposed(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_InverseTransposed in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_InverseTransposed in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3x3 ret = This->InverseTransposed();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_Trace(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_Trace in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_Trace in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->Trace();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_Orthonormalize_int_int_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x3_Orthonormalize_int_int_int in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_Orthonormalize_int_int_int in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int firstColumn = TypeFromQScriptValue<int>(context->argument(0));
    int secondColumn = TypeFromQScriptValue<int>(context->argument(1));
    int thirdColumn = TypeFromQScriptValue<int>(context->argument(2));
    This->Orthonormalize(firstColumn, secondColumn, thirdColumn);
    return QScriptValue();
}

static QScriptValue float3x3_RemoveScale(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_RemoveScale in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_RemoveScale in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    This->RemoveScale();
    return QScriptValue();
}

static QScriptValue float3x3_Transform_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_Transform_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_Transform_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 vector = TypeFromQScriptValue<float3>(context->argument(0));
    float3 ret = This->Transform(vector);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_Transform_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x3_Transform_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_Transform_float_float_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float x = TypeFromQScriptValue<float>(context->argument(0));
    float y = TypeFromQScriptValue<float>(context->argument(1));
    float z = TypeFromQScriptValue<float>(context->argument(2));
    float3 ret = This->Transform(x, y, z);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_Transform_float4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_Transform_float4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_Transform_float4 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4 vector = TypeFromQScriptValue<float4>(context->argument(0));
    float4 ret = This->Transform(vector);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_BatchTransform_float4_ptr_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_BatchTransform_float4_ptr_int in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_BatchTransform_float4_ptr_int in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4 * vectorArray = TypeFromQScriptValue<float4 *>(context->argument(0));
    int numVectors = TypeFromQScriptValue<int>(context->argument(1));
    This->BatchTransform(vectorArray, numVectors);
    return QScriptValue();
}

static QScriptValue float3x3_BatchTransform_float4_ptr_int_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x3_BatchTransform_float4_ptr_int_int in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_BatchTransform_float4_ptr_int_int in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4 * vectorArray = TypeFromQScriptValue<float4 *>(context->argument(0));
    int numVectors = TypeFromQScriptValue<int>(context->argument(1));
    int stride = TypeFromQScriptValue<int>(context->argument(2));
    This->BatchTransform(vectorArray, numVectors, stride);
    return QScriptValue();
}

static QScriptValue float3x3_IsFinite(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_IsFinite in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_IsFinite in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    bool ret = This->IsFinite();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_IsIdentity_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_IsIdentity_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_IsIdentity_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float epsilon = TypeFromQScriptValue<float>(context->argument(0));
    bool ret = This->IsIdentity(epsilon);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_IsLowerTriangular_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_IsLowerTriangular_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_IsLowerTriangular_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float epsilon = TypeFromQScriptValue<float>(context->argument(0));
    bool ret = This->IsLowerTriangular(epsilon);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_IsUpperTriangular_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_IsUpperTriangular_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_IsUpperTriangular_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float epsilon = TypeFromQScriptValue<float>(context->argument(0));
    bool ret = This->IsUpperTriangular(epsilon);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_IsInvertible_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_IsInvertible_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_IsInvertible_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float epsilon = TypeFromQScriptValue<float>(context->argument(0));
    bool ret = This->IsInvertible(epsilon);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_IsSymmetric_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_IsSymmetric_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_IsSymmetric_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float epsilon = TypeFromQScriptValue<float>(context->argument(0));
    bool ret = This->IsSymmetric(epsilon);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_IsSkewSymmetric_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_IsSkewSymmetric_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_IsSkewSymmetric_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float epsilon = TypeFromQScriptValue<float>(context->argument(0));
    bool ret = This->IsSkewSymmetric(epsilon);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_HasUnitaryScale_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_HasUnitaryScale_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_HasUnitaryScale_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float epsilonSq = TypeFromQScriptValue<float>(context->argument(0));
    bool ret = This->HasUnitaryScale(epsilonSq);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_HasNegativeScale(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_HasNegativeScale in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_HasNegativeScale in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    bool ret = This->HasNegativeScale();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_HasUniformScale_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_HasUniformScale_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_HasUniformScale_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float epsilonSq = TypeFromQScriptValue<float>(context->argument(0));
    bool ret = This->HasUniformScale(epsilonSq);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_IsOrthogonal_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_IsOrthogonal_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_IsOrthogonal_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float epsilon = TypeFromQScriptValue<float>(context->argument(0));
    bool ret = This->IsOrthogonal(epsilon);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_Equals_float3x3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_Equals_float3x3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_Equals_float3x3_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3x3 other = TypeFromQScriptValue<float3x3>(context->argument(0));
    float epsilon = TypeFromQScriptValue<float>(context->argument(1));
    bool ret = This->Equals(other, epsilon);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_ToEulerXYX(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_ToEulerXYX in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_ToEulerXYX in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ToEulerXYX();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_ToEulerXZX(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_ToEulerXZX in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_ToEulerXZX in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ToEulerXZX();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_ToEulerYXY(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_ToEulerYXY in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_ToEulerYXY in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ToEulerYXY();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_ToEulerYZY(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_ToEulerYZY in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_ToEulerYZY in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ToEulerYZY();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_ToEulerZXZ(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_ToEulerZXZ in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_ToEulerZXZ in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ToEulerZXZ();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_ToEulerZYZ(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_ToEulerZYZ in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_ToEulerZYZ in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ToEulerZYZ();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_ToEulerXYZ(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_ToEulerXYZ in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_ToEulerXYZ in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ToEulerXYZ();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_ToEulerXZY(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_ToEulerXZY in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_ToEulerXZY in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ToEulerXZY();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_ToEulerYXZ(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_ToEulerYXZ in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_ToEulerYXZ in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ToEulerYXZ();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_ToEulerYZX(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_ToEulerYZX in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_ToEulerYZX in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ToEulerYZX();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_ToEulerZXY(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_ToEulerZXY in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_ToEulerZXY in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ToEulerZXY();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_ToEulerZYX(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_ToEulerZYX in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_ToEulerZYX in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ToEulerZYX();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_ExtractScale(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_ExtractScale in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_ExtractScale in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ExtractScale();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_Decompose_Quat_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_Decompose_Quat_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_Decompose_Quat_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    Quat rotate = TypeFromQScriptValue<Quat>(context->argument(0));
    float3 scale = TypeFromQScriptValue<float3>(context->argument(1));
    This->Decompose(rotate, scale);
    return QScriptValue();
}

static QScriptValue float3x3_Decompose_float3x3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_Decompose_float3x3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_Decompose_float3x3_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3x3 rotate = TypeFromQScriptValue<float3x3>(context->argument(0));
    float3 scale = TypeFromQScriptValue<float3>(context->argument(1));
    This->Decompose(rotate, scale);
    return QScriptValue();
}

static QScriptValue float3x3_Mul_float3x3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_Mul_float3x3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_Mul_float3x3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3x3 rhs = TypeFromQScriptValue<float3x3>(context->argument(0));
    float3x3 ret = This->Mul(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_Mul_float3x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_Mul_float3x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_Mul_float3x4 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3x4 rhs = TypeFromQScriptValue<float3x4>(context->argument(0));
    float3x4 ret = This->Mul(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_Mul_float4x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_Mul_float4x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_Mul_float4x4 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4x4 rhs = TypeFromQScriptValue<float4x4>(context->argument(0));
    float4x4 ret = This->Mul(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_Mul_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_Mul_Quat in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_Mul_Quat in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    Quat rhs = TypeFromQScriptValue<Quat>(context->argument(0));
    float3x3 ret = This->Mul(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_Mul_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_Mul_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_Mul_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 rhs = TypeFromQScriptValue<float3>(context->argument(0));
    float3 ret = This->Mul(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_toString(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_toString in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 *This = TypeFromQScriptValue<float3x3*>(context->thisObject());
    if (!This && context->argumentCount() > 0) This = TypeFromQScriptValue<float3x3*>(context->argument(0)); // Qt oddity (bug?): Sometimes the built-in toString() function doesn't give us this from thisObject, but as the first argument.
    if (!This) { printf("Error! Invalid context->thisObject in function float3x3_toString in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    QString ret = This->toString();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_RotateX_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_RotateX_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float angleRadians = TypeFromQScriptValue<float>(context->argument(0));
    float3x3 ret = float3x3::RotateX(angleRadians);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_RotateY_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_RotateY_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float angleRadians = TypeFromQScriptValue<float>(context->argument(0));
    float3x3 ret = float3x3::RotateY(angleRadians);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_RotateZ_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_RotateZ_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float angleRadians = TypeFromQScriptValue<float>(context->argument(0));
    float3x3 ret = float3x3::RotateZ(angleRadians);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_RotateAxisAngle_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_RotateAxisAngle_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 axisDirection = TypeFromQScriptValue<float3>(context->argument(0));
    float angleRadians = TypeFromQScriptValue<float>(context->argument(1));
    float3x3 ret = float3x3::RotateAxisAngle(axisDirection, angleRadians);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_RotateFromTo_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_RotateFromTo_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 sourceDirection = TypeFromQScriptValue<float3>(context->argument(0));
    float3 targetDirection = TypeFromQScriptValue<float3>(context->argument(1));
    float3x3 ret = float3x3::RotateFromTo(sourceDirection, targetDirection);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_FromQuat_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_FromQuat_Quat in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat orientation = TypeFromQScriptValue<Quat>(context->argument(0));
    float3x3 ret = float3x3::FromQuat(orientation);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_FromRS_Quat_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_FromRS_Quat_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat rotate = TypeFromQScriptValue<Quat>(context->argument(0));
    float3 scale = TypeFromQScriptValue<float3>(context->argument(1));
    float3x3 ret = float3x3::FromRS(rotate, scale);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_FromRS_float3x3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_FromRS_float3x3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 rotate = TypeFromQScriptValue<float3x3>(context->argument(0));
    float3 scale = TypeFromQScriptValue<float3>(context->argument(1));
    float3x3 ret = float3x3::FromRS(rotate, scale);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_FromEulerXYX_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x3_FromEulerXYX_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float ex = TypeFromQScriptValue<float>(context->argument(0));
    float ey = TypeFromQScriptValue<float>(context->argument(1));
    float ex2 = TypeFromQScriptValue<float>(context->argument(2));
    float3x3 ret = float3x3::FromEulerXYX(ex, ey, ex2);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_FromEulerXZX_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x3_FromEulerXZX_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float ex = TypeFromQScriptValue<float>(context->argument(0));
    float ez = TypeFromQScriptValue<float>(context->argument(1));
    float ex2 = TypeFromQScriptValue<float>(context->argument(2));
    float3x3 ret = float3x3::FromEulerXZX(ex, ez, ex2);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_FromEulerYXY_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x3_FromEulerYXY_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float ey = TypeFromQScriptValue<float>(context->argument(0));
    float ex = TypeFromQScriptValue<float>(context->argument(1));
    float ey2 = TypeFromQScriptValue<float>(context->argument(2));
    float3x3 ret = float3x3::FromEulerYXY(ey, ex, ey2);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_FromEulerYZY_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x3_FromEulerYZY_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float ey = TypeFromQScriptValue<float>(context->argument(0));
    float ez = TypeFromQScriptValue<float>(context->argument(1));
    float ey2 = TypeFromQScriptValue<float>(context->argument(2));
    float3x3 ret = float3x3::FromEulerYZY(ey, ez, ey2);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_FromEulerZXZ_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x3_FromEulerZXZ_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float ez = TypeFromQScriptValue<float>(context->argument(0));
    float ex = TypeFromQScriptValue<float>(context->argument(1));
    float ez2 = TypeFromQScriptValue<float>(context->argument(2));
    float3x3 ret = float3x3::FromEulerZXZ(ez, ex, ez2);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_FromEulerZYZ_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x3_FromEulerZYZ_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float ez = TypeFromQScriptValue<float>(context->argument(0));
    float ey = TypeFromQScriptValue<float>(context->argument(1));
    float ez2 = TypeFromQScriptValue<float>(context->argument(2));
    float3x3 ret = float3x3::FromEulerZYZ(ez, ey, ez2);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_FromEulerXYZ_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x3_FromEulerXYZ_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float ex = TypeFromQScriptValue<float>(context->argument(0));
    float ey = TypeFromQScriptValue<float>(context->argument(1));
    float ez = TypeFromQScriptValue<float>(context->argument(2));
    float3x3 ret = float3x3::FromEulerXYZ(ex, ey, ez);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_FromEulerXZY_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x3_FromEulerXZY_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float ex = TypeFromQScriptValue<float>(context->argument(0));
    float ez = TypeFromQScriptValue<float>(context->argument(1));
    float ey = TypeFromQScriptValue<float>(context->argument(2));
    float3x3 ret = float3x3::FromEulerXZY(ex, ez, ey);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_FromEulerYXZ_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x3_FromEulerYXZ_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float ey = TypeFromQScriptValue<float>(context->argument(0));
    float ex = TypeFromQScriptValue<float>(context->argument(1));
    float ez = TypeFromQScriptValue<float>(context->argument(2));
    float3x3 ret = float3x3::FromEulerYXZ(ey, ex, ez);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_FromEulerYZX_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x3_FromEulerYZX_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float ey = TypeFromQScriptValue<float>(context->argument(0));
    float ez = TypeFromQScriptValue<float>(context->argument(1));
    float ex = TypeFromQScriptValue<float>(context->argument(2));
    float3x3 ret = float3x3::FromEulerYZX(ey, ez, ex);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_FromEulerZXY_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x3_FromEulerZXY_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float ez = TypeFromQScriptValue<float>(context->argument(0));
    float ex = TypeFromQScriptValue<float>(context->argument(1));
    float ey = TypeFromQScriptValue<float>(context->argument(2));
    float3x3 ret = float3x3::FromEulerZXY(ez, ex, ey);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_FromEulerZYX_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x3_FromEulerZYX_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float ez = TypeFromQScriptValue<float>(context->argument(0));
    float ey = TypeFromQScriptValue<float>(context->argument(1));
    float ex = TypeFromQScriptValue<float>(context->argument(2));
    float3x3 ret = float3x3::FromEulerZYX(ez, ey, ex);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_Scale_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3x3_Scale_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float sx = TypeFromQScriptValue<float>(context->argument(0));
    float sy = TypeFromQScriptValue<float>(context->argument(1));
    float sz = TypeFromQScriptValue<float>(context->argument(2));
    ScaleOp ret = float3x3::Scale(sx, sy, sz);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_Scale_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_Scale_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 scale = TypeFromQScriptValue<float3>(context->argument(0));
    ScaleOp ret = float3x3::Scale(scale);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_ScaleAlongAxis_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_ScaleAlongAxis_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 axis = TypeFromQScriptValue<float3>(context->argument(0));
    float scalingFactor = TypeFromQScriptValue<float>(context->argument(1));
    float3x3 ret = float3x3::ScaleAlongAxis(axis, scalingFactor);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_UniformScale_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_UniformScale_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float uniformScale = TypeFromQScriptValue<float>(context->argument(0));
    ScaleOp ret = float3x3::UniformScale(uniformScale);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_ShearX_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_ShearX_float_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float yFactor = TypeFromQScriptValue<float>(context->argument(0));
    float zFactor = TypeFromQScriptValue<float>(context->argument(1));
    float3x3 ret = float3x3::ShearX(yFactor, zFactor);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_ShearY_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_ShearY_float_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float xFactor = TypeFromQScriptValue<float>(context->argument(0));
    float zFactor = TypeFromQScriptValue<float>(context->argument(1));
    float3x3 ret = float3x3::ShearY(xFactor, zFactor);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_ShearZ_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3x3_ShearZ_float_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float xFactor = TypeFromQScriptValue<float>(context->argument(0));
    float yFactor = TypeFromQScriptValue<float>(context->argument(1));
    float3x3 ret = float3x3::ShearZ(xFactor, yFactor);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_Reflect_Plane(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_Reflect_Plane in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Plane p = TypeFromQScriptValue<Plane>(context->argument(0));
    float3x3 ret = float3x3::Reflect(p);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_MakeOrthographicProjection_float_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function float3x3_MakeOrthographicProjection_float_float_float_float in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float nearPlaneDistance = TypeFromQScriptValue<float>(context->argument(0));
    float farPlaneDistance = TypeFromQScriptValue<float>(context->argument(1));
    float horizontalViewportSize = TypeFromQScriptValue<float>(context->argument(2));
    float verticalViewportSize = TypeFromQScriptValue<float>(context->argument(3));
    float3x3 ret = float3x3::MakeOrthographicProjection(nearPlaneDistance, farPlaneDistance, horizontalViewportSize, verticalViewportSize);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_MakeOrthographicProjection_Plane(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3x3_MakeOrthographicProjection_Plane in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Plane target = TypeFromQScriptValue<Plane>(context->argument(0));
    float3x3 ret = float3x3::MakeOrthographicProjection(target);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_MakeOrthographicProjectionYZ(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_MakeOrthographicProjectionYZ in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 ret = float3x3::MakeOrthographicProjectionYZ();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_MakeOrthographicProjectionXZ(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_MakeOrthographicProjectionXZ in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 ret = float3x3::MakeOrthographicProjectionXZ();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3x3_MakeOrthographicProjectionXY(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3x3_MakeOrthographicProjectionXY in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 ret = float3x3::MakeOrthographicProjectionXY();
    return TypeToQScriptValue(engine, ret);
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
    printf("float3x3_ctor failed to choose the right function to call! Did you use 'var x = float3x3();' instead of 'var x = new float3x3();'?\n"); return QScriptValue();
}

static QScriptValue float3x3_SetRow_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 4 && QSVIsOfType<int>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)) && QSVIsOfType<float>(context->argument(3)))
        return float3x3_SetRow_int_float_float_float(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<int>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return float3x3_SetRow_int_float3(context, engine);
    printf("float3x3_SetRow_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float3x3_SetCol_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 4 && QSVIsOfType<int>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)) && QSVIsOfType<float>(context->argument(3)))
        return float3x3_SetCol_int_float_float_float(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<int>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return float3x3_SetCol_int_float3(context, engine);
    printf("float3x3_SetCol_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float3x3_SetRotatePart_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return float3x3_SetRotatePart_float3_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Quat>(context->argument(0)))
        return float3x3_SetRotatePart_Quat(context, engine);
    printf("float3x3_SetRotatePart_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float3x3_Transform_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return float3x3_Transform_float3(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return float3x3_Transform_float_float_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float4>(context->argument(0)))
        return float3x3_Transform_float4(context, engine);
    printf("float3x3_Transform_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float3x3_BatchTransform_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float4 *>(context->argument(0)) && QSVIsOfType<int>(context->argument(1)))
        return float3x3_BatchTransform_float4_ptr_int(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float4 *>(context->argument(0)) && QSVIsOfType<int>(context->argument(1)) && QSVIsOfType<int>(context->argument(2)))
        return float3x3_BatchTransform_float4_ptr_int_int(context, engine);
    printf("float3x3_BatchTransform_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float3x3_Decompose_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<Quat>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return float3x3_Decompose_Quat_float3(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float3x3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return float3x3_Decompose_float3x3_float3(context, engine);
    printf("float3x3_Decompose_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
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
    printf("float3x3_Mul_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float3x3_FromRS_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<Quat>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return float3x3_FromRS_Quat_float3(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float3x3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return float3x3_FromRS_float3x3_float3(context, engine);
    printf("float3x3_FromRS_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float3x3_Scale_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 3 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return float3x3_Scale_float_float_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return float3x3_Scale_float3(context, engine);
    printf("float3x3_Scale_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float3x3_MakeOrthographicProjection_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 4 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)) && QSVIsOfType<float>(context->argument(3)))
        return float3x3_MakeOrthographicProjection_float_float_float_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Plane>(context->argument(0)))
        return float3x3_MakeOrthographicProjection_Plane(context, engine);
    printf("float3x3_MakeOrthographicProjection_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

class float3x3_scriptclass : public QScriptClass
{
public:
    QScriptValue objectPrototype;
    float3x3_scriptclass(QScriptEngine *engine):QScriptClass(engine){}
    QScriptValue property(const QScriptValue &object, const QScriptString &name, uint id)
    {
        float3x3 *This = TypeFromQScriptValue<float3x3*>(object);
        if (!This) { printf("Error! Cannot convert QScriptValue to type float3x3 in file %s, line %d!\nTry using float3x3.get%s() and float3x3.set%s() to query the member variable '%s'!\n", __FILE__, __LINE__, Capitalize((QString)name).c_str(), Capitalize((QString)name).c_str(), ((QString)name).toStdString().c_str()); return QScriptValue(); }
        if ((QString)name == (QString)"zero") return TypeToQScriptValue(engine(), This->zero);
        if ((QString)name == (QString)"identity") return TypeToQScriptValue(engine(), This->identity);
        return QScriptValue();
    }
    void setProperty(QScriptValue &object, const QScriptString &name, uint id, const QScriptValue &value)
    {
        float3x3 *This = TypeFromQScriptValue<float3x3*>(object);
        if (!This) { printf("Error! Cannot convert QScriptValue to type float3x3 in file %s, line %d!\nTry using float3x3.get%s() and float3x3.set%s() to query the member variable '%s'!\n", __FILE__, __LINE__, Capitalize((QString)name).c_str(), Capitalize((QString)name).c_str(), ((QString)name).toStdString().c_str()); return; }
    }
    QueryFlags queryProperty(const QScriptValue &object, const QScriptString &name, QueryFlags flags, uint *id)
    {
        if ((QString)name == (QString)"zero") return flags;
        if ((QString)name == (QString)"identity") return flags;
        return 0;
    }
    QScriptValue prototype() const { return objectPrototype; }
};
QScriptValue register_float3x3_prototype(QScriptEngine *engine)
{
    engine->setDefaultPrototype(qMetaTypeId<float3x3*>(), QScriptValue());
    QScriptValue proto = engine->newVariant(qVariantFromValue((float3x3*)0));
    proto.setProperty("GetScale", engine->newFunction(float3x3_GetScale, 0));
    proto.setProperty("At", engine->newFunction(float3x3_At_int_int, 2));
    proto.setProperty("Row", engine->newFunction(float3x3_Row_int, 1));
    proto.setProperty("Col", engine->newFunction(float3x3_Col_int, 1));
    proto.setProperty("Diagonal", engine->newFunction(float3x3_Diagonal, 0));
    proto.setProperty("ScaleRow", engine->newFunction(float3x3_ScaleRow_int_float, 2));
    proto.setProperty("ScaleCol", engine->newFunction(float3x3_ScaleCol_int_float, 2));
    proto.setProperty("WorldX", engine->newFunction(float3x3_WorldX, 0));
    proto.setProperty("WorldY", engine->newFunction(float3x3_WorldY, 0));
    proto.setProperty("WorldZ", engine->newFunction(float3x3_WorldZ, 0));
    proto.setProperty("SetRow", engine->newFunction(float3x3_SetRow_selector, 4));
    proto.setProperty("SetRow", engine->newFunction(float3x3_SetRow_selector, 2));
    proto.setProperty("SetCol", engine->newFunction(float3x3_SetCol_selector, 4));
    proto.setProperty("SetCol", engine->newFunction(float3x3_SetCol_selector, 2));
    proto.setProperty("Set", engine->newFunction(float3x3_Set_float_float_float_float_float_float_float_float_float, 9));
    proto.setProperty("SetIdentity", engine->newFunction(float3x3_SetIdentity, 0));
    proto.setProperty("SwapColumns", engine->newFunction(float3x3_SwapColumns_int_int, 2));
    proto.setProperty("SwapRows", engine->newFunction(float3x3_SwapRows_int_int, 2));
    proto.setProperty("SetRotatePartX", engine->newFunction(float3x3_SetRotatePartX_float, 1));
    proto.setProperty("SetRotatePartY", engine->newFunction(float3x3_SetRotatePartY_float, 1));
    proto.setProperty("SetRotatePartZ", engine->newFunction(float3x3_SetRotatePartZ_float, 1));
    proto.setProperty("SetRotatePart", engine->newFunction(float3x3_SetRotatePart_selector, 2));
    proto.setProperty("SetRotatePart", engine->newFunction(float3x3_SetRotatePart_selector, 1));
    proto.setProperty("Determinant", engine->newFunction(float3x3_Determinant, 0));
    proto.setProperty("Inverse", engine->newFunction(float3x3_Inverse, 0));
    proto.setProperty("Inverted", engine->newFunction(float3x3_Inverted, 0));
    proto.setProperty("InverseOrthogonal", engine->newFunction(float3x3_InverseOrthogonal, 0));
    proto.setProperty("InverseOrthogonalUniformScale", engine->newFunction(float3x3_InverseOrthogonalUniformScale, 0));
    proto.setProperty("InverseOrthonormal", engine->newFunction(float3x3_InverseOrthonormal, 0));
    proto.setProperty("Transpose", engine->newFunction(float3x3_Transpose, 0));
    proto.setProperty("Transposed", engine->newFunction(float3x3_Transposed, 0));
    proto.setProperty("InverseTranspose", engine->newFunction(float3x3_InverseTranspose, 0));
    proto.setProperty("InverseTransposed", engine->newFunction(float3x3_InverseTransposed, 0));
    proto.setProperty("Trace", engine->newFunction(float3x3_Trace, 0));
    proto.setProperty("Orthonormalize", engine->newFunction(float3x3_Orthonormalize_int_int_int, 3));
    proto.setProperty("RemoveScale", engine->newFunction(float3x3_RemoveScale, 0));
    proto.setProperty("Transform", engine->newFunction(float3x3_Transform_selector, 1));
    proto.setProperty("Transform", engine->newFunction(float3x3_Transform_selector, 3));
    proto.setProperty("BatchTransform", engine->newFunction(float3x3_BatchTransform_selector, 2));
    proto.setProperty("BatchTransform", engine->newFunction(float3x3_BatchTransform_selector, 3));
    proto.setProperty("IsFinite", engine->newFunction(float3x3_IsFinite, 0));
    proto.setProperty("IsIdentity", engine->newFunction(float3x3_IsIdentity_float, 1));
    proto.setProperty("IsLowerTriangular", engine->newFunction(float3x3_IsLowerTriangular_float, 1));
    proto.setProperty("IsUpperTriangular", engine->newFunction(float3x3_IsUpperTriangular_float, 1));
    proto.setProperty("IsInvertible", engine->newFunction(float3x3_IsInvertible_float, 1));
    proto.setProperty("IsSymmetric", engine->newFunction(float3x3_IsSymmetric_float, 1));
    proto.setProperty("IsSkewSymmetric", engine->newFunction(float3x3_IsSkewSymmetric_float, 1));
    proto.setProperty("HasUnitaryScale", engine->newFunction(float3x3_HasUnitaryScale_float, 1));
    proto.setProperty("HasNegativeScale", engine->newFunction(float3x3_HasNegativeScale, 0));
    proto.setProperty("HasUniformScale", engine->newFunction(float3x3_HasUniformScale_float, 1));
    proto.setProperty("IsOrthogonal", engine->newFunction(float3x3_IsOrthogonal_float, 1));
    proto.setProperty("Equals", engine->newFunction(float3x3_Equals_float3x3_float, 2));
    proto.setProperty("ToEulerXYX", engine->newFunction(float3x3_ToEulerXYX, 0));
    proto.setProperty("ToEulerXZX", engine->newFunction(float3x3_ToEulerXZX, 0));
    proto.setProperty("ToEulerYXY", engine->newFunction(float3x3_ToEulerYXY, 0));
    proto.setProperty("ToEulerYZY", engine->newFunction(float3x3_ToEulerYZY, 0));
    proto.setProperty("ToEulerZXZ", engine->newFunction(float3x3_ToEulerZXZ, 0));
    proto.setProperty("ToEulerZYZ", engine->newFunction(float3x3_ToEulerZYZ, 0));
    proto.setProperty("ToEulerXYZ", engine->newFunction(float3x3_ToEulerXYZ, 0));
    proto.setProperty("ToEulerXZY", engine->newFunction(float3x3_ToEulerXZY, 0));
    proto.setProperty("ToEulerYXZ", engine->newFunction(float3x3_ToEulerYXZ, 0));
    proto.setProperty("ToEulerYZX", engine->newFunction(float3x3_ToEulerYZX, 0));
    proto.setProperty("ToEulerZXY", engine->newFunction(float3x3_ToEulerZXY, 0));
    proto.setProperty("ToEulerZYX", engine->newFunction(float3x3_ToEulerZYX, 0));
    proto.setProperty("ExtractScale", engine->newFunction(float3x3_ExtractScale, 0));
    proto.setProperty("Decompose", engine->newFunction(float3x3_Decompose_selector, 2));
    proto.setProperty("Mul", engine->newFunction(float3x3_Mul_selector, 1));
    proto.setProperty("toString", engine->newFunction(float3x3_toString, 0));
    float3x3_scriptclass *sc = new float3x3_scriptclass(engine);
    engine->setProperty("float3x3_scriptclass", QVariant::fromValue<QScriptClass*>(sc));
    proto.setScriptClass(sc);
    sc->objectPrototype = proto;
    engine->setDefaultPrototype(qMetaTypeId<float3x3>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<float3x3*>(), proto);
    QScriptValue ctor = engine->newFunction(float3x3_ctor, proto, 9);
    ctor.setProperty("RotateX", engine->newFunction(float3x3_RotateX_float, 1));
    ctor.setProperty("RotateY", engine->newFunction(float3x3_RotateY_float, 1));
    ctor.setProperty("RotateZ", engine->newFunction(float3x3_RotateZ_float, 1));
    ctor.setProperty("RotateAxisAngle", engine->newFunction(float3x3_RotateAxisAngle_float3_float, 2));
    ctor.setProperty("RotateFromTo", engine->newFunction(float3x3_RotateFromTo_float3_float3, 2));
    ctor.setProperty("FromQuat", engine->newFunction(float3x3_FromQuat_Quat, 1));
    ctor.setProperty("FromRS", engine->newFunction(float3x3_FromRS_selector, 2));
    ctor.setProperty("FromEulerXYX", engine->newFunction(float3x3_FromEulerXYX_float_float_float, 3));
    ctor.setProperty("FromEulerXZX", engine->newFunction(float3x3_FromEulerXZX_float_float_float, 3));
    ctor.setProperty("FromEulerYXY", engine->newFunction(float3x3_FromEulerYXY_float_float_float, 3));
    ctor.setProperty("FromEulerYZY", engine->newFunction(float3x3_FromEulerYZY_float_float_float, 3));
    ctor.setProperty("FromEulerZXZ", engine->newFunction(float3x3_FromEulerZXZ_float_float_float, 3));
    ctor.setProperty("FromEulerZYZ", engine->newFunction(float3x3_FromEulerZYZ_float_float_float, 3));
    ctor.setProperty("FromEulerXYZ", engine->newFunction(float3x3_FromEulerXYZ_float_float_float, 3));
    ctor.setProperty("FromEulerXZY", engine->newFunction(float3x3_FromEulerXZY_float_float_float, 3));
    ctor.setProperty("FromEulerYXZ", engine->newFunction(float3x3_FromEulerYXZ_float_float_float, 3));
    ctor.setProperty("FromEulerYZX", engine->newFunction(float3x3_FromEulerYZX_float_float_float, 3));
    ctor.setProperty("FromEulerZXY", engine->newFunction(float3x3_FromEulerZXY_float_float_float, 3));
    ctor.setProperty("FromEulerZYX", engine->newFunction(float3x3_FromEulerZYX_float_float_float, 3));
    ctor.setProperty("Scale", engine->newFunction(float3x3_Scale_selector, 3));
    ctor.setProperty("Scale", engine->newFunction(float3x3_Scale_selector, 1));
    ctor.setProperty("ScaleAlongAxis", engine->newFunction(float3x3_ScaleAlongAxis_float3_float, 2));
    ctor.setProperty("UniformScale", engine->newFunction(float3x3_UniformScale_float, 1));
    ctor.setProperty("ShearX", engine->newFunction(float3x3_ShearX_float_float, 2));
    ctor.setProperty("ShearY", engine->newFunction(float3x3_ShearY_float_float, 2));
    ctor.setProperty("ShearZ", engine->newFunction(float3x3_ShearZ_float_float, 2));
    ctor.setProperty("Reflect", engine->newFunction(float3x3_Reflect_Plane, 1));
    ctor.setProperty("MakeOrthographicProjection", engine->newFunction(float3x3_MakeOrthographicProjection_selector, 4));
    ctor.setProperty("MakeOrthographicProjection", engine->newFunction(float3x3_MakeOrthographicProjection_selector, 1));
    ctor.setProperty("MakeOrthographicProjectionYZ", engine->newFunction(float3x3_MakeOrthographicProjectionYZ, 0));
    ctor.setProperty("MakeOrthographicProjectionXZ", engine->newFunction(float3x3_MakeOrthographicProjectionXZ, 0));
    ctor.setProperty("MakeOrthographicProjectionXY", engine->newFunction(float3x3_MakeOrthographicProjectionXY, 0));
    ctor.setProperty("zero", TypeToQScriptValue(engine, float3x3::zero));
    ctor.setProperty("identity", TypeToQScriptValue(engine, float3x3::identity));
    engine->globalObject().setProperty("float3x3", ctor);
    return ctor;
}

