// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#if defined (_WINDOWS)
#if defined(SceneWidgetComponents_EXPORTS) 
#define SCENEWIDGET_MODULE_API __declspec(dllexport)
#else
#define SCENEWIDGET_MODULE_API __declspec(dllimport) 
#endif
#else
#define SCENEWIDGET_MODULE_API
#endif

