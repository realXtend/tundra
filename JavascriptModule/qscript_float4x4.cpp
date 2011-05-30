#include "QtScriptBindingsHelpers.h"

static QScriptValue float4x4_float4x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_float4x4 in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 ret;
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_float4x4_float_float_float_float_float_float_float_float_float_float_float_float_float_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 16) { printf("Error! Invalid number of arguments passed to function float4x4_float4x4_float_float_float_float_float_float_float_float_float_float_float_float_float_float_float_float in file %s, line %d!\nExpected 16, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float _00 = TypeFromQScriptValue<float>(context->argument(0));
    float _01 = TypeFromQScriptValue<float>(context->argument(1));
    float _02 = TypeFromQScriptValue<float>(context->argument(2));
    float _03 = TypeFromQScriptValue<float>(context->argument(3));
    float _10 = TypeFromQScriptValue<float>(context->argument(4));
    float _11 = TypeFromQScriptValue<float>(context->argument(5));
    float _12 = TypeFromQScriptValue<float>(context->argument(6));
    float _13 = TypeFromQScriptValue<float>(context->argument(7));
    float _20 = TypeFromQScriptValue<float>(context->argument(8));
    float _21 = TypeFromQScriptValue<float>(context->argument(9));
    float _22 = TypeFromQScriptValue<float>(context->argument(10));
    float _23 = TypeFromQScriptValue<float>(context->argument(11));
    float _30 = TypeFromQScriptValue<float>(context->argument(12));
    float _31 = TypeFromQScriptValue<float>(context->argument(13));
    float _32 = TypeFromQScriptValue<float>(context->argument(14));
    float _33 = TypeFromQScriptValue<float>(context->argument(15));
    float4x4 ret(_00, _01, _02, _03, _10, _11, _12, _13, _20, _21, _22, _23, _30, _31, _32, _33);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_float4x4_float3x3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_float4x4_float3x3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 other = TypeFromQScriptValue<float3x3>(context->argument(0));
    float4x4 ret(other);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_float4x4_float3x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_float4x4_float3x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x4 other = TypeFromQScriptValue<float3x4>(context->argument(0));
    float4x4 ret(other);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_float4x4_float4_float4_float4_float4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function float4x4_float4x4_float4_float4_float4_float4 in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4 col0 = TypeFromQScriptValue<float4>(context->argument(0));
    float4 col1 = TypeFromQScriptValue<float4>(context->argument(1));
    float4 col2 = TypeFromQScriptValue<float4>(context->argument(2));
    float4 col3 = TypeFromQScriptValue<float4>(context->argument(3));
    float4x4 ret(col0, col1, col2, col3);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_float4x4_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_float4x4_Quat in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat orientation = TypeFromQScriptValue<Quat>(context->argument(0));
    float4x4 ret(orientation);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_GetScale(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_GetScale in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_GetScale in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->GetScale();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_ComplementaryProjection(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_ComplementaryProjection in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_ComplementaryProjection in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4x4 ret = This->ComplementaryProjection();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_At_int_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4x4_At_int_int in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_At_int_int in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int row = TypeFromQScriptValue<int>(context->argument(0));
    int col = TypeFromQScriptValue<int>(context->argument(1));
    const float ret = This->At(row, col);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_Row_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_Row_int in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Row_int in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int row = TypeFromQScriptValue<int>(context->argument(0));
    const float4 & ret = This->Row(row);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_Row3_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_Row3_int in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Row3_int in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int row = TypeFromQScriptValue<int>(context->argument(0));
    const float3 & ret = This->Row3(row);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_Col_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_Col_int in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Col_int in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int col = TypeFromQScriptValue<int>(context->argument(0));
    const float4 ret = This->Col(col);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_Col3_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_Col3_int in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Col3_int in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int col = TypeFromQScriptValue<int>(context->argument(0));
    const float3 ret = This->Col3(col);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_Diagonal(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_Diagonal in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Diagonal in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    const float4 ret = This->Diagonal();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_Diagonal3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_Diagonal3 in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Diagonal3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    const float3 ret = This->Diagonal3();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_ScaleRow3_int_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4x4_ScaleRow3_int_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_ScaleRow3_int_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int row = TypeFromQScriptValue<int>(context->argument(0));
    float scalar = TypeFromQScriptValue<float>(context->argument(1));
    This->ScaleRow3(row, scalar);
    return QScriptValue();
}

static QScriptValue float4x4_ScaleRow_int_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4x4_ScaleRow_int_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_ScaleRow_int_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int row = TypeFromQScriptValue<int>(context->argument(0));
    float scalar = TypeFromQScriptValue<float>(context->argument(1));
    This->ScaleRow(row, scalar);
    return QScriptValue();
}

static QScriptValue float4x4_ScaleCol3_int_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4x4_ScaleCol3_int_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_ScaleCol3_int_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int row = TypeFromQScriptValue<int>(context->argument(0));
    float scalar = TypeFromQScriptValue<float>(context->argument(1));
    This->ScaleCol3(row, scalar);
    return QScriptValue();
}

static QScriptValue float4x4_ScaleCol_int_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4x4_ScaleCol_int_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_ScaleCol_int_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int col = TypeFromQScriptValue<int>(context->argument(0));
    float scalar = TypeFromQScriptValue<float>(context->argument(1));
    This->ScaleCol(col, scalar);
    return QScriptValue();
}

static QScriptValue float4x4_Float3x3Part(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_Float3x3Part in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Float3x3Part in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    const float3x3 ret = This->Float3x3Part();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_Float3x4Part(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_Float3x4Part in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Float3x4Part in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    const float3x4 & ret = This->Float3x4Part();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_TranslatePart(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_TranslatePart in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_TranslatePart in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->TranslatePart();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_RotatePart(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_RotatePart in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_RotatePart in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3x3 ret = This->RotatePart();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_WorldX(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_WorldX in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_WorldX in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->WorldX();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_WorldY(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_WorldY in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_WorldY in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->WorldY();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_WorldZ(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_WorldZ in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_WorldZ in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->WorldZ();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_SetRow3_int_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4x4_SetRow3_int_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_SetRow3_int_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int row = TypeFromQScriptValue<int>(context->argument(0));
    float3 rowVector = TypeFromQScriptValue<float3>(context->argument(1));
    This->SetRow3(row, rowVector);
    return QScriptValue();
}

static QScriptValue float4x4_SetRow3_int_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function float4x4_SetRow3_int_float_float_float in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_SetRow3_int_float_float_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int row = TypeFromQScriptValue<int>(context->argument(0));
    float m_r0 = TypeFromQScriptValue<float>(context->argument(1));
    float m_r1 = TypeFromQScriptValue<float>(context->argument(2));
    float m_r2 = TypeFromQScriptValue<float>(context->argument(3));
    This->SetRow3(row, m_r0, m_r1, m_r2);
    return QScriptValue();
}

static QScriptValue float4x4_SetRow_int_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float4x4_SetRow_int_float3_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_SetRow_int_float3_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int row = TypeFromQScriptValue<int>(context->argument(0));
    float3 rowVector = TypeFromQScriptValue<float3>(context->argument(1));
    float m_r3 = TypeFromQScriptValue<float>(context->argument(2));
    This->SetRow(row, rowVector, m_r3);
    return QScriptValue();
}

static QScriptValue float4x4_SetRow_int_float4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4x4_SetRow_int_float4 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_SetRow_int_float4 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int row = TypeFromQScriptValue<int>(context->argument(0));
    float4 rowVector = TypeFromQScriptValue<float4>(context->argument(1));
    This->SetRow(row, rowVector);
    return QScriptValue();
}

static QScriptValue float4x4_SetRow_int_float_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 5) { printf("Error! Invalid number of arguments passed to function float4x4_SetRow_int_float_float_float_float in file %s, line %d!\nExpected 5, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_SetRow_int_float_float_float_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int row = TypeFromQScriptValue<int>(context->argument(0));
    float m_r0 = TypeFromQScriptValue<float>(context->argument(1));
    float m_r1 = TypeFromQScriptValue<float>(context->argument(2));
    float m_r2 = TypeFromQScriptValue<float>(context->argument(3));
    float m_r3 = TypeFromQScriptValue<float>(context->argument(4));
    This->SetRow(row, m_r0, m_r1, m_r2, m_r3);
    return QScriptValue();
}

static QScriptValue float4x4_SetCol3_int_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4x4_SetCol3_int_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_SetCol3_int_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int column = TypeFromQScriptValue<int>(context->argument(0));
    float3 columnVector = TypeFromQScriptValue<float3>(context->argument(1));
    This->SetCol3(column, columnVector);
    return QScriptValue();
}

static QScriptValue float4x4_SetCol3_int_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function float4x4_SetCol3_int_float_float_float in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_SetCol3_int_float_float_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int column = TypeFromQScriptValue<int>(context->argument(0));
    float m_0c = TypeFromQScriptValue<float>(context->argument(1));
    float m_1c = TypeFromQScriptValue<float>(context->argument(2));
    float m_2c = TypeFromQScriptValue<float>(context->argument(3));
    This->SetCol3(column, m_0c, m_1c, m_2c);
    return QScriptValue();
}

static QScriptValue float4x4_SetCol_int_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float4x4_SetCol_int_float3_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_SetCol_int_float3_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int column = TypeFromQScriptValue<int>(context->argument(0));
    float3 columnVector = TypeFromQScriptValue<float3>(context->argument(1));
    float m_3c = TypeFromQScriptValue<float>(context->argument(2));
    This->SetCol(column, columnVector, m_3c);
    return QScriptValue();
}

static QScriptValue float4x4_SetCol_int_float4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4x4_SetCol_int_float4 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_SetCol_int_float4 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int column = TypeFromQScriptValue<int>(context->argument(0));
    float4 columnVector = TypeFromQScriptValue<float4>(context->argument(1));
    This->SetCol(column, columnVector);
    return QScriptValue();
}

