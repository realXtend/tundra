// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "Color.h"
#include "Quaternion.h"
#include "Vector3D.h"
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

QScriptValue toScriptValueVector3(QScriptEngine *engine, const Vector3df &s)
{
    QScriptValue obj = engine->newObject();
    obj.setProperty("x", QScriptValue(engine, s.x));
    obj.setProperty("y", QScriptValue(engine, s.y));
    obj.setProperty("z", QScriptValue(engine, s.z));
    createVector3Functions(obj, engine);

    return obj;
}

void fromScriptValueVector3(const QScriptValue &obj, Vector3df &s)
{
    s.x = (float)obj.property("x").toNumber();
    s.y = (float)obj.property("y").toNumber();
    s.z = (float)obj.property("z").toNumber();
}

void fromScriptValueVector3(const QScriptValue &obj, float3 &s)
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
    //if (!ctx->argument(0).isNumber())
    //    return ctx->throwError(QScriptContext::TypeError, "Vector3df mul(): argument is not a number");
    Vector3df vec;
    fromScriptValueVector3(ctx->thisObject(), vec);
    if (ctx->argument(0).isNumber()) // Multiply by scalar
    {
        float scalar = ctx->argument(0).toNumber();
        return toScriptValueVector3(engine, vec * scalar);
    }
    else if(ctx->argument(0).isObject()) // Multiply by vector3
    {
        Vector3df vec2 = engine->fromScriptValue<Vector3df>(ctx->argument(0));
        return toScriptValueVector3(engine, vec * vec2);
    }
    return ctx->throwError(QScriptContext::TypeError, "Vector3df mul(): argument is not a number or vector3df");
}

QScriptValue Vector3df_prototype_add(QScriptContext *ctx, QScriptEngine *engine)
{
    if (ctx->argumentCount() != 1)
        return ctx->throwError("Vector3df add() takes a single number argument.");
    //if (!ctx->argument(0).isNumber())
    //    return ctx->throwError(QScriptContext::TypeError, "Vector3df add(): argument is not a number");
    Vector3df vec;
    fromScriptValueVector3(ctx->thisObject(), vec);
    if(ctx->argument(0).isObject())
    {
        Vector3df vec2 = engine->fromScriptValue<Vector3df>(ctx->argument(0));
        return toScriptValueVector3(engine, vec + vec2);
    }
    return ctx->throwError(QScriptContext::TypeError, "Vector3df add(): argument is not a vector3df");
}

QScriptValue Vector3df_prototype_sub(QScriptContext *ctx, QScriptEngine *engine)
{
    if (ctx->argumentCount() != 1)
        return ctx->throwError("Vector3df sub() takes a single number argument.");
    //if (!ctx->argument(0).isNumber())
    //    return ctx->throwError(QScriptContext::TypeError, "Vector3df sub(): argument is not a number");
    Vector3df vec;
    fromScriptValueVector3(ctx->thisObject(), vec);
    if(ctx->argument(0).isObject())
    {
        Vector3df vec2 = engine->fromScriptValue<Vector3df>(ctx->argument(0));
        return toScriptValueVector3(engine, vec - vec2);
    }
    return ctx->throwError(QScriptContext::TypeError, "Vector3df sub(): argument is not a vector3df");
}

