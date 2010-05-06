// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_CommunicationsModuleApi_h
#define incl_CommunicationsModuleApi_h

#if defined (_WINDOWS)
#if defined(COMMUNICATIONS_MODULE_EXPORTS) 
#define COMMUNICATIONS_MODULE_API __declspec(dllexport)
#else
#define COMMUNICATIONS_MODULE_API __declspec(dllimport) 
#endif
#else
#define COMMUNICATIONS_MODULE_API
#endif

#endif // incl_CommunicationsModuleApi_h
