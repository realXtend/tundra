/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   SceneFwd.h
    @brief  Forward declarations and typedefs for Scene-related classes. */

#pragma once

#include <boost/smart_ptr.hpp>
#include <list>
#include <vector>
#include <map>
#include <QString>

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

typedef boost::shared_ptr<Scene> ScenePtr;
typedef boost::weak_ptr<Scene> SceneWeakPtr;
typedef boost::weak_ptr<Entity> EntityWeakPtr;
typedef boost::shared_ptr<Entity> EntityPtr;
typedef std::list<EntityPtr> EntityList;
typedef boost::shared_ptr<IComponent> ComponentPtr;
typedef boost::weak_ptr<IComponent> ComponentWeakPtr;
typedef boost::shared_ptr<IComponentFactory> ComponentFactoryPtr;
typedef std::vector<IAttribute*> AttributeVector;
typedef std::map<QString, ScenePtr> SceneMap;
