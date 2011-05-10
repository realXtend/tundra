/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   ForwardDefines.h
 *  @brief  Forward declarations and type defines for commonly used Foundation classes.
 */

#ifndef incl_Framework_ForwardDefines_h
#define incl_Framework_ForwardDefines_h

#include <boost/shared_ptr.hpp>
#include <boost/smart_ptr.hpp>

#include <vector>

class ModuleManager;
class ServiceManager;

typedef boost::shared_ptr<ModuleManager> ModuleManagerPtr;
typedef boost::shared_ptr<ServiceManager> ServiceManagerPtr;

class Framework;
class KeyBindings;
class Profiler;

class RenderServiceInterface;
typedef boost::shared_ptr<RenderServiceInterface> RendererServicePtr;
typedef boost::weak_ptr<RenderServiceInterface> RendererServiceWeakPtr;

class RaycastResult;

class UiServiceInterface;
typedef boost::shared_ptr<UiServiceInterface> UiServicePtr;
typedef boost::weak_ptr<UiServiceInterface> UiServiceWeakPtr;

#endif