QScriptValue Vector3df_prototype_inter(QScriptContext *ctx, QScriptEngine *engine)
{
    QScriptValue retValue;

    int argCount = ctx->argumentCount();
    if (argCount >= 2 && argCount <= 3)
    {
        Vector3df vec, vec2;
        fromScriptValueVector3(ctx->thisObject(), vec);

        // Ensure that the last argument is a number. 
        if (!ctx->argument(argCount - 1).isNumber())
            return ctx->throwError(QScriptContext::TypeError, "Vector3df interpolate(): argument(" + QString::number(argCount - 1) + ") isn't a number.");
        float d = ctx->argument(argCount - 1).toNumber();

        if (argCount == 2 && ctx->argument(0).isObject())
        {
            vec2 = engine->fromScriptValue<Vector3df>(ctx->argument(0));
            retValue = toScriptValueVector3(engine, vec.getInterpolated(vec2, d));
        }
        else if(ctx->argument(0).isObject() &&
                ctx->argument(1).isObject())
        {
            Vector3df vec3;
            vec2 = engine->fromScriptValue<Vector3df>(ctx->argument(0));
            vec3 = engine->fromScriptValue<Vector3df>(ctx->argument(1));
            vec = vec.interpolate(vec3, vec2, d);
            retValue = toScriptValueVector3(engine, vec); 
        }
        else
            return ctx->throwError(QScriptContext::TypeError, "Vector3df interpolate(): argument types are invalid.");
    }
    else
        return ctx->throwError(QScriptContext::TypeError, "Vector3df interpolate(): invalid number of arguments.");
    return retValue;
}

QScriptValue Vector3df_prototype_rotToDir(QScriptContext *ctx, QScriptEngine *engine)
{
    if (ctx->argumentCount() != 1)
        return ctx->throwError(QScriptContext::TypeError, "Vector3df rotToDir(): invalid number of arguments.");

    Vector3df vec1 = engine->fromScriptValue<Vector3df>(ctx->thisObject());
    Vector3df vec2 = engine->fromScriptValue<Vector3df>(ctx->argument(0));

    return toScriptValueVector3(engine, vec1.rotationToDirection(vec2));
}

QScriptValue Vector3df_prototype_cross(QScriptContext *ctx, QScriptEngine *engine)
{
    if (ctx->argumentCount() != 1)
        return ctx->throwError(QScriptContext::TypeError, "Vector3df crss(): invalid number of arguments."); 

    Vector3df vec1 = engine->fromScriptValue<Vector3df>(ctx->thisObject());
    Vector3df vec2 = engine->fromScriptValue<Vector3df>(ctx->argument(0));

    return toScriptValueVector3(engine, vec1.crossProduct(vec2));
}

QScriptValue Vector3df_prototype_dot(QScriptContext *ctx, QScriptEngine *engine)
{
    if (ctx->argumentCount() != 1)
        return ctx->throwError(QScriptContext::TypeError, "Vector3df dot(): invalid number of arguments."); 

    Vector3df vec1 = engine->fromScriptValue<Vector3df>(ctx->thisObject());
    Vector3df vec2 = engine->fromScriptValue<Vector3df>(ctx->argument(0));

    return QScriptValue(engine, vec1.dotProduct(vec2));
}

QScriptValue Vector3df_prototype_invert(QScriptContext *ctx, QScriptEngine *engine)
{
    Vector3df vec = engine->fromScriptValue<Vector3df>(ctx->thisObject());
    return toScriptValueVector3(engine, vec.invert());
}

//! @todo this code duplicates with IAttribute.
QScriptValue Vector3df_prototype_ToString(QScriptContext *ctx, QScriptEngine *engine)
{
    Vector3df value = engine->fromScriptValue<Vector3df>(ctx->thisObject());
    QString retVal = QString::number(value.x) + " " +
                     QString::number(value.y) + " " +
                     QString::number(value.z);
    return engine->toScriptValue(retVal);
}

QScriptValue Vector3df_prototype_FromString(QScriptContext *ctx, QScriptEngine *engine)
{
    if (ctx->argumentCount() != 1)
        return ctx->throwError(QScriptContext::TypeError, "Vector3df fromString(): invalid number of arguments."); 
    QStringList values = ctx->argument(0).toString().split(" ");
    if (values.count() != 3)
        return ctx->throwError(QScriptContext::TypeError, "Vector3df fromString(): invalid string value."); 
    
    Vector3df retValue(values[0].toFloat(), values[1].toFloat(), values[2].toFloat());
    return toScriptValueVector3(engine, retValue);
}

