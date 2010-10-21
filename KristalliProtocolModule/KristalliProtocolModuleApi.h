// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_KristalliProtocolModuleApi_h
#define incl_KristalliProtocolModuleApi_h

#if defined (_WINDOWS)
#if defined(KRISTALLIPROTOCOL_MODULE_EXPORTS) 
#define KRISTALLIPROTOCOL_MODULE_API __declspec(dllexport)
#else
#define KRISTALLIPROTOCOL_MODULE_API __declspec(dllimport) 
#endif
#else
#define KRISTALLIPROTOCOL_MODULE_API
#endif

#endif
