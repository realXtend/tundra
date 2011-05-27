#include "QtScriptBindingsHelpers.h"

static QScriptValue float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_float3 in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 ret;
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_float3_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3_float3_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float x = TypeFromQScriptValue<float>(context->argument(0));
    float y = TypeFromQScriptValue<float>(context->argument(1));
    float z = TypeFromQScriptValue<float>(context->argument(2));
    float3 ret(x, y, z);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_float3_float2_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3_float3_float2_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float2 xy = TypeFromQScriptValue<float2>(context->argument(0));
    float z = TypeFromQScriptValue<float>(context->argument(1));
    float3 ret(xy, z);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_xy(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_xy in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_xy in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float2 ret = This->xy();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_LengthSq(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_LengthSq in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_LengthSq in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->LengthSq();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_Length(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_Length in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_Length in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->Length();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_Normalize(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_Normalize in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_Normalize in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->Normalize();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_Normalized(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_Normalized in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_Normalized in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->Normalized();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_ScaleToLength_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_ScaleToLength_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_ScaleToLength_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float newLength = TypeFromQScriptValue<float>(context->argument(0));
    float ret = This->ScaleToLength(newLength);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_IsNormalized_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_IsNormalized_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_IsNormalized_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float epsilonSq = TypeFromQScriptValue<float>(context->argument(0));
    bool ret = This->IsNormalized(epsilonSq);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_IsZero_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_IsZero_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_IsZero_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float epsilonSq = TypeFromQScriptValue<float>(context->argument(0));
    bool ret = This->IsZero(epsilonSq);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_IsFinite(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_IsFinite in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_IsFinite in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    bool ret = This->IsFinite();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_IsPerpendicular_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3_IsPerpendicular_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_IsPerpendicular_float3_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 other = TypeFromQScriptValue<float3>(context->argument(0));
    float epsilon = TypeFromQScriptValue<float>(context->argument(1));
    bool ret = This->IsPerpendicular(other, epsilon);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_Equals_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3_Equals_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_Equals_float3_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 rhs = TypeFromQScriptValue<float3>(context->argument(0));
    float epsilon = TypeFromQScriptValue<float>(context->argument(1));
    bool ret = This->Equals(rhs, epsilon);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_Equals_float_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function float3_Equals_float_float_float_float in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_Equals_float_float_float_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float x = TypeFromQScriptValue<float>(context->argument(0));
    float y = TypeFromQScriptValue<float>(context->argument(1));
    float z = TypeFromQScriptValue<float>(context->argument(2));
    float epsilon = TypeFromQScriptValue<float>(context->argument(3));
    bool ret = This->Equals(x, y, z, epsilon);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_SumOfElements(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_SumOfElements in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_SumOfElements in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->SumOfElements();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_ProductOfElements(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_ProductOfElements in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_ProductOfElements in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->ProductOfElements();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_AverageOfElements(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_AverageOfElements in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_AverageOfElements in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->AverageOfElements();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_MinElement(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_MinElement in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_MinElement in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->MinElement();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_MinElementIndex(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_MinElementIndex in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_MinElementIndex in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int ret = This->MinElementIndex();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_MaxElement(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_MaxElement in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_MaxElement in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->MaxElement();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_MaxElementIndex(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_MaxElementIndex in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_MaxElementIndex in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    int ret = This->MaxElementIndex();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_Abs(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_Abs in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_Abs in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->Abs();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_Min_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Min_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_Min_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ceil = TypeFromQScriptValue<float>(context->argument(0));
    float3 ret = This->Min(ceil);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_Min_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Min_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_Min_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ceil = TypeFromQScriptValue<float3>(context->argument(0));
    float3 ret = This->Min(ceil);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_Max_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Max_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_Max_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float floor = TypeFromQScriptValue<float>(context->argument(0));
    float3 ret = This->Max(floor);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_Max_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Max_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_Max_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 floor = TypeFromQScriptValue<float3>(context->argument(0));
    float3 ret = This->Max(floor);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_Clamp_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3_Clamp_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_Clamp_float3_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 floor = TypeFromQScriptValue<float3>(context->argument(0));
    float3 ceil = TypeFromQScriptValue<float3>(context->argument(1));
    float3 ret = This->Clamp(floor, ceil);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_Clamp01(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_Clamp01 in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_Clamp01 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->Clamp01();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_Clamp_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3_Clamp_float_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_Clamp_float_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float floor = TypeFromQScriptValue<float>(context->argument(0));
    float ceil = TypeFromQScriptValue<float>(context->argument(1));
    float3 ret = This->Clamp(floor, ceil);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_Distance_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Distance_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_Distance_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 rhs = TypeFromQScriptValue<float3>(context->argument(0));
    float ret = This->Distance(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_DistanceSq_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_DistanceSq_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_DistanceSq_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 rhs = TypeFromQScriptValue<float3>(context->argument(0));
    float ret = This->DistanceSq(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_Dot_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Dot_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_Dot_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 rhs = TypeFromQScriptValue<float3>(context->argument(0));
    float ret = This->Dot(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_Cross_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Cross_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_Cross_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 rhs = TypeFromQScriptValue<float3>(context->argument(0));
    float3 ret = This->Cross(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_OuterProduct_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_OuterProduct_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_OuterProduct_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 rhs = TypeFromQScriptValue<float3>(context->argument(0));
    float3x3 ret = This->OuterProduct(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_Perpendicular_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3_Perpendicular_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_Perpendicular_float3_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 hint = TypeFromQScriptValue<float3>(context->argument(0));
    float3 hint2 = TypeFromQScriptValue<float3>(context->argument(1));
    float3 ret = This->Perpendicular(hint, hint2);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_AnotherPerpendicular_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3_AnotherPerpendicular_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_AnotherPerpendicular_float3_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 hint = TypeFromQScriptValue<float3>(context->argument(0));
    float3 hint2 = TypeFromQScriptValue<float3>(context->argument(1));
    float3 ret = This->AnotherPerpendicular(hint, hint2);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_Reflect_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Reflect_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_Reflect_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 normal = TypeFromQScriptValue<float3>(context->argument(0));
    float3 ret = This->Reflect(normal);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_Refract_float3_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3_Refract_float3_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_Refract_float3_float_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 normal = TypeFromQScriptValue<float3>(context->argument(0));
    float negativeSideRefractionIndex = TypeFromQScriptValue<float>(context->argument(1));
    float positiveSideRefractionIndex = TypeFromQScriptValue<float>(context->argument(2));
    float3 ret = This->Refract(normal, negativeSideRefractionIndex, positiveSideRefractionIndex);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_ProjectTo_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_ProjectTo_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_ProjectTo_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 direction = TypeFromQScriptValue<float3>(context->argument(0));
    float3 ret = This->ProjectTo(direction);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_ProjectToNorm_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_ProjectToNorm_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_ProjectToNorm_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 direction = TypeFromQScriptValue<float3>(context->argument(0));
    float3 ret = This->ProjectToNorm(direction);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_AngleBetween_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_AngleBetween_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_AngleBetween_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 other = TypeFromQScriptValue<float3>(context->argument(0));
    float ret = This->AngleBetween(other);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_AngleBetweenNorm_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_AngleBetweenNorm_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_AngleBetweenNorm_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 normalizedVector = TypeFromQScriptValue<float3>(context->argument(0));
    float ret = This->AngleBetweenNorm(normalizedVector);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_Decompose_float3_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3_Decompose_float3_float3_float3 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_Decompose_float3_float3_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 direction = TypeFromQScriptValue<float3>(context->argument(0));
    float3 outParallel = TypeFromQScriptValue<float3>(context->argument(1));
    float3 outPerpendicular = TypeFromQScriptValue<float3>(context->argument(2));
    This->Decompose(direction, outParallel, outPerpendicular);
    return QScriptValue();
}

static QScriptValue float3_Lerp_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3_Lerp_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_Lerp_float3_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 b = TypeFromQScriptValue<float3>(context->argument(0));
    float t = TypeFromQScriptValue<float>(context->argument(1));
    float3 ret = This->Lerp(b, t);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_SetFromScalar_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_SetFromScalar_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_SetFromScalar_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float scalar = TypeFromQScriptValue<float>(context->argument(0));
    This->SetFromScalar(scalar);
    return QScriptValue();
}

static QScriptValue float3_Set_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3_Set_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_Set_float_float_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float x = TypeFromQScriptValue<float>(context->argument(0));
    float y = TypeFromQScriptValue<float>(context->argument(1));
    float z = TypeFromQScriptValue<float>(context->argument(2));
    This->Set(x, y, z);
    return QScriptValue();
}

static QScriptValue float3_ToPos4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_ToPos4 in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_ToPos4 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4 ret = This->ToPos4();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_ToDir4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_ToDir4 in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_ToDir4 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4 ret = This->ToDir4();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_Add_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Add_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_Add_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 rhs = TypeFromQScriptValue<float3>(context->argument(0));
    float3 ret = This->Add(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_Sub_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Sub_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_Sub_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 rhs = TypeFromQScriptValue<float3>(context->argument(0));
    float3 ret = This->Sub(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_Mul_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Mul_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_Mul_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float rhs = TypeFromQScriptValue<float>(context->argument(0));
    float3 ret = This->Mul(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_Div_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_Div_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_Div_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float rhs = TypeFromQScriptValue<float>(context->argument(0));
    float3 ret = This->Div(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_Neg(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_Neg in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function float3_Neg in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->Neg();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_float3_QVector3D(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_float3_QVector3D in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    QVector3D other = TypeFromQScriptValue<QVector3D>(context->argument(0));
    float3 ret(other);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_toString(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_toString in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This && context->argumentCount() > 0) This = TypeFromQScriptValue<float3*>(context->argument(0)); // Qt oddity (bug?): Sometimes the built-in toString() function doesn't give us this from thisObject, but as the first argument.
    if (!This) { printf("Error! Invalid context->thisObject in function float3_toString in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    QString ret = This->toString();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_ScalarTripleProduct_float3_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3_ScalarTripleProduct_float3_float3_float3 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 u = TypeFromQScriptValue<float3>(context->argument(0));
    float3 v = TypeFromQScriptValue<float3>(context->argument(1));
    float3 w = TypeFromQScriptValue<float3>(context->argument(2));
    float ret = float3::ScalarTripleProduct(u, v, w);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_Orthogonalize_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3_Orthogonalize_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 a = TypeFromQScriptValue<float3>(context->argument(0));
    float3 b = TypeFromQScriptValue<float3>(context->argument(1));
    float3::Orthogonalize(a, b);
    return QScriptValue();
}

static QScriptValue float3_Orthogonalize_float3_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3_Orthogonalize_float3_float3_float3 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 a = TypeFromQScriptValue<float3>(context->argument(0));
    float3 b = TypeFromQScriptValue<float3>(context->argument(1));
    float3 c = TypeFromQScriptValue<float3>(context->argument(2));
    float3::Orthogonalize(a, b, c);
    return QScriptValue();
}

static QScriptValue float3_Orthonormalize_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function float3_Orthonormalize_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 a = TypeFromQScriptValue<float3>(context->argument(0));
    float3 b = TypeFromQScriptValue<float3>(context->argument(1));
    float3::Orthonormalize(a, b);
    return QScriptValue();
}

static QScriptValue float3_Orthonormalize_float3_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function float3_Orthonormalize_float3_float3_float3 in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 a = TypeFromQScriptValue<float3>(context->argument(0));
    float3 b = TypeFromQScriptValue<float3>(context->argument(1));
    float3 c = TypeFromQScriptValue<float3>(context->argument(2));
    float3::Orthonormalize(a, b, c);
    return QScriptValue();
}

static QScriptValue float3_FromScalar_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_FromScalar_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float scalar = TypeFromQScriptValue<float>(context->argument(0));
    float3 ret = float3::FromScalar(scalar);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue float3_x_get(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_x_get in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    return qScriptValueFromValue(context->engine(), This->x);
}

static QScriptValue float3_x_set(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_x_get in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float x = qscriptvalue_cast<float>(context->argument(0));
    This->x = x;
    return QScriptValue();
}

static QScriptValue float3_y_get(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_y_get in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    return qScriptValueFromValue(context->engine(), This->y);
}

static QScriptValue float3_y_set(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_y_get in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float y = qscriptvalue_cast<float>(context->argument(0));
    This->y = y;
    return QScriptValue();
}

static QScriptValue float3_z_get(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function float3_z_get in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    return qScriptValueFromValue(context->engine(), This->z);
}

static QScriptValue float3_z_set(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function float3_z_get in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 *This = TypeFromQScriptValue<float3*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float z = qscriptvalue_cast<float>(context->argument(0));
    This->z = z;
    return QScriptValue();
}

static QScriptValue float3_ctor(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return float3_float3(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return float3_float3_float_float_float(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float2>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return float3_float3_float2_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<QVector3D>(context->argument(0)))
        return float3_float3_QVector3D(context, engine);
    printf("float3_ctor failed to choose the right function to call! Did you use 'var x = float3();' instead of 'var x = new float3();'?\n"); return QScriptValue();
}

static QScriptValue float3_Equals_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return float3_Equals_float3_float(context, engine);
    if (context->argumentCount() == 4 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)) && QSVIsOfType<float>(context->argument(3)))
        return float3_Equals_float_float_float_float(context, engine);
    printf("float3_Equals_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float3_Min_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return float3_Min_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return float3_Min_float3(context, engine);
    printf("float3_Min_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float3_Max_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float>(context->argument(0)))
        return float3_Max_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return float3_Max_float3(context, engine);
    printf("float3_Max_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float3_Clamp_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return float3_Clamp_float3_float3(context, engine);
    if (context->argumentCount() == 2 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)))
        return float3_Clamp_float_float(context, engine);
    printf("float3_Clamp_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float3_Orthogonalize_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return float3_Orthogonalize_float3_float3(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)))
        return float3_Orthogonalize_float3_float3_float3(context, engine);
    printf("float3_Orthogonalize_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue float3_Orthonormalize_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return float3_Orthonormalize_float3_float3(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)))
        return float3_Orthonormalize_float3_float3_float3(context, engine);
    printf("float3_Orthonormalize_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

class float3_scriptclass : public QScriptClass
{
public:
    QScriptValue objectPrototype;
    float3_scriptclass(QScriptEngine *engine):QScriptClass(engine){}
    QScriptValue property(const QScriptValue &object, const QScriptString &name, uint id)
    {
        float3 *This = TypeFromQScriptValue<float3*>(object);
        if (!This) { printf("Error! Cannot convert QScriptValue to type float3 in file %s, line %d!\nTry using float3.get%s() and float3.set%s() to query the member variable '%s'!\n", __FILE__, __LINE__, Capitalize((QString)name).c_str(), Capitalize((QString)name).c_str(), ((QString)name).toStdString().c_str()); return QScriptValue(); }
        if ((QString)name == (QString)"x") return TypeToQScriptValue(engine(), This->x);
        if ((QString)name == (QString)"y") return TypeToQScriptValue(engine(), This->y);
        if ((QString)name == (QString)"z") return TypeToQScriptValue(engine(), This->z);
        if ((QString)name == (QString)"zero") return TypeToQScriptValue(engine(), This->zero);
        if ((QString)name == (QString)"one") return TypeToQScriptValue(engine(), This->one);
        if ((QString)name == (QString)"unitX") return TypeToQScriptValue(engine(), This->unitX);
        if ((QString)name == (QString)"unitY") return TypeToQScriptValue(engine(), This->unitY);
        if ((QString)name == (QString)"unitZ") return TypeToQScriptValue(engine(), This->unitZ);
        return QScriptValue();
    }
    void setProperty(QScriptValue &object, const QScriptString &name, uint id, const QScriptValue &value)
    {
        float3 *This = TypeFromQScriptValue<float3*>(object);
        if (!This) { printf("Error! Cannot convert QScriptValue to type float3 in file %s, line %d!\nTry using float3.get%s() and float3.set%s() to query the member variable '%s'!\n", __FILE__, __LINE__, Capitalize((QString)name).c_str(), Capitalize((QString)name).c_str(), ((QString)name).toStdString().c_str()); return; }
        if ((QString)name == (QString)"x") This->x = TypeFromQScriptValue<float>(value);
        if ((QString)name == (QString)"y") This->y = TypeFromQScriptValue<float>(value);
        if ((QString)name == (QString)"z") This->z = TypeFromQScriptValue<float>(value);
    }
    QueryFlags queryProperty(const QScriptValue &object, const QScriptString &name, QueryFlags flags, uint *id)
    {
        if ((QString)name == (QString)"x") return flags;
        if ((QString)name == (QString)"y") return flags;
        if ((QString)name == (QString)"z") return flags;
        if ((QString)name == (QString)"zero") return flags;
        if ((QString)name == (QString)"one") return flags;
        if ((QString)name == (QString)"unitX") return flags;
        if ((QString)name == (QString)"unitY") return flags;
        if ((QString)name == (QString)"unitZ") return flags;
        return 0;
    }
    QScriptValue prototype() const { return objectPrototype; }
};
QScriptValue register_float3_prototype(QScriptEngine *engine)
{
    engine->setDefaultPrototype(qMetaTypeId<float3*>(), QScriptValue());
    QScriptValue proto = engine->newVariant(qVariantFromValue((float3*)0));
    proto.setProperty("xy", engine->newFunction(float3_xy, 0));
    proto.setProperty("LengthSq", engine->newFunction(float3_LengthSq, 0));
    proto.setProperty("Length", engine->newFunction(float3_Length, 0));
    proto.setProperty("Normalize", engine->newFunction(float3_Normalize, 0));
    proto.setProperty("Normalized", engine->newFunction(float3_Normalized, 0));
    proto.setProperty("ScaleToLength", engine->newFunction(float3_ScaleToLength_float, 1));
    proto.setProperty("IsNormalized", engine->newFunction(float3_IsNormalized_float, 1));
    proto.setProperty("IsZero", engine->newFunction(float3_IsZero_float, 1));
    proto.setProperty("IsFinite", engine->newFunction(float3_IsFinite, 0));
    proto.setProperty("IsPerpendicular", engine->newFunction(float3_IsPerpendicular_float3_float, 2));
    proto.setProperty("Equals", engine->newFunction(float3_Equals_selector, 2));
    proto.setProperty("Equals", engine->newFunction(float3_Equals_selector, 4));
    proto.setProperty("SumOfElements", engine->newFunction(float3_SumOfElements, 0));
    proto.setProperty("ProductOfElements", engine->newFunction(float3_ProductOfElements, 0));
    proto.setProperty("AverageOfElements", engine->newFunction(float3_AverageOfElements, 0));
    proto.setProperty("MinElement", engine->newFunction(float3_MinElement, 0));
    proto.setProperty("MinElementIndex", engine->newFunction(float3_MinElementIndex, 0));
    proto.setProperty("MaxElement", engine->newFunction(float3_MaxElement, 0));
    proto.setProperty("MaxElementIndex", engine->newFunction(float3_MaxElementIndex, 0));
    proto.setProperty("Abs", engine->newFunction(float3_Abs, 0));
    proto.setProperty("Min", engine->newFunction(float3_Min_selector, 1));
    proto.setProperty("Max", engine->newFunction(float3_Max_selector, 1));
    proto.setProperty("Clamp", engine->newFunction(float3_Clamp_selector, 2));
    proto.setProperty("Clamp01", engine->newFunction(float3_Clamp01, 0));
    proto.setProperty("Distance", engine->newFunction(float3_Distance_float3, 1));
    proto.setProperty("DistanceSq", engine->newFunction(float3_DistanceSq_float3, 1));
    proto.setProperty("Dot", engine->newFunction(float3_Dot_float3, 1));
    proto.setProperty("Cross", engine->newFunction(float3_Cross_float3, 1));
    proto.setProperty("OuterProduct", engine->newFunction(float3_OuterProduct_float3, 1));
    proto.setProperty("Perpendicular", engine->newFunction(float3_Perpendicular_float3_float3, 2));
    proto.setProperty("AnotherPerpendicular", engine->newFunction(float3_AnotherPerpendicular_float3_float3, 2));
    proto.setProperty("Reflect", engine->newFunction(float3_Reflect_float3, 1));
    proto.setProperty("Refract", engine->newFunction(float3_Refract_float3_float_float, 3));
    proto.setProperty("ProjectTo", engine->newFunction(float3_ProjectTo_float3, 1));
    proto.setProperty("ProjectToNorm", engine->newFunction(float3_ProjectToNorm_float3, 1));
    proto.setProperty("AngleBetween", engine->newFunction(float3_AngleBetween_float3, 1));
    proto.setProperty("AngleBetweenNorm", engine->newFunction(float3_AngleBetweenNorm_float3, 1));
    proto.setProperty("Decompose", engine->newFunction(float3_Decompose_float3_float3_float3, 3));
    proto.setProperty("Lerp", engine->newFunction(float3_Lerp_float3_float, 2));
    proto.setProperty("SetFromScalar", engine->newFunction(float3_SetFromScalar_float, 1));
    proto.setProperty("Set", engine->newFunction(float3_Set_float_float_float, 3));
    proto.setProperty("ToPos4", engine->newFunction(float3_ToPos4, 0));
    proto.setProperty("ToDir4", engine->newFunction(float3_ToDir4, 0));
    proto.setProperty("Add", engine->newFunction(float3_Add_float3, 1));
    proto.setProperty("Sub", engine->newFunction(float3_Sub_float3, 1));
    proto.setProperty("Mul", engine->newFunction(float3_Mul_float, 1));
    proto.setProperty("Div", engine->newFunction(float3_Div_float, 1));
    proto.setProperty("Neg", engine->newFunction(float3_Neg, 0));
    proto.setProperty("toString", engine->newFunction(float3_toString, 0));
    proto.setProperty("getX", engine->newFunction(float3_x_get, 1));
    proto.setProperty("setX", engine->newFunction(float3_x_set, 1));
    proto.setProperty("getY", engine->newFunction(float3_y_get, 1));
    proto.setProperty("setY", engine->newFunction(float3_y_set, 1));
    proto.setProperty("getZ", engine->newFunction(float3_z_get, 1));
    proto.setProperty("setZ", engine->newFunction(float3_z_set, 1));
    float3_scriptclass *sc = new float3_scriptclass(engine);
    engine->setProperty("float3_scriptclass", QVariant::fromValue<QScriptClass*>(sc));
    proto.setScriptClass(sc);
    sc->objectPrototype = proto;
    engine->setDefaultPrototype(qMetaTypeId<float3>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<float3*>(), proto);
    QScriptValue ctor = engine->newFunction(float3_ctor, proto, 3);
    ctor.setProperty("ScalarTripleProduct", engine->newFunction(float3_ScalarTripleProduct_float3_float3_float3, 3));
    ctor.setProperty("Orthogonalize", engine->newFunction(float3_Orthogonalize_selector, 2));
    ctor.setProperty("Orthogonalize", engine->newFunction(float3_Orthogonalize_selector, 3));
    ctor.setProperty("Orthonormalize", engine->newFunction(float3_Orthonormalize_selector, 2));
    ctor.setProperty("Orthonormalize", engine->newFunction(float3_Orthonormalize_selector, 3));
    ctor.setProperty("FromScalar", engine->newFunction(float3_FromScalar_float, 1));
    ctor.setProperty("zero", TypeToQScriptValue(engine, float3::zero));
    ctor.setProperty("one", TypeToQScriptValue(engine, float3::one));
    ctor.setProperty("unitX", TypeToQScriptValue(engine, float3::unitX));
    ctor.setProperty("unitY", TypeToQScriptValue(engine, float3::unitY));
    ctor.setProperty("unitZ", TypeToQScriptValue(engine, float3::unitZ));
    engine->globalObject().setProperty("float3", ctor);
    return ctor;
}

