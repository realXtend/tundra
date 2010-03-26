// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_InputModuleApi_h
#define incl_InputModuleApi_h

#if defined (_WINDOWS)
#if defined(INPUTOIS_MODULE_EXPORTS) 
#define INPUTOIS_MODULE_API __declspec(dllexport)
#else
#define INPUTOIS_MODULE_API __declspec(dllimport) 
#endif
#else
#define INPUTOIS_MODULE_API
#endif

#endif
