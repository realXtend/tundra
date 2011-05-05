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
#include "Entity.h"

#include <QScriptEngine>
#include <QColor>
#include <QVector3D>
#include <QQuaternion>
#include <QScriptValueIterator>
// $ BEGIN_MOD $
#include <QSharedPointer>
// $ END_MOD $
#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("JavaScriptEngine")

Q_DECLARE_METATYPE(IAttribute*);
Q_DECLARE_METATYPE(Scene::EntityPtr);

// $ BEGIN_MOD $
typedef QSharedPointer<Vector3df> Vector3dfPtr;
Q_DECLARE_METATYPE(Vector3dfPtr)

typedef QSharedPointer<Quaternion> QuaternionPtr;
Q_DECLARE_METATYPE(QuaternionPtr)
// $ END_MOD $

QScriptValue toScriptValueEntity(QScriptEngine *engine, const Scene::EntityPtr &e)
{
    return engine->newQObject(e.get());
}

void fromScriptValueEntity(const QScriptValue &obj, const QPointer<Scene::Entity> &e)
{
  //XXX \todo
}

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

// $ BEGIN_MOD $
QScriptValue toScriptValueVector3(QScriptEngine *engine, const Vector3df &s)
{    
    return engine->globalObject().property("Vector3df").construct(QScriptValueList() << s.x << s.y << s.z);    
}

void fromScriptValueVector3(const QScriptValue &obj, Vector3df &s)
{    
    Vector3dfPtr vec = qscriptvalue_cast<Vector3dfPtr>(obj);
    s = *vec.data();    
}

QScriptValue Vector3df_prototype_normalize(QScriptContext *ctx, QScriptEngine *engine)
{
    Vector3dfPtr vec = qscriptvalue_cast<Vector3dfPtr>(ctx->thisObject());
    return toScriptValueVector3(engine, vec->normalize());
}

QScriptValue Vector3df_prototype_getLength(QScriptContext *ctx, QScriptEngine *engine)
{
    Vector3dfPtr vec = qscriptvalue_cast<Vector3dfPtr>(ctx->thisObject());
    return vec->getLength();
}

QScriptValue Vector3df_prototype_mul(QScriptContext *ctx, QScriptEngine *engine)
{
    if (ctx->argumentCount() != 1)
        return ctx->throwError("Vector3df mul() takes a single number argument.");
    if (!ctx->argument(0).isNumber())
        return ctx->throwError(QScriptContext::TypeError, "Vector3df mul(): argument is not a number");
    
    float scalar = ctx->argument(0).toNumber();    
    
    Vector3dfPtr vec = qscriptvalue_cast<Vector3dfPtr>(ctx->thisObject());
    
    return toScriptValueVector3(engine, (*vec.data()) * scalar);
}

QScriptValue Vector3df_prototype_dotProduct(QScriptContext *ctx, QScriptEngine *engine)
{
    if (ctx->argumentCount() != 1)
        return ctx->throwError("Vector3df dotProduct() takes a single Vector3df argument.");
        
    Vector3dfPtr vec = qscriptvalue_cast<Vector3dfPtr>(ctx->thisObject());
    Vector3dfPtr vec2 = qscriptvalue_cast<Vector3dfPtr>(ctx->argument(0));    
    if (!vec2)
        return ctx->throwError("Vector3df dotProduct(): argument is not a Vector3df.");
    
    return vec->dotProduct(*vec2.data());
}

QScriptValue Vector3df_prototype_crossProduct(QScriptContext *ctx, QScriptEngine *engine)
{
    if (ctx->argumentCount() != 1)
        return ctx->throwError("Vector3df crossProduct() takes a single Vector3df argument.");
    
    Vector3dfPtr vec = qscriptvalue_cast<Vector3dfPtr>(ctx->thisObject());
    Vector3dfPtr vec2 = qscriptvalue_cast<Vector3dfPtr>(ctx->argument(0));
    if (!vec2)
        return ctx->throwError("Vector3df crossProduct(): argument is not a Vector3df.");

    return toScriptValueVector3(engine, vec->crossProduct(*vec2.data()));
}

