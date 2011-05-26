/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   SceneFwd.h
 *  @brief  Forward declarations and typedefs for Scene-related clasess.
 */

#ifndef incl_Scene_SceneFwd
#define incl_Scene_SceneFwd

#include <boost/smart_ptr.hpp>
#include <list>
#include <vector>
#include <map>

#include <QString>
#include <QSharedPointer>
#include <QWeakPointer>

class Entity;
class Scene;

typedef boost::shared_ptr<Scene> ScenePtr;
typedef boost::weak_ptr<Scene> SceneWeakPtr;

typedef boost::weak_ptr<Entity> EntityWeakPtr;
typedef boost::shared_ptr<Entity> EntityPtr;

typedef std::list<EntityPtr> EntityList;

struct SceneDesc;
struct ComponentDesc;
struct AttributeDesc;
struct AssetDesc;

class IComponentFactory;
class IComponent;
class IAttribute;

typedef boost::shared_ptr<IComponent> ComponentPtr;
typedef boost::weak_ptr<IComponent> ComponentWeakPtr;
typedef boost::shared_ptr<IComponentFactory> ComponentFactoryPtr;
typedef std::vector<IAttribute*> AttributeVector;
typedef std::map<QString, ScenePtr> SceneMap;

#endif
