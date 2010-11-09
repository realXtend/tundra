/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   ForwardDefines.h
 *  @brief  Forward declarations and type defines for commonly used Naali framework classes.
 */

#ifndef incl_Framework_ForwardDefines_h
#define incl_Framework_ForwardDefines_h

#include <boost/shared_ptr.hpp>
#include <boost/smart_ptr.hpp>

//#include <vector>
#include <QList>

class ISoundService;

namespace Scene
{
    class Entity;
    class SceneManager;

    typedef boost::shared_ptr<SceneManager> ScenePtr;
    typedef boost::weak_ptr<SceneManager> SceneWeakPtr;
    typedef boost::weak_ptr<Entity> EntityWeakPtr;
    typedef boost::shared_ptr<Entity> EntityPtr;
}

namespace Console
{
    struct Command;
    struct CommandResult;
}

class IComponentFactory;
class IComponent;
class ComponentManager;
class ConfigurationManager;
class ModuleManager;
class IEventData;
class EventManager;
class ServiceManager;

typedef boost::shared_ptr<IComponent> ComponentPtr;
typedef boost::weak_ptr<IComponent> ComponentWeakPtr;
typedef boost::shared_ptr<IComponentFactory> ComponentFactoryPtr;
typedef boost::shared_ptr<ComponentManager> ComponentManagerPtr;
typedef boost::shared_ptr<ConfigurationManager> ConfigurationManagerPtr;
typedef boost::shared_ptr<ModuleManager> ModuleManagerPtr;
typedef boost::shared_ptr<EventManager> EventManagerPtr;
typedef boost::shared_ptr<ServiceManager> ServiceManagerPtr;

namespace Foundation
{
    class Platform;
    class Application;
    class ThreadTaskManager;
    class Framework;
    class KeyBindings;
    class MainWindow;
    class Profiler;

    typedef boost::shared_ptr<Platform> PlatformPtr;
    typedef boost::shared_ptr<Application> ApplicationPtr;
    typedef boost::shared_ptr<ThreadTaskManager> ThreadTaskManagerPtr;

    class RenderServiceInterface;
    typedef boost::shared_ptr<RenderServiceInterface> RendererPtr;
    typedef boost::weak_ptr<RenderServiceInterface> RendererWeakPtr;
}

class RaycastResult;

class IAttribute;
//#typedef std::vector<IAttribute*> AttributeVector;
typedef QList<IAttribute*> AttributeVector;

class KeyEvent;
class MouseEvent;
class InputContext;
class InputServiceInterface;
typedef boost::shared_ptr<InputContext> InputContextPtr;

class UiServiceInterface;
typedef boost::shared_ptr<UiServiceInterface> UiServicePtr;
typedef boost::weak_ptr<UiServiceInterface> UiServiceWeakPtr;

//! Use root logging only in foundation classes.
void RootLogFatal(const std::string &msg);
//! Use root logging only in foundation classes.
void RootLogCritical(const std::string &msg);
//! Use root logging only in foundation classes.
void RootLogError(const std::string &msg);
//! Use root logging only in foundation classes.
void RootLogWarning(const std::string &msg);
//! Use root logging only in foundation classes.
void RootLogNotice(const std::string &msg);
//! Use root logging only in foundation classes.
void RootLogInfo(const std::string &msg);
//! Use root logging only in foundation classes.
void RootLogTrace(const std::string &msg);
//! Use root logging only in foundation classes.
void RootLogDebug(const std::string &msg);

#endif
