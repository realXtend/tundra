// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MemoryLeakCheck.h"
#include "Color.h"
#include "Quaternion.h"
#include "Transform.h"
#include "Vector3D.h"
#include "Matrix4.h"
#include "IAttribute.h"
#include "AssetReference.h"

#include <QScriptEngine>
#include <QColor>
#include <QVector3D>
#include <QQuaternion>
#include <QScriptValueIterator>

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
/*
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
*/
QScriptValue Vector3df_prototype_normalize(QScriptContext *ctx, QScriptEngine *engine);
QScriptValue Vector3df_prototype_getLength(QScriptContext *ctx, QScriptEngine *engine);
QScriptValue Vector3df_prototype_mul(QScriptContext *ctx, QScriptEngine *engine);
QScriptValue toScriptValueVector3(QScriptEngine *engine, const Vector3df &s)
{
    QScriptValue obj = engine->newObject();
    obj.setProperty("x", QScriptValue(engine, s.x));
    obj.setProperty("y", QScriptValue(engine, s.y));
    obj.setProperty("z", QScriptValue(engine, s.z));

    //this should suffice only once for the prototype somehow, but couldn't get that to work
    //ctorVector3df.property("prototype").setProperty("normalize", normalizeVector3df);
    obj.prototype().setProperty("normalize", engine->newFunction(Vector3df_prototype_normalize));
    obj.prototype().setProperty("getLength", engine->newFunction(Vector3df_prototype_getLength));
    obj.prototype().setProperty("mul", engine->newFunction(Vector3df_prototype_mul));

    return obj;
}

void fromScriptValueVector3(const QScriptValue &obj, Vector3df &s)
{
    s.x = (float)obj.property("x").toNumber();
    s.y = (float)obj.property("y").toNumber();
    s.z = (float)obj.property("z").toNumber();
}

QScriptValue Vector3df_prototype_normalize(QScriptContext *ctx, QScriptEngine *engine)
{
    Vector3df vec;
    fromScriptValueVector3(ctx->thisObject(), vec);
      
    return toScriptValueVector3(engine, vec.normalize());
}

QScriptValue Vector3df_prototype_getLength(QScriptContext *ctx, QScriptEngine *engine)
{
    Vector3df vec;
    fromScriptValueVector3(ctx->thisObject(), vec);
      
    return vec.getLength();
}

QScriptValue Vector3df_prototype_mul(QScriptContext *ctx, QScriptEngine *engine)
{
    if (ctx->argumentCount() != 1)
        return ctx->throwError("Vector3df mul() takes a single number argument.");
    if (!ctx->argument(0).isNumber())
        return ctx->throwError(QScriptContext::TypeError, "Vector3df mul(): argument is not a number");
    float scalar = ctx->argument(0).toNumber();
    //XXX add vec*vec
    
    Vector3df vec;
    fromScriptValueVector3(ctx->thisObject(), vec);

    return toScriptValueVector3(engine, vec * scalar);
}
/*
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
*/
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
/*
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
*/
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

void fromScriptValueAssetReference(const QScriptValue &obj, AssetReference &s)
{
    s.ref = obj.property("ref").toString();
}

QScriptValue toScriptValueAssetReference(QScriptEngine *engine, const AssetReference &s)
{
    QScriptValue obj = engine->newObject();
    obj.setProperty("ref", QScriptValue(engine, s.ref));
    return obj;
}

void fromScriptValueAssetReferenceList(const QScriptValue &obj, AssetReferenceList &s)
{
    QScriptValueIterator it(obj);
  
    while (it.hasNext()) {
        it.next();
        AssetReference reference(it.value().toString());
        s.Append(reference);
    }
    
}

QScriptValue toScriptValueAssetReferenceList(QScriptEngine *engine, const AssetReferenceList &s)
{
    QScriptValue obj = engine->newObject();
  
    for( int i = 0; i < s.refs.size(); ++i)
    {
        obj.setProperty(i, QScriptValue(engine, s.refs[i].toString()));
    }

    return obj;
}

void fromScriptValueIAttribute(const QScriptValue &obj, IAttribute *&s)
{
}

QScriptValue createColor(QScriptContext *ctx, QScriptEngine *engine)
{
    Color newColor;
    return engine->toScriptValue(newColor);
}

