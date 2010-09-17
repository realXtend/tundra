// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Framework_ForwardDefines_h
#define incl_Framework_ForwardDefines_h

#include <boost/shared_ptr.hpp>
#include <boost/smart_ptr.hpp>

#include <vector>

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

namespace Foundation
{
    class ModuleManager;
    class ComponentManager;
    class ComponentFactoryInterface;
    class ServiceManager;
    class ConfigurationManager;
    class EventManager;
    class Platform;
    class Application;
    class ConfigurationManager;
    class ComponentInterface;
    class ThreadTaskManager;
    class Profiler;
    class Framework;
    class KeyBindings;
    class MainWindow;
    class EventDataInterface;

    typedef boost::shared_ptr<ModuleManager> ModuleManagerPtr;
    typedef boost::shared_ptr<ComponentManager> ComponentManagerPtr;
    typedef boost::shared_ptr<ComponentFactoryInterface> ComponentFactoryInterfacePtr;
    typedef boost::shared_ptr<ServiceManager> ServiceManagerPtr;
    typedef boost::shared_ptr<ConfigurationManager> ConfigurationManagerPtr;
    typedef boost::shared_ptr<EventManager> EventManagerPtr;
    typedef boost::shared_ptr<Platform> PlatformPtr;
    typedef boost::shared_ptr<Application> ApplicationPtr;
    typedef boost::shared_ptr<ThreadTaskManager> ThreadTaskManagerPtr;

    typedef boost::shared_ptr<ComponentInterface> ComponentInterfacePtr;
    typedef boost::shared_ptr<ComponentInterface> ComponentPtr;
    typedef boost::weak_ptr<ComponentInterface> ComponentWeakPtr;

    void RootLogFatal(const std::string &msg);
    void RootLogCritical(const std::string &msg);
    void RootLogError(const std::string &msg);
    void RootLogWarning(const std::string &msg);
    void RootLogNotice(const std::string &msg);
    void RootLogInfo(const std::string &msg);
    void RootLogTrace(const std::string &msg);
    void RootLogDebug(const std::string &msg);
}

class AttributeInterface;
typedef std::vector<AttributeInterface*> AttributeVector;

class KeyEvent;
class MouseEvent;
class InputContext;
typedef boost::shared_ptr<InputContext> InputContextPtr;

#endif
