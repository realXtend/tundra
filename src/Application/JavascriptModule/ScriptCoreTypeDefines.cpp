// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ScriptMetaTypeDefines.h"

#include "Color.h"
#include "IAttribute.h"
#include "IRenderer.h"
#include "AssetReference.h"
#include "EntityReference.h"
#include "Entity.h"
#include "Scene/Scene.h"
#include "LoggingFunctions.h"
#include "QScriptEngineHelpers.h"

#include <QScriptEngine>
#include <QScriptValueIterator>

#include "MemoryLeakCheck.h"

QScriptValue toScriptValueIAttribute(QScriptEngine *engine, IAttribute * const &s)
{
    QScriptValue obj = engine->newObject();
    if (s)
    {
        obj.setProperty("name", QScriptValue(engine, s->Name()));
        obj.setProperty("typename", QScriptValue(engine, s->TypeName()));
        obj.setProperty("value", engine->newVariant(s->ToQVariant()));
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
    // Directly a string type
    if (obj.isString())
        s.ref = obj.toString();
    else
    {
        // Object must have a 'ref' property.
        QScriptValue value = obj.property("ref");
        if (value.isValid() && value.isString())
        {
            s.ref = value.toString();

            // Optional 'type' property.
            value = obj.property("type");
            if (value.isValid() && value.isString())    
                s.type = value.toString();
        }
        else
            LogError("Can't convert QScriptValue to AssetReference! QScriptValue must be a string or an object with 'ref' string property.");
    }
}

QScriptValue AssetReference_prototype_toString(QScriptContext *ctx, QScriptEngine *engine)
{
    return QScriptValue(engine, QString("AssetReference(\"%1\", \"%2\")").arg(ctx->thisObject().property("ref").toString()).arg(ctx->thisObject().property("type").toString()));
}

QScriptValue toScriptValueAssetReference(QScriptEngine *engine, const AssetReference &s)
{
    QScriptValue obj = engine->newObject();
    obj.setProperty("ref", QScriptValue(engine, s.ref));
    obj.setProperty("type", QScriptValue(engine, s.type));
    obj.setProperty("toString", engine->newFunction(AssetReference_prototype_toString));
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

        // Ignore the 'length' property of Array 
        if (obj.isArray() && it.name() == "length")
            continue;

        QScriptValue iterObj = it.value();
        if (iterObj.isFunction())
            continue;
        AssetReference ref;

        // Accepts string or object. If anything else 
        // (null, undefined etc.) we append a empty ref.
        fromScriptValueAssetReference(iterObj, ref);
        s.Append(ref);
    }
}

QScriptValue AssetReferenceList_prototype_toString(QScriptContext *ctx, QScriptEngine *engine)
{
    QStringList values;
    for (qint32 i=0, len=ctx->thisObject().property("length").toInt32(); i<len; ++i)
        values << "\"" + ctx->thisObject().property(i).toString() + "\"";
    return QScriptValue(engine, "AssetReferenceList[" + values.join(", ") + "]");
}

QScriptValue toScriptValueAssetReferenceList(QScriptEngine *engine, const AssetReferenceList &s)
{
    QScriptValue obj = engine->newArray(s.refs.size());
    for(int i = 0; i < s.refs.size(); ++i)
        obj.setProperty(i, QScriptValue(engine, s[i].ref));
    // Override toString function in the Array, even if we are not actually passing AssetReference objects into it.
    // We still want nice print that indicates the Attribute type.
    obj.setProperty("toString", engine->newFunction(AssetReferenceList_prototype_toString));
    return obj;
}

QScriptValue EntityReference_prototype_Lookup(QScriptContext *ctx, QScriptEngine *engine);

QScriptValue EntityReference_prototype_toString(QScriptContext *ctx, QScriptEngine *engine)
{
    return QScriptValue(engine, QString("EntityReference(\"%1\")").arg(ctx->thisObject().property("ref").toString()));
}

void createEntityReferenceFunctions(QScriptValue &value, QScriptEngine *engine)
{
    // Expose native functions to script value.
    value.setProperty("Lookup", engine->newFunction(EntityReference_prototype_Lookup));
    value.setProperty("toString", engine->newFunction(EntityReference_prototype_toString));
}

void fromScriptValueEntityReference(const QScriptValue &obj, EntityReference &s)
{
    // Recurse detecting supported basic types for the 'ref' property (has priority over anything else)
    if (obj.property("ref").isValid())
        fromScriptValueEntityReference(obj.property("ref"), s);
    // null/undefined
    else if (obj.isNull() || obj.isUndefined())
        s.ref = "";
    // Entity name
    else if (obj.isString())
        s.ref = obj.toString();
    // Entity id. entity_id_t is uint, negative values are not allowed.
    else if (obj.isNumber() || obj.isVariant())
        s.ref = QString::number(obj.toUInt32());
    // Check if its a Entity*
    else if (obj.isQObject())
    {
        // If the object is an Entity, call EntityReference::Set() with it
        Entity* entity = qobject_cast<Entity*>(obj.toQObject());
        s.Set(entity);
    }
}

QScriptValue toScriptValueEntityReference(QScriptEngine *engine, const EntityReference &s)
{
    QScriptValue obj = engine->newObject();
    obj.setProperty("ref", QScriptValue(engine, s.ref));
    createEntityReferenceFunctions(obj, engine);
    return obj;
}