QScriptValue Vector3df_prototype_distance(QScriptContext *ctx, QScriptEngine *engine)
{
    if (ctx->argumentCount() != 1)
        return ctx->throwError(QScriptContext::TypeError, "Vector3df distance(): invalid number of arguments.");

    Vector3df vec1 = engine->fromScriptValue<Vector3df>(ctx->thisObject());
    Vector3df vec2 = engine->fromScriptValue<Vector3df>(ctx->argument(0));

    return vec1.getDistanceFrom(vec2);
}

QScriptValue Quaternion_prototype_ToEuler(QScriptContext *ctx, QScriptEngine *engine);
QScriptValue Quaternion_prototype_Normalize(QScriptContext *ctx, QScriptEngine *engine);
QScriptValue Quaternion_prototype_MakeIdentity(QScriptContext *ctx, QScriptEngine *engine);
QScriptValue Quaternion_prototype_Slerp(QScriptContext *ctx, QScriptEngine *engine);
QScriptValue Quaternion_prototype_ToString(QScriptContext *ctx, QScriptEngine *engine);
QScriptValue Quaternion_prototype_FromString(QScriptContext *ctx, QScriptEngine *engine);
QScriptValue Quaternion_prototype_RotationTo(QScriptContext *ctx, QScriptEngine *engine);
void createQuaternionFunctions(QScriptValue &value, QScriptEngine *engine)
{
    // Expose native functions to script value. 
    value.setProperty("toEuler", engine->newFunction(Quaternion_prototype_ToEuler));
    value.setProperty("normalize", engine->newFunction(Quaternion_prototype_Normalize));
    value.setProperty("makeIdentity", engine->newFunction(Quaternion_prototype_MakeIdentity));
    value.setProperty("slerp", engine->newFunction(Quaternion_prototype_Slerp));
    value.setProperty("toString", engine->newFunction(Quaternion_prototype_ToString));
    value.setProperty("fromString", engine->newFunction(Quaternion_prototype_FromString));
    value.setProperty("rotationTo", engine->newFunction(Quaternion_prototype_RotationTo));
}

QScriptValue toScriptValueQuaternion(QScriptEngine *engine, const Quaternion &s)
{
    QScriptValue obj = engine->newObject();
    obj.setProperty("x", QScriptValue(engine, s.x));
    obj.setProperty("y", QScriptValue(engine, s.y));
    obj.setProperty("z", QScriptValue(engine, s.z));
    obj.setProperty("w", QScriptValue(engine, s.w));
    createQuaternionFunctions(obj, engine);
    return obj;
}

void fromScriptValueQuaternion(const QScriptValue &obj, Quaternion &s)
{
    s.x = (float)obj.property("x").toNumber();
    s.y = (float)obj.property("y").toNumber();
    s.z = (float)obj.property("z").toNumber();
    s.w = (float)obj.property("w").toNumber();
}

QScriptValue Quaternion_prototype_ToEuler(QScriptContext *ctx, QScriptEngine *engine)
{
    Quaternion quat;
    fromScriptValueQuaternion(ctx->thisObject(), quat);
    Vector3df eulerVal;
    quat.toEuler(eulerVal);
    // Convert to degree.
    eulerVal.x = RADTODEG * eulerVal.x;
    eulerVal.y = RADTODEG * eulerVal.y;
    eulerVal.z = RADTODEG * eulerVal.z;

    return toScriptValueVector3(engine, eulerVal);
}

QScriptValue Quaternion_prototype_Normalize(QScriptContext *ctx, QScriptEngine *engine)
{
    Quaternion quat;
    fromScriptValueQuaternion(ctx->thisObject(), quat);
    
    return toScriptValueQuaternion(engine, quat.normalize());
}

QScriptValue Quaternion_prototype_MakeIdentity(QScriptContext *ctx, QScriptEngine *engine)
{
    Quaternion quat;
    fromScriptValueQuaternion(ctx->thisObject(), quat);
    
    return toScriptValueQuaternion(engine, quat.makeIdentity());
}

