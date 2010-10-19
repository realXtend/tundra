// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_LibraryModuleApi_h
#define incl_LibraryModuleApi_h

#if defined (_WINDOWS)
#if defined(LIBRARY_MODULE_EXPORTS) 
#define LIBRARY_MODULE_API __declspec(dllexport)
#else
#define LIBRARY_MODULE_API __declspec(dllimport) 
#endif
#else
#define LIBRARY_MODULE_API
#endif

#endif
