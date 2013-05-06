#include "QtScriptBindingsHelpers.h"
#include "Color.h"

void ToExistingScriptValue_Color(QScriptEngine *engine, const Color &value, QScriptValue obj)
{
    obj.setProperty("r", qScriptValueFromValue(engine, value.r), QScriptValue::Undeletable);
    obj.setProperty("g", qScriptValueFromValue(engine, value.g), QScriptValue::Undeletable);
    obj.setProperty("b", qScriptValueFromValue(engine, value.b), QScriptValue::Undeletable);
    obj.setProperty("a", qScriptValueFromValue(engine, value.a), QScriptValue::Undeletable);
}

static QScriptValue Color_Color(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Color_Color in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Color ret;
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Color_Color_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 3) { printf("Error! Invalid number of arguments passed to function Color_Color_float_float_float in file %s, line %d!\nExpected 3, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float nr = qscriptvalue_cast<float>(context->argument(0));
    float ng = qscriptvalue_cast<float>(context->argument(1));
    float nb = qscriptvalue_cast<float>(context->argument(2));
    Color ret(nr, ng, nb);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Color_Color_float_float_float_float(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 4) { printf("Error! Invalid number of arguments passed to function Color_Color_float_float_float_float in file %s, line %d!\nExpected 4, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    float nr = qscriptvalue_cast<float>(context->argument(0));
    float ng = qscriptvalue_cast<float>(context->argument(1));
    float nb = qscriptvalue_cast<float>(context->argument(2));
    float na = qscriptvalue_cast<float>(context->argument(3));
    Color ret(nr, ng, nb, na);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Color_ToFloat4_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Color_ToFloat4_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Color This = qscriptvalue_cast<Color>(context->thisObject());
    float4 ret = This.ToFloat4();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Color_Color_QColor(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Color_Color_QColor in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    QColor other = qscriptvalue_cast<QColor>(context->argument(0));
    Color ret(other);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Color_ToQColor_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Color_ToQColor_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Color This = qscriptvalue_cast<Color>(context->thisObject());
    QColor ret = This.ToQColor();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Color_ToString_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Color_ToString_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Color This = qscriptvalue_cast<Color>(context->thisObject());
    QString ret = This.ToString();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Color_toString_const(QScriptContext *context, QScriptEngine *engine)
{
    Color This;
    if (context->argumentCount() > 0) This = qscriptvalue_cast<Color>(context->argument(0)); // Qt oddity (bug?): Sometimes the built-in toString() function doesn't give us this from thisObject, but as the first argument.
    else This = qscriptvalue_cast<Color>(context->thisObject());
    QString ret = This.toString();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Color_SerializeToString_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function Color_SerializeToString_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    Color This = qscriptvalue_cast<Color>(context->thisObject());
    QString ret = This.SerializeToString();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Color_FromString_QString(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) { printf("Error! Invalid number of arguments passed to function Color_FromString_QString in file %s, line %d!\nExpected 1, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    QString str = qscriptvalue_cast<QString>(context->argument(0));
    Color ret = Color::FromString(str);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue Color_ctor(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return Color_Color(context, engine);
    if (context->argumentCount() == 3 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)))
        return Color_Color_float_float_float(context, engine);
    if (context->argumentCount() == 4 && QSVIsOfType<float>(context->argument(0)) && QSVIsOfType<float>(context->argument(1)) && QSVIsOfType<float>(context->argument(2)) && QSVIsOfType<float>(context->argument(3)))
        return Color_Color_float_float_float_float(context, engine);
    if (context->argumentCount() == 1 && QSVIsOfType<QColor>(context->argument(0)))
        return Color_Color_QColor(context, engine);
    printf("Color_ctor failed to choose the right function to call! Did you use 'var x = Color();' instead of 'var x = new Color();'?\n"); PrintCallStack(context->backtrace()); return QScriptValue();
}

void FromScriptValue_Color(const QScriptValue &obj, Color &value)
{
    value.r = qScriptValueToValue<float>(obj.property("r"));
    value.g = qScriptValueToValue<float>(obj.property("g"));
    value.b = qScriptValueToValue<float>(obj.property("b"));
    value.a = qScriptValueToValue<float>(obj.property("a"));
}

QScriptValue ToScriptValue_Color(QScriptEngine *engine, const Color &value)
{
    QScriptValue obj = engine->newVariant(QVariant::fromValue(value)); // The contents of this variant are NOT used. The real data lies in the data() pointer of this QScriptValue. This only exists to enable overload resolution to work for QObject slots.
    ToExistingScriptValue_Color(engine, value, obj);
    return obj;
}

QScriptValue ToScriptValue_const_Color(QScriptEngine *engine, const Color &value)
{
    QScriptValue obj = engine->newVariant(QVariant::fromValue(value)); // The contents of this variant are NOT used. The real data lies in the data() pointer of this QScriptValue. This only exists to enable overload resolution to work for QObject slots.
    obj.setPrototype(engine->defaultPrototype(qMetaTypeId<Color>()));
    obj.setProperty("r", qScriptValueFromValue(engine, value.r), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("g", qScriptValueFromValue(engine, value.g), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("b", qScriptValueFromValue(engine, value.b), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("a", qScriptValueFromValue(engine, value.a), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    return obj;
}

QScriptValue register_Color_prototype(QScriptEngine *engine)
{
    QScriptValue proto = engine->newObject();
    proto.setProperty("ToFloat4", engine->newFunction(Color_ToFloat4_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToQColor", engine->newFunction(Color_ToQColor_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("ToString", engine->newFunction(Color_ToString_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("toString", engine->newFunction(Color_toString_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("SerializeToString", engine->newFunction(Color_SerializeToString_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("metaTypeId", engine->toScriptValue<qint32>((qint32)qMetaTypeId<Color>()));
    engine->setDefaultPrototype(qMetaTypeId<Color>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<Color*>(), proto);
    qScriptRegisterMetaType(engine, ToScriptValue_Color, FromScriptValue_Color, proto);

    QScriptValue ctor = engine->newFunction(Color_ctor, proto, 4);
    ctor.setProperty("FromString", engine->newFunction(Color_FromString_QString, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("Red", qScriptValueFromValue(engine, Color::Red), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("Green", qScriptValueFromValue(engine, Color::Green), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("Blue", qScriptValueFromValue(engine, Color::Blue), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("White", qScriptValueFromValue(engine, Color::White), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("Black", qScriptValueFromValue(engine, Color::Black), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("Yellow", qScriptValueFromValue(engine, Color::Yellow), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("Cyan", qScriptValueFromValue(engine, Color::Cyan), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("Magenta", qScriptValueFromValue(engine, Color::Magenta), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    ctor.setProperty("Gray", qScriptValueFromValue(engine, Color::Gray), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    engine->globalObject().setProperty("Color", ctor, QScriptValue::Undeletable | QScriptValue::ReadOnly);

    return ctor;
}

