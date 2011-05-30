// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#if defined (_WINDOWS)
#if defined(PHYSICS_MODULE_EXPORTS)
#define PHYSICS_MODULE_API __declspec(dllexport)
#else
#define PHYSICS_MODULE_API __declspec(dllimport)
#endif
#else
#define PHYSICS_MODULE_API
#endif

