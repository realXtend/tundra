// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_CoreModuleApi_h
#define incl_CoreModuleApi_h

#ifdef MODULE_API
#undef MODULE_API
#endif

#if defined (_WINDOWS)
#if defined(MODULE_EXPORTS) 
#define MODULE_API __declspec(dllexport)
#else
#define MODULE_API __declspec(dllimport)
#endif
#endif

#ifndef MODULE_API
#define MODULE_API
#endif

#endif
