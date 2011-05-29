#include "QtScriptBindingsHelpers.h"

static QScriptValue Quat_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_Quat in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat ret;
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_Quat_float3x3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_Quat_float3x3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x3 rotationMatrix = TypeFromQScriptValue<float3x3>(context->argument(0));
    Quat ret(rotationMatrix);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_Quat_float3x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_Quat_float3x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3x4 rotationMatrix = TypeFromQScriptValue<float3x4>(context->argument(0));
    Quat ret(rotationMatrix);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_Quat_float4x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_Quat_float4x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float4x4 rotationMatrix = TypeFromQScriptValue<float4x4>(context->argument(0));
    Quat ret(rotationMatrix);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_Quat_float_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function Quat_Quat_float_float_float_float in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float x = TypeFromQScriptValue<float>(context->argument(0));
    float y = TypeFromQScriptValue<float>(context->argument(1));
    float z = TypeFromQScriptValue<float>(context->argument(2));
    float w = TypeFromQScriptValue<float>(context->argument(3));
    Quat ret(x, y, z, w);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_Quat_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Quat_Quat_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 rotationAxis = TypeFromQScriptValue<float3>(context->argument(0));
    float rotationAngle = TypeFromQScriptValue<float>(context->argument(1));
    Quat ret(rotationAxis, rotationAngle);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_WorldX(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_WorldX in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_WorldX in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->WorldX();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_WorldY(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_WorldY in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_WorldY in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->WorldY();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_WorldZ(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_WorldZ in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_WorldZ in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->WorldZ();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_Axis(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_Axis in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_Axis in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->Axis();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_Angle(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_Angle in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_Angle in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->Angle();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_Dot_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_Dot_Quat in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_Dot_Quat in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    Quat rhs = TypeFromQScriptValue<Quat>(context->argument(0));
    float ret = This->Dot(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_LengthSq(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_LengthSq in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_LengthSq in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->LengthSq();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_Length(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_Length in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_Length in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->Length();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_Normalize(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_Normalize in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_Normalize in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->Normalize();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_Normalized(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_Normalized in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_Normalized in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    Quat ret = This->Normalized();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_IsNormalized_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_IsNormalized_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_IsNormalized_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float epsilon = TypeFromQScriptValue<float>(context->argument(0));
    bool ret = This->IsNormalized(epsilon);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_IsInvertible_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_IsInvertible_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_IsInvertible_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float epsilon = TypeFromQScriptValue<float>(context->argument(0));
    bool ret = This->IsInvertible(epsilon);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_IsFinite(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_IsFinite in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_IsFinite in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    bool ret = This->IsFinite();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_Inverse(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_Inverse in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_Inverse in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    This->Inverse();
    return QScriptValue();
}

static QScriptValue Quat_Inverted(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_Inverted in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_Inverted in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    Quat ret = This->Inverted();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_InverseAndNormalize(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_InverseAndNormalize in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_InverseAndNormalize in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float ret = This->InverseAndNormalize();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_Conjugate(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_Conjugate in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_Conjugate in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    This->Conjugate();
    return QScriptValue();
}

static QScriptValue Quat_Conjugated(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_Conjugated in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_Conjugated in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    Quat ret = This->Conjugated();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_Transform_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Quat_Transform_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_Transform_float_float_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float x = TypeFromQScriptValue<float>(context->argument(0));
    float y = TypeFromQScriptValue<float>(context->argument(1));
    float z = TypeFromQScriptValue<float>(context->argument(2));
    float3 ret = This->Transform(x, y, z);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_Transform_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_Transform_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_Transform_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 vec = TypeFromQScriptValue<float3>(context->argument(0));
    float3 ret = This->Transform(vec);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_Transform_float4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_Transform_float4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_Transform_float4 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4 vec = TypeFromQScriptValue<float4>(context->argument(0));
    float4 ret = This->Transform(vec);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_Lerp_Quat_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Quat_Lerp_Quat_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_Lerp_Quat_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    Quat target = TypeFromQScriptValue<Quat>(context->argument(0));
    float t = TypeFromQScriptValue<float>(context->argument(1));
    Quat ret = This->Lerp(target, t);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_Slerp_Quat_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Quat_Slerp_Quat_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_Slerp_Quat_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    Quat target = TypeFromQScriptValue<Quat>(context->argument(0));
    float t = TypeFromQScriptValue<float>(context->argument(1));
    Quat ret = This->Slerp(target, t);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_AngleBetween_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_AngleBetween_Quat in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_AngleBetween_Quat in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    Quat target = TypeFromQScriptValue<Quat>(context->argument(0));
    float ret = This->AngleBetween(target);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_AxisFromTo_Quat(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_AxisFromTo_Quat in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_AxisFromTo_Quat in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    Quat target = TypeFromQScriptValue<Quat>(context->argument(0));
    float3 ret = This->AxisFromTo(target);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_ToAxisAngle_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Quat_ToAxisAngle_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_ToAxisAngle_float3_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 rotationAxis = TypeFromQScriptValue<float3>(context->argument(0));
    float rotationAngle = TypeFromQScriptValue<float>(context->argument(1));
    This->ToAxisAngle(rotationAxis, rotationAngle);
    return QScriptValue();
}

static QScriptValue Quat_SetFromAxisAngle_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Quat_SetFromAxisAngle_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_SetFromAxisAngle_float3_float in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 rotationAxis = TypeFromQScriptValue<float3>(context->argument(0));
    float rotationAngle = TypeFromQScriptValue<float>(context->argument(1));
    This->SetFromAxisAngle(rotationAxis, rotationAngle);
    return QScriptValue();
}

static QScriptValue Quat_Set_float3x3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_Set_float3x3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_Set_float3x3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3x3 matrix = TypeFromQScriptValue<float3x3>(context->argument(0));
    This->Set(matrix);
    return QScriptValue();
}

static QScriptValue Quat_Set_float3x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_Set_float3x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_Set_float3x4 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3x4 matrix = TypeFromQScriptValue<float3x4>(context->argument(0));
    This->Set(matrix);
    return QScriptValue();
}

static QScriptValue Quat_Set_float4x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_Set_float4x4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_Set_float4x4 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4x4 matrix = TypeFromQScriptValue<float4x4>(context->argument(0));
    This->Set(matrix);
    return QScriptValue();
}

static QScriptValue Quat_LookAt_float3_float3_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function Quat_LookAt_float3_float3_float3_float3 in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_LookAt_float3_float3_float3_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 localForward = TypeFromQScriptValue<float3>(context->argument(0));
    float3 targetDirection = TypeFromQScriptValue<float3>(context->argument(1));
    float3 localUp = TypeFromQScriptValue<float3>(context->argument(2));
    float3 worldUp = TypeFromQScriptValue<float3>(context->argument(3));
    This->LookAt(localForward, targetDirection, localUp, worldUp);
    return QScriptValue();
}

static QScriptValue Quat_ToEulerXYX(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_ToEulerXYX in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_ToEulerXYX in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ToEulerXYX();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_ToEulerXZX(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_ToEulerXZX in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_ToEulerXZX in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ToEulerXZX();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_ToEulerYXY(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_ToEulerYXY in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_ToEulerYXY in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ToEulerYXY();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_ToEulerYZY(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_ToEulerYZY in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_ToEulerYZY in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ToEulerYZY();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_ToEulerZXZ(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_ToEulerZXZ in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_ToEulerZXZ in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ToEulerZXZ();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_ToEulerZYZ(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_ToEulerZYZ in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_ToEulerZYZ in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ToEulerZYZ();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_ToEulerXYZ(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_ToEulerXYZ in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_ToEulerXYZ in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ToEulerXYZ();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_ToEulerXZY(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_ToEulerXZY in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_ToEulerXZY in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ToEulerXZY();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_ToEulerYXZ(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_ToEulerYXZ in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_ToEulerYXZ in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ToEulerYXZ();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_ToEulerYZX(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_ToEulerYZX in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_ToEulerYZX in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ToEulerYZX();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_ToEulerZXY(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_ToEulerZXY in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_ToEulerZXY in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ToEulerZXY();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_ToEulerZYX(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_ToEulerZYX in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_ToEulerZYX in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 ret = This->ToEulerZYX();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_ToFloat3x3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_ToFloat3x3 in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_ToFloat3x3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3x3 ret = This->ToFloat3x3();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_ToFloat3x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_ToFloat3x4 in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_ToFloat3x4 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3x4 ret = This->ToFloat3x4();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_ToFloat4x4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_ToFloat4x4 in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_ToFloat4x4 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4x4 ret = This->ToFloat4x4();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_Quat_QQuaternion(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_Quat_QQuaternion in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    QQuaternion other = TypeFromQScriptValue<QQuaternion>(context->argument(0));
    Quat ret(other);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_toString(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_toString in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This && context->argumentCount() > 0) This = TypeFromQScriptValue<Quat*>(context->argument(0)); // Qt oddity (bug?): Sometimes the built-in toString() function doesn't give us this from thisObject, but as the first argument.
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_toString in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    QString ret = This->toString();
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_Mul_float3x3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_Mul_float3x3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_Mul_float3x3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3x3 rhs = TypeFromQScriptValue<float3x3>(context->argument(0));
    Quat ret = This->Mul(rhs);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_Mul_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_Mul_float3 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_Mul_float3 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float3 vector = TypeFromQScriptValue<float3>(context->argument(0));
    float3 ret = This->Mul(vector);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_Mul_float4(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_Mul_float4 in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in function Quat_Mul_float4 in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float4 vector = TypeFromQScriptValue<float4>(context->argument(0));
    float4 ret = This->Mul(vector);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_RotateX_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_RotateX_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float angle = TypeFromQScriptValue<float>(context->argument(0));
    Quat ret = Quat::RotateX(angle);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_RotateY_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_RotateY_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float angle = TypeFromQScriptValue<float>(context->argument(0));
    Quat ret = Quat::RotateY(angle);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_RotateZ_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_RotateZ_float in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float angle = TypeFromQScriptValue<float>(context->argument(0));
    Quat ret = Quat::RotateZ(angle);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_RotateAxisAngle_float3_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Quat_RotateAxisAngle_float3_float in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 axisDirection = TypeFromQScriptValue<float3>(context->argument(0));
    float angle = TypeFromQScriptValue<float>(context->argument(1));
    Quat ret = Quat::RotateAxisAngle(axisDirection, angle);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_RotateFromTo_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) { printf("Error! Invalid number of arguments passed to function Quat_RotateFromTo_float3_float3 in file %s, line %d!\nExpected 2, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 sourceDirection = TypeFromQScriptValue<float3>(context->argument(0));
    float3 targetDirection = TypeFromQScriptValue<float3>(context->argument(1));
    Quat ret = Quat::RotateFromTo(sourceDirection, targetDirection);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_RotateFromTo_float3_float3_float3_float3(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function Quat_RotateFromTo_float3_float3_float3_float3 in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float3 sourceDirection = TypeFromQScriptValue<float3>(context->argument(0));
    float3 targetDirection = TypeFromQScriptValue<float3>(context->argument(1));
    float3 sourceDirection2 = TypeFromQScriptValue<float3>(context->argument(2));
    float3 targetDirection2 = TypeFromQScriptValue<float3>(context->argument(3));
    Quat ret = Quat::RotateFromTo(sourceDirection, targetDirection, sourceDirection2, targetDirection2);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_FromEulerXYX_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Quat_FromEulerXYX_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float x2 = TypeFromQScriptValue<float>(context->argument(0));
    float y = TypeFromQScriptValue<float>(context->argument(1));
    float x = TypeFromQScriptValue<float>(context->argument(2));
    Quat ret = Quat::FromEulerXYX(x2, y, x);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_FromEulerXZX_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Quat_FromEulerXZX_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float x2 = TypeFromQScriptValue<float>(context->argument(0));
    float z = TypeFromQScriptValue<float>(context->argument(1));
    float x = TypeFromQScriptValue<float>(context->argument(2));
    Quat ret = Quat::FromEulerXZX(x2, z, x);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_FromEulerYXY_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Quat_FromEulerYXY_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float y2 = TypeFromQScriptValue<float>(context->argument(0));
    float x = TypeFromQScriptValue<float>(context->argument(1));
    float y = TypeFromQScriptValue<float>(context->argument(2));
    Quat ret = Quat::FromEulerYXY(y2, x, y);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_FromEulerYZY_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Quat_FromEulerYZY_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float y2 = TypeFromQScriptValue<float>(context->argument(0));
    float z = TypeFromQScriptValue<float>(context->argument(1));
    float y = TypeFromQScriptValue<float>(context->argument(2));
    Quat ret = Quat::FromEulerYZY(y2, z, y);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_FromEulerZXZ_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Quat_FromEulerZXZ_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float z2 = TypeFromQScriptValue<float>(context->argument(0));
    float x = TypeFromQScriptValue<float>(context->argument(1));
    float z = TypeFromQScriptValue<float>(context->argument(2));
    Quat ret = Quat::FromEulerZXZ(z2, x, z);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_FromEulerZYZ_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Quat_FromEulerZYZ_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float z2 = TypeFromQScriptValue<float>(context->argument(0));
    float y = TypeFromQScriptValue<float>(context->argument(1));
    float z = TypeFromQScriptValue<float>(context->argument(2));
    Quat ret = Quat::FromEulerZYZ(z2, y, z);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_FromEulerXYZ_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Quat_FromEulerXYZ_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float x = TypeFromQScriptValue<float>(context->argument(0));
    float y = TypeFromQScriptValue<float>(context->argument(1));
    float z = TypeFromQScriptValue<float>(context->argument(2));
    Quat ret = Quat::FromEulerXYZ(x, y, z);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_FromEulerXZY_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Quat_FromEulerXZY_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float x = TypeFromQScriptValue<float>(context->argument(0));
    float z = TypeFromQScriptValue<float>(context->argument(1));
    float y = TypeFromQScriptValue<float>(context->argument(2));
    Quat ret = Quat::FromEulerXZY(x, z, y);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_FromEulerYXZ_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Quat_FromEulerYXZ_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float y = TypeFromQScriptValue<float>(context->argument(0));
    float x = TypeFromQScriptValue<float>(context->argument(1));
    float z = TypeFromQScriptValue<float>(context->argument(2));
    Quat ret = Quat::FromEulerYXZ(y, x, z);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_FromEulerYZX_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Quat_FromEulerYZX_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float y = TypeFromQScriptValue<float>(context->argument(0));
    float z = TypeFromQScriptValue<float>(context->argument(1));
    float x = TypeFromQScriptValue<float>(context->argument(2));
    Quat ret = Quat::FromEulerYZX(y, z, x);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_FromEulerZXY_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Quat_FromEulerZXY_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float z = TypeFromQScriptValue<float>(context->argument(0));
    float x = TypeFromQScriptValue<float>(context->argument(1));
    float y = TypeFromQScriptValue<float>(context->argument(2));
    Quat ret = Quat::FromEulerZXY(z, x, y);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_FromEulerZYX_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Quat_FromEulerZYX_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    float z = TypeFromQScriptValue<float>(context->argument(0));
    float y = TypeFromQScriptValue<float>(context->argument(1));
    float x = TypeFromQScriptValue<float>(context->argument(2));
    Quat ret = Quat::FromEulerZYX(z, y, x);
    return TypeToQScriptValue(engine, ret);
}

static QScriptValue Quat_x_get(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_x_get in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    return qScriptValueFromValue(context->engine(), This->x);
}

static QScriptValue Quat_x_set(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_x_set in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float x = qscriptvalue_cast<float>(context->argument(0));
    This->x = x;
    return QScriptValue();
}

static QScriptValue Quat_y_get(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_y_get in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    return qScriptValueFromValue(context->engine(), This->y);
}

static QScriptValue Quat_y_set(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_y_set in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float y = qscriptvalue_cast<float>(context->argument(0));
    This->y = y;
    return QScriptValue();
}

static QScriptValue Quat_z_get(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_z_get in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    return qScriptValueFromValue(context->engine(), This->z);
}

static QScriptValue Quat_z_set(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_z_set in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float z = qscriptvalue_cast<float>(context->argument(0));
    This->z = z;
    return QScriptValue();
}

static QScriptValue Quat_w_get(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Quat_w_get in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    return qScriptValueFromValue(context->engine(), This->w);
}

static QScriptValue Quat_w_set(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Quat_w_set in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); return QScriptValue(); }
    Quat *This = TypeFromQScriptValue<Quat*>(context->thisObject());
    if (!This) { printf("Error! Invalid context->thisObject in file %s, line %d\n!", __FILE__, __LINE__); return QScriptValue(); }
    float w = qscriptvalue_cast<float>(context->argument(0));
    This->w = w;
    return QScriptValue();
}

static QScriptValue Quat_ctor(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return Quat_Quat(context, engine);
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
    printf("Quat_ctor failed to choose the right function to call! Did you use 'var x = Quat();' instead of 'var x = new Quat();'?\n"); return QScriptValue();
}

static QScriptValue Quat_Transform_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 3 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return Quat_Transform_float_float_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return Quat_Transform_float3(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float4>(context->argument(0)))
        return Quat_Transform_float4(context, engine);
    printf("Quat_Transform_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue Quat_Set_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3x3>(context->argument(0)))
        return Quat_Set_float3x3(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3x4>(context->argument(0)))
        return Quat_Set_float3x4(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float4x4>(context->argument(0)))
        return Quat_Set_float4x4(context, engine);
    printf("Quat_Set_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue Quat_Mul_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 1 && QSVIsOfType<float3x3>(context->argument(0)))
        return Quat_Mul_float3x3(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float3>(context->argument(0)))
        return Quat_Mul_float3(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<float4>(context->argument(0)))
        return Quat_Mul_float4(context, engine);
    printf("Quat_Mul_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

static QScriptValue Quat_RotateFromTo_selector(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 2 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)))
        return Quat_RotateFromTo_float3_float3(context, engine);
    if (context->argumentCount() == 4 && QSVIsOfType<float3>(context->argument(0)) && QSVIsOfType<float3>(context->argument(1)) && QSVIsOfType<float3>(context->argument(2)) && QSVIsOfType<float3>(context->argument(3)))
        return Quat_RotateFromTo_float3_float3_float3_float3(context, engine);
    printf("Quat_RotateFromTo_selector failed to choose the right function to call in file %s, line %d!\n", __FILE__, __LINE__); return QScriptValue();
}

class Quat_scriptclass : public QScriptClass
{
public:
    QScriptValue objectPrototype;
    Quat_scriptclass(QScriptEngine *engine):QScriptClass(engine){}
    QScriptValue property(const QScriptValue &object, const QScriptString &name, uint id)
    {
        Quat *This = TypeFromQScriptValue<Quat*>(object);
        if (!This) { printf("Error! Cannot convert QScriptValue to type Quat in file %s, line %d!\nTry using Quat.get%s() and Quat.set%s() to query the member variable '%s'!\n", __FILE__, __LINE__, Capitalize((QString)name).c_str(), Capitalize((QString)name).c_str(), ((QString)name).toStdString().c_str()); return QScriptValue(); }
        if ((QString)name == (QString)"x") return TypeToQScriptValue(engine(), This->x);
        if ((QString)name == (QString)"y") return TypeToQScriptValue(engine(), This->y);
        if ((QString)name == (QString)"z") return TypeToQScriptValue(engine(), This->z);
        if ((QString)name == (QString)"w") return TypeToQScriptValue(engine(), This->w);
        if ((QString)name == (QString)"identity") return TypeToQScriptValue(engine(), This->identity);
        return QScriptValue();
    }
    void setProperty(QScriptValue &object, const QScriptString &name, uint id, const QScriptValue &value)
    {
        Quat *This = TypeFromQScriptValue<Quat*>(object);
        if (!This) { printf("Error! Cannot convert QScriptValue to type Quat in file %s, line %d!\nTry using Quat.get%s() and Quat.set%s() to query the member variable '%s'!\n", __FILE__, __LINE__, Capitalize((QString)name).c_str(), Capitalize((QString)name).c_str(), ((QString)name).toStdString().c_str()); return; }
        if ((QString)name == (QString)"x") This->x = TypeFromQScriptValue<float>(value);
        if ((QString)name == (QString)"y") This->y = TypeFromQScriptValue<float>(value);
        if ((QString)name == (QString)"z") This->z = TypeFromQScriptValue<float>(value);
        if ((QString)name == (QString)"w") This->w = TypeFromQScriptValue<float>(value);
    }
    QueryFlags queryProperty(const QScriptValue &object, const QScriptString &name, QueryFlags flags, uint *id)
    {
        if ((QString)name == (QString)"x") return flags;
        if ((QString)name == (QString)"y") return flags;
        if ((QString)name == (QString)"z") return flags;
        if ((QString)name == (QString)"w") return flags;
        if ((QString)name == (QString)"identity") return flags;
        return 0;
    }
    QScriptValue prototype() const { return objectPrototype; }
};
QScriptValue register_Quat_prototype(QScriptEngine *engine)
{
    engine->setDefaultPrototype(qMetaTypeId<Quat*>(), QScriptValue());
    QScriptValue proto = engine->newVariant(qVariantFromValue((Quat*)0));
    proto.setProperty("WorldX", engine->newFunction(Quat_WorldX, 0));
    proto.setProperty("WorldY", engine->newFunction(Quat_WorldY, 0));
    proto.setProperty("WorldZ", engine->newFunction(Quat_WorldZ, 0));
    proto.setProperty("Axis", engine->newFunction(Quat_Axis, 0));
    proto.setProperty("Angle", engine->newFunction(Quat_Angle, 0));
    proto.setProperty("Dot", engine->newFunction(Quat_Dot_Quat, 1));
    proto.setProperty("LengthSq", engine->newFunction(Quat_LengthSq, 0));
    proto.setProperty("Length", engine->newFunction(Quat_Length, 0));
    proto.setProperty("Normalize", engine->newFunction(Quat_Normalize, 0));
    proto.setProperty("Normalized", engine->newFunction(Quat_Normalized, 0));
    proto.setProperty("IsNormalized", engine->newFunction(Quat_IsNormalized_float, 1));
    proto.setProperty("IsInvertible", engine->newFunction(Quat_IsInvertible_float, 1));
    proto.setProperty("IsFinite", engine->newFunction(Quat_IsFinite, 0));
    proto.setProperty("Inverse", engine->newFunction(Quat_Inverse, 0));
    proto.setProperty("Inverted", engine->newFunction(Quat_Inverted, 0));
    proto.setProperty("InverseAndNormalize", engine->newFunction(Quat_InverseAndNormalize, 0));
    proto.setProperty("Conjugate", engine->newFunction(Quat_Conjugate, 0));
    proto.setProperty("Conjugated", engine->newFunction(Quat_Conjugated, 0));
    proto.setProperty("Transform", engine->newFunction(Quat_Transform_selector, 3));
    proto.setProperty("Transform", engine->newFunction(Quat_Transform_selector, 1));
    proto.setProperty("Lerp", engine->newFunction(Quat_Lerp_Quat_float, 2));
    proto.setProperty("Slerp", engine->newFunction(Quat_Slerp_Quat_float, 2));
    proto.setProperty("AngleBetween", engine->newFunction(Quat_AngleBetween_Quat, 1));
    proto.setProperty("AxisFromTo", engine->newFunction(Quat_AxisFromTo_Quat, 1));
    proto.setProperty("ToAxisAngle", engine->newFunction(Quat_ToAxisAngle_float3_float, 2));
    proto.setProperty("SetFromAxisAngle", engine->newFunction(Quat_SetFromAxisAngle_float3_float, 2));
    proto.setProperty("Set", engine->newFunction(Quat_Set_selector, 1));
    proto.setProperty("LookAt", engine->newFunction(Quat_LookAt_float3_float3_float3_float3, 4));
    proto.setProperty("ToEulerXYX", engine->newFunction(Quat_ToEulerXYX, 0));
    proto.setProperty("ToEulerXZX", engine->newFunction(Quat_ToEulerXZX, 0));
    proto.setProperty("ToEulerYXY", engine->newFunction(Quat_ToEulerYXY, 0));
    proto.setProperty("ToEulerYZY", engine->newFunction(Quat_ToEulerYZY, 0));
    proto.setProperty("ToEulerZXZ", engine->newFunction(Quat_ToEulerZXZ, 0));
    proto.setProperty("ToEulerZYZ", engine->newFunction(Quat_ToEulerZYZ, 0));
    proto.setProperty("ToEulerXYZ", engine->newFunction(Quat_ToEulerXYZ, 0));
    proto.setProperty("ToEulerXZY", engine->newFunction(Quat_ToEulerXZY, 0));
    proto.setProperty("ToEulerYXZ", engine->newFunction(Quat_ToEulerYXZ, 0));
    proto.setProperty("ToEulerYZX", engine->newFunction(Quat_ToEulerYZX, 0));
    proto.setProperty("ToEulerZXY", engine->newFunction(Quat_ToEulerZXY, 0));
    proto.setProperty("ToEulerZYX", engine->newFunction(Quat_ToEulerZYX, 0));
    proto.setProperty("ToFloat3x3", engine->newFunction(Quat_ToFloat3x3, 0));
    proto.setProperty("ToFloat3x4", engine->newFunction(Quat_ToFloat3x4, 0));
    proto.setProperty("ToFloat4x4", engine->newFunction(Quat_ToFloat4x4, 0));
    proto.setProperty("toString", engine->newFunction(Quat_toString, 0));
    proto.setProperty("Mul", engine->newFunction(Quat_Mul_selector, 1));
    proto.setProperty("getX", engine->newFunction(Quat_x_get, 1));
    proto.setProperty("setX", engine->newFunction(Quat_x_set, 1));
    proto.setProperty("getY", engine->newFunction(Quat_y_get, 1));
    proto.setProperty("setY", engine->newFunction(Quat_y_set, 1));
    proto.setProperty("getZ", engine->newFunction(Quat_z_get, 1));
    proto.setProperty("setZ", engine->newFunction(Quat_z_set, 1));
    proto.setProperty("getW", engine->newFunction(Quat_w_get, 1));
    proto.setProperty("setW", engine->newFunction(Quat_w_set, 1));
    Quat_scriptclass *sc = new Quat_scriptclass(engine);
    engine->setProperty("Quat_scriptclass", QVariant::fromValue<QScriptClass*>(sc));
    proto.setScriptClass(sc);
    sc->objectPrototype = proto;
    engine->setDefaultPrototype(qMetaTypeId<Quat>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<Quat*>(), proto);
    QScriptValue ctor = engine->newFunction(Quat_ctor, proto, 4);
    ctor.setProperty("RotateX", engine->newFunction(Quat_RotateX_float, 1));
    ctor.setProperty("RotateY", engine->newFunction(Quat_RotateY_float, 1));
    ctor.setProperty("RotateZ", engine->newFunction(Quat_RotateZ_float, 1));
    ctor.setProperty("RotateAxisAngle", engine->newFunction(Quat_RotateAxisAngle_float3_float, 2));
    ctor.setProperty("RotateFromTo", engine->newFunction(Quat_RotateFromTo_selector, 2));
    ctor.setProperty("RotateFromTo", engine->newFunction(Quat_RotateFromTo_selector, 4));
    ctor.setProperty("FromEulerXYX", engine->newFunction(Quat_FromEulerXYX_float_float_float, 3));
    ctor.setProperty("FromEulerXZX", engine->newFunction(Quat_FromEulerXZX_float_float_float, 3));
    ctor.setProperty("FromEulerYXY", engine->newFunction(Quat_FromEulerYXY_float_float_float, 3));
    ctor.setProperty("FromEulerYZY", engine->newFunction(Quat_FromEulerYZY_float_float_float, 3));
    ctor.setProperty("FromEulerZXZ", engine->newFunction(Quat_FromEulerZXZ_float_float_float, 3));
    ctor.setProperty("FromEulerZYZ", engine->newFunction(Quat_FromEulerZYZ_float_float_float, 3));
    ctor.setProperty("FromEulerXYZ", engine->newFunction(Quat_FromEulerXYZ_float_float_float, 3));
    ctor.setProperty("FromEulerXZY", engine->newFunction(Quat_FromEulerXZY_float_float_float, 3));
    ctor.setProperty("FromEulerYXZ", engine->newFunction(Quat_FromEulerYXZ_float_float_float, 3));
    ctor.setProperty("FromEulerYZX", engine->newFunction(Quat_FromEulerYZX_float_float_float, 3));
    ctor.setProperty("FromEulerZXY", engine->newFunction(Quat_FromEulerZXY_float_float_float, 3));
    ctor.setProperty("FromEulerZYX", engine->newFunction(Quat_FromEulerZYX_float_float_float, 3));
    ctor.setProperty("identity", TypeToQScriptValue(engine, Quat::identity));
    engine->globalObject().setProperty("Quat", ctor);
    return ctor;
}

