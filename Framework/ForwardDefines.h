/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   ForwardDefines.h
 *  @brief  Forward declarations and type defines for commonly used Foundation classes.
 */

#ifndef incl_Foundation_ForwardDefines_h
#define incl_Foundation_ForwardDefines_h

#include <boost/shared_ptr.hpp>
#include <boost/smart_ptr.hpp>

#include <vector>

namespace Console
{
    struct Command;
    struct CommandResult;
}

class ComponentManager;
class ConfigurationManager;
class ModuleManager;
class IEventData;
class EventManager;
class ServiceManager;

typedef boost::shared_ptr<ComponentManager> ComponentManagerPtr;
typedef boost::shared_ptr<ConfigurationManager> ConfigurationManagerPtr;
typedef boost::shared_ptr<ModuleManager> ModuleManagerPtr;
typedef boost::shared_ptr<EventManager> EventManagerPtr;
typedef boost::shared_ptr<ServiceManager> ServiceManagerPtr;

namespace Foundation
{
    class Framework;
    class KeyBindings;
    class Profiler;

    class RenderServiceInterface;
    typedef boost::shared_ptr<RenderServiceInterface> RendererPtr;
    typedef boost::weak_ptr<RenderServiceInterface> RendererWeakPtr;
}

class RaycastResult;

class UiServiceInterface;
typedef boost::shared_ptr<UiServiceInterface> UiServicePtr;
typedef boost::weak_ptr<UiServiceInterface> UiServiceWeakPtr;

#endif
