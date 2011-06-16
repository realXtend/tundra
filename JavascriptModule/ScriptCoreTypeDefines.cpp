// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "Color.h"
#include "Math/Quat.h"
#include "Math/float3.h"
#include "IAttribute.h"
#include "AssetReference.h"
#include "EntityReference.h"
#include "Entity.h"
#include "ScriptMetaTypeDefines.h"
#include "Scene.h"
#include "Transform.h"
#include "LoggingFunctions.h"

#include <QScriptEngine>
#include <QColor>
#include <QVector3D>
#include <QQuaternion>
#include <QScriptValueIterator>

#include "MemoryLeakCheck.h"

Q_DECLARE_METATYPE(IAttribute*);
Q_DECLARE_METATYPE(ScenePtr);
Q_DECLARE_METATYPE(EntityPtr);
Q_DECLARE_METATYPE(ComponentPtr);
Q_DECLARE_METATYPE(QList<Entity*>);
Q_DECLARE_METATYPE(Entity*);
Q_DECLARE_METATYPE(std::string);
Q_DECLARE_METATYPE(EntityList);

QScriptValue Color_prototype_ToString(QScriptContext *ctx, QScriptEngine *engine);
QScriptValue Color_prototype_FromString(QScriptContext *ctx, QScriptEngine *engine);
void createColorFunctions(QScriptValue &value, QScriptEngine *engine)
{
    // Expose native functions to script value.
    value.setProperty("toString", engine->newFunction(Color_prototype_ToString));
    value.setProperty("fromString", engine->newFunction(Color_prototype_FromString));
}

QScriptValue toScriptValueColor(QScriptEngine *engine, const Color &s)
{
    QScriptValue obj = engine->newObject();
    obj.setProperty("r", QScriptValue(engine, s.r));
    obj.setProperty("g", QScriptValue(engine, s.g));
    obj.setProperty("b", QScriptValue(engine, s.b));
    obj.setProperty("a", QScriptValue(engine, s.a));
    createColorFunctions(obj, engine);
    return obj;
}

void fromScriptValueColor(const QScriptValue &obj, Color &s)
{
    s.r = (float)obj.property("r").toNumber();
    s.g = (float)obj.property("g").toNumber();
    s.b = (float)obj.property("b").toNumber();
    s.a = (float)obj.property("a").toNumber();
}

//! @todo this code duplicates with IAttribute.
QScriptValue Color_prototype_ToString(QScriptContext *ctx, QScriptEngine *engine)
{
    Color value = engine->fromScriptValue<Color>(ctx->thisObject());
    QString retVal = QString::number(value.r) + " " +
                     QString::number(value.g) + " " +
                     QString::number(value.b) + " " +
                     QString::number(value.a);
    return engine->toScriptValue(retVal);
}

