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
typedef boost::shared_ptr<RenderServiceInterface> RendererServicePtr;
typedef boost::weak_ptr<RenderServiceInterface> RendererServiceWeakPtr;

