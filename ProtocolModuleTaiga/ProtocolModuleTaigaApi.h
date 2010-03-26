// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ProtocolModuleTaiga_ProtocolModuleTaigaApi_h
#define incl_ProtocolModuleTaiga_ProtocolModuleTaigaApi_h

#if defined (_WINDOWS)
#if defined(TAIGAPROTO_MODULE_EXPORTS) 
#define TAIGAPROTO_MODULE_API __declspec(dllexport)
#else
#define TAIGAPROTO_MODULE_API __declspec(dllimport) 
#endif
#else
#define TAIGAPROTO_MODULE_API
#endif

#endif