QScriptValue Vector3df_prototype_add(QScriptContext *ctx, QScriptEngine *engine)
{
    if (ctx->argumentCount() != 1)
        return ctx->throwError("Vector3df add() takes a single Vector3df argument.");
    
    Vector3dfPtr vec = qscriptvalue_cast<Vector3dfPtr>(ctx->thisObject());    
    Vector3dfPtr vec2 = qscriptvalue_cast<Vector3dfPtr>(ctx->argument(0));    
    if (!vec2)
        return ctx->throwError("Vector3df add(): argument is not a Vector3df.");
    
    return toScriptValueVector3(engine, *vec.data() + *vec2.data());
}

QScriptValue Vector3df_prototype_subtract(QScriptContext *ctx, QScriptEngine *engine)
{
    if (ctx->argumentCount() != 1)
        return ctx->throwError("Vector3df subtract() takes a single Vector3df argument.");
    
    Vector3dfPtr vec = qscriptvalue_cast<Vector3dfPtr>(ctx->thisObject());    
    Vector3dfPtr vec2 = qscriptvalue_cast<Vector3dfPtr>(ctx->argument(0));    
    if (!vec2)
        return ctx->throwError("Vector3df subtract(): argument is not a Vector3df.");
    
    return toScriptValueVector3(engine, *vec - *vec2);
}

QScriptValue Vector3df_prototype_toString(QScriptContext *ctx, QScriptEngine *engine)
{    
    Vector3dfPtr vec = qscriptvalue_cast<Vector3dfPtr>(ctx->thisObject());    
    if (!vec)
        return "Vector3df()";
    else
        return "Vector3df(x: "+QString::number(vec->x)+"; y: "+QString::number(vec->y)+"; z: "+QString::number(vec->z)+")";
    
}

QScriptValue Vector3df_prototype_x(QScriptContext *ctx, QScriptEngine *engine)
{   
    Vector3dfPtr vector = qscriptvalue_cast<Vector3dfPtr>(ctx->thisObject());
    if (!vector)
        return QScriptValue();

    QScriptValue result;
    if (ctx->argumentCount() == 1) //set
    {                
        float scalar = ctx->argument(0).toNumber();
        vector->x = scalar;
    } 
    else //get
        result = vector->x;
    return result; 
}

QScriptValue Vector3df_prototype_y(QScriptContext *ctx, QScriptEngine *engine)
{
    Vector3dfPtr vector = qscriptvalue_cast<Vector3dfPtr>(ctx->thisObject());
    if (!vector)
        return QScriptValue();
   
    QScriptValue result;
    if (ctx->argumentCount() == 1) //set
    {                
        float scalar = ctx->argument(0).toNumber();
        vector->y = scalar;
    } 
    else //get
        result = vector->y;
    return result; 
}

QScriptValue Vector3df_prototype_z(QScriptContext *ctx, QScriptEngine *engine)
{
    Vector3dfPtr vector = qscriptvalue_cast<Vector3dfPtr>(ctx->thisObject());
    if (!vector)
        return QScriptValue();
       
    QScriptValue result;
    if (ctx->argumentCount() == 1) //set
    {                
        float scalar = ctx->argument(0).toNumber();
        vector->z = scalar;
    } 
    else //get
        result = vector->z;
    return result; 
}


// QUATERNIONS

QScriptValue toScriptValueQuaternion(QScriptEngine *engine, const Quaternion &s)
{    
    return engine->globalObject().property("Quaternion").construct(QScriptValueList() << s.x << s.y << s.z << s.w);        
}

void fromScriptValueQuaternion(const QScriptValue &obj, Quaternion &s)
{    
    QuaternionPtr q = qscriptvalue_cast<QuaternionPtr>(obj);
    s = *q.data();    
}


QScriptValue Quaternion_prototype_toString(QScriptContext *ctx, QScriptEngine *engine)
{
    QuaternionPtr q = qscriptvalue_cast<QuaternionPtr>(ctx->thisObject());
    if (!q)
        return "Quaternion()";
    else
        return "Quaternion(x: "+QString::number(q->x)+"; y: "+QString::number(q->y)+"; z: "+QString::number(q->z)+"; w: "+QString::number(q->w)+")";
    

}

