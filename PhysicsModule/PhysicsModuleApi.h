// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Physics_PhysicsModuleApi_h
#define incl_Physics_PhysicsModuleApi_h

#if defined (_WINDOWS)
#if defined(PHYSICS_MODULE_EXPORTS)
#define PHYSICS_MODULE_API __declspec(dllexport)
#else
#define PHYSICS_MODULE_API __declspec(dllimport)
#endif
#else
#define PHYSICS_MODULE_API
#endif

#endif
