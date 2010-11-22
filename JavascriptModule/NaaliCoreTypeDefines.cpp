#include "StableHeaders.h"
#include "Color.h"
#include "Quaternion.h"
#include "Transform.h"
#include "Vector3D.h"
#include "Matrix4.h"
#include "IAttribute.h"

#include <QScriptEngine>
#include <QColor>
#include <QVector3D>
#include <QQuaternion>

#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("JavaScriptEngine")

Q_DECLARE_METATYPE(IAttribute*);

QScriptValue toScriptValueColor(QScriptEngine *engine, const Color &s)
{
    QScriptValue obj = engine->newObject();
    obj.setProperty("r", QScriptValue(engine, s.r));
    obj.setProperty("g", QScriptValue(engine, s.g));
    obj.setProperty("b", QScriptValue(engine, s.b));
    obj.setProperty("a", QScriptValue(engine, s.a));
    return obj;
}
 
void fromScriptValueColor(const QScriptValue &obj, Color &s)
{
    s.r = (float)obj.property("r").toNumber();
    s.g = (float)obj.property("g").toNumber();
    s.b = (float)obj.property("b").toNumber();
    s.a = (float)obj.property("a").toNumber();
}

QScriptValue toScriptValueQColor(QScriptEngine *engine, const QColor &s)
{
    QScriptValue obj = engine->newObject();
    obj.setProperty("r", QScriptValue(engine, s.red()));
    obj.setProperty("g", QScriptValue(engine, s.green()));
    obj.setProperty("b", QScriptValue(engine, s.blue()));
    obj.setProperty("a", QScriptValue(engine, s.alpha()));
    return obj;
}
 
void fromScriptValueQColor(const QScriptValue &obj, QColor &s)
{
    s.setRed((float)obj.property("r").toNumber());
    s.setGreen((float)obj.property("g").toNumber());
    s.setBlue((float)obj.property("b").toNumber());
    s.setAlpha((float)obj.property("a").toNumber());
}


QScriptValue toScriptValueVector3(QScriptEngine *engine, const Vector3df &s)
{
    QScriptValue obj = engine->newObject();
    obj.setProperty("x", QScriptValue(engine, s.x));
    obj.setProperty("y", QScriptValue(engine, s.y));
    obj.setProperty("z", QScriptValue(engine, s.z));
    return obj;
}
 
void fromScriptValueVector3(const QScriptValue &obj, Vector3df &s)
{
    s.x = (float)obj.property("x").toNumber();
    s.y = (float)obj.property("y").toNumber();
    s.z = (float)obj.property("z").toNumber();
}

QScriptValue toScriptValueQVector3D(QScriptEngine *engine, const QVector3D &s)
{
    QScriptValue obj = engine->newObject();
    obj.setProperty("x", QScriptValue(engine, s.x()));
    obj.setProperty("y", QScriptValue(engine, s.y()));
    obj.setProperty("z", QScriptValue(engine, s.z()));
    return obj;
}
 
void fromScriptValueQVector3D(const QScriptValue &obj, QVector3D &s)
{
    s.setX((float)obj.property("x").toNumber());
    s.setY((float)obj.property("y").toNumber());
    s.setZ((float)obj.property("z").toNumber());
}


QScriptValue toScriptValueQuaternion(QScriptEngine *engine, const Quaternion &s)
{
    QScriptValue obj = engine->newObject();
    obj.setProperty("x", QScriptValue(engine, s.x));
    obj.setProperty("y", QScriptValue(engine, s.y));
    obj.setProperty("z", QScriptValue(engine, s.z));
    obj.setProperty("w", QScriptValue(engine, s.w));
    return obj;
}
 
void fromScriptValueQuaternion(const QScriptValue &obj, Quaternion &s)
{
    s.x = (float)obj.property("x").toNumber();
    s.y = (float)obj.property("y").toNumber();
    s.z = (float)obj.property("z").toNumber();
    s.w = (float)obj.property("w").toNumber();
}

QScriptValue toScriptValueQQuaternion(QScriptEngine *engine, const QQuaternion &s)
{
    QScriptValue obj = engine->newObject();
    obj.setProperty("x", QScriptValue(engine, s.x()));
    obj.setProperty("y", QScriptValue(engine, s.y()));
    obj.setProperty("z", QScriptValue(engine, s.z()));
    obj.setProperty("w", QScriptValue(engine, s.scalar()));
    return obj;
}
 
void fromScriptValueQQuaternion(const QScriptValue &obj, QQuaternion &s)
{
    s.setX((float)obj.property("x").toNumber());
    s.setY((float)obj.property("y").toNumber());
    s.setZ((float)obj.property("z").toNumber());
    s.setScalar((float)obj.property("w").toNumber());
}

QScriptValue toScriptValueTransform(QScriptEngine *engine, const Transform &s)
{
    QScriptValue obj = engine->newObject();
    obj.setProperty("pos", toScriptValueVector3(engine, s.position));
    obj.setProperty("rot", toScriptValueVector3(engine, s.rotation));
    obj.setProperty("scale", toScriptValueVector3(engine, s.scale));
    return obj;
}

void fromScriptValueTransform(const QScriptValue &obj, Transform &s)
{
    fromScriptValueVector3(obj.property("pos"), s.position);
    fromScriptValueVector3(obj.property("rot"), s.rotation);
    fromScriptValueVector3(obj.property("scale"), s.scale);
}

QScriptValue toScriptValueIAttribute(QScriptEngine *engine, const IAttribute *&s)
{
    QScriptValue obj = engine->newObject();
    if(s)
    {
        obj.setProperty("name", QScriptValue(engine, QString::fromStdString(s->GetNameString())));
        obj.setProperty("typename", QScriptValue(engine, QString::fromStdString(s->TypeName())));
        obj.setProperty("value", QScriptValue(engine, QString::fromStdString(s->ToString())));
    }
    else
    {
        LogError("Fail to get attribute values from IAttribute pointer, cause pointer was a null. returning empty object.");
    }
    return obj;
}

void fromScriptValueIAttribute(const QScriptValue &obj, IAttribute *&s)
{
}

void RegisterNaaliCoreMetaTypes()
{
    qRegisterMetaType<Color>("Color");
    qRegisterMetaType<Vector3df>("Vector3df");
    qRegisterMetaType<Quaternion>("Quaternion");
    qRegisterMetaType<Transform>("Transform");
}

void ExposeNaaliCoreTypes(QScriptEngine *engine)
{
    qScriptRegisterMetaType(engine, toScriptValueColor, fromScriptValueColor);
    qScriptRegisterMetaType(engine, toScriptValueQColor, fromScriptValueQColor);
    qScriptRegisterMetaType(engine, toScriptValueVector3, fromScriptValueVector3);
    qScriptRegisterMetaType(engine, toScriptValueQVector3D, fromScriptValueQVector3D);
    qScriptRegisterMetaType(engine, toScriptValueQuaternion, fromScriptValueQuaternion);
    qScriptRegisterMetaType(engine, toScriptValueQQuaternion, fromScriptValueQQuaternion);
    qScriptRegisterMetaType(engine, toScriptValueTransform, fromScriptValueTransform);

    //qScriptRegisterMetaType<IAttribute*>(engine, toScriptValueIAttribute, fromScriptValueIAttribute);
    int id = qRegisterMetaType<IAttribute*>("IAttribute*");
    qScriptRegisterMetaType_helper(
        engine, id, reinterpret_cast<QScriptEngine::MarshalFunction>(toScriptValueIAttribute),
        reinterpret_cast<QScriptEngine::DemarshalFunction>(fromScriptValueIAttribute),
        QScriptValue());
}