QScriptValue Quaternion_prototype_normalize(QScriptContext *ctx, QScriptEngine *engine)
{
    QuaternionPtr q = qscriptvalue_cast<QuaternionPtr>(ctx->thisObject());    
    return toScriptValueQuaternion(engine, q->normalize());

}

QScriptValue Quaternion_prototype_toEuler(QScriptContext *ctx, QScriptEngine *engine)
{
    QuaternionPtr q = qscriptvalue_cast<QuaternionPtr>(ctx->thisObject());
    
    //get euler vector
    Vector3df v;
    q->toEuler(v);

    return toScriptValueVector3(engine, v);
}

QScriptValue Quaternion_prototype_rotationFromTo(QScriptContext *ctx, QScriptEngine *engine)
{
    if (ctx->argumentCount() != 2)
        return ctx->throwError("Quaternion rotationFromTo() takes two Vector3df arguments.");
    
    Vector3dfPtr vec1 = qscriptvalue_cast<Vector3dfPtr>(ctx->argument(0));
    if (!vec1)
        return ctx->throwError("Vector3df rotationFromTo(): first argument is not a Vector3df.");
    
    Vector3dfPtr vec2 = qscriptvalue_cast<Vector3dfPtr>(ctx->argument(0));    
    if (!vec2)
        return ctx->throwError("Vector3df rotationFromTo(): second argument is not a Vector3df.");
    
    Quaternion q;
    return toScriptValueQuaternion(engine,q.rotationFromTo(*vec1, *vec2));
}

QScriptValue Quaternion_prototype_product(QScriptContext *ctx, QScriptEngine *engine)
{
    if (ctx->argumentCount() != 1)
        return ctx->throwError("Quaternion product() takes a single argument(Vector3df or Quaternion).");
    
    QuaternionPtr this_q = qscriptvalue_cast<QuaternionPtr>(ctx->thisObject());

    //first check if argument is a Vector3df
    Vector3dfPtr v = qscriptvalue_cast<Vector3dfPtr>(ctx->argument(0));    
    if (v)
        return toScriptValueVector3(engine, (*this_q) * (*v));
    
    //if not, check Quaternion
    QuaternionPtr q = qscriptvalue_cast<QuaternionPtr>(ctx->thisObject());
    if (q)
        return toScriptValueQuaternion(engine,(*this_q) * (*q));

    //argument is not neither Vector3df nor Quaternion
    return ctx->throwError("Quaternion product() takes a single argument(Vector3df or Quaternion).");                
}

QScriptValue Quaternion_prototype_makeInverse(QScriptContext *ctx, QScriptEngine *engine)
{
    QuaternionPtr q = qscriptvalue_cast<QuaternionPtr>(ctx->thisObject());   
    return toScriptValueQuaternion(engine, q->makeInverse());  

}

QScriptValue Quaternion_prototype_x(QScriptContext *ctx, QScriptEngine *engine)
{
    QuaternionPtr q = qscriptvalue_cast<QuaternionPtr>(ctx->thisObject());
    if (!q)
        return QScriptValue();

    QScriptValue result;
    if (ctx->argumentCount() == 1) //set
    {                
        float scalar = ctx->argument(0).toNumber();
        q->x = scalar;
    } 
    else //get
    {        
        result = q->x;
    }
    return result;
}

QScriptValue Quaternion_prototype_y(QScriptContext *ctx, QScriptEngine *engine)
{
    QuaternionPtr q = qscriptvalue_cast<QuaternionPtr>(ctx->thisObject());
    if (!q)
        return QScriptValue();

    QScriptValue result;
    if (ctx->argumentCount() == 1) //set
    {                
        float scalar = ctx->argument(0).toNumber();
        q->y = scalar;
    } 
    else //get
    {        
        result = q->y;
    }
    return result;
}

QScriptValue Quaternion_prototype_z(QScriptContext *ctx, QScriptEngine *engine)
{
    QuaternionPtr q = qscriptvalue_cast<QuaternionPtr>(ctx->thisObject());
    if (!q)
        return QScriptValue();

    QScriptValue result;
    if (ctx->argumentCount() == 1) //set
    {                
        float scalar = ctx->argument(0).toNumber();
        q->z = scalar;
    } 
    else //get
    {        
        result = q->z;
    }
    return result;

}