QScriptValue Quaternion_prototype_Slerp(QScriptContext *ctx, QScriptEngine *engine)
{
    int argCount = ctx->argumentCount();
    if (argCount >= 2 && argCount > 3)
        return ctx->throwError(QScriptContext::TypeError, "Quaternion slerp(): invalid number of arguments.");
    if (!ctx->argument(argCount - 1).isNumber())
        return ctx->throwError(QScriptContext::TypeError, "Quaternion slerp(): argument(" + QString::number(argCount - 1) + ") isn't a number.");

    Quaternion quat1;
    fromScriptValueQuaternion(ctx->thisObject(), quat1);

    Quaternion quat2;
    Quaternion result;
    float time = ctx->argument(argCount - 1).toNumber();
    if (argCount != 3)
    {
        fromScriptValueQuaternion(ctx->argument(0), quat2);
        result = quat1.slerp(quat1, quat2, time);
    }
    else
    {
        Quaternion quat3;
        fromScriptValueQuaternion(ctx->argument(0), quat2);
        fromScriptValueQuaternion(ctx->argument(1), quat3);
        result = quat1.slerp(quat2, quat3, time);
    }

    return toScriptValueQuaternion(engine, result);
}
void createTransfromFunctions(QScriptValue& value, QScriptEngine* engine);
//! @todo This code is copy pasted from IAttribute.cpp. There should be a funtion that both could call.
QScriptValue Quaternion_prototype_ToString(QScriptContext *ctx, QScriptEngine *engine)
{
    Quaternion quat;
    fromScriptValueQuaternion(ctx->thisObject(), quat);
    QString retVal = QString::number(quat.x) + " " +
                     QString::number(quat.y) + " " +
                     QString::number(quat.z) + " " +
                     QString::number(quat.w);
    return QScriptValue(engine, retVal);

}

QScriptValue Quaternion_prototype_FromString(QScriptContext *ctx, QScriptEngine *engine)
{
    if (ctx->argumentCount() != 1)
        ctx->throwError(QScriptContext::TypeError, "Quaternion fromString(): invalid number of arguments.");
    QStringList values = ctx->argument(0).toString().split(" ");
    if (values.count() != 4)
        ctx->throwError(QScriptContext::TypeError, "Quaternion fromString(): invalid string value.");

    Quaternion quat(values[0].toFloat(), values[1].toFloat(), values[2].toFloat(), values[3].toFloat());
    return toScriptValueQuaternion(engine, quat);
}

QScriptValue Quaternion_prototype_RotationTo(QScriptContext *ctx, QScriptEngine *engine)
{
    int argCount = ctx->argumentCount();
    if (argCount != 2)
        return ctx->throwError(QScriptContext::TypeError, "Quaternion RotationTo(): Invalid number of arguments.");

    Quaternion quat;
    fromScriptValueQuaternion(ctx->thisObject(), quat);

    Quaternion result;    
    Vector3df vec2;
    Vector3df vec3;

    fromScriptValueVector3(ctx->argument(0), vec2);
    fromScriptValueVector3(ctx->argument(1), vec3);
    result = quat.rotationFromTo(vec2, vec3);

    return toScriptValueQuaternion(engine, result);
}

QScriptValue Transform_prototype_ToString(QScriptContext *ctx, QScriptEngine *engine);
QScriptValue Transform_prototype_FromString(QScriptContext *ctx, QScriptEngine *engine);
void createTransformFunctions(QScriptValue &value, QScriptEngine *engine)
{
    // Expose native functions to script value. 
    value.setProperty("toString", engine->newFunction(Transform_prototype_ToString));
    value.setProperty("fromString", engine->newFunction(Transform_prototype_FromString));
}

