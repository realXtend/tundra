/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   ScriptMetaTypeDefines.h
    @brief  Registration of the Core API objects to QtScript. */

#pragma once

#include "CoreTypes.h"
#include <QtScript>
#include <QDebug>

template<typename T> 
QScriptValue toScriptValueEnum(QScriptEngine *engine, const T &s)
{
    QScriptValue obj = engine->newObject();
    obj.setProperty("value", QScriptValue(engine, s));
    return obj;
}

template<typename T> 
void fromScriptValueEnum(const QScriptValue &obj, T &s)
{
    s = static_cast<T>(obj.property("value").toInt32());
}

template<typename T> 
QScriptValue toScriptS32OrSmaller(QScriptEngine * /*engine*/, const T &num)
{
    return QScriptValue(static_cast<qint32>(num));
}

template<typename T> 
QScriptValue toScriptU32OrSmaller(QScriptEngine * /*engine*/, const T &num)
{
    return QScriptValue(static_cast<quint32>(num));
}

template<typename T> 
QScriptValue toScriptS64(QScriptEngine *engine, const T &num)
{
    /// @note We could use QScriptValue(static_cast<qreal>(num)); to have proper 'number' type in JS.
    /// But we will generate s64 and u64 with 'object' type for consistency. As it seems that u64 cannot be converted at all with 'number' type.
    return engine->newVariant(QVariant::fromValue<qint64>(num));
}

template<typename T> 
QScriptValue toScriptU64(QScriptEngine *engine, const T &num)
{
    /// @note This will generate a object with type 'object', it seems there is no way to make u64 with 'number' type via QScriptValue ctors directly.
    return engine->newVariant(QVariant::fromValue<quint64>(num));
}

template<typename T> 
void fromScriptInt(const QScriptValue &obj, T &s)
{
    s = obj.toInt32();
}

template<typename T> 
void fromScriptUInt(const QScriptValue &obj, T &s)
{
    s = obj.toUInt32();
}

template<typename T> 
void fromScriptChar(const QScriptValue &obj, T &s)
{
    s = static_cast<s8>(obj.toInt32()); // There is no .toInt8()
}

template<typename T> 
void fromScriptUChar(const QScriptValue &obj, T &s)
{
    s = static_cast<u8>(obj.toUInt32()); // There is no .toUInt8()
}

template<typename T> 
void fromScriptShort(const QScriptValue &obj, T &s)
{
    s = static_cast<s16>(obj.toInt32()); // There is no .toInt16()
}

template<typename T> 
void fromScriptUShort(const QScriptValue &obj, T &s)
{
    s = obj.toUInt16();
}

template<typename T> 
void fromScriptLongLong(const QScriptValue &obj, T &s)
{
    bool ok = false;
    const QVariant temp = obj.toVariant();
    s = temp.toLongLong(&ok);
    if (!ok) s = 0;
}

template<typename T> 
void fromScriptULongLong(const QScriptValue &obj, T &s)
{
    bool ok = false;
    const QVariant temp = obj.toVariant();
    s = temp.toULongLong(&ok);
    if (!ok) s = 0;
}

/// Register Qt related stuff to QScriptEngine.
/// @todo replace this with qscriptgenerator.
void ExposeQtMetaTypes(QScriptEngine *engine);

/// Will register all meta data types that are needed to use the Core API objects.
void ExposeCoreApiMetaTypes(QScriptEngine *engine);

/// QtScript <-> C++ integer typedef interop tester
class IntegerTestRunner : public QObject
{
    Q_OBJECT
    /* C++ -> JS properties */
    Q_PROPERTY(size_t propSizeT READ getSizeT)
    Q_PROPERTY(entity_id_t propEntityId READ getEntityId)
    Q_PROPERTY(component_id_t propComponentId READ getComponentId)
    Q_PROPERTY(s8 propS8 READ getS8)
    Q_PROPERTY(u8 propU8 READ getU8)
    Q_PROPERTY(s16 propS16 READ getS16)
    Q_PROPERTY(u16 propU16 READ getU16)
    Q_PROPERTY(s32 propS32 READ getS32)
    Q_PROPERTY(u32 propU32 READ getU32)
    Q_PROPERTY(s64 propS64 READ getS64)
    Q_PROPERTY(u64 propU64 READ getU64)

public:
    IntegerTestRunner()
    {
        qDebug() << "IntegerTestRunner ctor";
    }

