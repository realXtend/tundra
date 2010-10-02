// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ScenePersistenceModuleApi_h
#define incl_ScenePersistenceModuleApi_h

#if defined (_WINDOWS)
#if defined(SCENEPERSISTENCE_MODULE_EXPORTS) 
#define SCENEPERSISTENCE_MODULE_API __declspec(dllexport)
#else
#define SCENEPERSISTENCE_MODULE_API __declspec(dllimport) 
#endif
#else
#define SCENEPERSISTENCE_MODULE_API
#endif

#endif