QScriptValue Color_prototype_FromString(QScriptContext *ctx, QScriptEngine *engine)
{
    if (ctx->argumentCount() != 1)
        return ctx->throwError(QScriptContext::TypeError, "Color fromString(): invalid number of arguments."); 
    QStringList values = ctx->argument(0).toString().split(" ");
    if (values.count() != 4)
        return ctx->throwError(QScriptContext::TypeError, "Color fromString(): invalid string value."); 
    
    Color retVal(values[0].toFloat(), values[1].toFloat(), values[2].toFloat(), values[3].toFloat());
    return toScriptValueColor(engine, retVal);
}
/*
QScriptValue Vector3df_prototype_normalize(QScriptContext *ctx, QScriptEngine *engine);
QScriptValue Vector3df_prototype_getLength(QScriptContext *ctx, QScriptEngine *engine);
QScriptValue Vector3df_prototype_add(QScriptContext *ctx, QScriptEngine *engine);
QScriptValue Vector3df_prototype_sub(QScriptContext *ctx, QScriptEngine *engine);
QScriptValue Vector3df_prototype_mul(QScriptContext *ctx, QScriptEngine *engine);
QScriptValue Vector3df_prototype_inter(QScriptContext *ctx, QScriptEngine *engine);
QScriptValue Vector3df_prototype_rotToDir(QScriptContext *ctx, QScriptEngine *engine);
QScriptValue Vector3df_prototype_cross(QScriptContext *ctx, QScriptEngine *engine);
QScriptValue Vector3df_prototype_dot(QScriptContext *ctx, QScriptEngine *engine);
QScriptValue Vector3df_prototype_invert(QScriptContext *ctx, QScriptEngine *engine);
QScriptValue Vector3df_prototype_ToString(QScriptContext *ctx, QScriptEngine *engine);
QScriptValue Vector3df_prototype_FromString(QScriptContext *ctx, QScriptEngine *engine);
QScriptValue Vector3df_prototype_distance(QScriptContext *ctx, QScriptEngine *engine);
void createVector3Functions(QScriptValue &value, QScriptEngine *engine)
{
    // Expose native functions to script value.
    value.setProperty("normalize", engine->newFunction(Vector3df_prototype_normalize));
    value.setProperty("length", engine->newFunction(Vector3df_prototype_getLength));
    value.setProperty("add", engine->newFunction(Vector3df_prototype_add));
    value.setProperty("sub", engine->newFunction(Vector3df_prototype_sub));
    value.setProperty("mul", engine->newFunction(Vector3df_prototype_mul));
    value.setProperty("lerp", engine->newFunction(Vector3df_prototype_inter));
    value.setProperty("rotToDir", engine->newFunction(Vector3df_prototype_rotToDir));
    value.setProperty("cross", engine->newFunction(Vector3df_prototype_cross));
    value.setProperty("dot", engine->newFunction(Vector3df_prototype_dot));
    value.setProperty("invert", engine->newFunction(Vector3df_prototype_invert));
    value.setProperty("toString", engine->newFunction(Vector3df_prototype_ToString));
    value.setProperty("fromString", engine->newFunction(Vector3df_prototype_FromString));
    value.setProperty("distance", engine->newFunction(Vector3df_prototype_distance));
}

QScriptValue toScriptValueFloat3(QScriptEngine *engine, const float3 &s)
{
    QScriptValue obj = engine->newObject();
    obj.setProperty("x", QScriptValue(engine, s.x));
    obj.setProperty("y", QScriptValue(engine, s.y));
    obj.setProperty("z", QScriptValue(engine, s.z));
    createVector3Functions(obj, engine);

    return obj;
}

void fromScriptValueFloat3(const QScriptValue &obj, float3 &s)
{
    s.x = (float)obj.property("x").toNumber();
    s.y = (float)obj.property("y").toNumber();
    s.z = (float)obj.property("z").toNumber();
}

QScriptValue Vector3df_prototype_normalize(QScriptContext *ctx, QScriptEngine *engine)
{
    float3 vec;
    fromScriptValueVector3(ctx->thisObject(), vec);
      
    return toScriptValueVector3(engine, vec.normalize());
}

QScriptValue Vector3df_prototype_getLength(QScriptContext *ctx, QScriptEngine *engine)
{
    float3 vec;
    fromScriptValueVector3(ctx->thisObject(), vec);
      
    return vec.getLength();
}

QScriptValue Vector3df_prototype_mul(QScriptContext *ctx, QScriptEngine *engine)
{
    if (ctx->argumentCount() != 1)
        return ctx->throwError("float3 mul() takes a single number argument.");
    //if (!ctx->argument(0).isNumber())
    //    return ctx->throwError(QScriptContext::TypeError, "float3 mul(): argument is not a number");
    float3 vec;
    fromScriptValueVector3(ctx->thisObject(), vec);
    if (ctx->argument(0).isNumber()) // Multiply by scalar
    {
        float scalar = ctx->argument(0).toNumber();
        return toScriptValueVector3(engine, vec * scalar);
    }
    else if(ctx->argument(0).isObject()) // Multiply by vector3
    {
        float3 vec2 = engine->fromScriptValue<float3>(ctx->argument(0));
        return toScriptValueVector3(engine, vec * vec2);
    }
    return ctx->throwError(QScriptContext::TypeError, "float3 mul(): argument is not a number or vector3df");
}

QScriptValue Vector3df_prototype_add(QScriptContext *ctx, QScriptEngine *engine)
{
    if (ctx->argumentCount() != 1)
        return ctx->throwError("float3 add() takes a single number argument.");
    //if (!ctx->argument(0).isNumber())
    //    return ctx->throwError(QScriptContext::TypeError, "float3 add(): argument is not a number");
    float3 vec;
    fromScriptValueVector3(ctx->thisObject(), vec);
    if(ctx->argument(0).isObject())
    {
        float3 vec2 = engine->fromScriptValue<float3>(ctx->argument(0));
        return toScriptValueVector3(engine, vec + vec2);
    }
    return ctx->throwError(QScriptContext::TypeError, "float3 add(): argument is not a vector3df");
}

QScriptValue Vector3df_prototype_sub(QScriptContext *ctx, QScriptEngine *engine)
{
    if (ctx->argumentCount() != 1)
        return ctx->throwError("float3 sub() takes a single number argument.");
    //if (!ctx->argument(0).isNumber())
    //    return ctx->throwError(QScriptContext::TypeError, "float3 sub(): argument is not a number");
    float3 vec;
    fromScriptValueVector3(ctx->thisObject(), vec);
    if(ctx->argument(0).isObject())
    {
        float3 vec2 = engine->fromScriptValue<float3>(ctx->argument(0));
        return toScriptValueVector3(engine, vec - vec2);
    }
    return ctx->throwError(QScriptContext::TypeError, "float3 sub(): argument is not a vector3df");
}

QScriptValue Vector3df_prototype_inter(QScriptContext *ctx, QScriptEngine *engine)
{
    QScriptValue retValue;

    int argCount = ctx->argumentCount();
    if (argCount >= 2 && argCount <= 3)
    {
        float3 vec, vec2;
        fromScriptValueVector3(ctx->thisObject(), vec);

        // Ensure that the last argument is a number. 
        if (!ctx->argument(argCount - 1).isNumber())
            return ctx->throwError(QScriptContext::TypeError, "float3 interpolate(): argument(" + QString::number(argCount - 1) + ") isn't a number.");
        float d = ctx->argument(argCount - 1).toNumber();

        if (argCount == 2 && ctx->argument(0).isObject())
        {
            vec2 = engine->fromScriptValue<float3>(ctx->argument(0));
            retValue = toScriptValueVector3(engine, vec.getInterpolated(vec2, d));
        }
        else if(ctx->argument(0).isObject() &&
                ctx->argument(1).isObject())
        {
            float3 vec3;
            vec2 = engine->fromScriptValue<float3>(ctx->argument(0));
            vec3 = engine->fromScriptValue<float3>(ctx->argument(1));
            vec = vec.interpolate(vec3, vec2, d);
            retValue = toScriptValueVector3(engine, vec); 
        }
        else
            return ctx->throwError(QScriptContext::TypeError, "float3 interpolate(): argument types are invalid.");
    }
    else
        return ctx->throwError(QScriptContext::TypeError, "float3 interpolate(): invalid number of arguments.");
    return retValue;
}

QScriptValue Vector3df_prototype_rotToDir(QScriptContext *ctx, QScriptEngine *engine)
{
    if (ctx->argumentCount() != 1)
        return ctx->throwError(QScriptContext::TypeError, "float3 rotToDir(): invalid number of arguments.");

    float3 vec1 = engine->fromScriptValue<float3>(ctx->thisObject());
    float3 vec2 = engine->fromScriptValue<float3>(ctx->argument(0));

    return toScriptValueVector3(engine, vec1.rotationToDirection(vec2));
}

QScriptValue Vector3df_prototype_cross(QScriptContext *ctx, QScriptEngine *engine)
{
    if (ctx->argumentCount() != 1)
        return ctx->throwError(QScriptContext::TypeError, "float3 crss(): invalid number of arguments."); 

    float3 vec1 = engine->fromScriptValue<float3>(ctx->thisObject());
    float3 vec2 = engine->fromScriptValue<float3>(ctx->argument(0));

    return toScriptValueVector3(engine, vec1.crossProduct(vec2));
}

QScriptValue Vector3df_prototype_dot(QScriptContext *ctx, QScriptEngine *engine)
{
    if (ctx->argumentCount() != 1)
        return ctx->throwError(QScriptContext::TypeError, "float3 dot(): invalid number of arguments."); 

    float3 vec1 = engine->fromScriptValue<float3>(ctx->thisObject());
    float3 vec2 = engine->fromScriptValue<float3>(ctx->argument(0));

    return QScriptValue(engine, vec1.dotProduct(vec2));
}

QScriptValue Vector3df_prototype_invert(QScriptContext *ctx, QScriptEngine *engine)
{
    float3 vec = engine->fromScriptValue<float3>(ctx->thisObject());
    return toScriptValueVector3(engine, vec.invert());
}

//! @todo this code duplicates with IAttribute.
QScriptValue Vector3df_prototype_ToString(QScriptContext *ctx, QScriptEngine *engine)
{
    float3 value = engine->fromScriptValue<float3>(ctx->thisObject());
    QString retVal = QString::number(value.x) + " " +
                     QString::number(value.y) + " " +
                     QString::number(value.z);
    return engine->toScriptValue(retVal);
}

QScriptValue Vector3df_prototype_FromString(QScriptContext *ctx, QScriptEngine *engine)
{
    if (ctx->argumentCount() != 1)
        return ctx->throwError(QScriptContext::TypeError, "float3 fromString(): invalid number of arguments."); 
    QStringList values = ctx->argument(0).toString().split(" ");
    if (values.count() != 3)
        return ctx->throwError(QScriptContext::TypeError, "float3 fromString(): invalid string value."); 
    
    float3 retValue(values[0].toFloat(), values[1].toFloat(), values[2].toFloat());
    return toScriptValueVector3(engine, retValue);
}

QScriptValue Vector3df_prototype_distance(QScriptContext *ctx, QScriptEngine *engine)
{
    if (ctx->argumentCount() != 1)
        return ctx->throwError(QScriptContext::TypeError, "float3 distance(): invalid number of arguments.");

    float3 vec1 = engine->fromScriptValue<float3>(ctx->thisObject());
    float3 vec2 = engine->fromScriptValue<float3>(ctx->argument(0));

    return vec1.getDistanceFrom(vec2);
}
*/
QScriptValue toScriptValueIAttribute(QScriptEngine *engine, IAttribute * const &s)
{
    QScriptValue obj = engine->newObject();
    if(s)
    {
        obj.setProperty("name", QScriptValue(engine, s->Name()));
        obj.setProperty("typename", QScriptValue(engine, s->TypeName()));
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
    if (obj.isString())
        s.ref = obj.toString();
    else
    {
        if (!obj.property("ref").isValid() || !obj.property("ref").isString())
            LogError("Can't convert QScriptValue to AssetReference! QScriptValue is not a string and it doesn't contain a ref attribute!");
        s.ref = obj.property("ref").toString();
        s.type = obj.property("type").toString();
    }
}

QScriptValue toScriptValueAssetReference(QScriptEngine *engine, const AssetReference &s)
{
    QScriptValue obj = engine->newObject();
    obj.setProperty("ref", QScriptValue(engine, s.ref));
    obj.setProperty("type", QScriptValue(engine, s.type));
    return obj;
}

void fromScriptValueAssetReferenceList(const QScriptValue &obj, AssetReferenceList &s)
{
    // Clear the old content as we are appending from the start!
    s.refs.clear();

    QScriptValueIterator it(obj);
    while(it.hasNext()) 
    {
        it.next();
        if (it.value().isString())
        {
            AssetReference reference(it.value().toString());
            s.Append(reference);
        }
    }
}

QScriptValue toScriptValueAssetReferenceList(QScriptEngine *engine, const AssetReferenceList &s)
{
    QScriptValue obj = engine->newObject();
    for(int i = 0; i < s.refs.size(); ++i)
        obj.setProperty(i, QScriptValue(engine, s[i].ref));
    return obj;
}

QScriptValue EntityReference_prototype_Lookup(QScriptContext *ctx, QScriptEngine *engine);
void createEntityReferenceFunctions(QScriptValue &value, QScriptEngine *engine)
{
    // Expose native functions to script value.
    value.setProperty("Lookup", engine->newFunction(EntityReference_prototype_Lookup));
}

void fromScriptValueEntityReference(const QScriptValue &obj, EntityReference &s)
{
    if (obj.isString())
        s.ref = obj.toString();
    else
    {
        if (!obj.property("ref").isValid())
            LogError("Can't convert QScriptValue to EntityReference! QScriptValue does not contain ref attribute!");
        else
        {
            QScriptValue ref = obj.property("ref");
            if (ref.isNull())
                s.ref = ""; // Empty the reference
            else if (ref.isString())
                s.ref = ref.toString();
            else if (ref.isNumber())
                s.ref = QString::number(ref.toInt32());
            else if (ref.isQObject())
            {
                // If the object is an Entity, call EntityReference::Set() with it
                Entity* entity = dynamic_cast<Entity*>(ref.toQObject());
                s.Set(entity);
            }
            else
                LogError("Can't convert QScriptValue to EntityReference! Ref attribute is not null, string, a number, or an entity");
        }
    }
}

QScriptValue toScriptValueEntityReference(QScriptEngine *engine, const EntityReference &s)
{
    QScriptValue obj = engine->newObject();
    obj.setProperty("ref", QScriptValue(engine, s.ref));
    createEntityReferenceFunctions(obj, engine);
    return obj;
}

QScriptValue EntityReference_prototype_Lookup(QScriptContext *ctx, QScriptEngine *engine)
{
    int argCount = ctx->argumentCount();
    if (argCount != 1)
        return ctx->throwError(QScriptContext::TypeError, "EntityReference Lookup(): Invalid number of arguments.");
    
    if (!ctx->argument(0).isQObject())
        return ctx->throwError(QScriptContext::TypeError, "EntityReference Lookup(): Argument is not a QObject");
    
    Scene* scene = dynamic_cast<Scene*>(ctx->argument(0).toQObject());
    
    EntityReference s;
    fromScriptValueEntityReference(ctx->thisObject(), s);
    
    EntityPtr entity = s.Lookup(scene);
    return engine->newQObject(entity.get());
}

void fromScriptValueEntityList(const QScriptValue &obj, QList<Entity*> &ents)
{
    ents.clear();
    QScriptValueIterator it(obj);
    while(it.hasNext())
    {
        it.next();
        QObject *qent = it.value().toQObject();
        if (qent)
        {
            Entity *ent = qobject_cast<Entity*>(qent);
            if (ent)
                ents.append(ent);
        }
    }
}

QScriptValue toScriptValueEntityList(QScriptEngine *engine, const QList<Entity*> &ents)
{
    QScriptValue obj = engine->newArray(ents.size());
    for(int i=0; i<ents.size(); ++i)
    {
        Entity *ent = ents.at(i);
        if (ent)
            obj.setProperty(i, engine->newQObject(ent));
    }
    return obj;
}

void fromScriptValueEntityStdList(const QScriptValue &obj, EntityList &ents)
{
    ents.clear();
    QScriptValueIterator it(obj);
    while(it.hasNext())
    {
        it.next();
        QObject *qent = it.value().toQObject();
        if (qent)
        {
            Entity *ent = qobject_cast<Entity*>(qent);
            if (ent)
                ents.push_back(ent->shared_from_this());
        }
    }
}

QScriptValue toScriptValueEntityStdList(QScriptEngine *engine, const EntityList &ents)
{
    QScriptValue obj = engine->newArray();
    std::list<EntityPtr>::const_iterator iter = ents.begin();
    int i = 0;
    while(iter != ents.end())
    {
        EntityPtr entPtr = (*iter);
        if (entPtr.get())
        {
            obj.setProperty(i, engine->newQObject(entPtr.get()));
            i++;
        }
        ++iter;
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
    if (ctx->argumentCount() >= 3) //RGB
    {
        if (ctx->argument(0).isNumber() &&
            ctx->argument(1).isNumber() &&
            ctx->argument(2).isNumber())
        {
            newColor.r = (float)ctx->argument(0).toNumber();
            newColor.g = (float)ctx->argument(1).toNumber();
            newColor.b = (float)ctx->argument(2).toNumber();
        }
        else
            return ctx->throwError(QScriptContext::TypeError, "Color(): arguments aren't numbers.");
    }
    else if(ctx->argumentCount() == 4) //RGBA
    {
        if (ctx->argument(3).isNumber())
            newColor.a = (float)ctx->argument(3).toNumber();
        else
            return ctx->throwError(QScriptContext::TypeError, "Color(): arguments aren't numbers.");
    }
    return engine->toScriptValue(newColor);
}
/*
QScriptValue createVector3df(QScriptContext *ctx, QScriptEngine *engine)
{
    float3 newVec;
    if (ctx->argumentCount() == 3)
    {
        if (ctx->argument(0).isNumber() &&
            ctx->argument(1).isNumber() &&
            ctx->argument(2).isNumber())
        {
            newVec.x = (f32)ctx->argument(0).toNumber();
            newVec.y = (f32)ctx->argument(1).toNumber();
            newVec.z = (f32)ctx->argument(2).toNumber();
        }
        else
            return ctx->throwError(QScriptContext::TypeError, "float3(): arguments aren't numbers.");
    }
    return engine->toScriptValue(newVec);
}
*/
QScriptValue createAssetReference(QScriptContext *ctx, QScriptEngine *engine)
{
    AssetReference newAssetRef;
    if (ctx->argumentCount() >= 1)
        if (ctx->argument(0).isString())
            newAssetRef.ref = ctx->argument(0).toString();
    if (ctx->argumentCount() == 2) // Both ref and it's type are given as arguments.
        if (ctx->argument(1).isString())
            newAssetRef.type = ctx->argument(0).toString();
    return engine->toScriptValue(newAssetRef);
}

QScriptValue createAssetReferenceList(QScriptContext *ctx, QScriptEngine *engine)
{
    AssetReferenceList newAssetRefList;
    if (ctx->argumentCount() >= 1)
    {
        if(ctx->argument(0).isArray())
            fromScriptValueAssetReferenceList(ctx->argument(0), newAssetRefList);
        else
            return ctx->throwError(QScriptContext::TypeError, "AssetReferenceList(): argument 0 type isn't array."); 
        if (ctx->argumentCount() == 2)
        {
            if(ctx->argument(1).isString())
                newAssetRefList.type = ctx->toString();
            else
                return ctx->throwError(QScriptContext::TypeError, "AssetReferenceList(): argument 1 type isn't string.");
        }
    }
    return engine->toScriptValue(newAssetRefList);
}

void RegisterCoreMetaTypes()
{
    qRegisterMetaType<ScenePtr>("ScenePtr");
    qRegisterMetaType<ComponentPtr>("ComponentPtr");
    qRegisterMetaType<Color>("Color");
    qRegisterMetaType<float3>("float3");
    qRegisterMetaType<AssetReference>("AssetReference");
    qRegisterMetaType<AssetReferenceList>("AssetReferenceList");
    qRegisterMetaType<EntityReference>("EntityReference");
    qRegisterMetaType<IAttribute*>("IAttribute*");
    qRegisterMetaType<QList<Entity*> >("QList<Entity*>");
    qRegisterMetaType<EntityList>("EntityList");
    qRegisterMetaType<std::string>("std::string");
}

void ExposeCoreTypes(QScriptEngine *engine)
{
    qScriptRegisterMetaType(engine, toScriptValueColor, fromScriptValueColor);
    qScriptRegisterMetaType(engine, toScriptValueAssetReference, fromScriptValueAssetReference);
    qScriptRegisterMetaType(engine, toScriptValueAssetReferenceList, fromScriptValueAssetReferenceList);
    qScriptRegisterMetaType(engine, toScriptValueEntityReference, fromScriptValueEntityReference);
    
    int id = qRegisterMetaType<ScenePtr>("ScenePtr");
    qScriptRegisterMetaType_helper(
        engine, id, reinterpret_cast<QScriptEngine::MarshalFunction>(qScriptValueFromBoostSharedPtr<Scene>),
        reinterpret_cast<QScriptEngine::DemarshalFunction>(qScriptValueToBoostSharedPtr<Scene>),
        QScriptValue());

    qScriptRegisterMetaType<ComponentPtr>(engine, qScriptValueFromBoostSharedPtr, qScriptValueToBoostSharedPtr);

    qScriptRegisterMetaType<IAttribute*>(engine, toScriptValueIAttribute, fromScriptValueIAttribute);
    qScriptRegisterMetaType<ScenePtr>(engine, qScriptValueFromBoostSharedPtr, qScriptValueToBoostSharedPtr);
    qScriptRegisterMetaType<EntityPtr>(engine, qScriptValueFromBoostSharedPtr, qScriptValueToBoostSharedPtr);
    qScriptRegisterMetaType<ComponentPtr>(engine, qScriptValueFromBoostSharedPtr, qScriptValueToBoostSharedPtr);
    qScriptRegisterMetaType<QList<Entity*> >(engine, toScriptValueEntityList, fromScriptValueEntityList);
    qScriptRegisterMetaType<EntityList>(engine, toScriptValueEntityStdList, fromScriptValueEntityStdList);
    qScriptRegisterMetaType<std::string>(engine, toScriptValueStdString, fromScriptValueStdString);
    
    // Register constructors
    QScriptValue ctorColor = engine->newFunction(createColor);
    engine->globalObject().setProperty("Color", ctorColor);
    engine->globalObject().property("Color").setProperty("fromString", engine->newFunction(Color_prototype_FromString));
    QScriptValue ctorAssetReference = engine->newFunction(createAssetReference);
    engine->globalObject().setProperty("AssetReference", ctorAssetReference);
    QScriptValue ctorAssetReferenceList = engine->newFunction(createAssetReferenceList);
    engine->globalObject().setProperty("AssetReferenceList", ctorAssetReferenceList);
}
