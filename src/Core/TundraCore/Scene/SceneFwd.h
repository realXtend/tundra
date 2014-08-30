/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   SceneFwd.h
    @brief  Forward declarations and typedefs for Scene-related classes. */

#pragma once

#include "CoreTypes.h"

#include <map>

class SceneAPI;
class Scene;
class Entity;
class IComponent;
class IComponentFactory;
class IAttribute;
class AttributeMetadata;
class ChangeRequest;

struct SceneDesc;
struct EntityDesc;
struct ComponentDesc;
struct AttributeDesc;
struct AssetDesc;
struct EntityReference;
struct ParentingTracker;

typedef shared_ptr<Scene> ScenePtr;
typedef weak_ptr<Scene> SceneWeakPtr;
typedef weak_ptr<Entity> EntityWeakPtr;
typedef shared_ptr<Entity> EntityPtr;
typedef std::list<EntityPtr> EntityList;
typedef shared_ptr<IComponent> ComponentPtr;
typedef weak_ptr<IComponent> ComponentWeakPtr;
typedef shared_ptr<IComponentFactory> ComponentFactoryPtr;
typedef std::vector<IAttribute*> AttributeVector;
typedef std::map<QString, ScenePtr> SceneMap;

typedef QList<EntityDesc> EntityDescList;
typedef QList<ComponentDesc> ComponentDescList;
typedef QList<AttributeDesc> AttributeDescList;
typedef QList<AssetDesc> AssetDescList;

Q_DECLARE_METATYPE(ScenePtr)
Q_DECLARE_METATYPE(EntityPtr)
Q_DECLARE_METATYPE(ComponentPtr)
Q_DECLARE_METATYPE(EntityList)
