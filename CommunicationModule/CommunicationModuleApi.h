// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_CommunicationModuleApi_h
#define incl_CommunicationModuleApi_h

#if defined (_WINDOWS)
#if defined(COMMS_MODULE_EXPORTS) 
#define COMMS_MODULE_API __declspec(dllexport)
#else
#define COMMS_MODULE_API __declspec(dllimport) 
#endif
#else
#define COMMS_MODULE_API
#endif

#endif
