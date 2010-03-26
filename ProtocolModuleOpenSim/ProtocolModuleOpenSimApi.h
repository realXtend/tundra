// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TaigaProtocolModuleApi_h
#define incl_TaigaProtocolModuleApi_h

#if defined (_WINDOWS)
#if defined(OSPROTO_MODULE_EXPORTS) 
#define OSPROTO_MODULE_API __declspec(dllexport)
#else
#define OSPROTO_MODULE_API __declspec(dllimport) 
#endif
#else
#define OSPROTO_MODULE_API
#endif

#endif