QScriptValue Quaternion_prototype_w(QScriptContext *ctx, QScriptEngine *engine)
{
    QuaternionPtr q = qscriptvalue_cast<QuaternionPtr>(ctx->thisObject());
    if (!q)
        return QScriptValue();

    QScriptValue result;
    if (ctx->argumentCount() == 1) //set
    {                
        float scalar = ctx->argument(0).toNumber();
        q->w= scalar;
    } 
    else //get
    {        
        result = q->w;
    }
    return result;
}
// $END_MOD $

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
// $ BEGIN_MOD $
    Vector3dfPtr pos = qscriptvalue_cast<Vector3dfPtr>(obj.property("pos"));    
    Vector3dfPtr rot = qscriptvalue_cast<Vector3dfPtr>(obj.property("rot"));    
    Vector3dfPtr scale = qscriptvalue_cast<Vector3dfPtr>(obj.property("scale"));    

    s.position = *pos.data();
    s.rotation = *rot.data();
    s.scale = *scale.data();
// $ END_MOD $
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
    // Clear the old content as we are appending from the start!
    s.refs.clear();

    QScriptValueIterator it(obj);
    while (it.hasNext()) 
    {
        it.next();
        AssetReference reference(it.value().toString());
        s.Append(reference);
    }
}

QScriptValue toScriptValueAssetReferenceList(QScriptEngine *engine, const AssetReferenceList &s)
{
    QScriptValue obj = engine->newObject();
    for(int i = 0; i < s.refs.size(); ++i)
        obj.setProperty(i, QScriptValue(engine, s[i].ref));
    return obj;
}

void fromScriptValueEntityList(const QScriptValue &obj, QList<Scene::Entity*> &ents)
{
    ents.clear();
    QScriptValueIterator it(obj);
    while (it.hasNext())
    {
        it.next();
        QObject *qent = it.value().toQObject();
        if (qent)
        {
            Scene::Entity *ent = qobject_cast<Scene::Entity*>(qent);
            if (ent)
                ents.append(ent);
        }
    }
}

QScriptValue toScriptValueEntityList(QScriptEngine *engine, const QList<Scene::Entity*> &ents)
{
    QScriptValue obj = engine->newArray(ents.size());
    for(int i=0; i<ents.size(); ++i)
    {
        Scene::Entity *ent = ents.at(i);
        if (ent)
            obj.setProperty(i, engine->newQObject(ent));
    }
    return obj;
}

void fromScriptValueStdString(const QScriptValue &obj, std::string &s)
{
    s = obj.toString().toStdString();
}

QScriptValue toScriptValueStdString(QScriptEngine *engine, const std::string &s)
{
    return engine->newVariant(QString::fromStdString(s));
}

void fromScriptValueIAttribute(const QScriptValue &obj, IAttribute *&s)
{
}

QScriptValue createColor(QScriptContext *ctx, QScriptEngine *engine)
{
    Color newColor;
    return engine->toScriptValue(newColor);
}

// $ BEGIN_MOD $
QScriptValue createVector3df(QScriptContext *ctx, QScriptEngine *engine)
{    
    Vector3df *newVec = new Vector3df();
    if ((ctx->argumentCount() == 3) && (ctx->isCalledAsConstructor()))
    {
        if (!ctx->argument(0).isNumber())
            return ctx->throwError(QScriptContext::TypeError, "new Vector3df(x,y,z): x argument is not a number");
        if (!ctx->argument(1).isNumber())
            return ctx->throwError(QScriptContext::TypeError, "new Vector3df(x,y,z): y argument is not a number");
        if (!ctx->argument(2).isNumber())
            return ctx->throwError(QScriptContext::TypeError, "new Vector3df(x,y,z): z argument is not a number");

        newVec->x = ctx->argument(0).toNumber();
        newVec->y = ctx->argument(1).toNumber();
        newVec->z = ctx->argument(2).toNumber();
    }       
    else if (ctx->argumentCount() != 0) 
        return ctx->throwError("Error creating Vector3df. Usage: new Vector3df() or new Vector3df(x, y, z).");

    Vector3dfPtr pointer(newVec);
    return engine->newVariant(ctx->thisObject(), qVariantFromValue(pointer));    
}