QScriptValue createVector3df(QScriptContext *ctx, QScriptEngine *engine)
{
    Vector3df newVec;
    newVec.x = 0;
    newVec.y = 0;
    newVec.z = 0;
    return engine->toScriptValue(newVec);
}

QScriptValue createQuaternion(QScriptContext *ctx, QScriptEngine *engine)
{
    Quaternion newQuat;
    return engine->toScriptValue(newQuat);
}

QScriptValue createTransform(QScriptContext *ctx, QScriptEngine *engine)
{
    Transform newTransform;
    return engine->toScriptValue(newTransform);
}

QScriptValue createAssetReference(QScriptContext *ctx, QScriptEngine *engine)
{
    AssetReference newAssetRef;
    return engine->toScriptValue(newAssetRef);
}

QScriptValue createAssetReferenceList(QScriptContext *ctx, QScriptEngine *engine)
{
    AssetReferenceList newAssetRefList;
    return engine->toScriptValue(newAssetRefList);
}

void RegisterNaaliCoreMetaTypes()
{
    qRegisterMetaType<Color>("Color");
    qRegisterMetaType<Vector3df>("Vector3df");
    qRegisterMetaType<Quaternion>("Quaternion");
    qRegisterMetaType<Transform>("Transform");
    qRegisterMetaType<AssetReference>("AssetReference");
    qRegisterMetaType<AssetReferenceList>("AssetReferenceList");
}

void ExposeNaaliCoreTypes(QScriptEngine *engine)
{
    qScriptRegisterMetaType(engine, toScriptValueColor, fromScriptValueColor);
    //qScriptRegisterMetaType(engine, toScriptValueQColor, fromScriptValueQColor);
    qScriptRegisterMetaType(engine, toScriptValueVector3, fromScriptValueVector3);
    //qScriptRegisterMetaType(engine, toScriptValueQVector3D, fromScriptValueQVector3D);
    qScriptRegisterMetaType(engine, toScriptValueQuaternion, fromScriptValueQuaternion);
    //qScriptRegisterMetaType(engine, toScriptValueQQuaternion, fromScriptValueQQuaternion);
    qScriptRegisterMetaType(engine, toScriptValueTransform, fromScriptValueTransform);
    qScriptRegisterMetaType(engine, toScriptValueAssetReference, fromScriptValueAssetReference);
    qScriptRegisterMetaType(engine, toScriptValueAssetReferenceList, fromScriptValueAssetReferenceList);

    //qScriptRegisterMetaType<IAttribute*>(engine, toScriptValueIAttribute, fromScriptValueIAttribute);
    int id = qRegisterMetaType<IAttribute*>("IAttribute*");
    qScriptRegisterMetaType_helper(
        engine, id, reinterpret_cast<QScriptEngine::MarshalFunction>(toScriptValueIAttribute),
        reinterpret_cast<QScriptEngine::DemarshalFunction>(fromScriptValueIAttribute),
        QScriptValue());

    // Register constructors
    QScriptValue ctorColor = engine->newFunction(createColor);
    engine->globalObject().setProperty("Color", ctorColor);
    QScriptValue ctorTransform = engine->newFunction(createTransform);
    engine->globalObject().setProperty("Transform", ctorTransform);
    QScriptValue ctorAssetReference = engine->newFunction(createAssetReference);
    engine->globalObject().setProperty("AssetReference", ctorAssetReference);
    QScriptValue ctorAssetReferenceList = engine->newFunction(createAssetReferenceList);
    engine->globalObject().setProperty("AssetReferenceList", ctorAssetReferenceList);

    // Register both constructors and methods (with js prototype style)
    // http://doc.qt.nokia.com/latest/scripting.html#prototype-based-programming-with-the-qtscript-c-api
    /* doesn't work for some reason, is now hacked in toScriptValue to every instance (bad!)
    QScriptValue protoVector3df = engine->newObject();
    protoVector3df.setProperty("normalize2", engine->newFunction(Vector3df_prototype_normalize));*/
    QScriptValue ctorVector3df = engine->newFunction(createVector3df); //, protoVector3df);
    engine->globalObject().setProperty("Vector3df", ctorVector3df);
    
    QScriptValue ctorQuaternion = engine->newFunction(createQuaternion);
    engine->globalObject().setProperty("Quaternion", ctorQuaternion);
}
