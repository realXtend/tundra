#include "QtScriptBindingsHelpers.h"
#include "ConfigAPI.h"

void ToExistingScriptValue_ConfigData(QScriptEngine *engine, const ConfigData &value, QScriptValue obj)
{
    obj.setProperty("file", qScriptValueFromValue(engine, value.file), QScriptValue::Undeletable);
    obj.setProperty("section", qScriptValueFromValue(engine, value.section), QScriptValue::Undeletable);
    obj.setProperty("key", qScriptValueFromValue(engine, value.key), QScriptValue::Undeletable);
    obj.setProperty("value", qScriptValueFromValue(engine, value.value), QScriptValue::Undeletable);
    obj.setProperty("defaultValue", qScriptValueFromValue(engine, value.defaultValue), QScriptValue::Undeletable);
}

static QScriptValue ConfigData_ConfigData(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function ConfigData_ConfigData in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    ConfigData ret;
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue ConfigData_ConfigData_QString_QString_QString_QVariant_QVariant(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 5) { printf("Error! Invalid number of arguments passed to function ConfigData_ConfigData_QString_QString_QString_QVariant_QVariant in file %s, line %d!\nExpected 5, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    QString cfgFile = qscriptvalue_cast<QString>(context->argument(0));
    QString cfgSection = qscriptvalue_cast<QString>(context->argument(1));
    QString cfgKey = qscriptvalue_cast<QString>(context->argument(2));
    QVariant cfgValue = qscriptvalue_cast<QVariant>(context->argument(3));
    QVariant cfgDefaultValue = qscriptvalue_cast<QVariant>(context->argument(4));
    ConfigData ret(cfgFile, cfgSection, cfgKey, cfgValue, cfgDefaultValue);
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue ConfigData_ToString_const(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0) { printf("Error! Invalid number of arguments passed to function ConfigData_ToString_const in file %s, line %d!\nExpected 0, but got %d!\n", __FILE__, __LINE__, context->argumentCount()); PrintCallStack(context->backtrace()); return QScriptValue(); }
    ConfigData This = qscriptvalue_cast<ConfigData>(context->thisObject());
    QString ret = This.ToString();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue ConfigData_toString_const(QScriptContext *context, QScriptEngine *engine)
{
    ConfigData This;
    if (context->argumentCount() > 0) This = qscriptvalue_cast<ConfigData>(context->argument(0)); // Qt oddity (bug?): Sometimes the built-in toString() function doesn't give us this from thisObject, but as the first argument.
    else This = qscriptvalue_cast<ConfigData>(context->thisObject());
    QString ret = This.toString();
    return qScriptValueFromValue(engine, ret);
}

static QScriptValue ConfigData_ctor(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return ConfigData_ConfigData(context, engine);
    if (context->argumentCount() == 5 && QSVIsOfType<QString>(context->argument(0)) && QSVIsOfType<QString>(context->argument(1)) && QSVIsOfType<QString>(context->argument(2)) && QSVIsOfType<QVariant>(context->argument(3)) && QSVIsOfType<QVariant>(context->argument(4)))
        return ConfigData_ConfigData_QString_QString_QString_QVariant_QVariant(context, engine);
    printf("ConfigData_ctor failed to choose the right function to call! Did you use 'var x = ConfigData();' instead of 'var x = new ConfigData();'?\n"); PrintCallStack(context->backtrace()); return QScriptValue();
}

void FromScriptValue_ConfigData(const QScriptValue &obj, ConfigData &value)
{
    value.file = qScriptValueToValue<QString>(obj.property("file"));
    value.section = qScriptValueToValue<QString>(obj.property("section"));
    value.key = qScriptValueToValue<QString>(obj.property("key"));
    value.value = qScriptValueToValue<QVariant>(obj.property("value"));
    value.defaultValue = qScriptValueToValue<QVariant>(obj.property("defaultValue"));
}

QScriptValue ToScriptValue_ConfigData(QScriptEngine *engine, const ConfigData &value)
{
    QScriptValue obj = engine->newVariant(QVariant::fromValue(value)); // The contents of this variant are NOT used. The real data lies in the data() pointer of this QScriptValue. This only exists to enable overload resolution to work for QObject slots.
    ToExistingScriptValue_ConfigData(engine, value, obj);
    return obj;
}

QScriptValue ToScriptValue_const_ConfigData(QScriptEngine *engine, const ConfigData &value)
{
    QScriptValue obj = engine->newVariant(QVariant::fromValue(value)); // The contents of this variant are NOT used. The real data lies in the data() pointer of this QScriptValue. This only exists to enable overload resolution to work for QObject slots.
    obj.setPrototype(engine->defaultPrototype(qMetaTypeId<ConfigData>()));
    obj.setProperty("file", qScriptValueFromValue(engine, value.file), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("section", qScriptValueFromValue(engine, value.section), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("key", qScriptValueFromValue(engine, value.key), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("value", qScriptValueFromValue(engine, value.value), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    obj.setProperty("defaultValue", qScriptValueFromValue(engine, value.defaultValue), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    return obj;
}

QScriptValue register_ConfigData_prototype(QScriptEngine *engine)
{
    QScriptValue proto = engine->newObject();
    proto.setProperty("ToString", engine->newFunction(ConfigData_ToString_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("toString", engine->newFunction(ConfigData_toString_const, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    proto.setProperty("metaTypeId", engine->toScriptValue<qint32>((qint32)qMetaTypeId<ConfigData>()));
    engine->setDefaultPrototype(qMetaTypeId<ConfigData>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<ConfigData*>(), proto);
    qScriptRegisterMetaType(engine, ToScriptValue_ConfigData, FromScriptValue_ConfigData, proto);

    QScriptValue ctor = engine->newFunction(ConfigData_ctor, proto, 5);
    engine->globalObject().setProperty("ConfigData", ctor, QScriptValue::Undeletable | QScriptValue::ReadOnly);

    return ctor;
}