QScriptValue toScriptValueTransform(QScriptEngine *engine, const Transform &s)
{
    QScriptValue obj = engine->newObject();
    obj.setProperty("pos", toScriptValueVector3(engine, s.pos));
    obj.setProperty("rot", toScriptValueVector3(engine, s.rot));
    obj.setProperty("scale", toScriptValueVector3(engine, s.scale));
    createTransfromFunctions(obj,engine);

    return obj;
}

QScriptValue Transform_prototype_multiply(QScriptContext *ctx, QScriptEngine *engine);
void createTransfromFunctions(QScriptValue& value, QScriptEngine* engine)
{
      value.setProperty("multiply", engine->newFunction(Transform_prototype_multiply));
}

QScriptValue Transform_prototype_multiply(QScriptContext *ctx, QScriptEngine *engine)
{
     if (ctx->argumentCount() != 1)
        return ctx->throwError(QScriptContext::TypeError, "Transfrom multiply() : invalid number of arguments.");

    Transform t1 = engine->fromScriptValue<Transform>(ctx->thisObject());
    Transform t2 = engine->fromScriptValue<Transform>(ctx->argument(0));

    return toScriptValueTransform(engine, t1.Mul(t2));
}


void fromScriptValueTransform(const QScriptValue &obj, Transform &s)
{
    fromScriptValueVector3(obj.property("pos"), s.pos);
    fromScriptValueVector3(obj.property("rot"), s.rot);
    fromScriptValueVector3(obj.property("scale"), s.scale);
}

//! @todo this code duplicates with IAttribute.
QScriptValue Transform_prototype_ToString(QScriptContext *ctx, QScriptEngine *engine)
{
    Transform value = engine->fromScriptValue<Transform>(ctx->thisObject());
    QString retVal = QString::number(value.pos.x) + " " +
                     QString::number(value.pos.y) + " " +
                     QString::number(value.pos.z) + " " +
                     QString::number(value.rot.x) + " " +
                     QString::number(value.rot.y) + " " +
                     QString::number(value.rot.z) + " " +
                     QString::number(value.scale.x) + " " +
                     QString::number(value.scale.y) + " " +
                     QString::number(value.scale.z);
    return engine->toScriptValue(retVal);
}

QScriptValue Transform_prototype_FromString(QScriptContext *ctx, QScriptEngine *engine)
{
    if (ctx->argumentCount() != 1)
        return ctx->throwError(QScriptContext::TypeError, "Transform fromString(): invalid number of arguments."); 
    QStringList values = ctx->argument(0).toString().split(" ");
    if (values.count() != 9)
        return ctx->throwError(QScriptContext::TypeError, "Transform fromString(): invalid string value."); 
    
    Transform retVal(Vector3df(values[0].toFloat(), values[1].toFloat(), values[2].toFloat()), //Pos
                     Vector3df(values[3].toFloat(), values[4].toFloat(), values[5].toFloat()), //Rot
                     Vector3df(values[6].toFloat(), values[7].toFloat(), values[8].toFloat()));//Scale
    return toScriptValueTransform(engine, retVal);
}

QScriptValue toScriptValueIAttribute(QScriptEngine *engine, IAttribute * const &s)
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

void fromScriptValueEntitStdyList(const QScriptValue &obj, EntityList &ents)
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

QScriptValue createVector3df(QScriptContext *ctx, QScriptEngine *engine)
{
    Vector3df newVec;
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
            return ctx->throwError(QScriptContext::TypeError, "Vector3df(): arguments aren't numbers.");
    }
    return engine->toScriptValue(newVec);
}

QScriptValue createQuaternion(QScriptContext *ctx, QScriptEngine *engine)
{
    Quaternion newQuat;
    if (ctx->argumentCount() == 3)
    {
        if (ctx->argument(0).isNumber() &&
            ctx->argument(1).isNumber() &&
            ctx->argument(2).isNumber())
        {
            newQuat.set(DEGTORAD * (f32)ctx->argument(0).toNumber(),
                        DEGTORAD * (f32)ctx->argument(1).toNumber(),
                        DEGTORAD * (f32)ctx->argument(2).toNumber());
        }
        else
            return ctx->throwError(QScriptContext::TypeError, "Quaternion(): arguments aren't numbers.");
    }
    return engine->toScriptValue(newQuat);
}

