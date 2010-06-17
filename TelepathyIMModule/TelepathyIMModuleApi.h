// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TelepathyIMModuleApi_h
#define incl_TelepathyIMModuleApi_h

#if defined (_WINDOWS)
#if defined(TELEPATHY_IM_MODULE_EXPORTS) 
#define TELEPATHY_IM_MODULE_API __declspec(dllexport)
#else
#define COMMS_MODULE_API __declspec(dllimport) 
#endif
#else
#define COMMS_MODULE_API
#define TELEPATHY_IM_MODULE_API
#endif

#endif // incl_TelepathyIMModuleApi_h
