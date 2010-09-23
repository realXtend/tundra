// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_AvatarModuleApi_h
#define incl_AvatarModuleApi_h

#if defined (_WINDOWS)
    #if defined(AVATAR_MODULE_EXPORTS) 
        #define AV_MODULE_API __declspec(dllexport)
    #else
        #define AV_MODULE_API __declspec(dllimport) 
    #endif
#else
    #define AV_MODULE_API
#endif

#endif