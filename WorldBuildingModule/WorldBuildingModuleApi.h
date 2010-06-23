// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_WorldBuildingModuleApi_h
#define incl_WorldBuildingModuleApi_h

#if defined (_WINDOWS)
    #if defined(WORLD_BUILDING_MODULE_EXPORTS) 
        #define WB_MODULE_API __declspec(dllexport)
    #else
        #define WB_MODULE_API __declspec(dllimport) 
    #endif
#else
    #define WB_MODULE_API
#endif

#endif