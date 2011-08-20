//$ HEADER_NEW_FILE $
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_CameraModuleApi_h
#define incl_CameraModuleApi_h

#if defined (_WINDOWS)
#if defined(CAMERA_MODULE_EXPORTS) 
#define CAMERA_MODULE_API __declspec(dllexport)
#else
#define CAMERA_MODULE_API __declspec(dllimport) 
#endif
#else
#define CAMERA_MODULE_API
#endif

#endif // incl_CameraModuleApi_h
