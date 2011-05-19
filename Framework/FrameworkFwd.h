/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   FrameworkFwd.h
 *  @brief  Forward declarations and type defines for commonly used Framework classes.
 */

#pragma once

#include <boost/smart_ptr.hpp>

class Framework;
class Profiler;
class ModuleManager;
class ServiceManager;
class RenderServiceInterface;
class RaycastResult;

typedef boost::shared_ptr<ModuleManager> ModuleManagerPtr;
typedef boost::shared_ptr<ServiceManager> ServiceManagerPtr;
class IRenderer;
typedef boost::shared_ptr<IRenderer> RendererServicePtr;
typedef boost::weak_ptr<IRenderer> RendererServiceWeakPtr;

