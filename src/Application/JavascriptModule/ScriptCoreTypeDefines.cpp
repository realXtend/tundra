// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "Color.h"
#include "IAttribute.h"
#include "AssetReference.h"
#include "EntityReference.h"
#include "Entity.h"
#include "ScriptMetaTypeDefines.h"
#include "Scene/Scene.h"
#include "LoggingFunctions.h"
#include "QScriptEngineHelpers.h"

#include <QScriptEngine>
#include <QScriptValueIterator>

#include "MemoryLeakCheck.h"

Q_DECLARE_METATYPE(IAttribute*)
Q_DECLARE_METATYPE(ScenePtr)
Q_DECLARE_METATYPE(EntityPtr)
Q_DECLARE_METATYPE(ComponentPtr)
Q_DECLARE_METATYPE(Entity::ComponentVector)
Q_DECLARE_METATYPE(QList<Entity*>)
Q_DECLARE_METATYPE(QList<QObject*>)
Q_DECLARE_METATYPE(Entity*)
Q_DECLARE_METATYPE(std::string)
Q_DECLARE_METATYPE(EntityList)
Q_DECLARE_METATYPE(Scene::EntityMap)
Q_DECLARE_METATYPE(Entity::ComponentMap)

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

/// @todo this code duplicates with IAttribute.
QScriptValue Color_prototype_ToString(QScriptContext *ctx, QScriptEngine *engine)
{
    return engine->toScriptValue(engine->fromScriptValue<Color>(ctx->thisObject()).SerializeToString());
}

QScriptValue Color_prototype_FromString(QScriptContext *ctx, QScriptEngine *engine)
{
    if (ctx->argumentCount() != 1)
        return ctx->throwError(QScriptContext::TypeError, "Color fromString(): invalid number of arguments."); 
    QStringList values = ctx->argument(0).toString().split(" ");
    if (values.count() != 4)
        return ctx->throwError(QScriptContext::TypeError, "Color fromString(): invalid string value."); 

    return toScriptValueColor(engine, Color::FromString(ctx->argument(0).toString()));
}