QScriptValue createQuaternion(QScriptContext *ctx, QScriptEngine *engine)
{
    Quaternion *newQuat = new Quaternion();
    if ((ctx->argumentCount() == 4) && (ctx->isCalledAsConstructor()))//Quaternion(x, y, z, w)
    {
        if (!ctx->argument(0).isNumber())
            return ctx->throwError(QScriptContext::TypeError, "new Quaternion(x,y,z,w): x argument is not a number");
        if (!ctx->argument(1).isNumber())
            return ctx->throwError(QScriptContext::TypeError, "new Quaternion(x,y,z,w): y argument is not a number");
        if (!ctx->argument(2).isNumber())
            return ctx->throwError(QScriptContext::TypeError, "new Quaternion(x,y,z,w): z argument is not a number");
        if (!ctx->argument(3).isNumber())
            return ctx->throwError(QScriptContext::TypeError, "new Quaternion(x,y,z,w): w argument is not a number");

        newQuat->x = ctx->argument(0).toNumber();
        newQuat->y = ctx->argument(1).toNumber();
        newQuat->z = ctx->argument(2).toNumber();
        newQuat->w = ctx->argument(3).toNumber();
    }
    else if ((ctx->argumentCount() == 1) && (ctx->isCalledAsConstructor())) //Quaternion(v): v is euler vector (radians)
    {
        Vector3dfPtr vec1 = qscriptvalue_cast<Vector3dfPtr>(ctx->thisObject());;        
        if (!vec1)
            return ctx->throwError("new Quaternion(v): v argument is not a Vector3df.");
        newQuat->set(*vec1);
    }
    else if (ctx->argumentCount() != 0)
        return ctx->throwError("Error creating Quaternion. Usage: new Quaternion(), new Quaternion(v) or new Quaternion(x, y, z, w).");

    QuaternionPtr pointer(newQuat);
    return engine->newVariant(ctx->thisObject(), qVariantFromValue(pointer));
}
// $ END_MOD $

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
    qRegisterMetaType<Scene::EntityPtr>("EntityPtr");
    qRegisterMetaType<Color>("Color");
// $ BEGIN_MOD $
    qRegisterMetaType<Vector3df>("Vector3df");
    qRegisterMetaType<Quaternion>("Quaternion");
// $ END_MOD $
    qRegisterMetaType<Transform>("Transform");
    qRegisterMetaType<AssetReference>("AssetReference");
    qRegisterMetaType<AssetReferenceList>("AssetReferenceList");
}