void fromScriptValueEntityList(const QScriptValue &obj, QList<Entity*> &ents)
{
    ents.clear();
    QScriptValueIterator it(obj);
    while(it.hasNext())
    {
        it.next();
        QScriptValue value = it.value();
        Entity *ent = qobject_cast<Entity*>(value.toQObject());
        if (ent)
            ents.append(ent);
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

void fromScriptValueRaycastResultList(const QScriptValue &obj, QList<RaycastResult*> &objs)
{
    objs.clear();
    QScriptValueIterator it(obj);
    while(it.hasNext())
    {
        it.next();
        RaycastResult *rr = qobject_cast<RaycastResult*>(it.value().toQObject());
        if (rr)
            objs.append(rr);
    }
}

QScriptValue toScriptValueRaycastResultList(QScriptEngine *engine, const QList<RaycastResult*> &objs)
{
    QScriptValue obj = engine->newArray(objs.size());
    for(int i=0; i<objs.size(); ++i)
    {
        QObject* rr = objs.at(i);
        if (rr)
            obj.setProperty(i, engine->newQObject(rr));
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
        QScriptValue value = it.value();
        Entity *ent = qobject_cast<Entity*>(value.toQObject());
        if (ent)
            ents.push_back(ent->shared_from_this());
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

QScriptValue createAssetReference(QScriptContext *ctx, QScriptEngine *engine)
{
    AssetReference newAssetRef;
    if (ctx->argumentCount() >= 1 && ctx->argument(0).isString())
        newAssetRef.ref = ctx->argument(0).toString();
    if (ctx->argumentCount() == 2 && ctx->argument(1).isString()) // Both ref and it's type are given as arguments.
        newAssetRef.type = ctx->argument(1).toString();
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

QScriptValue createEntityReference(QScriptContext *ctx, QScriptEngine *engine)
{
    EntityReference entRef;
    if (ctx->argumentCount() >= 1)
        fromScriptValueEntityReference(ctx->argument(0), entRef);
    return engine->toScriptValue(entRef);
}
QScriptValue createIntegerTesterRunner(QScriptContext *ctx, QScriptEngine * /*engine*/)
{
    if (!ctx->engine() || ctx->thisObject().isNull())
        return QScriptValue();
    return ctx->engine()->newQObject(ctx->thisObject(), new IntegerTestRunner(), QScriptEngine::AutoOwnership);
}

void ExposeCoreTypes(QScriptEngine *engine)
{
    qScriptRegisterMetaType<AssetReference>(engine, toScriptValueAssetReference, fromScriptValueAssetReference);
    qScriptRegisterMetaType<AssetReferenceList>(engine, toScriptValueAssetReferenceList, fromScriptValueAssetReferenceList);
    qScriptRegisterMetaType<EntityReference>(engine, toScriptValueEntityReference, fromScriptValueEntityReference);

    qScriptRegisterMetaType<IAttribute*>(engine, toScriptValueIAttribute, fromScriptValueIAttribute);
    qScriptRegisterMetaType<ScenePtr>(engine, qScriptValueFromBoostSharedPtr, qScriptValueToBoostSharedPtr);
    qScriptRegisterMetaType<EntityPtr>(engine, qScriptValueFromBoostSharedPtr, qScriptValueToBoostSharedPtr);
    qScriptRegisterMetaType<ComponentPtr>(engine, qScriptValueFromBoostSharedPtr, qScriptValueToBoostSharedPtr);
    qScriptRegisterMetaType<QList<Entity*> >(engine, toScriptValueEntityList, fromScriptValueEntityList);
    qScriptRegisterMetaType<QList<QObject*> >(engine, toScriptValueQObjectList, fromScriptValueQObjectList);
    qScriptRegisterMetaType<QList<RaycastResult*> >(engine, toScriptValueRaycastResultList, fromScriptValueRaycastResultList);
    qScriptRegisterMetaType<EntityList>(engine, toScriptValueEntityStdList, fromScriptValueEntityStdList);
    qScriptRegisterMetaType<Scene::EntityMap>(engine, toScriptValueEntityMap, fromScriptValueEntityMap);
    qScriptRegisterMetaType<Entity::ComponentMap>(engine, toScriptValueComponentMap, fromScriptValueComponentMap);
    qScriptRegisterMetaType<Entity::ComponentVector>(engine, toScriptValueComponentVector, fromScriptValueComponentVector);
    qScriptRegisterMetaType<std::string>(engine, toScriptValueStdString, fromScriptValueStdString);

    // Register constructors
    QScriptValue ctorAssetReference = engine->newFunction(createAssetReference);
    engine->globalObject().setProperty("AssetReference", ctorAssetReference);
    QScriptValue ctorAssetReferenceList = engine->newFunction(createAssetReferenceList);
    engine->globalObject().setProperty("AssetReferenceList", ctorAssetReferenceList);
    QScriptValue ctorEntityReference = engine->newFunction(createEntityReference);
    engine->globalObject().setProperty("EntityReference", ctorEntityReference);
    
    // Test objects
    qScriptRegisterQObjectMetaType<IntegerTestRunner*>(engine);
    QScriptValue ctorIntegerTesterRunner = engine->newFunction(createIntegerTesterRunner);
    engine->globalObject().setProperty("IntegerTestRunner", ctorIntegerTesterRunner);
}
