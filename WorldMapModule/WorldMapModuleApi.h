// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_WorldMapModuleApi_h
#define incl_WorldMapModuleApi_h

#if defined (_WINDOWS)
#if defined(WORLDMAP_MODULE_EXPORTS) 
#define WORLDMAP_MODULE_API __declspec(dllexport)
#else
#define WORLDMAP_MODULE_API __declspec(dllimport) 
#endif
#else
#define WORLDMAP_MODULE_API
#endif

#endif
