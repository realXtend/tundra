// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_CameraInputModuleApi_h
#define incl_CameraInputModuleApi_h

#if defined (_WINDOWS)
#if defined(CAMERAINPUT_MODULE_EXPORTS) 
#define CAMERAINPUT_MODULE_API __declspec(dllexport)
#else
#define CAMERAINPUT_MODULE_API __declspec(dllimport) 
#endif
#else
#define AR_MODULE_API
#endif

#endif