static QScriptValue float4x4_SetCol_int_float_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 5) { printf("Error! Invalid number of arguments passed to function float4x4_SetCol_int_float_float_float_float in file %s, line %d!\nExpected 5, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_SetCol_int_float_float_float_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int column = TypeFromQScriptValue<int>(context->argument(0));
    float m_0c = TypeFromQScriptValue<float>(context->argument(1));
    float m_1c = TypeFromQScriptValue<float>(context->argument(2));
    float m_2c = TypeFromQScriptValue<float>(context->argument(3));
    float m_3c = TypeFromQScriptValue<float>(context->argument(4));
    This->SetCol(column, m_0c, m_1c, m_2c, m_3c);
    return QScriptValue();
}

static QScriptValue float4x4_Set_float_float_float_float_float_float_float_float_float_float_float_float_float_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 16) { printf("Error! Invalid number of arguments passed to function float4x4_Set_float_float_float_float_float_float_float_float_float_float_float_float_float_float_float_float in file %s, line %d!\nExpected 16, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Set_float_float_float_float_float_float_float_float_float_float_float_float_float_float_float_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float _00 = TypeFromQScriptValue<float>(context->argument(0));
    float _01 = TypeFromQScriptValue<float>(context->argument(1));
    float _02 = TypeFromQScriptValue<float>(context->argument(2));
    float _03 = TypeFromQScriptValue<float>(context->argument(3));
    float _10 = TypeFromQScriptValue<float>(context->argument(4));
    float _11 = TypeFromQScriptValue<float>(context->argument(5));
    float _12 = TypeFromQScriptValue<float>(context->argument(6));
    float _13 = TypeFromQScriptValue<float>(context->argument(7));
    float _20 = TypeFromQScriptValue<float>(context->argument(8));
    float _21 = TypeFromQScriptValue<float>(context->argument(9));
    float _22 = TypeFromQScriptValue<float>(context->argument(10));
    float _23 = TypeFromQScriptValue<float>(context->argument(11));
    float _30 = TypeFromQScriptValue<float>(context->argument(12));
    float _31 = TypeFromQScriptValue<float>(context->argument(13));
    float _32 = TypeFromQScriptValue<float>(context->argument(14));
    float _33 = TypeFromQScriptValue<float>(context->argument(15));
    This->Set(_00, _01, _02, _03, _10, _11, _12, _13, _20, _21, _22, _23, _30, _31, _32, _33);
    return QScriptValue();
}

static QScriptValue float4x4_Set3x3Part_float3x3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_Set3x3Part_float3x3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Set3x3Part_float3x3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3x3 rotation = TypeFromQScriptValue<float3x3>(context->argument(0));
    This->Set3x3Part(rotation);
    return QScriptValue();
}

static QScriptValue float4x4_Set3x4Part_float3x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_Set3x4Part_float3x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Set3x4Part_float3x4 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3x4 rotateTranslate = TypeFromQScriptValue<float3x4>(context->argument(0));
    This->Set3x4Part(rotateTranslate);
    return QScriptValue();
}

static QScriptValue float4x4_SetIdentity(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_SetIdentity in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_SetIdentity in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    This->SetIdentity();
    return QScriptValue();
}

static QScriptValue float4x4_SwapColumns_int_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4x4_SwapColumns_int_int in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_SwapColumns_int_int in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int col1 = TypeFromQScriptValue<int>(context->argument(0));
    int col2 = TypeFromQScriptValue<int>(context->argument(1));
    This->SwapColumns(col1, col2);
    return QScriptValue();
}

static QScriptValue float4x4_SwapColumns3_int_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4x4_SwapColumns3_int_int in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_SwapColumns3_int_int in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int col1 = TypeFromQScriptValue<int>(context->argument(0));
    int col2 = TypeFromQScriptValue<int>(context->argument(1));
    This->SwapColumns3(col1, col2);
    return QScriptValue();
}

static QScriptValue float4x4_SwapRows_int_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4x4_SwapRows_int_int in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_SwapRows_int_int in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int row1 = TypeFromQScriptValue<int>(context->argument(0));
    int row2 = TypeFromQScriptValue<int>(context->argument(1));
    This->SwapRows(row1, row2);
    return QScriptValue();
}

static QScriptValue float4x4_SwapRows3_int_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4x4_SwapRows3_int_int in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_SwapRows3_int_int in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int row1 = TypeFromQScriptValue<int>(context->argument(0));
    int row2 = TypeFromQScriptValue<int>(context->argument(1));
    This->SwapRows3(row1, row2);
    return QScriptValue();
}

static QScriptValue float4x4_SetTranslatePart_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float4x4_SetTranslatePart_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_SetTranslatePart_float_float_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float tx = TypeFromQScriptValue<float>(context->argument(0));
    float ty = TypeFromQScriptValue<float>(context->argument(1));
    float tz = TypeFromQScriptValue<float>(context->argument(2));
    This->SetTranslatePart(tx, ty, tz);
    return QScriptValue();
}

static QScriptValue float4x4_SetTranslatePart_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_SetTranslatePart_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_SetTranslatePart_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 offset = TypeFromQScriptValue<float3>(context->argument(0));
    This->SetTranslatePart(offset);
    return QScriptValue();
}

static QScriptValue float4x4_SetRotatePartX_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_SetRotatePartX_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_SetRotatePartX_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float angle = TypeFromQScriptValue<float>(context->argument(0));
    This->SetRotatePartX(angle);
    return QScriptValue();
}

static QScriptValue float4x4_SetRotatePartY_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_SetRotatePartY_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_SetRotatePartY_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float angle = TypeFromQScriptValue<float>(context->argument(0));
    This->SetRotatePartY(angle);
    return QScriptValue();
}

static QScriptValue float4x4_SetRotatePartZ_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_SetRotatePartZ_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_SetRotatePartZ_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float angle = TypeFromQScriptValue<float>(context->argument(0));
    This->SetRotatePartZ(angle);
    return QScriptValue();
}

static QScriptValue float4x4_SetRotatePart_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4x4_SetRotatePart_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_SetRotatePart_float3_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 axisDirection = TypeFromQScriptValue<float3>(context->argument(0));
    float angle = TypeFromQScriptValue<float>(context->argument(1));
    This->SetRotatePart(axisDirection, angle);
    return QScriptValue();
}

static QScriptValue float4x4_SetRotatePart_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_SetRotatePart_Quat in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_SetRotatePart_Quat in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    Quat orientation = TypeFromQScriptValue<Quat>(context->argument(0));
    This->SetRotatePart(orientation);
    return QScriptValue();
}

static QScriptValue float4x4_SetRotatePart_float3x3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_SetRotatePart_float3x3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_SetRotatePart_float3x3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3x3 rotation = TypeFromQScriptValue<float3x3>(context->argument(0));
    This->SetRotatePart(rotation);
    return QScriptValue();
}