void ExposeNaaliCoreTypes(QScriptEngine *engine)
{
    qScriptRegisterMetaType(engine, toScriptValueColor, fromScriptValueColor);
// $ BEGIN_MOD $
    qScriptRegisterMetaType(engine, toScriptValueVector3, fromScriptValueVector3);
    qScriptRegisterMetaType(engine, toScriptValueQuaternion, fromScriptValueQuaternion);
// $ END_MOD $
    qScriptRegisterMetaType(engine, toScriptValueTransform, fromScriptValueTransform);
    qScriptRegisterMetaType(engine, toScriptValueAssetReference, fromScriptValueAssetReference);
    qScriptRegisterMetaType(engine, toScriptValueAssetReferenceList, fromScriptValueAssetReferenceList);
    
    //qScriptRegisterMetaType<IAttribute*>(engine, toScriptValueIAttribute, fromScriptValueIAttribute);
    int id = qRegisterMetaType<IAttribute*>("IAttribute*");
    qScriptRegisterMetaType_helper(
        engine, id, reinterpret_cast<QScriptEngine::MarshalFunction>(toScriptValueIAttribute),
        reinterpret_cast<QScriptEngine::DemarshalFunction>(fromScriptValueIAttribute),
        QScriptValue());

    id = qRegisterMetaType<Scene::EntityPtr>("Scene::EntityPtr");
    qScriptRegisterMetaType_helper(
        engine, id, reinterpret_cast<QScriptEngine::MarshalFunction>(toScriptValueEntity),
        reinterpret_cast<QScriptEngine::DemarshalFunction>(fromScriptValueEntity),
        QScriptValue());

    id = qRegisterMetaType< QList<Scene::Entity*> >("QList<Scene::Entity*>");
    qScriptRegisterMetaType_helper(
        engine, id, reinterpret_cast<QScriptEngine::MarshalFunction>(toScriptValueEntityList),
        reinterpret_cast<QScriptEngine::DemarshalFunction>(fromScriptValueEntityList),
        QScriptValue());
 
    id = qRegisterMetaType<std::string>("std::string");
    qScriptRegisterMetaType_helper(
        engine, id, reinterpret_cast<QScriptEngine::MarshalFunction>(toScriptValueStdString),
        reinterpret_cast<QScriptEngine::DemarshalFunction>(fromScriptValueStdString),
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

// $ BEGIN_MOD $
    // Register Vector3df both constructors and methods (with js prototype style)
    // http://doc.qt.nokia.com/latest/scripting.html#prototype-based-programming-with-the-qtscript-c-api
    // http://doc.qt.nokia.com/latest/qscriptengine.html#newFunction-2    
    // http://doc.qt.nokia.com/latest/scripting.html#managing-non-qobject-based-objects
    QScriptValue protoVector3df = engine->newObject();
    protoVector3df.setProperty("toString", engine->newFunction(Vector3df_prototype_toString));
    protoVector3df.setProperty("normalize", engine->newFunction(Vector3df_prototype_normalize));    
    protoVector3df.setProperty("getLength", engine->newFunction(Vector3df_prototype_getLength));
    protoVector3df.setProperty("mul", engine->newFunction(Vector3df_prototype_mul));
    protoVector3df.setProperty("add", engine->newFunction(Vector3df_prototype_add));
    protoVector3df.setProperty("subtract", engine->newFunction(Vector3df_prototype_subtract));
    protoVector3df.setProperty("dotProduct", engine->newFunction(Vector3df_prototype_dotProduct));
    protoVector3df.setProperty("crossProduct", engine->newFunction(Vector3df_prototype_crossProduct));        
    // Vector3df setter and getter
    protoVector3df.setProperty("x", engine->newFunction(Vector3df_prototype_x), QScriptValue::PropertyGetter|QScriptValue::PropertySetter);    
    protoVector3df.setProperty("y", engine->newFunction(Vector3df_prototype_y), QScriptValue::PropertyGetter|QScriptValue::PropertySetter);    
    protoVector3df.setProperty("z", engine->newFunction(Vector3df_prototype_z), QScriptValue::PropertyGetter|QScriptValue::PropertySetter);        
    QScriptValue ctorVector3df = engine->newFunction(createVector3df, protoVector3df); //now is working
    engine->globalObject().setProperty("Vector3df", ctorVector3df);
    
    // Register Quaternion both constructors and methods (with js prototype style)
    QScriptValue protoQuaternion = engine->newObject();
    protoQuaternion.setProperty("toString", engine->newFunction(Quaternion_prototype_toString));
    protoQuaternion.setProperty("normalize", engine->newFunction(Quaternion_prototype_normalize));    
    protoQuaternion.setProperty("toEuler", engine->newFunction(Quaternion_prototype_toEuler));
    protoQuaternion.setProperty("rotationFromTo", engine->newFunction(Quaternion_prototype_rotationFromTo));
    protoQuaternion.setProperty("product", engine->newFunction(Quaternion_prototype_product));
    protoQuaternion.setProperty("makeInverse", engine->newFunction(Quaternion_prototype_makeInverse));
    // Quaterinon setter and getter
    protoQuaternion.setProperty("x", engine->newFunction(Quaternion_prototype_x), QScriptValue::PropertyGetter|QScriptValue::PropertySetter);
    protoQuaternion.setProperty("y", engine->newFunction(Quaternion_prototype_y), QScriptValue::PropertyGetter|QScriptValue::PropertySetter);
    protoQuaternion.setProperty("z", engine->newFunction(Quaternion_prototype_z), QScriptValue::PropertyGetter|QScriptValue::PropertySetter);
    protoQuaternion.setProperty("w", engine->newFunction(Quaternion_prototype_w), QScriptValue::PropertyGetter|QScriptValue::PropertySetter);
    
    QScriptValue ctorQuaternion = engine->newFunction(createQuaternion, protoQuaternion);
    engine->globalObject().setProperty("Quaternion", ctorQuaternion);
// $ END_MOD $
}
