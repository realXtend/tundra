// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ARModuleApi_h
#define incl_ARModuleApi_h

#if defined (_WINDOWS)
#if defined(AR_MODULE_EXPORTS) 
#define AR_MODULE_API __declspec(dllexport)
#else
#define AR_MODULE_API __declspec(dllimport) 
#endif
#else
#define AR_MODULE_API
#endif

#endif