    ~IntegerTestRunner()
    {
        qDebug() << "IntegerTestRunner dtor";
    }

public slots:
    /* C++ -> JS */

    size_t getSizeT() const
    {
        size_t test = 1;
        qDebug() << "IntegerTestRunner::getSizeT: C++ returning size_t " << test;
        return test;
    }

    entity_id_t getEntityId() const
    {
        entity_id_t test = 1;
        qDebug() << "IntegerTestRunner::getEntityId: C++ returning entity_id_t " << test;
        return test;
    }

    component_id_t getComponentId() const
    {
        component_id_t test = 2;
        qDebug() << "IntegerTestRunner::getComponentId: C++ returning component_id_t " << test;
        return test;
    }

    s8 getS8() const
    {
        s8 test = -15;
        qDebug() << "IntegerTestRunner::getS8: C++ returning s8 " << test;
        return test;
    }

    u8 getU8() const
    {
        u8 test = 235;
        qDebug() << "IntegerTestRunner::getU8: C++ returning u8 " << test;
        return test;
    }

    s16 getS16() const
    {
        s16 test = -20100;
        qDebug() << "IntegerTestRunner::getS16: C++ returning s16" << test;
        return test;
    }

    u16 getU16() const
    {
        u16 test = 65200;
        qDebug() << "IntegerTestRunner::getU16: C++ returning u16" << test;
        return test;
    }

    s32 getS32() const
    {
        s32 test = -210001500;
        qDebug() << "IntegerTestRunner::getS32: C++ returning s32" << test;
        return test;
    }

    u32 getU32() const
    {
        u32 test = 4000500500;
        qDebug() << "IntegerTestRunner::getU32: C++ returning u32" << test;
        return test;
    }

    s64 getS64() const
    {
        s64 test = -9100000000500000000;
        qDebug() << "IntegerTestRunner::getS64: C++ returning s64" << test;
        return test;
    }

    u64 getU64() const
    {
        u64 test = 15000000500000000000;
        qDebug() << "IntegerTestRunner::getU64: C++ returning u64" << test;
        return test;
    }

    /* JS -> C++ */
    void setSizeT(size_t test) const
    {
        qDebug() << "IntegerTestRunner::setSizeT: C++ received size_t " << test;
        emit onSizeT(test);
    }

    void setEntityId(entity_id_t test) const
    {
        qDebug() << "IntegerTestRunner::setEntityId: C++ received entity_id_t " << test;
        emit onEnitityId(test);
    }

    void setComponentId(component_id_t test) const
    {
        qDebug() << "IntegerTestRunner::setComponentId : C++ received component_id_t " << test;
        emit onComponentId(test);
    }

    void setS8(s8 test) const
    {
        qDebug() << "IntegerTestRunner::setS8: C++ received s8 " << test;
        emit onS8(test);
    }

    void setU8(u8 test) const
    {
        qDebug() << "IntegerTestRunner::setU8: C++ received u8 " << test;
        emit onU8(test);
    }

    void setS16(s16 test) const
    {
        qDebug() << "IntegerTestRunner::setS16: C++ received s16" << test;
        emit onS16(test);
    }

    void setU16(u16 test) const
    {
        qDebug() << "IntegerTestRunner::getU16: C++ received u16" << test;
        emit onU16(test);
    }

    void setS32(s32 test) const
    {
        qDebug() << "IntegerTestRunner::getS32: C++ received s32" << test;
        emit onS32(test);
    }

    void setU32(u32 test) const
    {
        qDebug() << "IntegerTestRunner::setU32: C++ received u32" << test;
        emit onU32(test);
    }

    void setS64(s64 test) const
    {
        qDebug() << "IntegerTestRunner::setS64: C++ received s64" << test;
        emit onS64(test);
    }

    void setU64(u64 test) const
    {
        qDebug() << "IntegerTestRunner::setU64: C++ received u64" << test;
        emit onU64(test);
    }

signals:
    /* C++ -> JS signals */
    void onSizeT(size_t test) const;
    void onEnitityId(entity_id_t test) const;
    void onComponentId(component_id_t test) const;
    void onS8(s8 test) const;
    void onU8(u8 test) const;
    void onS16(s16 test) const;
    void onU16(u16 test) const;
    void onS32(s32 test) const;
    void onU32(u32 test) const;
    void onS64(s64 test) const;
    void onU64(u64 test) const;
};
