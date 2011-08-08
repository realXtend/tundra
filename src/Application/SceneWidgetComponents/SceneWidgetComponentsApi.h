// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#if defined (_WINDOWS)
#if defined(SCENEWIDGET_MODULE_EXPORTS) 
#define SCENEWIDGET_MODULE_API __declspec(dllexport)
#else
#define SCENEWIDGET_MODULE_API __declspec(dllimport) 
#endif
#else
#define SCENEWIDGET_MODULE_API
#endif

