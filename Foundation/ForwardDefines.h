// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Framework_ForwardDefines_h
#define incl_Framework_ForwardDefines_h

#include <boost/shared_ptr.hpp>
#include <Poco/Logger.h>

namespace Scene
{
    class Entity;
    class SceneManager;

    typedef boost::shared_ptr<SceneManager> ScenePtr;
    typedef boost::weak_ptr<SceneManager> SceneWeakPtr;
    typedef boost::weak_ptr<Entity> EntityWeakPtr;
    typedef boost::shared_ptr<Entity> EntityPtr;
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
    class ComponentInterfaceAbstract;

    typedef boost::shared_ptr<ModuleManager> ModuleManagerPtr;
    typedef boost::shared_ptr<ComponentManager> ComponentManagerPtr;
    typedef boost::shared_ptr<ComponentFactoryInterface> ComponentFactoryInterfacePtr;
    typedef boost::shared_ptr<ServiceManager> ServiceManagerPtr;
    typedef boost::shared_ptr<ConfigurationManager> ConfigurationManagerPtr;
    typedef boost::shared_ptr<EventManager> EventManagerPtr;
    typedef boost::shared_ptr<Platform> PlatformPtr;
    typedef boost::shared_ptr<Application> ApplicationPtr;

    typedef boost::shared_ptr<ComponentInterfaceAbstract> ComponentInterfacePtr;
    typedef boost::shared_ptr<ComponentInterfaceAbstract> ComponentPtr;
    typedef boost::weak_ptr<ComponentInterfaceAbstract> ComponentWeakPtr;
    
    //! Use root logging only in foundation classes.
    static void RootLogFatal(const std::string &msg)
    {
        Poco::Logger::get("Foundation").fatal("Fatal: " + msg);
    }
    static void RootLogCritical(const std::string &msg)
    {
        Poco::Logger::get("Foundation").critical("Critical: " + msg);
    }
    static void RootLogError(const std::string &msg)
    {
        Poco::Logger::get("Foundation").error("Error: " + msg);
    }
    static void RootLogWarning(const std::string &msg)
    {
        Poco::Logger::get("Foundation").warning("Warning: " + msg);
    }
    static void RootLogNotice(const std::string &msg)
    {
        Poco::Logger::get("Foundation").notice("Notice: " + msg);
    }
    static void RootLogInfo(const std::string &msg)
    {
        Poco::Logger::get("Foundation").information(msg);
    }
    static void RootLogTrace(const std::string &msg)
    {
        Poco::Logger::get("Foundation").trace("Trace: " + msg);
    }
    static void RootLogDebug(const std::string &msg)
    {
        Poco::Logger::get("Foundation").debug("Debug: " + msg);
    }
}

#endif