QScriptValue createTransform(QScriptContext *ctx, QScriptEngine *engine)
{
    Transform newTransform;
    if (ctx->argumentCount() == 3) // Support three Vector3df as arguments.
    {
        //! todo! Figure out how this could be more safe.
        if (ctx->argument(0).isObject() &&
            ctx->argument(1).isObject() &&
            ctx->argument(2).isObject())
        {
            Vector3df pos = engine->fromScriptValue<Vector3df>(ctx->argument(0));
            Vector3df rot = engine->fromScriptValue<Vector3df>(ctx->argument(1));
            Vector3df scale = engine->fromScriptValue<Vector3df>(ctx->argument(2));
            newTransform.pos = pos;
            newTransform.rot = rot;
            newTransform.scale = scale;
        }
    }
    return engine->toScriptValue(newTransform);
}

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
    qRegisterMetaType<Vector3df>("Vector3df");
    qRegisterMetaType<Quaternion>("Quaternion");
    qRegisterMetaType<Transform>("Transform");
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
    qScriptRegisterMetaType(engine, toScriptValueVector3, fromScriptValueVector3);
    qScriptRegisterMetaType(engine, toScriptValueQuaternion, fromScriptValueQuaternion);
    qScriptRegisterMetaType(engine, toScriptValueTransform, fromScriptValueTransform);
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
    qScriptRegisterMetaType<EntityList>(engine, toScriptValueEntityStdList, fromScriptValueEntitStdyList);
    qScriptRegisterMetaType<std::string>(engine, toScriptValueStdString, fromScriptValueStdString);
    
    // Register constructors
    QScriptValue ctorVector3 = engine->newFunction(createVector3df);
    engine->globalObject().setProperty("Vector3df", ctorVector3);
    engine->globalObject().property("Vector3df").setProperty("fromString", engine->newFunction(Vector3df_prototype_FromString));
    QScriptValue ctorColor = engine->newFunction(createColor);
    engine->globalObject().setProperty("Color", ctorColor);
    engine->globalObject().property("Color").setProperty("fromString", engine->newFunction(Color_prototype_FromString));
    QScriptValue ctorTransform = engine->newFunction(createTransform);
    engine->globalObject().setProperty("Transform", ctorTransform);
    engine->globalObject().property("Transform").setProperty("fromString", engine->newFunction(Transform_prototype_FromString));
    QScriptValue ctorAssetReference = engine->newFunction(createAssetReference);
    engine->globalObject().setProperty("AssetReference", ctorAssetReference);
    QScriptValue ctorAssetReferenceList = engine->newFunction(createAssetReferenceList);
    engine->globalObject().setProperty("AssetReferenceList", ctorAssetReferenceList);

    // Register both constructors and methods (with js prototype style)
    // http://doc.qt.nokia.com/latest/scripting.html#prototype-based-programming-with-the-qtscript-c-api
    /* doesn't work for some reason, is now hacked in toScriptValue to every instance (bad!) */
    /*QScriptValue protoVector3df = engine->newObject();
    protoVector3df.setProperty("normalize2", engine->newFunction(Vector3df_prototype_normalize)); //leaving in for debug/test purposes
    QScriptValue ctorVector3df = engine->newFunction(createVector3df, protoVector3df); //this is supposed to work according to docs, doesnt.
    engine->globalObject().setProperty("Vector3df", ctorVector3df);*/
    
    QScriptValue ctorQuaternion = engine->newFunction(createQuaternion);
    engine->globalObject().setProperty("Quaternion", ctorQuaternion);
    engine->globalObject().property("Quaternion").setProperty("fromString", engine->newFunction(Quaternion_prototype_FromString));
}