static QScriptValue float4x4_Determinant3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_Determinant3 in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Determinant3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->Determinant3();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_Determinant4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_Determinant4 in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Determinant4 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->Determinant4();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_SubMatrix_int_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4x4_SubMatrix_int_int in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_SubMatrix_int_int in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int i = TypeFromQScriptValue<int>(context->argument(0));
    int j = TypeFromQScriptValue<int>(context->argument(1));
    float3x3 ret = This->SubMatrix(i, j);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_Minor_int_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4x4_Minor_int_int in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Minor_int_int in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int i = TypeFromQScriptValue<int>(context->argument(0));
    int j = TypeFromQScriptValue<int>(context->argument(1));
    float ret = This->Minor(i, j);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_Adjugate(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_Adjugate in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Adjugate in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4x4 ret = This->Adjugate();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_CholeskyDecompose_float4x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_CholeskyDecompose_float4x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_CholeskyDecompose_float4x4 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4x4 outL = TypeFromQScriptValue<float4x4>(context->argument(0));
    bool ret = This->CholeskyDecompose(outL);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_LUDecompose_float4x4_float4x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4x4_LUDecompose_float4x4_float4x4 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_LUDecompose_float4x4_float4x4 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4x4 outLower = TypeFromQScriptValue<float4x4>(context->argument(0));
    float4x4 outUpper = TypeFromQScriptValue<float4x4>(context->argument(1));
    bool ret = This->LUDecompose(outLower, outUpper);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_Inverse(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_Inverse in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Inverse in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    bool ret = This->Inverse();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_Inverted(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_Inverted in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Inverted in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4x4 ret = This->Inverted();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_InverseAffine(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_InverseAffine in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_InverseAffine in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    bool ret = This->InverseAffine();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_InverseAffineUniformScale(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_InverseAffineUniformScale in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_InverseAffineUniformScale in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    bool ret = This->InverseAffineUniformScale();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_InverseAffineNoScale(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_InverseAffineNoScale in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_InverseAffineNoScale in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    This->InverseAffineNoScale();
    return QScriptValue();
}

static QScriptValue float4x4_Transpose(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_Transpose in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Transpose in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    This->Transpose();
    return QScriptValue();
}

static QScriptValue float4x4_Transposed(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_Transposed in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Transposed in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4x4 ret = This->Transposed();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_InverseTranspose(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_InverseTranspose in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_InverseTranspose in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    bool ret = This->InverseTranspose();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_InverseTransposed(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_InverseTransposed in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_InverseTransposed in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4x4 ret = This->InverseTransposed();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_Trace(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_Trace in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Trace in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->Trace();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_Orthogonalize3_int_int_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float4x4_Orthogonalize3_int_int_int in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Orthogonalize3_int_int_int in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int firstColumn = TypeFromQScriptValue<int>(context->argument(0));
    int secondColumn = TypeFromQScriptValue<int>(context->argument(1));
    int thirdColumn = TypeFromQScriptValue<int>(context->argument(2));
    This->Orthogonalize3(firstColumn, secondColumn, thirdColumn);
    return QScriptValue();
}

static QScriptValue float4x4_Orthogonalize3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_Orthogonalize3 in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Orthogonalize3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    This->Orthogonalize3();
    return QScriptValue();
}

static QScriptValue float4x4_Orthonormalize3_int_int_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float4x4_Orthonormalize3_int_int_int in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Orthonormalize3_int_int_int in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int firstColumn = TypeFromQScriptValue<int>(context->argument(0));
    int secondColumn = TypeFromQScriptValue<int>(context->argument(1));
    int thirdColumn = TypeFromQScriptValue<int>(context->argument(2));
    This->Orthonormalize3(firstColumn, secondColumn, thirdColumn);
    return QScriptValue();
}

static QScriptValue float4x4_Orthonormalize3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_Orthonormalize3 in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Orthonormalize3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    This->Orthonormalize3();
    return QScriptValue();
}

static QScriptValue float4x4_RemoveScale(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_RemoveScale in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_RemoveScale in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    This->RemoveScale();
    return QScriptValue();
}

static QScriptValue float4x4_Pivot(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_Pivot in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Pivot in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    This->Pivot();
    return QScriptValue();
}

static QScriptValue float4x4_TransformPos_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_TransformPos_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_TransformPos_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 pointVector = TypeFromQScriptValue<float3>(context->argument(0));
    float3 ret = This->TransformPos(pointVector);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_TransformPos_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float4x4_TransformPos_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_TransformPos_float_float_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float x = TypeFromQScriptValue<float>(context->argument(0));
    float y = TypeFromQScriptValue<float>(context->argument(1));
    float z = TypeFromQScriptValue<float>(context->argument(2));
    float3 ret = This->TransformPos(x, y, z);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_TransformDir_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_TransformDir_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_TransformDir_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 directionVector = TypeFromQScriptValue<float3>(context->argument(0));
    float3 ret = This->TransformDir(directionVector);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_TransformDir_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float4x4_TransformDir_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_TransformDir_float_float_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float x = TypeFromQScriptValue<float>(context->argument(0));
    float y = TypeFromQScriptValue<float>(context->argument(1));
    float z = TypeFromQScriptValue<float>(context->argument(2));
    float3 ret = This->TransformDir(x, y, z);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_Transform_float4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_Transform_float4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Transform_float4 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4 vector = TypeFromQScriptValue<float4>(context->argument(0));
    float4 ret = This->Transform(vector);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_Transform_float4_ptr_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4x4_Transform_float4_ptr_int in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Transform_float4_ptr_int in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4 * vectorArray = TypeFromQScriptValue<float4 *>(context->argument(0));
    int numVectors = TypeFromQScriptValue<int>(context->argument(1));
    This->Transform(vectorArray, numVectors);
    return QScriptValue();
}

static QScriptValue float4x4_Transform_float4_ptr_int_int(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float4x4_Transform_float4_ptr_int_int in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Transform_float4_ptr_int_int in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4 * vectorArray = TypeFromQScriptValue<float4 *>(context->argument(0));
    int numVectors = TypeFromQScriptValue<int>(context->argument(1));
    int strideBytes = TypeFromQScriptValue<int>(context->argument(2));
    This->Transform(vectorArray, numVectors, strideBytes);
    return QScriptValue();
}

static QScriptValue float4x4_IsFinite(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_IsFinite in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_IsFinite in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    bool ret = This->IsFinite();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_IsIdentity_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_IsIdentity_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_IsIdentity_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float epsilon = TypeFromQScriptValue<float>(context->argument(0));
    bool ret = This->IsIdentity(epsilon);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_IsLowerTriangular_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_IsLowerTriangular_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_IsLowerTriangular_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float epsilon = TypeFromQScriptValue<float>(context->argument(0));
    bool ret = This->IsLowerTriangular(epsilon);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_IsUpperTriangular_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_IsUpperTriangular_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_IsUpperTriangular_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float epsilon = TypeFromQScriptValue<float>(context->argument(0));
    bool ret = This->IsUpperTriangular(epsilon);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_IsInvertible_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_IsInvertible_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_IsInvertible_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float epsilon = TypeFromQScriptValue<float>(context->argument(0));
    bool ret = This->IsInvertible(epsilon);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_IsSymmetric_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_IsSymmetric_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_IsSymmetric_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float epsilon = TypeFromQScriptValue<float>(context->argument(0));
    bool ret = This->IsSymmetric(epsilon);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_IsSkewSymmetric_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_IsSkewSymmetric_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_IsSkewSymmetric_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float epsilon = TypeFromQScriptValue<float>(context->argument(0));
    bool ret = This->IsSkewSymmetric(epsilon);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_IsIdempotent_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_IsIdempotent_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_IsIdempotent_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float epsilon = TypeFromQScriptValue<float>(context->argument(0));
    bool ret = This->IsIdempotent(epsilon);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_HasUnitaryScale_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_HasUnitaryScale_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_HasUnitaryScale_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float epsilon = TypeFromQScriptValue<float>(context->argument(0));
    bool ret = This->HasUnitaryScale(epsilon);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_HasNegativeScale(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_HasNegativeScale in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_HasNegativeScale in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    bool ret = This->HasNegativeScale();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_HasUniformScale_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_HasUniformScale_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_HasUniformScale_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float epsilon = TypeFromQScriptValue<float>(context->argument(0));
    bool ret = This->HasUniformScale(epsilon);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_IsOrthogonal3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_IsOrthogonal3_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_IsOrthogonal3_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float epsilon = TypeFromQScriptValue<float>(context->argument(0));
    bool ret = This->IsOrthogonal3(epsilon);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_Equals_float4x4_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4x4_Equals_float4x4_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Equals_float4x4_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4x4 other = TypeFromQScriptValue<float4x4>(context->argument(0));
    float epsilon = TypeFromQScriptValue<float>(context->argument(1));
    bool ret = This->Equals(other, epsilon);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_ContainsProjection_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_ContainsProjection_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_ContainsProjection_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float epsilon = TypeFromQScriptValue<float>(context->argument(0));
    bool ret = This->ContainsProjection(epsilon);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_ToEulerXYX(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_ToEulerXYX in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_ToEulerXYX in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ToEulerXYX();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_ToEulerXZX(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_ToEulerXZX in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_ToEulerXZX in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ToEulerXZX();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_ToEulerYXY(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_ToEulerYXY in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_ToEulerYXY in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ToEulerYXY();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_ToEulerYZY(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_ToEulerYZY in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_ToEulerYZY in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ToEulerYZY();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_ToEulerZXZ(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_ToEulerZXZ in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_ToEulerZXZ in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ToEulerZXZ();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_ToEulerZYZ(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_ToEulerZYZ in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_ToEulerZYZ in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ToEulerZYZ();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_ToEulerXYZ(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_ToEulerXYZ in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_ToEulerXYZ in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ToEulerXYZ();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_ToEulerXZY(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_ToEulerXZY in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_ToEulerXZY in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ToEulerXZY();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_ToEulerYXZ(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_ToEulerYXZ in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_ToEulerYXZ in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ToEulerYXZ();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_ToEulerYZX(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_ToEulerYZX in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_ToEulerYZX in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ToEulerYZX();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_ToEulerZXY(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_ToEulerZXY in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_ToEulerZXY in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ToEulerZXY();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_ToEulerZYX(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_ToEulerZYX in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_ToEulerZYX in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ToEulerZYX();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_ExtractScale(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_ExtractScale in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_ExtractScale in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ExtractScale();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_Decompose_float3_Quat_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float4x4_Decompose_float3_Quat_float3 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Decompose_float3_Quat_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 translate = TypeFromQScriptValue<float3>(context->argument(0));
    Quat rotate = TypeFromQScriptValue<Quat>(context->argument(1));
    float3 scale = TypeFromQScriptValue<float3>(context->argument(2));
    This->Decompose(translate, rotate, scale);
    return QScriptValue();
}

static QScriptValue float4x4_Decompose_float3_float3x3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float4x4_Decompose_float3_float3x3_float3 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Decompose_float3_float3x3_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 translate = TypeFromQScriptValue<float3>(context->argument(0));
    float3x3 rotate = TypeFromQScriptValue<float3x3>(context->argument(1));
    float3 scale = TypeFromQScriptValue<float3>(context->argument(2));
    This->Decompose(translate, rotate, scale);
    return QScriptValue();
}

static QScriptValue float4x4_Decompose_float3_float3x4_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float4x4_Decompose_float3_float3x4_float3 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Decompose_float3_float3x4_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 translate = TypeFromQScriptValue<float3>(context->argument(0));
    float3x4 rotate = TypeFromQScriptValue<float3x4>(context->argument(1));
    float3 scale = TypeFromQScriptValue<float3>(context->argument(2));
    This->Decompose(translate, rotate, scale);
    return QScriptValue();
}

static QScriptValue float4x4_Decompose_float3_float4x4_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float4x4_Decompose_float3_float4x4_float3 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Decompose_float3_float4x4_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 translate = TypeFromQScriptValue<float3>(context->argument(0));
    float4x4 rotate = TypeFromQScriptValue<float4x4>(context->argument(1));
    float3 scale = TypeFromQScriptValue<float3>(context->argument(2));
    This->Decompose(translate, rotate, scale);
    return QScriptValue();
}

static QScriptValue float4x4_float4x4_QMatrix4x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_float4x4_QMatrix4x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    QMatrix4x4 m = TypeFromQScriptValue<QMatrix4x4>(context->argument(0));
    float4x4 ret(m);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_toString(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_toString in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This && context->argumentCount() > 0) This = TypeFromQScriptValue<float4x4*>(context->argument(0)); // Qt oddity (bug?): Sometimes the built-in toString() function doesn't give us this from thisObject, but as the first argument.
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_toString in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    QString ret = This->toString();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_Mul_float3x3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_Mul_float3x3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Mul_float3x3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3x3 rhs = TypeFromQScriptValue<float3x3>(context->argument(0));
    float4x4 ret = This->Mul(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_Mul_float3x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_Mul_float3x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Mul_float3x4 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3x4 rhs = TypeFromQScriptValue<float3x4>(context->argument(0));
    float4x4 ret = This->Mul(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_Mul_float4x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_Mul_float4x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Mul_float4x4 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4x4 rhs = TypeFromQScriptValue<float4x4>(context->argument(0));
    float4x4 ret = This->Mul(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_Mul_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_Mul_Quat in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Mul_Quat in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    Quat rhs = TypeFromQScriptValue<Quat>(context->argument(0));
    float4x4 ret = This->Mul(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_MulPos_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_MulPos_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_MulPos_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 pointVector = TypeFromQScriptValue<float3>(context->argument(0));
    float3 ret = This->MulPos(pointVector);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_MulDir_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_MulDir_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_MulDir_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 directionVector = TypeFromQScriptValue<float3>(context->argument(0));
    float3 ret = This->MulDir(directionVector);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_Mul_float4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_Mul_float4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 *This = TypeFromQScriptValue<float4x4*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float4x4_Mul_float4 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4 vector = TypeFromQScriptValue<float4>(context->argument(0));
    float4 ret = This->Mul(vector);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_Translate_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float4x4_Translate_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float tx = TypeFromQScriptValue<float>(context->argument(0));
    float ty = TypeFromQScriptValue<float>(context->argument(1));
    float tz = TypeFromQScriptValue<float>(context->argument(2));
    TranslateOp ret = float4x4::Translate(tx, ty, tz);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_Translate_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_Translate_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 offset = TypeFromQScriptValue<float3>(context->argument(0));
    TranslateOp ret = float4x4::Translate(offset);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_RotateX_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_RotateX_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float angleRadians = TypeFromQScriptValue<float>(context->argument(0));
    float4x4 ret = float4x4::RotateX(angleRadians);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_RotateX_float_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4x4_RotateX_float_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float angleRadians = TypeFromQScriptValue<float>(context->argument(0));
    float3 pointOnAxis = TypeFromQScriptValue<float3>(context->argument(1));
    float4x4 ret = float4x4::RotateX(angleRadians, pointOnAxis);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_RotateY_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_RotateY_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float angleRadians = TypeFromQScriptValue<float>(context->argument(0));
    float4x4 ret = float4x4::RotateY(angleRadians);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_RotateY_float_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4x4_RotateY_float_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float angleRadians = TypeFromQScriptValue<float>(context->argument(0));
    float3 pointOnAxis = TypeFromQScriptValue<float3>(context->argument(1));
    float4x4 ret = float4x4::RotateY(angleRadians, pointOnAxis);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_RotateZ_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_RotateZ_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float angleRadians = TypeFromQScriptValue<float>(context->argument(0));
    float4x4 ret = float4x4::RotateZ(angleRadians);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_RotateZ_float_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4x4_RotateZ_float_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float angleRadians = TypeFromQScriptValue<float>(context->argument(0));
    float3 pointOnAxis = TypeFromQScriptValue<float3>(context->argument(1));
    float4x4 ret = float4x4::RotateZ(angleRadians, pointOnAxis);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_RotateAxisAngle_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4x4_RotateAxisAngle_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 axisDirection = TypeFromQScriptValue<float3>(context->argument(0));
    float angleRadians = TypeFromQScriptValue<float>(context->argument(1));
    float4x4 ret = float4x4::RotateAxisAngle(axisDirection, angleRadians);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_RotateAxisAngle_float3_float_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float4x4_RotateAxisAngle_float3_float_float3 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 axisDirection = TypeFromQScriptValue<float3>(context->argument(0));
    float angleRadians = TypeFromQScriptValue<float>(context->argument(1));
    float3 pointOnAxis = TypeFromQScriptValue<float3>(context->argument(2));
    float4x4 ret = float4x4::RotateAxisAngle(axisDirection, angleRadians, pointOnAxis);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_RotateFromTo_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4x4_RotateFromTo_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 sourceDirection = TypeFromQScriptValue<float3>(context->argument(0));
    float3 targetDirection = TypeFromQScriptValue<float3>(context->argument(1));
    float4x4 ret = float4x4::RotateFromTo(sourceDirection, targetDirection);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_RotateFromTo_float3_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float4x4_RotateFromTo_float3_float3_float3 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 sourceDirection = TypeFromQScriptValue<float3>(context->argument(0));
    float3 targetDirection = TypeFromQScriptValue<float3>(context->argument(1));
    float3 centerPoint = TypeFromQScriptValue<float3>(context->argument(2));
    float4x4 ret = float4x4::RotateFromTo(sourceDirection, targetDirection, centerPoint);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_RotateFromTo_float3_float3_float3_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 5) { printf("Error! Invalid number of arguments passed to function float4x4_RotateFromTo_float3_float3_float3_float3_float3 in file %s, line %d!\nExpected 5, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 centerPoint = TypeFromQScriptValue<float3>(context->argument(0));
    float3 sourceDirection = TypeFromQScriptValue<float3>(context->argument(1));
    float3 targetDirection = TypeFromQScriptValue<float3>(context->argument(2));
    float3 sourceDirection2 = TypeFromQScriptValue<float3>(context->argument(3));
    float3 targetDirection2 = TypeFromQScriptValue<float3>(context->argument(4));
    float4x4 ret = float4x4::RotateFromTo(centerPoint, sourceDirection, targetDirection, sourceDirection2, targetDirection2);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_FromQuat_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_FromQuat_Quat in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat orientation = TypeFromQScriptValue<Quat>(context->argument(0));
    float4x4 ret = float4x4::FromQuat(orientation);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_FromQuat_Quat_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4x4_FromQuat_Quat_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat orientation = TypeFromQScriptValue<Quat>(context->argument(0));
    float3 pointOnAxis = TypeFromQScriptValue<float3>(context->argument(1));
    float4x4 ret = float4x4::FromQuat(orientation, pointOnAxis);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_FromTRS_float3_Quat_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float4x4_FromTRS_float3_Quat_float3 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 translate = TypeFromQScriptValue<float3>(context->argument(0));
    Quat rotate = TypeFromQScriptValue<Quat>(context->argument(1));
    float3 scale = TypeFromQScriptValue<float3>(context->argument(2));
    float4x4 ret = float4x4::FromTRS(translate, rotate, scale);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_FromTRS_float3_float3x3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float4x4_FromTRS_float3_float3x3_float3 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 translate = TypeFromQScriptValue<float3>(context->argument(0));
    float3x3 rotate = TypeFromQScriptValue<float3x3>(context->argument(1));
    float3 scale = TypeFromQScriptValue<float3>(context->argument(2));
    float4x4 ret = float4x4::FromTRS(translate, rotate, scale);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_FromTRS_float3_float3x4_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float4x4_FromTRS_float3_float3x4_float3 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 translate = TypeFromQScriptValue<float3>(context->argument(0));
    float3x4 rotate = TypeFromQScriptValue<float3x4>(context->argument(1));
    float3 scale = TypeFromQScriptValue<float3>(context->argument(2));
    float4x4 ret = float4x4::FromTRS(translate, rotate, scale);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_FromTRS_float3_float4x4_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float4x4_FromTRS_float3_float4x4_float3 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 translate = TypeFromQScriptValue<float3>(context->argument(0));
    float4x4 rotate = TypeFromQScriptValue<float4x4>(context->argument(1));
    float3 scale = TypeFromQScriptValue<float3>(context->argument(2));
    float4x4 ret = float4x4::FromTRS(translate, rotate, scale);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_FromEulerXYX_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float4x4_FromEulerXYX_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float x2 = TypeFromQScriptValue<float>(context->argument(0));
    float y = TypeFromQScriptValue<float>(context->argument(1));
    float x = TypeFromQScriptValue<float>(context->argument(2));
    float4x4 ret = float4x4::FromEulerXYX(x2, y, x);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_FromEulerXZX_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float4x4_FromEulerXZX_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float x2 = TypeFromQScriptValue<float>(context->argument(0));
    float z = TypeFromQScriptValue<float>(context->argument(1));
    float x = TypeFromQScriptValue<float>(context->argument(2));
    float4x4 ret = float4x4::FromEulerXZX(x2, z, x);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_FromEulerYXY_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float4x4_FromEulerYXY_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float y2 = TypeFromQScriptValue<float>(context->argument(0));
    float x = TypeFromQScriptValue<float>(context->argument(1));
    float y = TypeFromQScriptValue<float>(context->argument(2));
    float4x4 ret = float4x4::FromEulerYXY(y2, x, y);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_FromEulerYZY_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float4x4_FromEulerYZY_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float y2 = TypeFromQScriptValue<float>(context->argument(0));
    float z = TypeFromQScriptValue<float>(context->argument(1));
    float y = TypeFromQScriptValue<float>(context->argument(2));
    float4x4 ret = float4x4::FromEulerYZY(y2, z, y);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_FromEulerZXZ_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float4x4_FromEulerZXZ_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float z2 = TypeFromQScriptValue<float>(context->argument(0));
    float x = TypeFromQScriptValue<float>(context->argument(1));
    float z = TypeFromQScriptValue<float>(context->argument(2));
    float4x4 ret = float4x4::FromEulerZXZ(z2, x, z);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_FromEulerZYZ_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float4x4_FromEulerZYZ_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float z2 = TypeFromQScriptValue<float>(context->argument(0));
    float y = TypeFromQScriptValue<float>(context->argument(1));
    float z = TypeFromQScriptValue<float>(context->argument(2));
    float4x4 ret = float4x4::FromEulerZYZ(z2, y, z);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_FromEulerXYZ_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float4x4_FromEulerXYZ_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float x = TypeFromQScriptValue<float>(context->argument(0));
    float y = TypeFromQScriptValue<float>(context->argument(1));
    float z = TypeFromQScriptValue<float>(context->argument(2));
    float4x4 ret = float4x4::FromEulerXYZ(x, y, z);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_FromEulerXZY_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float4x4_FromEulerXZY_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float x = TypeFromQScriptValue<float>(context->argument(0));
    float z = TypeFromQScriptValue<float>(context->argument(1));
    float y = TypeFromQScriptValue<float>(context->argument(2));
    float4x4 ret = float4x4::FromEulerXZY(x, z, y);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_FromEulerYXZ_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float4x4_FromEulerYXZ_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float y = TypeFromQScriptValue<float>(context->argument(0));
    float x = TypeFromQScriptValue<float>(context->argument(1));
    float z = TypeFromQScriptValue<float>(context->argument(2));
    float4x4 ret = float4x4::FromEulerYXZ(y, x, z);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_FromEulerYZX_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float4x4_FromEulerYZX_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float y = TypeFromQScriptValue<float>(context->argument(0));
    float z = TypeFromQScriptValue<float>(context->argument(1));
    float x = TypeFromQScriptValue<float>(context->argument(2));
    float4x4 ret = float4x4::FromEulerYZX(y, z, x);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_FromEulerZXY_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float4x4_FromEulerZXY_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float z = TypeFromQScriptValue<float>(context->argument(0));
    float x = TypeFromQScriptValue<float>(context->argument(1));
    float y = TypeFromQScriptValue<float>(context->argument(2));
    float4x4 ret = float4x4::FromEulerZXY(z, x, y);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_FromEulerZYX_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float4x4_FromEulerZYX_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float z = TypeFromQScriptValue<float>(context->argument(0));
    float y = TypeFromQScriptValue<float>(context->argument(1));
    float x = TypeFromQScriptValue<float>(context->argument(2));
    float4x4 ret = float4x4::FromEulerZYX(z, y, x);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_Scale_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float4x4_Scale_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float sx = TypeFromQScriptValue<float>(context->argument(0));
    float sy = TypeFromQScriptValue<float>(context->argument(1));
    float sz = TypeFromQScriptValue<float>(context->argument(2));
    ScaleOp ret = float4x4::Scale(sx, sy, sz);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_Scale_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_Scale_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 scale = TypeFromQScriptValue<float3>(context->argument(0));
    ScaleOp ret = float4x4::Scale(scale);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_Scale_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4x4_Scale_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 scale = TypeFromQScriptValue<float3>(context->argument(0));
    float3 scaleCenter = TypeFromQScriptValue<float3>(context->argument(1));
    float4x4 ret = float4x4::Scale(scale, scaleCenter);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_ScaleAlongAxis_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4x4_ScaleAlongAxis_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 axis = TypeFromQScriptValue<float3>(context->argument(0));
    float scalingFactor = TypeFromQScriptValue<float>(context->argument(1));
    float4x4 ret = float4x4::ScaleAlongAxis(axis, scalingFactor);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_ScaleAlongAxis_float3_float_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float4x4_ScaleAlongAxis_float3_float_float3 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 axis = TypeFromQScriptValue<float3>(context->argument(0));
    float scalingFactor = TypeFromQScriptValue<float>(context->argument(1));
    float3 scaleCenter = TypeFromQScriptValue<float3>(context->argument(2));
    float4x4 ret = float4x4::ScaleAlongAxis(axis, scalingFactor, scaleCenter);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_UniformScale_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_UniformScale_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float uniformScale = TypeFromQScriptValue<float>(context->argument(0));
    ScaleOp ret = float4x4::UniformScale(uniformScale);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_ShearX_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4x4_ShearX_float_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float yFactor = TypeFromQScriptValue<float>(context->argument(0));
    float zFactor = TypeFromQScriptValue<float>(context->argument(1));
    float4x4 ret = float4x4::ShearX(yFactor, zFactor);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_ShearY_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4x4_ShearY_float_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float xFactor = TypeFromQScriptValue<float>(context->argument(0));
    float zFactor = TypeFromQScriptValue<float>(context->argument(1));
    float4x4 ret = float4x4::ShearY(xFactor, zFactor);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_ShearZ_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float4x4_ShearZ_float_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float xFactor = TypeFromQScriptValue<float>(context->argument(0));
    float yFactor = TypeFromQScriptValue<float>(context->argument(1));
    float4x4 ret = float4x4::ShearZ(xFactor, yFactor);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_Reflect_Plane(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_Reflect_Plane in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Plane p = TypeFromQScriptValue<Plane>(context->argument(0));
    float4x4 ret = float4x4::Reflect(p);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_MakePerspectiveProjection_float_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function float4x4_MakePerspectiveProjection_float_float_float_float in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float nearPlaneDistance = TypeFromQScriptValue<float>(context->argument(0));
    float farPlaneDistance = TypeFromQScriptValue<float>(context->argument(1));
    float horizontalFov = TypeFromQScriptValue<float>(context->argument(2));
    float verticalFov = TypeFromQScriptValue<float>(context->argument(3));
    float4x4 ret = float4x4::MakePerspectiveProjection(nearPlaneDistance, farPlaneDistance, horizontalFov, verticalFov);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_MakeOrthographicProjection_float_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function float4x4_MakeOrthographicProjection_float_float_float_float in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float nearPlaneDistance = TypeFromQScriptValue<float>(context->argument(0));
    float farPlaneDistance = TypeFromQScriptValue<float>(context->argument(1));
    float horizontalViewportSize = TypeFromQScriptValue<float>(context->argument(2));
    float verticalViewportSize = TypeFromQScriptValue<float>(context->argument(3));
    float4x4 ret = float4x4::MakeOrthographicProjection(nearPlaneDistance, farPlaneDistance, horizontalViewportSize, verticalViewportSize);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_MakeOrthographicProjection_Plane(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float4x4_MakeOrthographicProjection_Plane in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Plane target = TypeFromQScriptValue<Plane>(context->argument(0));
    float4x4 ret = float4x4::MakeOrthographicProjection(target);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_MakeOrthographicProjectionYZ(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_MakeOrthographicProjectionYZ in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 ret = float4x4::MakeOrthographicProjectionYZ();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_MakeOrthographicProjectionXZ(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_MakeOrthographicProjectionXZ in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 ret = float4x4::MakeOrthographicProjectionXZ();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_MakeOrthographicProjectionXY(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float4x4_MakeOrthographicProjectionXY in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 ret = float4x4::MakeOrthographicProjectionXY();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_LookAt_float3_float3_float3_float3_bool(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 5) { printf("Error! Invalid number of arguments passed to function float4x4_LookAt_float3_float3_float3_float3_bool in file %s, line %d!\nExpected 5, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 localForward = TypeFromQScriptValue<float3>(context->argument(0));
    float3 targetDirection = TypeFromQScriptValue<float3>(context->argument(1));
    float3 localUp = TypeFromQScriptValue<float3>(context->argument(2));
    float3 worldUp = TypeFromQScriptValue<float3>(context->argument(3));
    bool rightHanded = TypeFromQScriptValue<bool>(context->argument(4));
    float4x4 ret = float4x4::LookAt(localForward, targetDirection, localUp, worldUp, rightHanded);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float4x4_ctor(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return float4x4_float4x4(context, engine);
    if (context->argumentCount() == 16 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)) && QSVIsOfType<float>(context->argument(3)) && QSVIsOfType<float>(context->argument(4)) && QSVIsOfType<float>(context->argument(5)) && QSVIsOfType<float>(context->argument(6)) && QSVIsOfType<float>(context->argument(7)) && QSVIsOfType<float>(context->argument(8)) && QSVIsOfType<float>(context->argument(9)) && QSVIsOfType<float>(context->argument(10)) && QSVIsOfType<float>(context->argument(11)) && QSVIsOfType<float>(context->argument(12)) && QSVIsOfType<float>(context->argument(13)) && QSVIsOfType<float>(context->argument(14)) && QSVIsOfType<float>(context->argument(15)))
        return float4x4_float4x4_float_float_float_float_float_float_float_float_float_float_float_float_float_float_float_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3x3>(context->argument(0)))
        return float4x4_float4x4_float3x3(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3x4>(context->argument(0)))
        return float4x4_float4x4_float3x4(context, engine);
    if (context->argumentCount() == 4 && QSVIsOfType<float4>(context->argument(0)) && QSVIsOfType<float4>(context->argument(1)) && QSVIsOfType<float4>(context->argument(2)) && QSVIsOfType<float4>(context->argument(3)))
        return float4x4_float4x4_float4_float4_float4_float4(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Quat>(context->argument(0)))
        return float4x4_float4x4_Quat(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<QMatrix4x4>(context->argument(0)))
        return float4x4_float4x4_QMatrix4x4(context, engine);
    printf("float4x4_ctor failed to choose the right function to call! Did you use 'var x = float4x4();' instead of 'var x = new float4x4();'?\n"); return QScriptValue();
}

static QScriptValue float4x4_SetRow3_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<int>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return float4x4_SetRow3_int_float3(context, engine);
    if (context->argumentCount() == 4 && QSVIsOfType<int>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)) && QSVIsOfType<float>(context->argument(3)))
        return float4x4_SetRow3_int_float_float_float(context, engine);
    printf("float4x4_SetRow3_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float4x4_SetRow_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 3 && QSVIsOfType<int>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return float4x4_SetRow_int_float3_float(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<int>(context->argument(0)) && QSVIsOfType<float4>(context->argument(1)))
        return float4x4_SetRow_int_float4(context, engine);
    if (context->argumentCount() == 5 && QSVIsOfType<int>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)) && QSVIsOfType<float>(context->argument(3)) && QSVIsOfType<float>(context->argument(4)))
        return float4x4_SetRow_int_float_float_float_float(context, engine);
    printf("float4x4_SetRow_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float4x4_SetCol3_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<int>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return float4x4_SetCol3_int_float3(context, engine);
    if (context->argumentCount() == 4 && QSVIsOfType<int>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)) && QSVIsOfType<float>(context->argument(3)))
        return float4x4_SetCol3_int_float_float_float(context, engine);
    printf("float4x4_SetCol3_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float4x4_SetCol_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 3 && QSVIsOfType<int>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return float4x4_SetCol_int_float3_float(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<int>(context->argument(0)) && QSVIsOfType<float4>(context->argument(1)))
        return float4x4_SetCol_int_float4(context, engine);
    if (context->argumentCount() == 5 && QSVIsOfType<int>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)) && QSVIsOfType<float>(context->argument(3)) && QSVIsOfType<float>(context->argument(4)))
        return float4x4_SetCol_int_float_float_float_float(context, engine);
    printf("float4x4_SetCol_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float4x4_SetTranslatePart_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 3 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return float4x4_SetTranslatePart_float_float_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return float4x4_SetTranslatePart_float3(context, engine);
    printf("float4x4_SetTranslatePart_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float4x4_SetRotatePart_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return float4x4_SetRotatePart_float3_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Quat>(context->argument(0)))
        return float4x4_SetRotatePart_Quat(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3x3>(context->argument(0)))
        return float4x4_SetRotatePart_float3x3(context, engine);
    printf("float4x4_SetRotatePart_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float4x4_Orthogonalize3_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 3 && QSVIsOfType<int>(context->argument(0)) && QSVIsOfType<int>(context->argument(1)) && QSVIsOfType<int>(context->argument(2)))
        return float4x4_Orthogonalize3_int_int_int(context, engine);
    if (context->argumentCount() == 0)
        return float4x4_Orthogonalize3(context, engine);
    printf("float4x4_Orthogonalize3_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float4x4_Orthonormalize3_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 3 && QSVIsOfType<int>(context->argument(0)) && QSVIsOfType<int>(context->argument(1)) && QSVIsOfType<int>(context->argument(2)))
        return float4x4_Orthonormalize3_int_int_int(context, engine);
    if (context->argumentCount() == 0)
        return float4x4_Orthonormalize3(context, engine);
    printf("float4x4_Orthonormalize3_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float4x4_TransformPos_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return float4x4_TransformPos_float3(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return float4x4_TransformPos_float_float_float(context, engine);
    printf("float4x4_TransformPos_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float4x4_TransformDir_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return float4x4_TransformDir_float3(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return float4x4_TransformDir_float_float_float(context, engine);
    printf("float4x4_TransformDir_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float4x4_Transform_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float4>(context->argument(0)))
        return float4x4_Transform_float4(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float4 *>(context->argument(0)) && QSVIsOfType<int>(context->argument(1)))
        return float4x4_Transform_float4_ptr_int(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float4 *>(context->argument(0)) && QSVIsOfType<int>(context->argument(1)) && QSVIsOfType<int>(context->argument(2)))
        return float4x4_Transform_float4_ptr_int_int(context, engine);
    printf("float4x4_Transform_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float4x4_Decompose_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 3 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<Quat>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)))
        return float4x4_Decompose_float3_Quat_float3(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3x3>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)))
        return float4x4_Decompose_float3_float3x3_float3(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3x4>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)))
        return float4x4_Decompose_float3_float3x4_float3(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float4x4>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)))
        return float4x4_Decompose_float3_float4x4_float3(context, engine);
    printf("float4x4_Decompose_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float4x4_Mul_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3x3>(context->argument(0)))
        return float4x4_Mul_float3x3(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3x4>(context->argument(0)))
        return float4x4_Mul_float3x4(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float4x4>(context->argument(0)))
        return float4x4_Mul_float4x4(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Quat>(context->argument(0)))
        return float4x4_Mul_Quat(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float4>(context->argument(0)))
        return float4x4_Mul_float4(context, engine);
    printf("float4x4_Mul_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float4x4_Translate_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 3 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return float4x4_Translate_float_float_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return float4x4_Translate_float3(context, engine);
    printf("float4x4_Translate_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float4x4_RotateX_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return float4x4_RotateX_float(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return float4x4_RotateX_float_float3(context, engine);
    printf("float4x4_RotateX_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float4x4_RotateY_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return float4x4_RotateY_float(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return float4x4_RotateY_float_float3(context, engine);
    printf("float4x4_RotateY_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float4x4_RotateZ_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return float4x4_RotateZ_float(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return float4x4_RotateZ_float_float3(context, engine);
    printf("float4x4_RotateZ_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float4x4_RotateAxisAngle_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return float4x4_RotateAxisAngle_float3_float(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)))
        return float4x4_RotateAxisAngle_float3_float_float3(context, engine);
    printf("float4x4_RotateAxisAngle_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float4x4_RotateFromTo_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return float4x4_RotateFromTo_float3_float3(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)))
        return float4x4_RotateFromTo_float3_float3_float3(context, engine);
    if (context->argumentCount() == 5 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)) && QSVIsOfType<float3>(context->argument(3)) && QSVIsOfType<float3>(context->argument(4)))
        return float4x4_RotateFromTo_float3_float3_float3_float3_float3(context, engine);
    printf("float4x4_RotateFromTo_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float4x4_FromQuat_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<Quat>(context->argument(0)))
        return float4x4_FromQuat_Quat(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<Quat>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return float4x4_FromQuat_Quat_float3(context, engine);
    printf("float4x4_FromQuat_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float4x4_FromTRS_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 3 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<Quat>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)))
        return float4x4_FromTRS_float3_Quat_float3(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3x3>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)))
        return float4x4_FromTRS_float3_float3x3_float3(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3x4>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)))
        return float4x4_FromTRS_float3_float3x4_float3(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float4x4>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)))
        return float4x4_FromTRS_float3_float4x4_float3(context, engine);
    printf("float4x4_FromTRS_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float4x4_Scale_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 3 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return float4x4_Scale_float_float_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return float4x4_Scale_float3(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return float4x4_Scale_float3_float3(context, engine);
    printf("float4x4_Scale_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float4x4_ScaleAlongAxis_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return float4x4_ScaleAlongAxis_float3_float(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)))
        return float4x4_ScaleAlongAxis_float3_float_float3(context, engine);
    printf("float4x4_ScaleAlongAxis_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float4x4_MakeOrthographicProjection_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 4 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)) && QSVIsOfType<float>(context->argument(3)))
        return float4x4_MakeOrthographicProjection_float_float_float_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<Plane>(context->argument(0)))
        return float4x4_MakeOrthographicProjection_Plane(context, engine);
    printf("float4x4_MakeOrthographicProjection_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

class float4x4_scriptclass : public QScriptClass
{
public:
    QScriptValue objectPrototype;
    float4x4_scriptclass(QScriptEngine *engine):QScriptClass(engine){}
    QScriptValue property(const QScriptValue &object, const QScriptString &name, uint id)
    {
        float4x4 *This = TypeFromQScriptValue<float4x4*>(object);
        if (!This) { printf("Error! Cannot convert QScriptValue to type float4x4 in file %s, line %d!\nTry using float4x4.get%s() and float4x4.set%s() to query the member variable '%s'!\n", __FILE__, __LINE__, Capitalize((QString)name).c_str(), Capitalize((QString)name).c_str(), ((QString)name).toStdString().c_str()); return QScriptValue(); }
        QString name_ = (QString)name;
        return QScriptValue();
    }
    void setProperty(QScriptValue &object, const QScriptString &name, uint id, const QScriptValue &value)
    {
        float4x4 *This = TypeFromQScriptValue<float4x4*>(object);
        if (!This) { printf("Error! Cannot convert QScriptValue to type float4x4 in file %s, line %d!\nTry using float4x4.get%s() and float4x4.set%s() to query the member variable '%s'!\n", __FILE__, __LINE__, Capitalize((QString)name).c_str(), Capitalize((QString)name).c_str(), ((QString)name).toStdString().c_str()); return; }
        QString name_ = (QString)name;
    }
    QueryFlags queryProperty(const QScriptValue &object, const QScriptString &name, QueryFlags flags, uint *id)
    {
        QString name_ = (QString)name;
        return 0;
    }
    QScriptValue prototype() const { return objectPrototype; }
};
QScriptValue register_float4x4_prototype(QScriptEngine *engine)
{
    engine->setDefaultPrototype(qMetaTypeId<float4x4*>(), QScriptValue());
    QScriptValue proto = engine->newVariant(qVariantFromValue((float4x4*)0));
    proto.setProperty("GetScale", engine->newFunction(float4x4_GetScale, 0));
    proto.setProperty("ComplementaryProjection", engine->newFunction(float4x4_ComplementaryProjection, 0));
    proto.setProperty("At", engine->newFunction(float4x4_At_int_int, 2));
    proto.setProperty("Row", engine->newFunction(float4x4_Row_int, 1));
    proto.setProperty("Row3", engine->newFunction(float4x4_Row3_int, 1));
    proto.setProperty("Col", engine->newFunction(float4x4_Col_int, 1));
    proto.setProperty("Col3", engine->newFunction(float4x4_Col3_int, 1));
    proto.setProperty("Diagonal", engine->newFunction(float4x4_Diagonal, 0));
    proto.setProperty("Diagonal3", engine->newFunction(float4x4_Diagonal3, 0));
    proto.setProperty("ScaleRow3", engine->newFunction(float4x4_ScaleRow3_int_float, 2));
    proto.setProperty("ScaleRow", engine->newFunction(float4x4_ScaleRow_int_float, 2));
    proto.setProperty("ScaleCol3", engine->newFunction(float4x4_ScaleCol3_int_float, 2));
    proto.setProperty("ScaleCol", engine->newFunction(float4x4_ScaleCol_int_float, 2));
    proto.setProperty("Float3x3Part", engine->newFunction(float4x4_Float3x3Part, 0));
    proto.setProperty("Float3x4Part", engine->newFunction(float4x4_Float3x4Part, 0));
    proto.setProperty("TranslatePart", engine->newFunction(float4x4_TranslatePart, 0));
    proto.setProperty("RotatePart", engine->newFunction(float4x4_RotatePart, 0));
    proto.setProperty("WorldX", engine->newFunction(float4x4_WorldX, 0));
    proto.setProperty("WorldY", engine->newFunction(float4x4_WorldY, 0));
    proto.setProperty("WorldZ", engine->newFunction(float4x4_WorldZ, 0));
    proto.setProperty("SetRow3", engine->newFunction(float4x4_SetRow3_selector, 2));
    proto.setProperty("SetRow3", engine->newFunction(float4x4_SetRow3_selector, 4));
    proto.setProperty("SetRow", engine->newFunction(float4x4_SetRow_selector, 3));
    proto.setProperty("SetRow", engine->newFunction(float4x4_SetRow_selector, 2));
    proto.setProperty("SetRow", engine->newFunction(float4x4_SetRow_selector, 5));
    proto.setProperty("SetCol3", engine->newFunction(float4x4_SetCol3_selector, 2));
    proto.setProperty("SetCol3", engine->newFunction(float4x4_SetCol3_selector, 4));
    proto.setProperty("SetCol", engine->newFunction(float4x4_SetCol_selector, 3));
    proto.setProperty("SetCol", engine->newFunction(float4x4_SetCol_selector, 2));
    proto.setProperty("SetCol", engine->newFunction(float4x4_SetCol_selector, 5));
    proto.setProperty("Set", engine->newFunction(float4x4_Set_float_float_float_float_float_float_float_float_float_float_float_float_float_float_float_float, 16));
    proto.setProperty("Set3x3Part", engine->newFunction(float4x4_Set3x3Part_float3x3, 1));
    proto.setProperty("Set3x4Part", engine->newFunction(float4x4_Set3x4Part_float3x4, 1));
    proto.setProperty("SetIdentity", engine->newFunction(float4x4_SetIdentity, 0));
    proto.setProperty("SwapColumns", engine->newFunction(float4x4_SwapColumns_int_int, 2));
    proto.setProperty("SwapColumns3", engine->newFunction(float4x4_SwapColumns3_int_int, 2));
    proto.setProperty("SwapRows", engine->newFunction(float4x4_SwapRows_int_int, 2));
    proto.setProperty("SwapRows3", engine->newFunction(float4x4_SwapRows3_int_int, 2));
    proto.setProperty("SetTranslatePart", engine->newFunction(float4x4_SetTranslatePart_selector, 3));
    proto.setProperty("SetTranslatePart", engine->newFunction(float4x4_SetTranslatePart_selector, 1));
    proto.setProperty("SetRotatePartX", engine->newFunction(float4x4_SetRotatePartX_float, 1));
    proto.setProperty("SetRotatePartY", engine->newFunction(float4x4_SetRotatePartY_float, 1));
    proto.setProperty("SetRotatePartZ", engine->newFunction(float4x4_SetRotatePartZ_float, 1));
    proto.setProperty("SetRotatePart", engine->newFunction(float4x4_SetRotatePart_selector, 2));
    proto.setProperty("SetRotatePart", engine->newFunction(float4x4_SetRotatePart_selector, 1));
    proto.setProperty("Determinant3", engine->newFunction(float4x4_Determinant3, 0));
    proto.setProperty("Determinant4", engine->newFunction(float4x4_Determinant4, 0));
    proto.setProperty("SubMatrix", engine->newFunction(float4x4_SubMatrix_int_int, 2));
    proto.setProperty("Minor", engine->newFunction(float4x4_Minor_int_int, 2));
    proto.setProperty("Adjugate", engine->newFunction(float4x4_Adjugate, 0));
    proto.setProperty("CholeskyDecompose", engine->newFunction(float4x4_CholeskyDecompose_float4x4, 1));
    proto.setProperty("LUDecompose", engine->newFunction(float4x4_LUDecompose_float4x4_float4x4, 2));
    proto.setProperty("Inverse", engine->newFunction(float4x4_Inverse, 0));
    proto.setProperty("Inverted", engine->newFunction(float4x4_Inverted, 0));
    proto.setProperty("InverseAffine", engine->newFunction(float4x4_InverseAffine, 0));
    proto.setProperty("InverseAffineUniformScale", engine->newFunction(float4x4_InverseAffineUniformScale, 0));
    proto.setProperty("InverseAffineNoScale", engine->newFunction(float4x4_InverseAffineNoScale, 0));
    proto.setProperty("Transpose", engine->newFunction(float4x4_Transpose, 0));
    proto.setProperty("Transposed", engine->newFunction(float4x4_Transposed, 0));
    proto.setProperty("InverseTranspose", engine->newFunction(float4x4_InverseTranspose, 0));
    proto.setProperty("InverseTransposed", engine->newFunction(float4x4_InverseTransposed, 0));
    proto.setProperty("Trace", engine->newFunction(float4x4_Trace, 0));
    proto.setProperty("Orthogonalize3", engine->newFunction(float4x4_Orthogonalize3_selector, 3));
    proto.setProperty("Orthogonalize3", engine->newFunction(float4x4_Orthogonalize3_selector, 0));
    proto.setProperty("Orthonormalize3", engine->newFunction(float4x4_Orthonormalize3_selector, 3));
    proto.setProperty("Orthonormalize3", engine->newFunction(float4x4_Orthonormalize3_selector, 0));
    proto.setProperty("RemoveScale", engine->newFunction(float4x4_RemoveScale, 0));
    proto.setProperty("Pivot", engine->newFunction(float4x4_Pivot, 0));
    proto.setProperty("TransformPos", engine->newFunction(float4x4_TransformPos_selector, 1));
    proto.setProperty("TransformPos", engine->newFunction(float4x4_TransformPos_selector, 3));
    proto.setProperty("TransformDir", engine->newFunction(float4x4_TransformDir_selector, 1));
    proto.setProperty("TransformDir", engine->newFunction(float4x4_TransformDir_selector, 3));
    proto.setProperty("Transform", engine->newFunction(float4x4_Transform_selector, 1));
    proto.setProperty("Transform", engine->newFunction(float4x4_Transform_selector, 2));
    proto.setProperty("Transform", engine->newFunction(float4x4_Transform_selector, 3));
    proto.setProperty("IsFinite", engine->newFunction(float4x4_IsFinite, 0));
    proto.setProperty("IsIdentity", engine->newFunction(float4x4_IsIdentity_float, 1));
    proto.setProperty("IsLowerTriangular", engine->newFunction(float4x4_IsLowerTriangular_float, 1));
    proto.setProperty("IsUpperTriangular", engine->newFunction(float4x4_IsUpperTriangular_float, 1));
    proto.setProperty("IsInvertible", engine->newFunction(float4x4_IsInvertible_float, 1));
    proto.setProperty("IsSymmetric", engine->newFunction(float4x4_IsSymmetric_float, 1));
    proto.setProperty("IsSkewSymmetric", engine->newFunction(float4x4_IsSkewSymmetric_float, 1));
    proto.setProperty("IsIdempotent", engine->newFunction(float4x4_IsIdempotent_float, 1));
    proto.setProperty("HasUnitaryScale", engine->newFunction(float4x4_HasUnitaryScale_float, 1));
    proto.setProperty("HasNegativeScale", engine->newFunction(float4x4_HasNegativeScale, 0));
    proto.setProperty("HasUniformScale", engine->newFunction(float4x4_HasUniformScale_float, 1));
    proto.setProperty("IsOrthogonal3", engine->newFunction(float4x4_IsOrthogonal3_float, 1));
    proto.setProperty("Equals", engine->newFunction(float4x4_Equals_float4x4_float, 2));
    proto.setProperty("ContainsProjection", engine->newFunction(float4x4_ContainsProjection_float, 1));
    proto.setProperty("ToEulerXYX", engine->newFunction(float4x4_ToEulerXYX, 0));
    proto.setProperty("ToEulerXZX", engine->newFunction(float4x4_ToEulerXZX, 0));
    proto.setProperty("ToEulerYXY", engine->newFunction(float4x4_ToEulerYXY, 0));
    proto.setProperty("ToEulerYZY", engine->newFunction(float4x4_ToEulerYZY, 0));
    proto.setProperty("ToEulerZXZ", engine->newFunction(float4x4_ToEulerZXZ, 0));
    proto.setProperty("ToEulerZYZ", engine->newFunction(float4x4_ToEulerZYZ, 0));
    proto.setProperty("ToEulerXYZ", engine->newFunction(float4x4_ToEulerXYZ, 0));
    proto.setProperty("ToEulerXZY", engine->newFunction(float4x4_ToEulerXZY, 0));
    proto.setProperty("ToEulerYXZ", engine->newFunction(float4x4_ToEulerYXZ, 0));
    proto.setProperty("ToEulerYZX", engine->newFunction(float4x4_ToEulerYZX, 0));
    proto.setProperty("ToEulerZXY", engine->newFunction(float4x4_ToEulerZXY, 0));
    proto.setProperty("ToEulerZYX", engine->newFunction(float4x4_ToEulerZYX, 0));
    proto.setProperty("ExtractScale", engine->newFunction(float4x4_ExtractScale, 0));
    proto.setProperty("Decompose", engine->newFunction(float4x4_Decompose_selector, 3));
    proto.setProperty("toString", engine->newFunction(float4x4_toString, 0));
    proto.setProperty("Mul", engine->newFunction(float4x4_Mul_selector, 1));
    proto.setProperty("MulPos", engine->newFunction(float4x4_MulPos_float3, 1));
    proto.setProperty("MulDir", engine->newFunction(float4x4_MulDir_float3, 1));
    float4x4_scriptclass *sc = new float4x4_scriptclass(engine);
    engine->setProperty("float4x4_scriptclass", QVariant::fromValue<QScriptClass*>(sc));
    proto.setScriptClass(sc);
    sc->objectPrototype = proto;
    engine->setDefaultPrototype(qMetaTypeId<float4x4>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<float4x4*>(), proto);
    QScriptValue ctor = engine->newFunction(float4x4_ctor, proto, 16);
    ctor.setProperty("Translate", engine->newFunction(float4x4_Translate_selector, 3));
    ctor.setProperty("Translate", engine->newFunction(float4x4_Translate_selector, 1));
    ctor.setProperty("RotateX", engine->newFunction(float4x4_RotateX_selector, 1));
    ctor.setProperty("RotateX", engine->newFunction(float4x4_RotateX_selector, 2));
    ctor.setProperty("RotateY", engine->newFunction(float4x4_RotateY_selector, 1));
    ctor.setProperty("RotateY", engine->newFunction(float4x4_RotateY_selector, 2));
    ctor.setProperty("RotateZ", engine->newFunction(float4x4_RotateZ_selector, 1));
    ctor.setProperty("RotateZ", engine->newFunction(float4x4_RotateZ_selector, 2));
    ctor.setProperty("RotateAxisAngle", engine->newFunction(float4x4_RotateAxisAngle_selector, 2));
    ctor.setProperty("RotateAxisAngle", engine->newFunction(float4x4_RotateAxisAngle_selector, 3));
    ctor.setProperty("RotateFromTo", engine->newFunction(float4x4_RotateFromTo_selector, 2));
    ctor.setProperty("RotateFromTo", engine->newFunction(float4x4_RotateFromTo_selector, 3));
    ctor.setProperty("RotateFromTo", engine->newFunction(float4x4_RotateFromTo_selector, 5));
    ctor.setProperty("FromQuat", engine->newFunction(float4x4_FromQuat_selector, 1));
    ctor.setProperty("FromQuat", engine->newFunction(float4x4_FromQuat_selector, 2));
    ctor.setProperty("FromTRS", engine->newFunction(float4x4_FromTRS_selector, 3));
    ctor.setProperty("FromEulerXYX", engine->newFunction(float4x4_FromEulerXYX_float_float_float, 3));
    ctor.setProperty("FromEulerXZX", engine->newFunction(float4x4_FromEulerXZX_float_float_float, 3));
    ctor.setProperty("FromEulerYXY", engine->newFunction(float4x4_FromEulerYXY_float_float_float, 3));
    ctor.setProperty("FromEulerYZY", engine->newFunction(float4x4_FromEulerYZY_float_float_float, 3));
    ctor.setProperty("FromEulerZXZ", engine->newFunction(float4x4_FromEulerZXZ_float_float_float, 3));
    ctor.setProperty("FromEulerZYZ", engine->newFunction(float4x4_FromEulerZYZ_float_float_float, 3));
    ctor.setProperty("FromEulerXYZ", engine->newFunction(float4x4_FromEulerXYZ_float_float_float, 3));
    ctor.setProperty("FromEulerXZY", engine->newFunction(float4x4_FromEulerXZY_float_float_float, 3));
    ctor.setProperty("FromEulerYXZ", engine->newFunction(float4x4_FromEulerYXZ_float_float_float, 3));
    ctor.setProperty("FromEulerYZX", engine->newFunction(float4x4_FromEulerYZX_float_float_float, 3));
    ctor.setProperty("FromEulerZXY", engine->newFunction(float4x4_FromEulerZXY_float_float_float, 3));
    ctor.setProperty("FromEulerZYX", engine->newFunction(float4x4_FromEulerZYX_float_float_float, 3));
    ctor.setProperty("Scale", engine->newFunction(float4x4_Scale_selector, 3));
    ctor.setProperty("Scale", engine->newFunction(float4x4_Scale_selector, 1));
    ctor.setProperty("Scale", engine->newFunction(float4x4_Scale_selector, 2));
    ctor.setProperty("ScaleAlongAxis", engine->newFunction(float4x4_ScaleAlongAxis_selector, 2));
    ctor.setProperty("ScaleAlongAxis", engine->newFunction(float4x4_ScaleAlongAxis_selector, 3));
    ctor.setProperty("UniformScale", engine->newFunction(float4x4_UniformScale_float, 1));
    ctor.setProperty("ShearX", engine->newFunction(float4x4_ShearX_float_float, 2));
    ctor.setProperty("ShearY", engine->newFunction(float4x4_ShearY_float_float, 2));
    ctor.setProperty("ShearZ", engine->newFunction(float4x4_ShearZ_float_float, 2));
    ctor.setProperty("Reflect", engine->newFunction(float4x4_Reflect_Plane, 1));
    ctor.setProperty("MakePerspectiveProjection", engine->newFunction(float4x4_MakePerspectiveProjection_float_float_float_float, 4));
    ctor.setProperty("MakeOrthographicProjection", engine->newFunction(float4x4_MakeOrthographicProjection_selector, 4));
    ctor.setProperty("MakeOrthographicProjection", engine->newFunction(float4x4_MakeOrthographicProjection_selector, 1));
    ctor.setProperty("MakeOrthographicProjectionYZ", engine->newFunction(float4x4_MakeOrthographicProjectionYZ, 0));
    ctor.setProperty("MakeOrthographicProjectionXZ", engine->newFunction(float4x4_MakeOrthographicProjectionXZ, 0));
    ctor.setProperty("MakeOrthographicProjectionXY", engine->newFunction(float4x4_MakeOrthographicProjectionXY, 0));
    ctor.setProperty("LookAt", engine->newFunction(float4x4_LookAt_float3_float3_float3_float3_bool, 5));
    ctor.setProperty("zero", TypeToQScriptValue(engine, float4x4::zero));
    ctor.setProperty("identity", TypeToQScriptValue(engine, float4x4::identity));
    engine->globalObject().setProperty("float4x4", ctor);
    return ctor;
}

