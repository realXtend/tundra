// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_KinectModule_KinectAPI_h
#define incl_KinectModule_KinectAPI_h

#if defined (_WINDOWS)
#if defined(KINECT_MODULE_EXPORTS) 
#define KINECT_VOIP_MODULE_API __declspec(dllexport)
#else
#define KINECT_VOIP_MODULE_API __declspec(dllimport) 
#endif
#else
#define KINECT_VOIP_MODULE_API
#endif

#endif // incl_MumbleVoipModuleApi_h