QScriptValue toScriptValueIAttribute(QScriptEngine *engine, IAttribute * const &s)
{
    QScriptValue obj = engine->newObject();
    if(s)
    {
        obj.setProperty("name", QScriptValue(engine, s->Name()));
        obj.setProperty("typename", QScriptValue(engine, s->TypeName()));
        obj.setProperty("value", QScriptValue(engine, QString::fromStdString(s->ToString())));
        obj.setProperty("owner", qScriptValueFromQObject(engine, s->Owner()));
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
            else if (ref.isQObject())
            {
                // If the object is an Entity, call EntityReference::Set() with it
                Entity* entity = dynamic_cast<Entity*>(ref.toQObject());
                s.Set(entity);
            }
            else if (ref.isNumber() || ref.isVariant())
                s.ref = QString::number(ref.toInt32());
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

void fromScriptValueQObjectList(const QScriptValue &obj, QList<QObject*> &objs)
{
    objs.clear();
    QScriptValueIterator it(obj);
    while(it.hasNext())
    {
        it.next();
        QObject *qobj = it.value().toQObject();
        if (qobj)
            objs.append(qobj);
    }
}

QScriptValue toScriptValueQObjectList(QScriptEngine *engine, const QList<QObject*> &objs)
{
    QScriptValue obj = engine->newArray(objs.size());
    for(int i=0; i<objs.size(); ++i)
    {
        QObject* qobj = objs.at(i);
        if (qobj)
            obj.setProperty(i, engine->newQObject(qobj));
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
    // Expose the list of entities as a JavaScript _numeric_ array. (array[0]: first entity, array[1]: second entity, etc.)
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

QScriptValue toScriptValueEntityMap(QScriptEngine *engine, const Scene::EntityMap &entities)
{
    // Expose the set of entities as a JavaScript _associative_ array. (array[entity1Id] = entity1, etc.)
    QScriptValue obj = engine->newObject();
    Scene::EntityMap::const_iterator iter = entities.begin();
    while(iter != entities.end())
    {
        if ((*iter).second)
            obj.setProperty((*iter).first, engine->newQObject((*iter).second.get()));
        ++iter;
    }
    return obj;
}

void fromScriptValueEntityMap(const QScriptValue &obj, Scene::EntityMap &entities)
{
    entities.clear();
    QScriptValueIterator it(obj);
    while(it.hasNext())
    {
        it.next();
        QObject *o = it.value().toQObject();
        if (o)
        {
            Entity *e = qobject_cast<Entity *>(o);
            if (e)
                entities[e->Id()] = e->shared_from_this();
        }
    }
}

QScriptValue toScriptValueComponentMap(QScriptEngine *engine, const Entity::ComponentMap &components)
{
    // Expose the set of components as a JavaScript _associative_ array. (array[component1Id] = component11, etc.)
    QScriptValue obj = engine->newObject();
    Entity::ComponentMap::const_iterator iter = components.begin();
    while(iter != components.end())
    {
        if ((*iter).second)
            obj.setProperty((*iter).first, engine->newQObject((*iter).second.get()));
        ++iter;
    }
    return obj;
}

void fromScriptValueComponentVector(const QScriptValue &/*obj*/, Entity::ComponentVector &/*components*/)
{
    // Left empty deliberately, since we do not have need of conversion from QScriptValue to component vector
}

QScriptValue toScriptValueComponentVector(QScriptEngine *engine, const Entity::ComponentVector &components)
{
    QScriptValue obj = engine->newArray();
    Entity::ComponentVector::const_iterator iter = components.begin();
    quint32 i = 0;
    while(iter != components.end())
    {
        ComponentPtr comp = (*iter);
        if (comp.get())
        {
            obj.setProperty(i, engine->newQObject(comp.get()));
            i++;
        }
        ++iter;
    }

    return obj;
}

void fromScriptValueComponentMap(const QScriptValue &obj, Entity::ComponentMap &components)
{
    components.clear();
    QScriptValueIterator it(obj);
    while(it.hasNext())
    {
        it.next();
        QObject *o = it.value().toQObject();
        if (o)
        {
            IComponent *c = qobject_cast<IComponent  *>(o);
            if (c)
                components[c->TypeId()] = c->shared_from_this();
        }
    }
}

void fromScriptValueStdString(const QScriptValue &obj, std::string &s)
{
    s = obj.toString().toStdString();
}

QScriptValue toScriptValueStdString(QScriptEngine *engine, const std::string &s)
{
    return engine->newVariant(QString::fromStdString(s));
}

void fromScriptValueIAttribute(const QScriptValue & /*obj*/, IAttribute *& /*s*/)
{
    // Left empty deliberately, since we do not have need of conversion from QScriptValue to IAttribute
}

QScriptValue createColor(QScriptContext *ctx, QScriptEngine *engine)
{
    Color newColor;
    if (ctx->argumentCount() >= 3)
    {
        if (ctx->argument(0).isNumber() && ctx->argument(1).isNumber() && ctx->argument(2).isNumber())
        {
            newColor.r = (float)ctx->argument(0).toNumber();
            newColor.g = (float)ctx->argument(1).toNumber();
            newColor.b = (float)ctx->argument(2).toNumber();
        }
        else
            return ctx->throwError(QScriptContext::TypeError, "Color(): arguments aren't numbers.");

        if (ctx->argumentCount() == 4) // Has alpha
        {
            if (ctx->argument(3).isNumber())
                newColor.a = (float)ctx->argument(3).toNumber();
            else
                return ctx->throwError(QScriptContext::TypeError, "Color(): 4th argument is not a number.");
        }
    }
    else
        return ctx->throwError(QScriptContext::TypeError, "Color(): not enought arguments: minimum of 3 needed.");

    return engine->toScriptValue(newColor);
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
    qRegisterMetaType<AssetReference>("AssetReference");
    qRegisterMetaType<AssetReferenceList>("AssetReferenceList");
    qRegisterMetaType<EntityReference>("EntityReference");
    qRegisterMetaType<IAttribute*>("IAttribute*");
    qRegisterMetaType<QList<Entity*> >("QList<Entity*>");
    qRegisterMetaType<QList<QObject*> >("QList<QObject*>");
    qRegisterMetaType<EntityList>("EntityList");
    qRegisterMetaType<Scene::EntityMap>("EntityMap");
    qRegisterMetaType<Entity::ComponentMap>("ComponentMap");
    qRegisterMetaType<Entity::ComponentVector>("ComponentVector");
    qRegisterMetaType<std::string>("std::string");
}

void ExposeCoreTypes(QScriptEngine *engine)
{
    qScriptRegisterMetaType(engine, toScriptValueColor, fromScriptValueColor);
    qScriptRegisterMetaType(engine, toScriptValueAssetReference, fromScriptValueAssetReference);
    qScriptRegisterMetaType(engine, toScriptValueAssetReferenceList, fromScriptValueAssetReferenceList);
    qScriptRegisterMetaType(engine, toScriptValueEntityReference, fromScriptValueEntityReference);

    qScriptRegisterMetaType<ComponentPtr>(engine, qScriptValueFromBoostSharedPtr, qScriptValueToBoostSharedPtr);

    qScriptRegisterMetaType<IAttribute*>(engine, toScriptValueIAttribute, fromScriptValueIAttribute);
    qScriptRegisterMetaType<ScenePtr>(engine, qScriptValueFromBoostSharedPtr, qScriptValueToBoostSharedPtr);
    qScriptRegisterMetaType<EntityPtr>(engine, qScriptValueFromBoostSharedPtr, qScriptValueToBoostSharedPtr);
    qScriptRegisterMetaType<ComponentPtr>(engine, qScriptValueFromBoostSharedPtr, qScriptValueToBoostSharedPtr);
    qScriptRegisterMetaType<QList<Entity*> >(engine, toScriptValueEntityList, fromScriptValueEntityList);
    qScriptRegisterMetaType<QList<QObject*> >(engine, toScriptValueQObjectList, fromScriptValueQObjectList);
    qScriptRegisterMetaType<EntityList>(engine, toScriptValueEntityStdList, fromScriptValueEntityStdList);
    qScriptRegisterMetaType<Scene::EntityMap>(engine, toScriptValueEntityMap, fromScriptValueEntityMap);
    qScriptRegisterMetaType<Entity::ComponentMap>(engine, toScriptValueComponentMap, fromScriptValueComponentMap);
    qScriptRegisterMetaType<Entity::ComponentVector>(engine, toScriptValueComponentVector, fromScriptValueComponentVector);
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
