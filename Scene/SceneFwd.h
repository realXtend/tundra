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

namespace Scene
{
    class Entity;
    class SceneManager;

    typedef boost::shared_ptr<SceneManager> ScenePtr;

    typedef boost::weak_ptr<Entity> EntityWeakPtr;
    typedef boost::weak_ptr<SceneManager> SceneWeakPtr;

}

typedef boost::shared_ptr<Scene::Entity> EntityPtr;

typedef std::list<EntityPtr> EntityList;

struct SceneDesc;

class IComponentFactory;
class IComponent;
class IAttribute;

typedef boost::shared_ptr<IComponent> ComponentPtr;
typedef boost::weak_ptr<IComponent> ComponentWeakPtr;
typedef boost::shared_ptr<IComponentFactory> ComponentFactoryPtr;
typedef std::vector<IAttribute*> AttributeVector;
typedef std::map<QString, Scene::ScenePtr> SceneMap;

class SceneInteract;
typedef QWeakPointer<SceneInteract> SceneInteractWeakPtr;

#endif
