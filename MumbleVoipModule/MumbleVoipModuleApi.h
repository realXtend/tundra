// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModuleApi_h
#define incl_MumbleVoipModuleApi_h

#if defined (_WINDOWS)
#if defined(MUMBLE_VOIP_MODULE_EXPORTS) 
#define MUMBLE_VOIP_MODULE_API __declspec(dllexport)
#else
#define MUMBLE_VOIP_MODULE_API __declspec(dllimport) 
#endif
#else
#define MUMBLE_VOIP_MODULE_API
#endif

#endif // incl_MumbleVoipModuleApi_h
