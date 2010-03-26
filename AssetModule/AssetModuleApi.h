// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_AssetModuleApi_h
#define incl_AssetModuleApi_h

#if defined (_WINDOWS)
#if defined(ASSET_MODULE_EXPORTS) 
#define ASSET_MODULE_API __declspec(dllexport)
#else
#define ASSET_MODULE_API __declspec(dllimport) 
#endif
#else
#define ASSET_MODULE_API
#endif

#endif
